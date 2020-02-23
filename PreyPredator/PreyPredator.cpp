// PreyPredator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "Grid.h"
#include <ctime>

#include "GridMPI_GC.h"
#include "GridMPI_OptGC.h"

#include "Graph.h"

int main(int argc, char **argv)
{
	_setmode(_fileno(stdout), _O_U16TEXT);

	//Non-MPI
	//Grid grid = Grid(100, 100);

	Graph graph = Graph(1000,500);
	//graph.drawGraph("100x100", "500x500", "1000x2000", 0.041, 0.490, 3.953, "Sequential (100 Generations)");

	vector<vector<string>> x;
	vector<vector<float>> y;

	vector<string> x1 = { "100x100","500x500","1000x2000" };
	vector<string> x2 = { "100x100","500x500","1000x2000" };
	vector<string> x3 = { "100x100","500x500","1000x2000" };
	vector<string> x4 = { "100x100","500x500","1000x2000" };

	vector<float> y1 = { 8.776f,113.193f,749.602f };
	vector<float> y2 = { 7.103f,91.779f,622.124f };
	vector<float> y3 = { 6.238f,86.233f,585.327f };
	vector<float> y4 = { 119.988f,111.142f,497.201f };

	x.push_back(x1); x.push_back(x2); x.push_back(x3); x.push_back(x4);
	y.push_back(y1); y.push_back(y2); y.push_back(y3); y.push_back(y4);

	graph.drawGraph(x,y,"Hybrid (10000 Generations, 2 Threads)");
	
	//MPI

	/*int nprocs;
	int myid;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Status status;

	wcout << L" id: " << myid << endl;
	//fflush(stdout);

	GridMPI_OptGC grid = GridMPI_OptGC(100,100, myid, nprocs);
	*/
	
	//fflush(stdout);
	//grid.destroyGrid();

	
	//grid.calculateGenerations();

	//grid.debugArray(grid.currentWorld, L"World");

	system("pause");
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

