// GameOfLife.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "Grid.h"
#include <ctime>
/*#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
*/

using namespace std;
//using namespace cv;

int main(int *argc, char ***argv)
{

	
	//printf("Hellow World!\n");

	//Grid grid = Grid(100,100);
	Grid grid = Grid(100, 100);

	//Mat img = Mat(grid.height,grid.width, CV_16U, Scalar(255));

	//grid.debugArray(grid.currentWorld);

	//grid.debugArray(grid.currentArray, "Ghost Array Old");

	

	grid.debugArray(grid.currentWorld, "World");
	/*for (int i = 0; i < grid.height;i++) {
		for (int j = 0; j < grid.width;j++) {
			//img.at<uchar>(i,j) = (int)grid.currentWorld[i][j]*255;
		}
	}

	namedWindow("Display window", WINDOW_AUTOSIZE);// Create a window for display.
	imshow("Display window", img);
	waitKey();*/

	
	//system("pause");

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
