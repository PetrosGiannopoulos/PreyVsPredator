#pragma once

#include <vector>
#include <iterator>
#include <iostream>
#include <string>
#include <time.h>
#include <Windows.h>
#include <omp.h>
#include <opencv2\opencv.hpp>
#include <mpi.h>

//0: plain MPI, 2: hybrid 2 threads, 4: hybrid 4 threads
#define MODE 2

using namespace std;
using namespace cv;

class GridMPI_OptGC {

public:
	int width, height;

	vector<vector<int>> currentWorld;
	vector<vector<int>> nextWorld;

	vector<vector<int>> divTable;
	vector<vector<int>> nextDivTable;

	int nprocs;
	int myid;

	int end = 0;
public:

	GridMPI_OptGC(int width, int height, int id, int nprocs) {

		this->myid = id;
		this->nprocs = nprocs;
		//srand(time(NULL));
		this->width = width;
		this->height = height;


		if (myid == 0) {

			//initialize array with ghost cells
			initializeArray();

			calculateGenerations();
			
			destroyGrid();

		}
		else {
			//wait for rows to be sent
			MPI_Barrier(MPI_COMM_WORLD);

			//recieve rows
			int rows;
			MPI_Recv(&rows, width + 2, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			//wait for pieces to be sent
			MPI_Barrier(MPI_COMM_WORLD);

			//recieve piece
			divTable.clear();
			int* table = new int[width + 2];
			for (int j = 0; j < rows; j++) {

				MPI_Recv(&(*table), width + 2, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				vector<int> row;
				for (int k = 0; k < width + 2; k++)row.push_back(table[k]);

				divTable.push_back(row);
			}
			delete[] table;
			while (end==0) {
				
				//evolve piece
				evolveMPI();
				
				
				MPI_Barrier(MPI_COMM_WORLD);
				
				//sendPiece();
				
				send_GCRow();
				recv_GCRow();

				
				//wait for end signal
				MPI_Barrier(MPI_COMM_WORLD);
				
				MPI_Recv(&end, 1, MPI_INT, 0, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				
			}
			
			sendPiece();
			

			destroyGrid();
		}

	}

	void initializeArray() {
		for (int i = 0; i < height; i++) {

			vector<int> row;
			for (int j = 0; j < width; j++) {

				row.push_back(randomValue(0, 1));
				//wcout << randomValue(0, 1) << endl;
			}

			currentWorld.push_back(row);
			//nextWorld.push_back(row);
		}

		//add ghost cells

		for (int i = 0; i < height + 2; i++) {

			vector<int> row;
			for (int j = 0; j < width + 2; j++) {

				int value = 0;

				if (i == 0 && (j>0 && j<width + 1)) {
					value = currentWorld[height - 1][j - 1];
				}

				if (i == height + 1 && (j > 0 && j < width + 1)) {
					value = currentWorld[0][j - 1];
				}

				if (j == 0 && (i > 0 && i < height + 1)) {
					value = currentWorld[i - 1][width - 1];
				}

				if (j == width + 1 && (i > 0 && i < height + 1)) {
					value = currentWorld[i - 1][0];
				}

				if ((i > 0 && i < height + 1) && (j > 0 && j < width + 1)) {
					value = currentWorld[i - 1][j - 1];
				}

				row.push_back(value);

			}
			nextWorld.push_back(row);

		}

		currentWorld.clear();
		for (int i = 0; i < height + 2; i++) {
			vector<int> row;
			for (int j = 0; j < width + 2; j++) {
				row.push_back(nextWorld[i][j]);

			}

			currentWorld.push_back(row);
		}
		//wcout << L" h:" << currentWorld.size() << L" w:" << currentWorld[0].size() << endl;

		debugArrayImage(currentWorld, L"World", "init");
		//wcout << L" h: " << currentWorld.size() << L" w: " << currentWorld[0].size();
		//fflush(stdout);
	}

	void recalculateGhostCells() {
		//wcout << L" h:" << currentWorld.size() << L" w:" << currentWorld[0].size()<<endl;
		nextWorld.clear();
		for (int i = 0; i < height + 2; i++) {

			vector<int> row;
			for (int j = 0; j < width + 2; j++) {

				int value = 0;

				if (i == 0 && (j>0 && j<width + 1)) {
					value = currentWorld[height - 1][j - 1];
				}

				if (i == height + 1 && (j > 0 && j < width + 1)) {
					value = currentWorld[0][j - 1];
				}

				if (j == 0 && (i > 0 && i < height + 1)) {
					value = currentWorld[i - 1][width - 1];
				}

				if (j == width + 1 && (i > 0 && i < height + 1)) {
					value = currentWorld[i - 1][0];
				}

				//inside table - non ghost cell area
				if ((i > 0 && i < height + 1) && (j > 0 && j < width + 1)) {
					value = currentWorld[i - 1][j - 1];
				}

				row.push_back(value);

			}
			nextWorld.push_back(row);
		}
		//print();
		currentWorld.clear();
		for (int i = 0; i < height + 2; i++) {
			vector<int> row;
			for (int j = 0; j < width + 2; j++) {
				row.push_back(nextWorld[i][j]);

			}
			currentWorld.push_back(row);
		}
		
	}

	void calculateGenerations() {

		int steps = 10000;

		clock_t begin = clock();

		splitArrayToPieces();

		for (int stepI = 0; stepI < steps;stepI++) {


			//wait for other evolutions to finish
			MPI_Barrier(MPI_COMM_WORLD);

			evolveMPI();
			
			send_GCRow();

			recv_GCRow();
			//mergePieces();
			
			//recalculateGhostCells();
			
			if(stepI<steps-1)sendEndSignal(0);
			//if(stepI%100==0)debugArrayImage(currentWorld, L"World", "result");

		}

		sendEndSignal(1);
		
		mergePieces();

		

		clock_t end = clock();

		double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

		wcout << L"Time to complete: " << elapsed_secs << endl;

		debugArrayImage(currentWorld, L"World", "result");

	}

	void send_GCRow() {
		int rows = divTable.size();
		if (myid == 0) {
			
			int* divArrayRow = convertVectorToArray(divTable[rows-2]);
			MPI_Send(&(*divArrayRow), width+2, MPI_INT, 1, 5, MPI_COMM_WORLD);

			delete[] divArrayRow;
		}
		else if (myid == nprocs - 1) {
			int* divArrayRow = convertVectorToArray(divTable[1]);
			MPI_Send(&(*divArrayRow), width + 2, MPI_INT, myid-1, 5, MPI_COMM_WORLD);
			delete[] divArrayRow;
		}
		else {
			int* divArrayRowF = convertVectorToArray(divTable[1]);
			MPI_Send(&(*divArrayRowF), width + 2, MPI_INT, myid - 1, 5, MPI_COMM_WORLD);

			int* divArrayRowL = convertVectorToArray(divTable[rows - 2]);
			MPI_Send(&(*divArrayRowL), width + 2, MPI_INT, myid + 1, 5, MPI_COMM_WORLD);

			delete[] divArrayRowF;
			delete[] divArrayRowL;
		}

		MPI_Barrier(MPI_COMM_WORLD);

		

	}

	void recv_GCRow() {

		int rows = divTable.size();
		
		if (myid==0) {
			int *divArrayRow = new int[width + 2];
			MPI_Recv(&(*divArrayRow), width + 2, MPI_INT, 1, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
#ifdef MODE

#if MODE == 0
#elif MODE == 2	
			#pragma omp parallel for schedule(guided,2)
#elif MODE == 4
			#pragma omp parallel for schedule(guided,4)
#endif
#endif
			for (int i = 0; i < width + 2;i++) {
				divTable[rows - 1][i] = divArrayRow[i];
			}

			delete[] divArrayRow;
		}
		else if(myid==nprocs-1){
			int *divArrayRow = new int[width + 2];
			MPI_Recv(&(*divArrayRow), width + 2, MPI_INT, myid-1, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

#ifdef MODE

#if MODE == 0
#elif MODE == 2	
			#pragma omp parallel for schedule(guided,2)
#elif MODE == 4
			#pragma omp parallel for schedule(guided,4)
#endif
#endif
			//#pragma omp parallel for num_threads(4) schedule(guided)
			for (int i = 0; i < width + 2; i++) {
				divTable[0][i] = divArrayRow[i];
			}

			delete[] divArrayRow;
		}
		else {

			int *divArrayRowF = new int[width + 2];

			MPI_Recv(&(*divArrayRowF), width + 2, MPI_INT, myid - 1, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			int *divArrayRowL = new int[width + 2];

			MPI_Recv(&(*divArrayRowL), width + 2, MPI_INT, myid + 1, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
#ifdef MODE

#if MODE == 0
#elif MODE == 2	
			#pragma omp parallel for schedule(guided,2)
#elif MODE == 4
			#pragma omp parallel for schedule(guided,4)
#endif
#endif
			//#pragma omp parallel for num_threads(4) schedule(guided)
			for (int i = 0; i < width + 2; i++) {
				divTable[0][i] = divArrayRowF[i];
				divTable[rows-1][i] = divArrayRowL[i];
			}

			delete[] divArrayRowF;
			delete[] divArrayRowL;
		}

		
	}

	void sendEndSignal(int endSignal) {

		for (int i = 1; i < nprocs;i++) {
			MPI_Send(&endSignal, 1, MPI_INT, i, 4, MPI_COMM_WORLD);
		}

		MPI_Barrier(MPI_COMM_WORLD);
	}

	void splitArrayToPieces() {

		vector<vector<int>> tempTable;
		vector<vector<vector<int>>> tables;
		vector<int> rowSizes;
		divTable.clear();
		
		for (int k = 0; k < nprocs; k++) {

			tempTable.clear();

			int NWstart = k * ((width+2) / nprocs);
			int NHstart = k * ((height+2) / nprocs);
			int NHsize = (height+2) / nprocs;

			if (k == (nprocs - 1)) NHsize = (height+2) / nprocs + (height+2) % nprocs;

			//add overlapping start
			if (k != 0) {

				vector<int> row;
				for (int j = 0; j < width+2; j++) {

					row.push_back(currentWorld[NHstart - 1][j]);
				}
				if(k==0)divTable.push_back(row);
				tempTable.push_back(row);
			}

			//get split window part
			for (int i = NHstart; i < (NHstart + NHsize); i++) {

				vector<int> row;
				for (int j = 0; j < width+2; j++) {

					row.push_back(currentWorld[i][j]);
				}

				if (k == 0)divTable.push_back(row);
				tempTable.push_back(row);
			}

			//add overlapping end

			if (k != (nprocs - 1)) {
				vector<int> row;
				for (int j = 0; j < width+2; j++) {
					row.push_back(currentWorld[NHstart + NHsize][j]);
				}
				if (k == 0)divTable.push_back(row);
				tempTable.push_back(row);
			}

			if (k == 0)continue;
			int rows = tempTable.size();
			rowSizes.push_back(rows);
			tables.push_back(tempTable);
			
		}

		for (int i = 1; i < nprocs; i++) {
			//send numofrows
			MPI_Send(&rowSizes[i - 1], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		}
		//run MPI_Barrier to unblock waiting processes
		MPI_Barrier(MPI_COMM_WORLD);
		for (int i = 1; i < nprocs; i++) {

			//send pieces to processes
			for (int j = 0; j < rowSizes[i - 1]; j++) {
				int *divArrayRow = convertVectorToArray(tables[i-1][j]);
				MPI_Send(&(*divArrayRow), width + 2, MPI_INT, i, 1, MPI_COMM_WORLD);
				delete[] divArrayRow;
			}
			
		}

		//run MPI_Barrier to unblock waiting processes
		MPI_Barrier(MPI_COMM_WORLD);
	}

	void mergePieces() {


		//wait for other processes to send their rows back
		MPI_Barrier(MPI_COMM_WORLD);
		vector<int> rowsTable;
		for (int i = 1; i < nprocs;i++) {

			int rows;
			MPI_Recv(&rows, width + 2, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			rowsTable.push_back(rows);

		}

		//wait for other processes to send their pieces back
		MPI_Barrier(MPI_COMM_WORLD);

		vector<vector<vector<int>>> individualPieces;
		individualPieces.push_back(divTable);
		int* table = new int[width + 2];
		for (int i = 1; i < nprocs; i++) {
			vector<vector<int>> piece;
			for (int j = 0; j < rowsTable[i - 1];j++) {
				
				MPI_Recv(&(*table), width + 2, MPI_INT, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				vector<int> row;
				for (int k = 0; k < width + 2; k++)row.push_back(table[k]);

				
				piece.push_back(row);
			}
			individualPieces.push_back(piece);
		}
		delete[] table;

		currentWorld.clear();
		for (int i = 0; i < nprocs;i++) {

			vector<vector<int>> piece(individualPieces[i]);
			
			int pH = piece.size();
			int pW = piece[0].size();

			//wcout << L" h: " << pH << L" w: " << pW << " id: " << i<< endl;
			for (int j = 0; j < pH;j++) {
				vector<int> row;
				for (int k = 0; k < pW; k++) {

					row.push_back(piece[j][k]);
					
				}
				
				
				if (i == 0) {
					if(j < pH - 1) currentWorld.push_back(row);
				}
				else if ((i > 0 && i < nprocs - 1)) {

					if((j > 0 && j < pH - 1)) currentWorld.push_back(row);
				}
				else if (i == nprocs - 1) {

					if((j > 0))currentWorld.push_back(row);
				}
				
			}
			
		}
	}

	void sendPiece() {

		//send rows back
		int rows = divTable.size();
		MPI_Send(&rows, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
		MPI_Barrier(MPI_COMM_WORLD);

		
		for (int i = 0; i < rows;i++) {
			int *divArrayRow = convertVectorToArray(divTable[i]);
			MPI_Send(&(*divArrayRow), width + 2, MPI_INT, 0, 3, MPI_COMM_WORLD);

			delete[] divArrayRow;
		}

		//send piece back
		MPI_Barrier(MPI_COMM_WORLD);
	}

	void evolveMPI() {
		initKNextWorld();

		//TODO change arrays

		

		int h = divTable.size();
		int w = divTable[0].size();

#ifdef MODE

#if MODE == 0
#elif MODE == 2	
		#pragma omp parallel for schedule(guided,2)
#elif MODE == 4
		#pragma omp parallel for schedule(guided,4)
#endif
#endif
		//#pragma omp parallel for num_threads(4) schedule(guided)
		//#pragma omp parallel for schedule(guided,2)
		for (int i = 1; i<h-1; i++) {
			for (int j = 1; j < w-1; j++) {

				int globalI = i +myid * (h - 2);
				int globalJ = j + myid * (w - 2);

				int numNeighbors = 0;
				int numValidBreedingFish = 0;
				int numValidBreedingShark = 0;
				int numNFish = 0;
				int numNShark = 0;

				int nI[] = { i,i - 1,i + 1,i,i - 1,i + 1,i - 1,i + 1 };
				int nJ[] = { j - 1,j - 1,j - 1,j + 1,j + 1,j + 1,j,j };

				//#pragma omp parallel for reduction(+:numNFish,numNShark,numValidBreedingFish,numValidBreedingShark)
				for (int k = 0; k < 8; k++) {

					int value = getNMod(nI, nJ, k, h, w);

					//wcout << value << endl;

					if (value > 0)numNFish++;
					if (value < 0)numNShark++;

					if (value >= 2)numValidBreedingFish++;
					if (value <= -3)numValidBreedingShark++;
					//numNeighbors += value;

				}

				//wcout << L"NumFish: " << numNFish << L"| NumShark: " << numNShark<<endl;
				
				//rules of life 
				if (divTable[i][j] == 0) {
					//empty cell - breeding stage
					nextDivTable[i][j] = 0;
					if (numNFish >= 4 && numValidBreedingFish >= 3 && numNFish > numNShark) {
						nextDivTable[i][j] = 1;
						//continue;
					}
					else if (numNShark >= 4 && numValidBreedingShark >= 3 && numNFish < numNShark) {
						nextDivTable[i][j] = -1;
						//continue;
					}

					//continue;
				}
				else {
					//alive cell - health state

					if (divTable[i][j] > 0) {
						//fish
						if (divTable[i][j] == 10) {
							nextDivTable[i][j] = 0;
							//continue;
						}
						else if (numNShark >= 5) {
							nextDivTable[i][j] = 0;
							//continue;
						}
						else if (numNFish == 8) {
							nextDivTable[i][j] = 0;
							//continue;
						}
						//if a fish didnt die, increment age by 1
						else nextDivTable[i][j]++;
					}
					else if (divTable[i][j] < 0) {
						//shark
						if (divTable[i][j] == -20) {
							nextDivTable[i][j] = 0;
							//continue;
						}
						else if (numNShark >= 6 && numNFish == 0) {
							//starvation
							nextDivTable[i][j] = 0;
							//continue;
						}
						//random death cause
						else if (shouldSharkDie(globalI*width + globalJ)) {
							nextDivTable[i][j] = 0;
							//continue;
						}
						
						//if a shark didnt die, increment age by 1
						else nextDivTable[i][j]--;
					}

				}

			}
		}

		copyKToCurrent();
		//copyBackToCurrent();
	}

	~GridMPI_OptGC() {
		destroyGrid();
	}

	float randomRange(float a, float b) {

		float random = ((float)rand()) / (float)RAND_MAX;
		float diff = b - a;
		float r = random * diff;
		return a + r;
	}

	int randomValue(float a, float b) {

		float n = randomRange(a, b);

		//shark = -1, fish = 1, water =0 

		/*if (n < 0.5f)return 1;
		else if (n > 0.75f)return -1;*/

		if (n <= 0.25f)return 1;
		else if (n < 0.466454)return -1;
		return 0;


		//return (int)100 * (n);
	}

	bool shouldSharkDie(int seed) {

		srand(seed);
		int n = randomRange(1, 33*50);

		if (n < 2)return true;
		return false;
	}

	void print() {
		wcout << endl << L" PRINT!" << endl;
	}

	int** convertVectorToArray(vector<vector<int>> &v) {

		size_t sH = v.size();
		size_t sW = v[0].size();

		int** array2D = new int*[sH];


		for (int i = 0; i < sH; i++) {

			array2D[i] = new int[sW];
			for (int j = 0; j < sW; j++) {

				array2D[i][j] = v[i][j];
			}

		}

		return array2D;

	}

	int* convertVectorToArray(vector<int> &v) {

		//wcout << L"sH: " << v.size() << endl;

		size_t sH = v.size();
		int* array1D = new int[sH];
		for (int i = 0; i < sH; i++) {
			array1D[i] = v[i];
		}
		return array1D;

	}

	vector<vector<int>> convertArrayToVector(int** v) {

		size_t sH = sizeof(v) / sizeof(v[0]);
		size_t sW = sizeof(v[0]) / sizeof(int);

		//wcout << L"sH: " << sH << endl;
		//wcout << L"sW: " << sW << endl;
		int** array2D = new int*[sH];

		vector<vector<int>> result;
		for (int i = 0; i < sH; i++) {

			vector<int> row;
			for (int j = 0; j < sW; j++) {

				//array2D[i][j] = v[i][j];
				row.push_back(v[i][j]);
			}

			result.push_back(row);


		}

		return result;

	}

	vector<int> convertArrayToVector(int* v, int size) {

		int sH = size;
		//int* array1D = new int[sH];

		vector<int> result;
		for (int i = 0; i < sH; i++) {
			result.push_back(v[i]);
		}
		return result;

	}

	int getNMod(int(&nI)[8], int(&nJ)[8], int n, int h, int w) {


		int inI = nI[n];
		int inJ = nJ[n];

		int mI = 0;
		int mJ = 0;

		if (inI > 0) {
			//mI = inI % (height);
			mI = reduce(inI, height);
		}
		else {
			inI = h + inI;
			//mI = inI % (height);
			mI = reduce(inI, h);
		}

		if (inJ > 0) {
			//mJ = inJ % (width);
			mJ = reduce(inJ, w);
		}
		else {
			inJ = w + inJ;
			//mJ = inJ % (width);
			mJ = reduce(inJ, w);
		}

		//cout << "mI: " << mI << "| mJ: " << mJ << endl;
		return divTable[mI][mJ];
	}

	inline int reduce(const int input, const int ceil) {

		//https://www.youtube.com/watch?v=nXaxk27zwlk&feature=youtu.be&t=56m34s
		// apply the modulo operator only when needed
		// (i.e. when the input is greater than the ceiling)
		return input >= ceil ? input % ceil : input;
		//return input % ceil;
		// NB: the assumption here is that the numbers are positive
	}

	void copyBackToCurrent(vector<vector<int>> v) {


		//#pragma omp parallel for
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				//currentWorld[i][j] = nextWorld[i][j];
				currentWorld[i][j] = v[i][j];
			}
		}

		//currentWorld.swap(nextWorld);

	}



	void initKNextWorld() {

		nextDivTable.clear();
		int h = divTable.size();
		int w = divTable[0].size();
		for (int i = 0; i < h;i++) {
			vector<int> row;
			for (int j = 0; j < w;j++) {
				row.push_back(divTable[i][j]);
			}
			nextDivTable.push_back(row);
		}

	}

	void copyKToCurrent() {
		
		
		int h = divTable.size();
		int w = divTable[0].size();
		divTable.clear();
		for (int i = 0; i < h; i++) {
			vector<int> row;
			for (int j = 0; j < w; j++) {
				row.push_back(nextDivTable[i][j]);
			}
			divTable.push_back(row);
			
		}

	}

	void debugArray(vector<vector<int>> v, wstring text = L"") {

		wcout << text;

		int h = v.size();
		int w = v[0].size();
		wcout << endl;

		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {

				wstring t = L"0";
				if (v[i][j] == 1)t = L"1";
				wcout << L" " << t << L" ";
			}
			//cout << endl<< endl;
			wcout << endl;
		}

		fflush(stdout);
	}

	void destroyGrid() {
		MPI_Finalize();
	}

	void debugArrayImage(vector<vector<int>> v, wstring text = L"", string outputWindow = "image") {

		


		wcout <<endl<< L" " << text << endl;

		int h = v.size();
		int w = v[0].size();

		Mat image(h, w, CV_8UC3, Scalar(0, 0, 0));
		

		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {

				//wstring t = L"▫";
				//if (v[i][j] == 1)t = L"□";
				//wcout << L" "<<t;



				Vec3b color = Vec3b(0, 0, 0);

				//shark =blue, fish = red, water = black

				if (v[i][j] < 0) color = Vec3b(162, 79, 94);
				if (v[i][j]>0)color = Vec3b(5, 105, 216);



				image.at<Vec3b>(Point(j, i)) = color;
			}
			//cout << endl<< endl;
			//wcout << endl;
		}

		//resize(image, image, Size(1000,600));



		namedWindow(outputWindow, WINDOW_AUTOSIZE);
		moveWindow(outputWindow, 100, 100);
		imshow(outputWindow, image);
		//waitKey(100) & 0XFF;
		waitKey(0);
		destroyAllWindows();
		waitKey(1);

		//system("pause");
	}

};