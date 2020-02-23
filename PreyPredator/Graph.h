#pragma once
#include <iostream>
#include <string>
#include <time.h>
#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <iomanip>
#include <sstream>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class Graph {

public:

	int width, height;

public:

	Graph(int width, int height) {

		this->width = width;
		this->height = height;

	}

	void drawGraph(vector<vector<string>> x, vector<vector<float>> y, string title) {

		Mat graphImage = Mat(height, width, CV_8UC3, Scalar(0, 0, 0));

		int graph_Width = 700;

		//draw axis lines
		line(graphImage, Point(150, 45), Point(150, height - 40), CV_RGB(255, 255, 255), 1, CV_AA);
		line(graphImage, Point(150, height - 40), Point(150 + graph_Width, height - 40), CV_RGB(255, 255, 255), 1, CV_AA);

		int fontFace = FONT_HERSHEY_PLAIN;//FONT_HERSHEY_SCRIPT_SIMPLEX;

		double fontScale = 0.7;
		int thickness = 1;

		vector<Point> bulletValues;


		//draw x_axis text
		for (int i = 0; i < x[0].size(); i++) {

			float x_step = graph_Width / (x[0].size() + 1);
			float x_pos = 150 + x_step*(i + 1);

			Point textPos = Point(x_pos - 20, height - 20);

			putText(graphImage, x[0][i], textPos, fontFace, fontScale, Scalar::all(255), thickness, CV_AA);

			bulletValues.push_back(Point(x_pos, 0));
		}

		double fontScaleAxis = 1;
		putText(graphImage, "Grid Size:(Width x Height)", Point(760, height - 50), FONT_HERSHEY_PLAIN, fontScaleAxis, Scalar::all(255), thickness, CV_AA);
		putText(graphImage, "Time to complete: (in seconds)", Point(20, 40), FONT_HERSHEY_PLAIN, fontScaleAxis, Scalar::all(255), thickness, CV_AA);
		putText(graphImage, title, Point(width / 2 - 100, 40), FONT_HERSHEY_PLAIN, fontScaleAxis, Scalar::all(255), thickness, CV_AA);

		float minValue = FLT_MAX;
		float maxValue = -FLT_MAX;

		for (int i = 0; i < y.size();i++) {

			for (int j = 0; j < y[0].size();j++) {

				if (y[i][j] < minValue)minValue = (y[i][j]);
				if (y[i][j] > maxValue)maxValue = (y[i][j]);
			}
		}

		vector<vector<float>> normValues;
		float newMaxValue = height - 100;
		float newMinValue = 100;
		for (int i = 0; i < y.size(); i++) {

			vector<float> pushValues;
			for (int j = 0; j < y[0].size(); j++) {

				float normV = (newMaxValue - newMinValue)-(y[i][j] - minValue)*(newMaxValue - newMinValue) / (maxValue-minValue) + newMinValue;
				pushValues.push_back(normV);
			}
			normValues.push_back(pushValues);
		}
		
		vector<Scalar> lineColors = { CV_RGB(178,34,34),CV_RGB(227,105,5),CV_RGB(238,210,20) ,CV_RGB(255,255,255) };
		vector<vector<Point>> bullets;
		for (int j = 0; j < y.size(); j++) {
			for (int i = 0; i < normValues[0].size(); i++) {

				Point textPos = Point(90-j*28, (int)normValues[j][i]);

				double y_value = y[j][i];
				stringstream stream;
				stream << fixed << setprecision(3) << y_value;
				string s = stream.str();

				putText(graphImage, s, textPos, fontFace, fontScale, lineColors[j], thickness, CV_AA);

				bulletValues[i].y = textPos.y;
				
			}
			bullets.push_back(bulletValues);
		}

		int radius = 3;
		int bulletThickness = 1;
		for (int j = 0; j < bullets.size();j++) {
			for (int i = 0; i < bulletValues.size(); i++) {


				circle(graphImage, bullets[j][i], radius, CV_RGB(255, 255, 255), bulletThickness);
			}
		}
		
		//draw lines between bullets
		for (int j = 0; j < bullets.size(); j++) {
			for (int i = 0; i < bulletValues.size() - 1; i++) {

				Point p1 = bullets[j][i];
				Point p2 = bullets[j][i + 1];

				line(graphImage, p1, p2, lineColors[j], 1, CV_AA);

			}
		}

		putText(graphImage, "red = 2 processors", Point(760, 20), FONT_HERSHEY_PLAIN, fontScaleAxis, lineColors[0], thickness, CV_AA);
		putText(graphImage, "orange = 3 processors", Point(760, 40), FONT_HERSHEY_PLAIN, fontScaleAxis, lineColors[1], thickness, CV_AA);
		putText(graphImage, "yellow = 4 processors", Point(760, 60), FONT_HERSHEY_PLAIN, fontScaleAxis, lineColors[2], thickness, CV_AA);
		putText(graphImage, "white = 8 processors", Point(760, 80), FONT_HERSHEY_PLAIN, fontScaleAxis, lineColors[3], thickness, CV_AA);

		//draw image
		namedWindow("performanceAnalysis", WINDOW_AUTOSIZE);
		imshow("performanceAnalysis", graphImage);
		//waitKey(100) & 0XFF;
		waitKey(0);
		destroyAllWindows();
		waitKey(1);
	}

	void drawGraph(string x1, string x2, string x3 ,float y1, float y2, float y3, string title) {
		Mat graphImage = Mat(height, width, CV_8UC3, Scalar(0, 0, 0));

		int graph_Width = 700;

		//draw axis lines
		line(graphImage, Point(150, 45), Point(150, height-40), CV_RGB(255, 255, 255),1,CV_AA);
		line(graphImage, Point(150, height-40), Point(150+graph_Width, height - 40), CV_RGB(255, 255, 255),1,CV_AA);

		vector<string> x_axis;
		vector<float>  y_axis;

		x_axis.push_back(x1);x_axis.push_back(x2);x_axis.push_back(x3);
		y_axis.push_back(y1); y_axis.push_back(y2); y_axis.push_back(y3);
		
		int fontFace = FONT_HERSHEY_PLAIN;//FONT_HERSHEY_SCRIPT_SIMPLEX;
		
		double fontScale = 0.7;
		int thickness = 1;
		
		vector<Point> bulletValues;
		

		//draw x_axis text
		for (int i = 0; i < x_axis.size();i++) {

			float x_step = graph_Width / (x_axis.size()+1);
			float x_pos = 150 + x_step*(i+1);

			Point textPos = Point(x_pos-20,height-20);

			putText(graphImage, x_axis[i], textPos, fontFace, fontScale,Scalar::all(255), thickness, CV_AA);

			bulletValues.push_back(Point(x_pos,0));
		}

		double fontScaleAxis = 1;
		putText(graphImage, "Grid Size:(Width x Height)", Point(760, height - 50), FONT_HERSHEY_PLAIN, fontScaleAxis, Scalar::all(255), thickness, CV_AA);
		putText(graphImage, "Time to complete: (in seconds)", Point(20, 40), FONT_HERSHEY_PLAIN, fontScaleAxis, Scalar::all(255), thickness, CV_AA);
		putText(graphImage, title, Point(width/2-100, 40), FONT_HERSHEY_PLAIN, fontScaleAxis, Scalar::all(255), thickness, CV_AA);

		float minValue = FLT_MAX;
		float maxValue = -FLT_MAX;
		//normalize values in range (0,graph height)
		for (int i = 0; i < y_axis.size();i++) {

			float y_value = y_axis[i];

			if (y_value > maxValue)maxValue = y_value;
			if (y_value < minValue)minValue = y_value;
		}
		vector<float> normValues;
		float newMaxValue = height - 100;
		float newMinValue = 100;
		for (int i = 0; i < y_axis.size(); i++) {


			float normValue = (newMaxValue-newMinValue)-(y_axis[i] - minValue)*(newMaxValue - newMinValue) / (maxValue - minValue) + newMinValue;
			normValues.push_back(normValue);
		}

		for (int i = 0; i < normValues.size();i++) {

			Point textPos = Point(60,(int)normValues[i]);

			double y_value = y_axis[i];
			stringstream stream;
			stream << fixed << setprecision(3) << y_value;
			string s = stream.str();

			putText(graphImage, s, textPos, fontFace, fontScale, Scalar::all(255), thickness, CV_AA);

			bulletValues[i].y = textPos.y;
		}

		int radius = 3;
		int bulletThickness = 1;
		for (int i = 0; i < bulletValues.size();i++) {


			circle(graphImage, bulletValues[i], radius, CV_RGB(255, 255, 255), bulletThickness);
		}

		//draw lines between bullets
		for (int i = 0; i < bulletValues.size()-1; i++) {

			Point p1 = bulletValues[i];
			Point p2 = bulletValues[i+1];

			line(graphImage, p1, p2, CV_RGB(100, 100, 100), 1, CV_AA);

		}


		namedWindow("performanceAnalysis", WINDOW_AUTOSIZE);
		imshow("performanceAnalysis", graphImage);
		//waitKey(100) & 0XFF;
		waitKey(0);
		destroyAllWindows();
		waitKey(1);
	}
};