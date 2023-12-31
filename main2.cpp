#include <cmath>
#include <iostream>
#include <vector>
#include <deque>
#include <SDL2/SDL.h>
#include <cassert>

#ifdef __SSE2__
#include <emmintrin.h>
#endif

#ifdef _OPENMP
#include <omp.h>
#endif

/* OpenMP and SIMD capable Mandelbrot set renderer,
 * using the boundary trace algorithm.
 * Copyright and written by Joel Yliluoma, March 12 2010.
 * License: Creative Commons CC-by.
 * Portions by 8bitbubsy.
 */

static const double cre = -0.3703121, cim = -0.6488103, rad = 0.003834095;
//static const double cre=-1.9990992988592150, cim=0, rad=48.3E-13;
//static const double cre=-0.5, cim=0, rad=3;
static const double minr = cre - rad * .5, mini = cim - rad * .5;
static const double maxr = cre + rad * .5, maxi = cim + rad * .5;
//static const int Width = 8192/2, Height = 6144/2, MaxIter = 21089, DefaultIter = 1024;
static const unsigned Width = 1024, Height = 768, MaxIter = 3496, DefaultIter = 1024;
static const double stepr = (maxr - minr) / Width;
static const double stepi = (maxi - mini) / Height;
static const bool PaintSolids = true;
static const bool PaintPaletted = true;

static inline unsigned Mand(double r, double i, double x, double y, unsigned firstiter = 0)
{
    unsigned c = firstiter;
    for (;;)
    {
        double r2 = r * r, i2 = i * i;

        if (r2 + i2 >= 4.0) break;
        if (++c >= MaxIter) break;

        double ri = r * i;
        i = ri + ri + y;
        r = r2 - i2 + x;
    }
    return c;
}

static unsigned Iterate(double cr, double ci)
{
    return Mand(cr, ci, cr, ci, 0);
}

#ifdef __SSE2__
template<unsigned which>
static inline double GetD(__m128d v)
{
    return _mm_cvtsd_f64(_mm_shuffle_pd(v, v, _MM_SHUFFLE2(0, which)));
}
template<unsigned which>
static inline unsigned GetI(__m128i v)
{
    if (MaxIter < 32768) return _mm_extract_epi16(v, which * 2);
    else return _mm_cvtsi128_si32(_mm_shuffle_epi32(v, _MM_SHUFFLE(0, 0, 0, which)));
}

static __m128i TwoMand(__m128d x, __m128d y)
{
    __m128d r = x, i = y;

    const __m128d threshold = _mm_set_pd(4.0, 4.0);
    unsigned c = 0;
    for (;;)
    {
        __m128d r2 = _mm_mul_pd(r, r), i2 = _mm_mul_pd(i, i);
        __m128i comparison = _mm_castpd_si128(
            _mm_cmpge_pd(_mm_add_pd(r2, i2), threshold));
        if (_mm_extract_epi16(comparison, 0))
        {
            if (_mm_extract_epi16(comparison, 4))
            {
                // Part1 and Part2 both escaped here
                break;
            }
            // Part1 escaped here, continue iterating Part2 alone
            double r1 = GetD<1>(r), i1 = GetD<1>(i);
            double x1 = GetD<1>(x), y1 = GetD<1>(y);
            return _mm_set_epi32(0, 0, c, Mand(r1, i1, x1, y1, c));
        }
        else if (_mm_extract_epi16(comparison, 4))
        {
            // Part2 escaped here, continue iterating Part1 alone
            double r0 = GetD<0>(r), i0 = GetD<0>(i);
            double x0 = GetD<0>(x), y0 = GetD<0>(y);
            return _mm_set_epi32(0, 0, Mand(r0, i0, x0, y0, c), c);
        }

        if (++c >= MaxIter) break;

        // Iterate Part1 and Part2 simultaneously
        __m128d ri = _mm_mul_pd(r, i);
        i = _mm_add_pd(_mm_add_pd(ri, ri), y);
        r = _mm_add_pd(_mm_sub_pd(r2, i2), x);
    }
    // Reached here if Part1 and Part2 had the same result
    return _mm_set_epi32(0, 0, c, c);
}
#endif /* SSE2 */

enum {
    Calculated = 0x80000000U,
    Scanned = 0x40000000U,
    IterMask = 0x3FFFFFFFU
};
static unsigned data[Width * Height] = { 0 };

static void LoadOne(unsigned pos)
{
    unsigned result = Iterate(minr + (pos % Width) * stepr,
        mini + (pos / Width) * stepi);
    data[pos] |= result;
}

#ifdef __SSE2__
static void LoadTwo(unsigned pos1, unsigned pos2)
{
    __m128d r =
        _mm_add_pd(_mm_set1_pd(minr),
            _mm_mul_pd(_mm_set_pd(pos1 % Width, pos2 % Width),
                _mm_set1_pd(stepr)));
    __m128d i =
        _mm_add_pd(_mm_set1_pd(mini),
            _mm_mul_pd(_mm_set_pd(pos1 / Width, pos2 / Width),
                _mm_set1_pd(stepi)));
    __m128i results = TwoMand(r, i);
    data[pos1] |= GetI<0>(results);
    data[pos2] |= GetI<1>(results);
}
#endif

class BoundaryScanner
{
    std::deque<unsigned> scanqueue;
#ifdef __SSE2__
    std::deque<unsigned> calcqueue;
#endif
    unsigned miny, maxy;

public:
    BoundaryScanner(unsigned firstline, unsigned lastline)
        : scanqueue(),
#ifdef __SSE2__
        calcqueue(),
#endif
        miny(firstline), maxy(lastline)
    {
        /* Add the edges of this partition to scanning list */
        for (unsigned y = miny; y < maxy; ++y)
        {
            AddScanQueue(y * Width + 0);
            AddScanQueue(y * Width + (Width - 1));
        }
        for (unsigned x = 1; x < Width - 1; ++x)
        {
            AddScanQueue(miny * Width + x);
            if (maxy > miny + 1)
                AddScanQueue((maxy - 1) * Width + x);
        }
    }

    void Loop()
    {
        while (!scanqueue.empty())
        {
            unsigned pos = scanqueue.front();
            scanqueue.pop_front();
            Scan(pos);
        }
    }

private:
    void Scan(unsigned pos)
    {
        const int loffs = -1, roffs = +1, uoffs = -Width, doffs = +Width;
        const unsigned x = pos % Width, y = pos / Width;
        const bool ll = x > 0, rr = x + 1 < Width;
        const bool uu = y > miny, dd = y + 1 < maxy;

        //AddCalculateQueue(pos); -- already there.
        if (ll) AddCalculateQueue(pos + loffs);
        if (rr) AddCalculateQueue(pos + roffs);
        if (uu) AddCalculateQueue(pos + uoffs);
        if (dd) AddCalculateQueue(pos + doffs);
        FlushCalculateQueue();

        unsigned center = data[pos] & IterMask;
        bool l = ll && (data[pos + loffs] & IterMask) != center;
        bool r = rr && (data[pos + roffs] & IterMask) != center;
        bool u = uu && (data[pos + uoffs] & IterMask) != center;
        bool d = dd && (data[pos + doffs] & IterMask) != center;

        if (l) AddScanQueue(pos + loffs);
        if (r) AddScanQueue(pos + roffs);
        if (u) AddScanQueue(pos + uoffs);
        if (d) AddScanQueue(pos + doffs);

        if ((uu && ll) && (l || u)) AddScanQueue(pos + uoffs + loffs);
        if ((uu && rr) && (r || u)) AddScanQueue(pos + uoffs + roffs);
        if ((dd && ll) && (l || d)) AddScanQueue(pos + doffs + loffs);
        if ((dd && rr) && (r || d)) AddScanQueue(pos + doffs + roffs);
    }

    void AddScanQueue(unsigned pos)
    {
        if (data[pos] & Scanned) return;
        data[pos] |= Scanned;
        scanqueue.push_back(pos);

        /* Anything that will be scanned must also
         * be calculated (the center value).
         * So we might as well do that here.
         */
        AddCalculateQueue(pos);
    }

    void AddCalculateQueue(unsigned pos)
    {
        if (data[pos] & Calculated) return;
        data[pos] |= Calculated;
#ifdef __SSE2__
        /* On SSE2, we queue the Iteration requests, so that
         * we can preform them in parallel when they're finally
         * required. Without SSE2, there's no advantage in
         * queuing them, so we perform them immediately.
         */
        calcqueue.push_back(pos);
#else
        LoadOne(pos);
#endif
    }

    void FlushCalculateQueue()
    {
#ifdef __SSE2__
        /* As explained above, we use the queue only on SSE2. */
        while (!calcqueue.empty())
        {
            unsigned pos = calcqueue.back(); calcqueue.pop_back();
            if (!calcqueue.empty())
            {
                unsigned pos2 = calcqueue.back(); calcqueue.pop_back();
                LoadTwo(pos, pos2);
                continue;
            }
            LoadOne(pos);
        }
#endif
    }
};

int main()
{
#ifdef __SSE2__
    std::cerr << "Using SSE2 for calculating two pixels simultaneously on single core.\n";
#else
    std::cerr <<
        "SSE2 disabled. If your CPU has SSE2 support, add /arch:SSE2 /D__SSE2__=1,\n"
        "-msse2 or -xW to your compiler commandline depending on compiler in order\n"
        "to use SSE2 for calculating two pixels simultaneously on single core.\n";
#endif
#ifndef _OPENMP
    /* No threading: Scan the entire image at once. */
    std::cerr <<
        "No OpenMP support. Add -fopenmp, -openmp or /openmp to your\n"
        "compiler commandline depending on compiler to get OpenMP support.\n";
    BoundaryScanner worker(0, Height);
    worker.Loop();
#else
    /* OpenMP version -- threaded parallelism */
    {
        /* This is basically the same algorithm as OpenMP guided
         * scheduling. The chunksize controls the minimum granularity.
         */
        int next = 0;
        const int incr = 1;
        const int chunksize = Height / 16;
        const int end = Height;
        omp_lock_t lock; omp_init_lock(&lock);
#pragma omp parallel shared(next)
        {
            const int nthreads = omp_get_num_threads();
#pragma omp single
            std::cerr << "OpenMP enabled -- using " << nthreads << " threads.\n";
            for (;;)
            {
                omp_set_lock(&lock);
#pragma omp flush(next)
                if (next >= end) { omp_unset_lock(&lock); break; }
                int A = next, B = end;
                int n = (end - A) / incr;
                int q = std::max(chunksize, (n + nthreads - 1) / nthreads);
                if (q <= n)
                    B = A + q * incr;
                next = B;
#pragma omp flush(next)
                std::cerr << "Work section " << A << " to " << (B - 1)
                    << " with thread " << omp_get_thread_num()
                    << " / " << nthreads << std::endl;
                omp_unset_lock(&lock);
                BoundaryScanner worker(A, B);
                worker.Loop();
            }
        }
        omp_destroy_lock(&lock);
    }
#endif

    assert(SDL_Init(SDL_INIT_VIDEO) >= 0);
    struct Term { Term() {} ~Term() { SDL_Quit(); } } AutoCloseSDLatEnd;
    SDL_Surface* screen = 0;
    assert(screen = SDL_SetVideoMode(Width, Height, 32, SDL_HWSURFACE));

    bool running = !SDL_MUSTLOCK(screen) || SDL_LockSurface(screen) >= 0;

    unsigned char Palette[MaxIter + 1][4] = { { 0 } };
    for (unsigned iter = 0; iter <= MaxIter; ++iter)
    {
        Palette[iter][0] = (unsigned char)(128.0 + 127.0 * std::cos(iter / 100.0));
        Palette[iter][1] = (unsigned char)(128.0 - 127.0 * std::cos(iter / 80.0));
        Palette[iter][2] = (unsigned char)(128.0 - 127.0 * std::sin(iter / 70.0 + 2.0));
    }

    unsigned run = DefaultIter;
    for (unsigned y = 0; y < Height; ++y)
        for (unsigned x = 0; x < Width; ++x)
        {
            unsigned iter = PaintPaletted ? run : 3235;
            if (data[y * Width + x] & Calculated)
            {
                iter = PaintPaletted ? (data[y * Width + x] & IterMask) : 3496;
                if (PaintSolids) run = iter;
            }
            memcpy(
                (unsigned char*)screen->pixels + 4 * (y * Width + x),
                Palette[iter], 4);
        }

    if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
    SDL_RenderPresent(screen);
    while (running)
    {
        unsigned long start = SDL_GetTicks();
        while ((SDL_GetTicks() - start) < 2)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN)
                    running = false;
            }
        }
    }

    return 0;
}
