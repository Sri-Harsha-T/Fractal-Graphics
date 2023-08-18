#include <SDL2/SDL.h>
#include <Windows.h>
#include <iostream>

long double map(long double value, long double in_min, long double in_max, long double out_min, long double out_max) {
	return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int main(int argc, char* argv[]){

	int WIDTH = 800;
	int HEIGHT = 800;
	int MAX_ITERATIONS = 200;

	double min_re = -2.0;
	double max_re = 2.0;
	double min_im = -2.0;
	long double max_im = 2.0;
	long double zoom = 1.0;

	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Event event;

	SDL_CreateWindowAndRenderer(1440, 960, 0, &window, &renderer);
	SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);

	while (true) {

		/*if (GetKeyState('W')) {

		}

		SDL_RenderPresent(renderer);

		if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
			return 0;
		}
		if (GetKeyState('Q')) {
			return 0;
		}

		double w = (max_re - min_re) * 0.3;
		double h = (max_im - min_im) * 0.3;
		if (GetKeyState('W') ) {
			min_im -= h;
			max_im -= h;
			SDL_PumpEvents();
		}
		if (GetKeyState('S')) {
			min_im += h;
			max_im += h;
			SDL_PumpEvents();
		}
		if (GetKeyState('A')) {
			min_re -= w;
			max_re -= w;
			SDL_PumpEvents();
		}
		if (GetKeyState('D')) {
			min_re += w;
			max_re += w;
			SDL_PumpEvents();
		}
*/
		if (event.type==SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
			auto zoom_x = [&](double z)
			{
				int mousex, mousey;
				SDL_GetMouseState(&mousex, &mousey);
				std::cout << mousex << "\t" << mousey << "\n";
				//mouse point will be new center point
				double cr = min_re + (max_re - min_re) * mousex / WIDTH;
				double ci = min_im + (max_im - min_im) * mousey / HEIGHT;

				//zoom
				double tminr = cr - (max_re - min_re) / 2 / z;
				max_re = cr + (max_re - min_re) / 2 / z;
				min_re = tminr;

				double tmini = ci - (max_im - min_im) / 2 / z;
				max_im = ci + (max_im - min_im) / 2 / z;
				min_im = tmini;
			};

			zoom_x(5);
			zoom *= 5;
		}
		
		for (int x = 0; x < WIDTH; x++) {

			for (int y = 0; y < HEIGHT; y++) {

				double cr = map(x, 0, WIDTH, min_re, max_re);	//min_re + (max_re - min_re) * x / W;
				double ci = map(y, 0, HEIGHT, min_im, max_im);	// min_im + (max_im - min_im) * y / H;
				double re = 0, im = 0;
				int iter;
				for (iter = 0; iter < MAX_ITERATIONS; iter++)
				{
					double tr = re * re - im * im + cr;
					im = 2 * re * im + ci;
					re = tr;
					if (re * re + im * im > 4.0) break;
				}

				long double t = (long double)iter / (long double)MAX_ITERATIONS;
				long double b1 = 8.5 * (1 - t) * (1 - t) * (1 - t) * t;
				long double g1 = 15 * (1 - t) * (1 - t) * t * t;
				long double r1 = 9 * (1 - t) * t * t * t;
				SDL_SetRenderDrawColor(renderer, (int)(256 * r1), (int)(256 * g1), (int)(256 * b1), 255);
				SDL_RenderDrawPoint(renderer, x, y);
			}
		}
		//SDL_Delay(5000);
	}

	return 0;
}
