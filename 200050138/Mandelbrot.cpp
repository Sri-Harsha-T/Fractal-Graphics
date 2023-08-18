#pragma once
#include "Mandelbrot.h"
#ifndef MANDELBROT_CPP
#define MANDELBROT_CPP

//By Sri Harsha Thota, Roll Number:200050138
#include <cstddef>

inline Quad_Tree_Square_Node::Quad_Tree_Square_Node() {
    Top_Left_coord_x = 0;
    Top_Left_coord_y = 0;
    length = 0;
}

inline Quad_Tree_Square_Node::Quad_Tree_Square_Node(int TLCX, int TLCY, int len) {
    Top_Left_coord_x = TLCX;
    Top_Left_coord_y = TLCY;
    length = len;
}

inline DynamicDequeue::DynamicDequeue() {//Constructor function
    N = INIT_SIZE;
    A = new Quad_Tree_Square_Node[N];           //allocating dynamic memory array
    f = 0;
    r = 0;
}

inline bool DynamicDequeue::isEmpty() {//Boolean function to check whether the deque is empty
    if (f == r) return true;      //if both f and r are equal then the dequecan either be empty or full
                               //the ambiguity is avoided by not utilising a single memory location and readjusting the capacity accordingly
                               //hence f==r means the deque is empty
    else return false;
}

inline bool DynamicDequeue::isFull() {//the capacity as said before is adjusted to N-1, leaving single memory location to avoid confusion
    if (size() == N - 1) return true;
    else return false;
}

inline void DynamicDequeue::grow() {//when the deque is at its capacity and further elements are to be added, we expand it 
                            //eithe linearly ot exponentially based on factors in the included header file
    //unsigned int x = nextSizeLinear();
    Quad_Tree_Square_Node* B = new Quad_Tree_Square_Node[N];//creating a dynamic array to store values of the already existing deque
    unsigned int x = nextSizeExponential();
    for (unsigned int i = 0; i < N; i++) B[i] = A[i];
    delete[] A;//de-allocating the previous array in order to avoid memory leak
    A = new Quad_Tree_Square_Node[x];//allocating a new array with linear/exponentially increased capacity
    for (unsigned int j = f; j < N; j++) A[j - f] = B[j];//reassigning values without loss of continuity in the deque
    for (unsigned int j = 0; j < r; j++) A[(j + N - f) % N] = B[j];
    delete[] B;//de-allocating the array B to avoid memory leak
    r = N + r;//re-assigning value to r based on the deque in the new array
    //N = nextSizeLinear();//readjusting the size
    N = nextSizeExponential();
}

inline unsigned int DynamicDequeue::size() {//returns the size/total elements of the deque
    if (isEmpty()) return 0;
    else return (N - f + r) % N;
}

inline void DynamicDequeue::insertFront(Quad_Tree_Square_Node x) {//Insertion/Enqueue from the front
    if (isFull()) {//array grows if it is full in capacity
        grow();
    }
    f = (N + f - 1) % N;//re-adjusting the value for front index
    A[f] = x;//assigning value according to the enqueue
}

inline void DynamicDequeue::insertRear(Quad_Tree_Square_Node x) {//Insertion/Enqueue from the back/rear
    if (isFull()) {//array grows if it is full in capacity
        grow();
    }
    A[r % N] = x;//assigning value accrding to the enqueue
    r = (N + r + 1) % N;//re-adjusting the value of rear index
}

inline bool DynamicDequeue::removeFront(Quad_Tree_Square_Node* x) {//Dequeue operation from the front
    if (isEmpty()) return false;
    *x = A[f];
    f = (N + f + 1) % N;//re-adjusting the value of front index
    return true;
}

inline bool DynamicDequeue::removeRear(Quad_Tree_Square_Node* x) {//Dequeue operation from the back
    if (isEmpty())return false;
    r = (N + r - 1) % N;//re-adjusting the value of rear index
    *x = A[r];
    return true;
}

inline bool DynamicDequeue::front(Quad_Tree_Square_Node* x) {//Returning the front element
    if (isEmpty()) return false;
    *x = A[f];
    return true;
}

inline bool DynamicDequeue::rear(Quad_Tree_Square_Node* x) {//Returning the back element
    if (isEmpty()) return false;
    *x = A[(N + r - 1) % N];
    return true;
}

inline Mandelbrot::Mandelbrot() {
    DynamicDequeue a;
    Quad_Tree = a;
}

inline bool Mandelbrot::inCardoidOrBulb(double x, double y) {
    double y2 = y * y;
    double q = (x - 0.25) * (x - 0.25) + y2;
    if (q * (q + (x - 0.25)) < y2 / 4.0 || (x + 1.0) * (x + 1.0) + y2 < 0.0625) return true;
    return false;
}

inline int Mandelbrot::escape_time(int ix, int iy) {
    double x = xscale * ix + xmin;
    double y = -yscale * ix + ymax;
    if (inCardoidOrBulb(x, y)) return MAX_ITERATIONS;
    double cx = x;
    double cy = y;
    double zx = x;
    double zy = y;
    int iter;
    for (iter = 0; iter < MAX_ITERATIONS; iter++) {
        double zxtemp = zx * zx - zy * zy;
        zy = 2 * zx * zy + cy;
        zx = zxtemp + cx;
        if (zx * zx + zy * zy - 4.0 > 0.00001) {
            break;
        }
    }
    return iter;
}

inline void Mandelbrot::colour(int ix, int iy, int* r1, int* g1, int* b1) {
    double t = double(escape_time(ix, iy))/double(MAX_ITERATIONS);
    *r1 = int(9 * (1 - t) * t * t * t * 256);
    *g1 = int(15 * (1 - t) * (1 - t) * t * t * 256);
    *b1 = int(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 256);
    return;
}

inline void Mandelbrot::colour(int iter, int* r1, int* g1, int* b1) {
    double t = double(iter) / double(MAX_ITERATIONS);
    *r1 = int(9 * (1 - t) * t * t * t * 256);
    *g1 = int(15 * (1 - t) * (1 - t) * t * t * 256);
    *b1 = int(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 256);
    return;
}

inline int Mandelbrot::Mandelbrot_Set() {

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event event;

    SDL_CreateWindowAndRenderer(1440, 960, 0, &window, &renderer);
    SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);

    Quad_Tree_Square_Node sq1(0, 0, SIDE / 2);
    Quad_Tree_Square_Node sq2(SIDE / 2, 0, SIDE / 2);
    Quad_Tree_Square_Node sq3(SIDE, 0, SIDE / 2);

    Quad_Tree.insertRear(sq1);
    Quad_Tree.insertRear(sq2);
    Quad_Tree.insertRear(sq3);

    while (!Quad_Tree.isEmpty()) {

        Quad_Tree_Square_Node* sq = new Quad_Tree_Square_Node;
        Quad_Tree.removeRear(sq);
        int iter, diff_iter;

        int l = sq->length;
        int ix = sq->Top_Left_coord_x;
        int iy = sq->Top_Left_coord_y;
        int l2 = l / 2;

        if (l == 1) {
            iter = escape_time(ix, iy);
        }
        else if (l == 2) {
            int it[4];
            it[0] = escape_time(ix, iy);
            it[1] = escape_time(ix + l - 1, iy);
            it[2] = escape_time(ix + l - 1, iy + l - 1);
            it[3] = escape_time(ix, iy + l - 1);
            iter = (it[0] + it[1] + it[2] + it[3]) / 4;
            diff_iter = std::max(it[0], std::max(it[1], std::max(it[2], it[3]))) - std::min(it[0], std::min(it[1], std::min(it[2], it[3])));
        }
        else {
            int it[9];
            it[0] = escape_time(ix, iy);
            it[1] = escape_time(ix + l2 - 1, iy);
            it[2] = escape_time(ix + l - 1, iy);
            it[3] = escape_time(ix + l - 1, iy + l2 - 1);
            it[4] = escape_time(ix + l2 -1, iy + l2 - 1);
            it[5] = escape_time(ix + l - 1, iy + l - 1);
            it[6] = escape_time(ix + l2 - 1, iy + l - 1);
            it[7] = escape_time(ix, iy + l - 1);
            it[8] = escape_time(ix, iy + l2 - 1);
            iter = (it[0] + it[1] + it[2] + it[3] + it[4] + it[5] + it[6] + it[7] + it[8]) / 9;
            int max_it = it[0], min_it = it[0];
            for (int i = 0; i < 9; i++) {
                if (max_it < it[i]) max_it = it[i];
                if (min_it > it[i]) min_it = it[i];
            }
            diff_iter = max_it - min_it;
        }

        int r2, g2, b2;
        colour(iter, &r2, &g2, &b2);
        int yn = SIDE - iy - l;

        if (line > 0) {
            int b = l - line;
            SDL_Rect a;
            a.x = ix + 1 + b / 2;
            a.y = iy + 1 + b / 2;
            a.w = b;
            a.h = b;
            SDL_SetRenderDrawColor(renderer, r2, g2, b2, 255);
            SDL_RenderFillRect(renderer, &a);
            SDL_RenderPresent(renderer);
            SDL_Rect a2;
            a2.x = ix + 1 + b / 2;
            a2.y = yn + 1 + b / 2;
            a2.w = b;
            a2.h = b;
            SDL_SetRenderDrawColor(renderer, r2, g2, b2, 255);
            SDL_RenderFillRect(renderer, &a2);
            SDL_RenderPresent(renderer);
        }
        else {
            SDL_Rect c;
            c.x = ix + l / 2;
            c.y = iy + l / 2;
            c.w = l;
            c.h = l;
            SDL_SetRenderDrawColor(renderer, r2, g2, b2, 255);
            SDL_RenderFillRect(renderer, &c);
            SDL_RenderPresent(renderer);
            SDL_Rect c2;
            c2.x = ix + l / 2;
            c2.y = yn + l / 2;
            c2.w = l;
            c2.h = l;
            SDL_SetRenderDrawColor(renderer, r2, g2, b2, 255);
            SDL_RenderFillRect(renderer, &c2);
            SDL_RenderPresent(renderer);
        }
        int ixn = ix + l2;
        int iyn = iy + l2;
        if (l > 1 && diff_iter > 0) {
            Quad_Tree.insertRear(Quad_Tree_Square_Node(ix,iy,l2));
            Quad_Tree.insertRear(Quad_Tree_Square_Node(ixn, iy, l2));
            Quad_Tree.insertRear(Quad_Tree_Square_Node(ixn, iyn, l2));
            Quad_Tree.insertRear(Quad_Tree_Square_Node(ix, iyn, l2));
        }
    }

    SDL_Delay(5000); //wait time for window
    SDL_DestroyWindow(window);
    SDL_QUIT;

    return EXIT_SUCCESS;
}

#endif // !MANDELBROT_CPP
