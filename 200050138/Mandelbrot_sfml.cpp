//Mandelbrot_sfml.cpp for CS293 Course Project
//By Sri Harsha Thota, 200050138
//Simple implementation of Mandelbrot zoom in SFML Graphics


#include <SFML/Graphics.hpp>
using namespace std;
using namespace sf;

const int W = 1440;//WIDTH of the Window
const int H = 900;//Height of the Window

int max_iter = 152;//Maximum iteration, can be altered according to the mouse wheel event
double zoom = 1.0;//initial zoom of the mandelbrot set
double min_re = -2.5, max_re = 1;//minimum and maximum bounds for range of real values for scaling
double min_im = -1, max_im = 1;//minimum and maximum bounds for range of imaginary values for scaling



int main(int argc, char* argv[])
{
	RenderWindow window(VideoMode(W, H), "Mandelbrot Zoom");
	Image image; image.create(W, H);	//Creating the image window for the mandelbrot set
	Texture texture;
	Sprite sprite;
	

	while (window.isOpen())			//while loop for running the program
	{
		Event e;
		while (window.pollEvent(e))
		{
			if (e.type == Event::Closed) window.close();
			if (e.type == Event::KeyPressed)
			{
				//move delta
				double w = (max_re - min_re)*0.3;
				double h = (max_im - min_im)*0.3;

				if (e.key.code == Keyboard::Left) { min_re -= w, max_re -= w; }	//event when the left key is pressed
				if (e.key.code == Keyboard::Right) { min_re += w, max_re += w; }	//event when the right key is pressed
				if (e.key.code == Keyboard::Up) { min_im -= h, max_im -= h; }	//event when the up key is pressed
				if (e.key.code == Keyboard::Down) { min_im += h, max_im += h; }	//event when the down key is pressed
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))		//event when escape is pressed
				{
				    return 0;
				}

			}
			//Set Iteration level by mouse wheel
			//the more iteration level the better image result
			if (e.type == Event::MouseWheelScrolled)					//event when mouse wheel is scrolled, max_iterations are changed
			{
				if (e.MouseWheelScrolled)
				{
					if (e.mouseWheelScroll.wheel == Mouse::VerticalWheel)
					{
						if (e.mouseWheelScroll.delta > 0)max_iter *= 2;
						else max_iter /= 2;
						if (max_iter < 1)max_iter = 1;
					}
				}
			}
			if (e.type == Event::MouseButtonPressed)					//when mouse buttons are pressed, the centre changes to the coordinates where the button press occured 
													//and the mandelbrot plot zooms in or out depending upon the left/right button clicked
			{
				
				//Left Click to ZoomIn
				if (e.mouseButton.button == Mouse::Left)				//when right button is clicked
				{
					//mouse point will be new center point
					double z=5;
					double cr = min_re + (max_re - min_re)*e.mouseButton.x / W;	//change of centre  coordinate in real axis
					double ci = min_im + (max_im - min_im)*e.mouseButton.y / H;	//change of centre coordinate in imaginary axis

					//zoom
					double tminr = cr - (max_re - min_re) / 2 / z;
					max_re = cr + (max_re - min_re) / 2 / z;			//change in the bounds of ranges in real axis
					min_re = tminr;

					double tmini = ci - (max_im - min_im) / 2 / z;		//change in the bounds of ranges in imaginary axis
					max_im = ci + (max_im - min_im) / 2 / z;
					min_im = tmini;
					zoom *= 5;
				}
				//Right Click to ZoomOut
				if (e.mouseButton.button == Mouse::Right)				//when left button is clicked
				{
					double z=0.2;
					double cr = min_re + (max_re - min_re)*e.mouseButton.x / W;	//change of centre  coordinate in real axis
					double ci = min_im + (max_im - min_im)*e.mouseButton.y / H;	//change of centre coordinate in imaginary axis

					//zoom
					double tminr = cr - (max_re - min_re) / 2 / z;
					max_re = cr + (max_re - min_re) / 2 / z;			//change in the bounds of ranges in real axis
					min_re = tminr;

					double tmini = ci - (max_im - min_im) / 2 / z;		//change in the bounds of ranges in imaginary axis
					max_im = ci + (max_im - min_im) / 2 / z;
					min_im = tmini;
					zoom /= 5;
				}
			}
		}
		window.clear();									//clearing the window incase one of the above events occured
		for (int y = 0; y < H; y++)								//iterating through the pixels along imaginary axis
		{
			for (int x = 0; x < W; x++)							//iterating through the pixels along/parallel to real axis
			{
				double cr = min_re + (max_re - min_re)*x / W;				//mapping the centres according to the set bounds for calculation of number of iterations
				double ci = min_im + (max_im - min_im)*y / H;				
				double re = 0, im = 0;
				int iter;
				for (iter = 0; iter < max_iter; iter++)				//calculating the number of iterations before which the comlpex number diverges if it does
				{
					double tr = re * re - im * im + cr;				//repeating the process of z'=z*z+c
					im = 2 * re*im + ci;
					re = tr;
					if (re*re + im * im > 2 * 2) break;				//breaking from the loop if divergence is found to occur
				}
				long double t = (double)iter/double(max_iter);			//variable determing the colour of a pixel in the mandelbrot plot
				int b1 = int (8.5 * (1 - t) * (1 - t) * (1 - t) * t * 256);		//using bernstein polynomials for the coliurs of the pixels(r,g,b) for the mandelbrot plot
				int g1 = int (15 * (1 - t) * (1 - t) * t * t*256);			
				int r1 = int(9 * (1 - t) * t * t * t*256);

				image.setPixel(x, y, Color(r1,g1,b1));				//setting the colours for the images
			}
		}
		texture.loadFromImage(image);
		sprite.setTexture(texture);
		window.draw(sprite);




		window.display();
	}
	return 0;
}
				
