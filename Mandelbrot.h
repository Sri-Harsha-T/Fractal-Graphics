#pragma once
#ifndef MANDELBROT_H
#define MANDELBROT_H
#include <SDL2/SDL.h>
//#include <Windows.h>
#include <cmath>
#include <algorithm>


#define CONST_GROWTH 100
#define EXPO_GROWTH_FACTOR 2
#define INIT_SIZE 1000

int SIDE;
int HEIGHT;
int WIDTH;
int MAX_ITERATIONS;
int line;

double xmin;
double xmax;
double xd;
double ymax;
double ymin;
double yd;
double xscale;
double yscale;

struct Quad_Tree_Square_Node {
	int Top_Left_coord_x;
	int Top_Left_coord_y;
	int length;
	Quad_Tree_Square_Node();
	Quad_Tree_Square_Node(int TLCX, int TLCY, int len);
};



class DynamicDequeue {
private:
	Quad_Tree_Square_Node* A; // the array used for implementing the dynamic dequeue
	unsigned int N; // the current size of array A
	unsigned int f; // index where first element of queue is present (if not empty)
	unsigned int r; // index where next element will be inserted
	unsigned int nextSizeLinear() { return N + CONST_GROWTH; }
	unsigned int nextSizeExponential() { return N * EXPO_GROWTH_FACTOR; }
public:
	DynamicDequeue(); // default constructor
	bool isEmpty(); // is the queue empty?
	bool isFull(); // is the queue full?
	void grow(); // grow the queue to its next size (linear or exponential)
	unsigned int size(); // return the current number of elements in the queue
	void insertFront(Quad_Tree_Square_Node x); // insert given value in front of the queue; grow array size as necessary
	void insertRear(Quad_Tree_Square_Node x); // insert given value in rear of the queue; grow array size as necessary
	bool removeFront(Quad_Tree_Square_Node* x); // remove the value in front of the queue and place it in *x; returns false if queue is empty, true otherwise
	bool removeRear(Quad_Tree_Square_Node* x); // remove the value in rear of the queue and place it in *x; returns false if queue is empty, true otherwise
	bool front(Quad_Tree_Square_Node* x); // place value in front of queue in *x; returns false if queue is empty, true otherwise
	bool rear(Quad_Tree_Square_Node* x); // place value in rear of queue in *x; returns false if queue is empty, true otherwise
};


class Mandelbrot {
public:
	DynamicDequeue Quad_Tree;
	Mandelbrot();
	int Mandelbrot_Set();
	bool inCardoidOrBulb(double x, double y);
	int escape_time(int ix, int iy);
	void colour(int ix, int iy, int* r1, int* g1, int* b1);
	void colour(int iter, int* r1, int* g1, int* b1);

};


#endif
