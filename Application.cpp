//Application.cpp for the Method implementing the possible improved version of Mandelbrot zoom using queues and quad_trees
//By Sri Harsha Thota, Roll no.: 200050138 

#include "Mandelbrot.cpp"


int main(int argc, char* argv[]) {

    SIDE = 1024;				//side of the initial square
    HEIGHT = 1024;				//Height of the rectangle/square considered initially
    WIDTH = 1536;				//width of the window 
    MAX_ITERATIONS = 1440;			//maximum iterations for checking the divergence of the points on complex plane
    line = 0;					//width of the line

    xmin = -2.0;				//minimum and maximum values to scale the pixel coordinates along real and imaginary axes
    xmax = 1.0;
    xd = xmax - xmin;				//range in which values are scaled
    ymax = xd / 3.0;
    ymin = -ymax;
    yd = ymax - ymin;
    xscale = xd / double(WIDTH);		//scale of real and imaginary parts of numbers
    yscale = yd / double(HEIGHT);

	Mandelbrot x;				//calling the Mandelbrot class constructor
    SDL_Init(SDL_INIT_EVERYTHING);		//initialising the SDL windows/application

    SDL_Window* window;			
    SDL_Renderer* renderer;
    SDL_Event event;				//setting variables for window, renderer and events

    SDL_CreateWindowAndRenderer(1536, 1024, 0, &window, &renderer);	//creating the window
    SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);

    Quad_Tree_Square_Node sq1(0, 0, SIDE / 2);			//creating 3 squares along the top horizontal axis for initalisng the queue
    Quad_Tree_Square_Node sq2(SIDE / 2, 0, SIDE / 2);
    Quad_Tree_Square_Node sq3(SIDE, 0, SIDE / 2);

    x.Quad_Tree.insertRear(sq1);					//inserting the squares in the queue/quad_tree
    x.Quad_Tree.insertRear(sq2);
    x.Quad_Tree.insertRear(sq3);

    while (!x.Quad_Tree.isEmpty()) {					//loop similar to BFS

        Quad_Tree_Square_Node* sq = new Quad_Tree_Square_Node;
        x.Quad_Tree.removeFront(sq);					//getting the front square
        int iter, diff_iter;

        int l = sq->length;						//setting vatriables such as length and coordinates of the top left corner of the squares
        int ix = sq->Top_Left_coord_x;
        int iy = sq->Top_Left_coord_y;
        int l2 = l / 2;

        if (l == 1) {							//if l==1 then the square cannot be divided further
            iter = x.escape_time(ix, iy);				//calculating the iteration at which the point related to square diverges
        }
        else if (l == 2) {						//dividing the square into 4 equal parts
            int it[4];
            it[0] = x.escape_time(ix, iy);				//calculating the iteration at which the point related to square diverges for each component square
            it[1] = x.escape_time(ix + l - 1, iy);
            it[2] = x.escape_time(ix + l - 1, iy + l - 1);
            it[3] = x.escape_time(ix, iy + l - 1);
            iter = (it[0] + it[1] + it[2] + it[3]) / 4;		//taking the mean iteration value
            diff_iter = std::max(it[0], std::max(it[1], std::max(it[2], it[3]))) - std::min(it[0], std::min(it[1], std::min(it[2], it[3])));	//taking the range of iteration values
        }
        else {								//if square is larger than 2*2 pixels
            int it[9];
            it[0] = x.escape_time(ix, iy);				//calculating the iteration at which the point related to square diverges for each component square
            it[1] = x.escape_time(ix + l2 - 1, iy);
            it[2] = x.escape_time(ix + l - 1, iy);
            it[3] = x.escape_time(ix + l - 1, iy + l2 - 1);
            it[4] = x.escape_time(ix + l2 - 1, iy + l2 - 1);
            it[5] = x.escape_time(ix + l - 1, iy + l - 1);
            it[6] = x.escape_time(ix + l2 - 1, iy + l - 1);
            it[7] = x.escape_time(ix, iy + l - 1);
            it[8] = x.escape_time(ix, iy + l2 - 1);
            iter = (it[0] + it[1] + it[2] + it[3] + it[4] + it[5] + it[6] + it[7] + it[8]) / 9;//taking the mean iteration value
            int max_it = it[0], min_it = it[0];
            for (int i = 0; i < 9; i++) {
                if (max_it < it[i]) max_it = it[i];
                if (min_it > it[i]) min_it = it[i];
            }
            diff_iter = max_it - min_it;					//calculating the iteration at which the point related to square diverges for each component square
        }

        int r2, g2, b2;							//colouring the squares enclosing the area
        x.colour(iter, &r2, &g2, &b2);
        int yn = SIDE - iy - l;

        if (line > 0) {							//if the line width is positive
            int b = l - line;							//finding the difference between the value of l and line width
            SDL_Rect a;							//creating a rectangle to colour the region
            a.x = ix + 1 ;//+ b / 2;						//setting the coordinates of the sides
            a.y = iy + 1 ;//+ b / 2;						
            a.w = b;								//setting the edge length of the square we draw
            a.h = b;
            SDL_SetRenderDrawColor(renderer, r2, g2, b2, 255);		//Drawing the rectangle
            SDL_RenderFillRect(renderer, &a);
            SDL_RenderPresent(renderer);
            SDL_Rect a2;
            a2.x = ix + 1 ;//+ b / 2;						//same process for the second divided part of the square
            a2.y = yn + 1 ;//+ b / 2;
            a2.w = b;
            a2.h = b;
            SDL_SetRenderDrawColor(renderer, r2, g2, b2, 255);
            SDL_RenderFillRect(renderer, &a2);
            SDL_RenderPresent(renderer);
        }
        else {									//if line width is still 0
            SDL_Rect a;
            a.x = ix ;//+ l / 2;
            a.y = iy ;//+ l / 2;
            a.w = l;
            a.h = l;
            SDL_SetRenderDrawColor(renderer, r2, g2, b2, 255);		//setting edges and coordinates of the squares
            SDL_RenderFillRect(renderer, &a);
            SDL_RenderPresent(renderer);
            SDL_Rect a2;
            a2.x = ix ;//+ l / 2;
            a2.y = yn ;//+ l / 2;
            a2.w = l;
            a2.h = l;
            SDL_SetRenderDrawColor(renderer, r2, g2, b2, 255);		//drawing the squares on the canvas
            SDL_RenderFillRect(renderer, &a2);
            SDL_RenderPresent(renderer);
        }
        int ixn = ix + l2;
        int iyn = iy + l2;
        if (l > 1 && diff_iter > 0) {						//checking if the length od side of square is more than one pixel and range of iteration values is positive
            x.Quad_Tree.insertFront(Quad_Tree_Square_Node(ix, iy, l2));	//adding smaller and newer squares into the queue
            x.Quad_Tree.insertFront(Quad_Tree_Square_Node(ixn, iy, l2));
            x.Quad_Tree.insertFront(Quad_Tree_Square_Node(ixn, iyn, l2));
            x.Quad_Tree.insertFront(Quad_Tree_Square_Node(ix, iyn, l2));
        }
    }

    SDL_Delay(5000); //wait time for window
    SDL_DestroyWindow(window);
    SDL_QUIT;

    return EXIT_SUCCESS;
}
