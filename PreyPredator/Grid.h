#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <time.h>
#include <Windows.h>
#include <omp.h>
#include <io.h>
#include <fcntl.h>

#include <opencv2/opencv.hpp>
//#include <Random123\philox.h>

using namespace std;
using namespace cv;

class Grid {

public:
	int width, height;

	vector<vector<int>> currentWorld;
	vector<vector<int>> nextWorld;

	vector<vector<int>> currentArray;

	vector<vector<bool>> randomCauses;
	
public:
	
	Grid(int width,int height) {

		//srand(time(NULL));
		//srand(0);
		this->width = width;
		this->height = height;

		for (int i = 0; i < height; i++) {


			vector<int> row;
			vector<bool> brow;
			for (int j = 0; j < width; j++) {

				row.push_back(randomValue(0, 1));
				//wcout << randomValue(0, 1) << endl;
				brow.push_back(false);
			}

			currentWorld.push_back(row);
			nextWorld.push_back(row); 
			randomCauses.push_back(brow);
		}

		
		debugArray(currentWorld, L"My world","Initial Data:: Sharks: -1, Water: 0, Fish: 1");
		calculateGenerations();

	}

	float randomRange(float a, float b) {
		
		float random = ((float)rand()) / (float)RAND_MAX;
		float diff = b - a;
		float r = random * diff;
		return a + r;
	}

	inline int randomRange(int a, int b) {
		return (rand() % (b - a + 1) + a);
	}

	int randomValue(float a,float b) {

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
		int n = randomRange(1, 33);

		if (n < 2)return true;
		return false;
	}

	void calculateGenerations() {
		int numOfSteps = 100;

		clock_t begin = clock();
		
		for (int stepI = 0; stepI < numOfSteps; stepI++) {

			
			evolveParallelNested();
			//evolve();
			

			//debugArray(currentWorld, L"World", "output");
			//if ((stepI % 100) == 0)debugArray(currentWorld, L"World", "output: " + to_string(stepI) + " th iteration");
			//system("pause");
		}
		

		clock_t end = clock();

		double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

		wcout << L"Time to complete: " << elapsed_secs << endl;

		debugArray(currentWorld, L"World", "output_ fish: orange, shark: blue, water: black");
	}

	void evolve() {

	
		initNextWorld();
		
		for(int i=0;i<height;i++){
			for (int j = 0; j < width;j++) {

				int numNeighbors = 0;
				int numValidBreedingFish = 0;
				int numValidBreedingShark = 0;
				int numNFish = 0;
				int numNShark = 0;

				int nI[] = { i,i - 1,i + 1,i,i - 1,i + 1,i - 1,i + 1 };
				int nJ[] = { j - 1,j - 1,j - 1,j + 1,j + 1,j + 1,j,j };

				for (int k = 0; k < 8; k++) {

					int value = getNMod(nI, nJ, k);

					//wcout << value << endl;

					if (value > 0)numNFish++;
					if (value < 0)numNShark++;

					if (value >= 2)numValidBreedingFish++;
					if (value <= -3)numValidBreedingShark++;
					//numNeighbors += value;

				}

				//wcout << L"NumFish: " << numNFish << L"| NumShark: " << numNShark<<endl;

				//rules of life 
				if (currentWorld[i][j] == 0) {
					//empty cell - breeding stage
					nextWorld[i][j] = 0;
					if (numNFish >= 4 && numValidBreedingFish >= 3 && numNFish > numNShark) {
						nextWorld[i][j] = 1;
						//continue;
					}
					else if (numNShark >= 4 && numValidBreedingShark >= 3 && numNFish < numNShark) {
						nextWorld[i][j] = -1;
						//continue;
					}

					//continue;
				}
				else {
					//alive cell - health state

					if (currentWorld[i][j] > 0) {
						//fish
						if (currentWorld[i][j] == 10) {
							nextWorld[i][j] = 0;
							//continue;
						}
						else if (numNShark >= 5) {
							nextWorld[i][j] = 0;
							//continue;
						}
						else if (numNFish == 8) {
							nextWorld[i][j] = 0;
							//continue;
						}
						//if a fish didnt die, increment age by 1
						else nextWorld[i][j]++;
					}
					else if (currentWorld[i][j] < 0) {
						//shark
						if (currentWorld[i][j] == -20) {
							nextWorld[i][j] = 0;
							//continue;
						}
						else if (numNShark >= 6 && numNFish == 0) {
							//starvation
							nextWorld[i][j] = 0;
							//continue;
						}
						//random death cause
						else if (shouldSharkDie(i*width+j)) {
							nextWorld[i][j] = 0;
							//continue;
						}

						//if a shark didnt die, increment age by 1
						else nextWorld[i][j]--;
					}

				}
				
			}
		}

		copyBackToCurrent();
	}

	void evolveParallel() {
		initNextWorldParallel();

		//int numValidBreedingFish = 0;
		//int numValidBreedingShark = 0;
		//int numNFish = 0;
		//int numNShark = 0;

		#pragma omp parallel for num_threads(4) schedule(guided)//reduction(+:numNFish,numNShark,numValidBreedingFish,numValidBreedingShark)
		for (int k = 0; k < height*width; k++) {
			int numNeighbors = 0;

			int numValidBreedingFish = 0;
			int numValidBreedingShark = 0;
			int numNFish = 0;
			int numNShark = 0;

			int i = k / width;
			int j = k - width * i;

			int nI[] = { i,i - 1,i + 1,i,i - 1,i + 1,i - 1,i + 1 };
			int nJ[] = { j - 1,j - 1,j - 1,j + 1,j + 1,j + 1,j,j };

			//#pragma omp parallel for reduction(+:numNFish,numNShark,numValidBreedingFish,numValidBreedingShark)
			for (int l = 0; l < 8; l++) {

				int value = getNMod(nI, nJ, l);

				if (value > 0)numNFish++;
				if (value < 0)numNShark++;

				if (value >= 2)numValidBreedingFish++;
				if (value <= -3)numValidBreedingShark++;
				
				//numNeighbors += getNMod(nI, nJ, l);
			}

			//rules of life 
			if (currentWorld[i][j] == 0) {
				//empty cell - breeding stage
				nextWorld[i][j] = 0;
				if (numNFish >= 4 && numValidBreedingFish >= 3 && numNFish > numNShark) {
					nextWorld[i][j] = 1;
					//continue;
				}
				else if (numNShark >= 4 && numValidBreedingShark >= 3 && numNFish < numNShark) {
					nextWorld[i][j] = -1;
					//continue;
				}

				//continue;
			}
			else {
				//alive cell - health state

				if (currentWorld[i][j] > 0) {
					//fish
					if (currentWorld[i][j] == 10) {
						nextWorld[i][j] = 0;
						//continue;
					}
					else if (numNShark >= 5) {
						nextWorld[i][j] = 0;
						//continue;
					}
					else if (numNFish == 8) {
						nextWorld[i][j] = 0;
						//continue;
					}
					//if a fish didnt die, increment age by 1
					else nextWorld[i][j]++;
				}
				else if (currentWorld[i][j] < 0) {
					//shark
					if (currentWorld[i][j] == -20) {
						nextWorld[i][j] = 0;
						//continue;
					}
					else if (numNShark >= 6 && numNFish == 0) {
						//starvation
						nextWorld[i][j] = 0;
						//continue;
					}
					//random death cause
					else if (shouldSharkDie(i*width + j)) {
						nextWorld[i][j] = 0;
						//continue;
					}

					//if a shark didnt die, increment age by 1
					else nextWorld[i][j]--;
				}

			}

			
		}

	}
	
	void evolveParallelNested() {
		initNextWorldParallel();
		
		//initNextWorld()

		//#pragma omp parallel for schedule(dynamic,8)
		#pragma omp parallel for schedule(guided,4)
		//#pragma omp parallel for num_threads(4) schedule(guided)
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				
				int numNeighbors = 0;

				int numValidBreedingFish = 0;
				int numValidBreedingShark = 0;
			    int numNFish = 0;
				int numNShark = 0;


				int nI[] = { i,i - 1,i + 1,i,i - 1,i + 1,i - 1,i + 1 };
				int nJ[] = { j - 1,j - 1,j - 1,j + 1,j + 1,j + 1,j,j };

				//#pragma omp parallel for reduction(+:numNFish,numNShark,numValidBreedingFish,numValidBreedingShark)
				//#pragma omp parallel for
			    //#pragma omp parallel for reduction(+: numNFish,numNShark,numValidBreedingFish,numValidBreedingShark)
				for (int l = 0; l < 8; l++) {

					int value = getNMod(nI, nJ, l);

					if (value > 0)numNFish+=1;
					if (value < 0)numNShark+=1;

					if (value >= 2)numValidBreedingFish+=1;
					if (value <= -3)numValidBreedingShark+=1;

					//numNeighbors += getNMod(nI, nJ, l);
				}

				//rules of life 
				if (currentWorld[i][j] == 0) {
					//empty cell - breeding stage
					nextWorld[i][j] = 0;
					if (numNFish >= 4 && numValidBreedingFish >= 3 && numNFish > numNShark) {
						nextWorld[i][j] = 1;
						//continue;
					}
					else if (numNShark >= 4 && numValidBreedingShark >= 3 && numNFish < numNShark) {
						nextWorld[i][j] = -1;
						//continue;
					}
					
					//continue;
				}
				else {
					//alive cell - health state

					if (currentWorld[i][j] > 0) {
						//fish
						if (currentWorld[i][j] == 10) {
							nextWorld[i][j] = 0;
							//continue;
						}
						else if (numNShark >= 5) {
							nextWorld[i][j] = 0;
							//continue;
						}
						else if (numNFish == 8) {
							nextWorld[i][j] = 0;
							//continue;
						}
						//if a fish didnt die, increment age by 1
						else nextWorld[i][j]++;
					}
					else if (currentWorld[i][j] < 0) {
						//shark
						if (currentWorld[i][j] == -20) {
							nextWorld[i][j] = 0;
							//continue;
						}
						else if (numNShark >= 6 && numNFish == 0) {
							//starvation
							nextWorld[i][j] = 0;
							//continue;
						}
						//random death cause
						else if (shouldSharkDie(i*width + j)) {
							nextWorld[i][j] = 0;
							//continue;
						}

						//if a shark didnt die, increment age by 1
						else nextWorld[i][j]--;
					}

				}

				
			}
		}
		copyBackToCurrentParallel();
	}

	void calcRandomCauses() {
		
		#pragma omp parallel for 
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {

				randomCauses[i][j] = shouldSharkDie(i*width + j);
			}
		}
	}

	int getNMod(int (&nI)[8], int (&nJ)[8],int n) {

		//wcout << L"n: " << n<< L" ( " << nI[n]<< L","<< nJ[n]<<L") "<< endl;
		
		int inI = nI[n];
		int inJ = nJ[n];

		int mI = 0;
		int mJ = 0;

		if (inI > 0) {
			//mI = inI % (height);
			mI = reduce(inI,height);
		}
		else {
			inI = height + inI;
			//mI = inI % (height);
			mI = reduce(inI, height);
		}

		if (inJ > 0) {
			//mJ = inJ % (width);
			mJ = reduce(inJ,width);
		}
		else {
			inJ = width + inJ;
			//mJ = inJ % (width);
			mJ = reduce(inJ, width);
		}

		//wcout << L"index: " << mI << L"," << mJ << endl;
		//wcout << L"Previously: " << inI << L"," << inJ << endl;

		//cout << "mI: " << mI << "| mJ: " << mJ << endl;
		return currentWorld[mI][mJ];
	}

	 inline int reduce(const int input, const int ceil) {

		//https://www.youtube.com/watch?v=nXaxk27zwlk&feature=youtu.be&t=56m34s
		// apply the modulo operator only when needed
		// (i.e. when the input is greater than the ceiling)
		return input >= ceil ? input % ceil : input;
	    //return input % ceil;
		// NB: the assumption here is that the numbers are positive
	}


	 void copyBackToCurrent() {

		 for (int i = 0; i < height; i++) {
			 for (int j = 0; j < width; j++) {
				 currentWorld[i][j] = nextWorld[i][j];
			 }
		 }

		 //currentWorld.swap(nextWorld);

	 }

	 inline void copyBackToCurrentParallel() {

		 #pragma omp parallel for schedule(guided,4)
		 for (int i = 0; i < height; i++) {
			 for (int j = 0; j < width; j++) {
				 currentWorld[i][j] = nextWorld[i][j];
			 }
		 }

		 //currentWorld.swap(nextWorld);

	 }


	 void initNextWorld() {


		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				nextWorld[i][j] = currentWorld[i][j];
			}
		}

		//nextWorld.swap(currentWorld);

		
	}

	 inline void initNextWorldParallel() {

		#pragma omp parallel for schedule(guided,4)
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				nextWorld[i][j] = currentWorld[i][j];
			}
		}
		//currentWorld.swap(nextWorld);
		//nextWorld.swap(currentWorld);

		
		
	}

	void debugArray(vector<vector<int>> v, wstring text = L"", string outputWindow="image") {

		Mat image(height, width, CV_8UC3,Scalar(0,0,0));
		

		wcout << text;

		int h = v.size();
		int w = v[0].size();
		wcout << endl;

		for (int i = 0; i < h;i++) {
			for (int j = 0; j < w; j++) {

				//wstring t = L"▫";
				//if (v[i][j] == 1)t = L"□";
				//wcout << L" "<<t;



				Vec3b color = Vec3b(0,0,0);

				//shark =blue, fish = red, water = black

				if (v[i][j] < 0) color = Vec3b(162,79, 94);
				if (v[i][j]>0)color = Vec3b(5,105, 216);

				

				image.at<Vec3b>(Point(j, i)) = color;
			}
			//cout << endl<< endl;
			//wcout << endl;
		}

		//resize(image, image, Size(1000,600));
	
		

		namedWindow(outputWindow,WINDOW_AUTOSIZE);
		imshow(outputWindow,image);
		//waitKey(100) & 0XFF;
		waitKey(0);
		destroyAllWindows();
		waitKey(1);
	}

};