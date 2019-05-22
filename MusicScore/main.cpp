#include "stdafx.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <stdio.h>

#include <stdlib.h>

using namespace std;
using namespace cv;

bool isHead_fill_new(cv::Mat& image, int i, int j, int interval, int Wofhead, int& Hofhead)
{
	int* arr = new int[Wofhead];
	for (int jj=-9; jj<Wofhead-9; jj++)
	{
		int count = 0;
		for (int ii=0; ii<interval; ii++)
		{
			if (image.at<cv::Vec3b>(i+ii, j+jj)[0] == 0)
				count++;
		}
		arr[jj] = count;
	}
	int min = arr[0];
	int max = arr[0];
	for (int k = 0; k < Wofhead; k++)
	{
		if (min > arr[k])
			min = arr[k];
		if (max < arr[k])
			max = arr[k];
	}
	int* arr2 = new int[Wofhead-1];
	for (int k = 0; k < Wofhead - 1; k++)
		arr2[k] = arr[k+1] - arr[k];
	int countL = 0;
	int countR = 0;
	for (int k = 0; k < (Wofhead-1) / 2; k++)
	{
		if (arr2[k] < 0) //작아지는 부분 있으면
			countL++;
	}
	for (int k = Wofhead / 2; k < Wofhead-1; k++)
	{
		if (arr2[k] > 0)
			countR++;
	}
	if (max >= interval - 5 && max - min >= 10  && countL < 2 && countR < 2)
	{
		image.at<cv::Vec3b>(i, j)[0] = 255;
		image.at<cv::Vec3b>(i, j)[1] = 255;
		return true;
	}
	else return false;
}
bool isHead_fill(cv::Mat& image, cv::Mat temp, int i, int j, int interval, int Wofhead, int& Hofhead)
{
	int count = 0;
	
	for (int ii = 0; ii < interval; ii++)
	{
		for (int jj = 0; jj < Wofhead; jj++)
		{
			if (image.at<cv::Vec3b>(i+ii-9, j+jj)[0] == temp.at<cv::Vec3b>(ii, jj)[0])
				count++;
		}
	}
	double MatchRate = (double)count / (double)(interval*Wofhead);
	if (MatchRate > 0.8)
	{
		//Hofhead = 
		image.at<cv::Vec3b>(i, j)[0] = 255;
		image.at<cv::Vec3b>(i, j)[1] = 255;
		return true;
	}
	else return false;
}

bool isHead_empty(cv::Mat& image,cv::Mat temp, int i, int j, int interval, int Wofhead, int& Hofhead)
{
	int count = 0;

	for (int ii = 0; ii < interval; ii++)
	{
		for (int jj = 0; jj < Wofhead; jj++)
		{
			if (image.at<cv::Vec3b>(i + ii - 10, j + jj)[0] == temp.at<cv::Vec3b>(ii, jj)[0])
				count++;
		}
	}
	double MatchRate = (double)count / (double)(interval*Wofhead);
	if (MatchRate > 0.5)
	{
		//Hofhead = 
		image.at<cv::Vec3b>(i, j)[2] = 128;
		return true;
	}
	else return false;
}
bool isRest(cv::Mat& image, int i, int j, int& BeatofRest)
{
	return true;
}


void binarization(Mat& inputImg, Mat& resultImg, uchar threshold) {
	if (inputImg.empty()) {
		cout << "[Error] binarization :: img parameter is empty!" << endl;
		return;
	}

	for (int y = 0; y < inputImg.rows; y++) {
		for (int x = 0; x < inputImg.cols; x++) {
			if (inputImg.at<Vec3b>(y, x)[0] > threshold
				||inputImg.at<Vec3b>(y,x)[1]>threshold
				||inputImg.at<Vec3b>(y,x)[2]>threshold) {
				resultImg.at<Vec3b>(y, x)[0] = 255;
				resultImg.at<Vec3b>(y, x)[1] = 255;
				resultImg.at<Vec3b>(y, x)[2] = 255;
			}
			else {
				resultImg.at<Vec3b>(y, x)[0] = 0;
				resultImg.at<Vec3b>(y, x)[1] = 0;
				resultImg.at<Vec3b>(y, x)[2] = 0;
			}
		}
	}
}


Mat MyErode(const Mat& image, int kSize) {

	Mat result(image.size(), image.type());
	Rect imgRect(0, 0, image.cols, image.rows);
	int uc = image.cols;
	int ur = image.rows;

	for (int j = 0; j < ur; j++) { //모든 행
		for (int i = 0; i < uc; i++) { //모든 열

			Rect tmpRect(i - kSize / 2, j - kSize / 2, kSize, kSize);//마스크
			Rect in = imgRect & tmpRect;
			uchar min = 255;
			for (int k = in.x; k < in.x + in.width; k++) {
				for (int l = in.y; l < in.y + in.height; l++) {
					if (image.at<uchar>(l, k) < min)
						min = image.at<uchar>(l, k);
				}
			}
			result.at<uchar>(j, i) = min;
		}
	}
	return result;
}



Mat MyDilate(const Mat& image) {
	Mat result(image.size(), image.type());
	Rect imgRect(0, 0, image.cols, image.rows);
	int uc = image.cols;
	int ur = image.rows;

	for (int j = 0; j < ur; j++) { //모든 행
		for (int i = 0; i < uc; i++) { //모든 열
			Rect tmpRect(i - 1, j - 1, 3, 3);
			Rect in = imgRect & tmpRect;
			uchar max = 0;

			for (int k = in.x; k < in.x + in.width; k++) {
				for (int l = in.y; l < in.y + in.height; l++) {
					if (image.at<uchar>(l, k) > max)
						max = image.at<uchar>(l, k);
				}
			}
			result.at<uchar>(j, i) = max;
		}
	}
	return result;
}

void RemoveLine(Mat& img_input, vector<int>& lineArr)
{
	int* hist = new int[img_input.rows];
	for (int i = 0; i < img_input.rows; i++)
		hist[i] = 0;
	for (int y = 0; y < img_input.rows; y++)
	{
		for (int x = 0; x < img_input.cols; x++)
		{
			if (img_input.at<Vec3b>(y, x)[0] != 255)
				hist[y] ++;
		}
	}

	for (int i = 0; i < img_input.rows; i++) {
		if (hist[i] > 1000) // 1000은 임계값
		{
			if (i > 0 && hist[i-1])
			{
				//i가 오선높이
				lineArr.push_back(i);
				for (int j = 0; j < img_input.cols; j++)
				{
					//img_input.at<Vec3b>(i, j)[0] = 255;
					//img_input.at<Vec3b>(i, j)[1] = 255;
					img_input.at<Vec3b>(i, j)[2] = 255;
				}
			}
		}
	}

	//img_input = MyErode(img_input, 5);
	//img_input = MyDilate(img_input);

	/////////////////////////////////lineArr에 push된 값 중 겹치는 값 제거
	for (int i = 0; i < lineArr.size();i++)
	{
		int j = i;
		for (; j < lineArr.size();)
		{
			if (lineArr[j + 1] - lineArr[j] == 1)
				j++;
			else break;
		}
		int count = 0;
		if (count < j - i)
		{
			vector<int>::iterator it = lineArr.begin();
			while (count < j - i&&it<lineArr.end()-i)
			{
				it = lineArr.begin();
				it += i;
				lineArr.erase(it); //lineArr[i]를 지우고 뒤에걸 땡겨라
				count++;
				if (count >= j - i||it>=lineArr.end()-i)
					break;
				it = lineArr.begin();
				it += j - 1;
				lineArr.erase(it); //lineArr[j-1]를 지우고 뒤에걸 땡겨라
				count++;
			}
		}
	}

	///////////////////////
	for (int i = 0; i < lineArr.size(); i++)
		cout << lineArr[i] << endl;

	
}

int main()
{
	cv::Mat image;
	vector<int> lineArr;

	image = cv::imread("score.jpg", CV_LOAD_IMAGE_COLOR);

	binarization(image, image, 220);

	RemoveLine(image, lineArr);

	/*
	int interval = 20;//오선사이의간격
	int Wofhead = 26; //대가리길이

	cv::Mat Head_fill = cv::imread("Head_fill.jpg");
	cv::Mat Head_empty = cv::imread("Head_empty.jpg");

	cv::Mat temp_Head_fill(interval, Wofhead, CV_8UC3); //interval에 맞게 확대/축소된 템플릿
	cv::Mat temp_Head_empty(interval, Wofhead, CV_8UC3);


	///////////////////////////////////////////////////////////////////////////////////////////////////////
	double rate_fill = (double)interval / (double)Head_fill.rows;
	for (int i = 0; i < temp_Head_fill.rows; i++)
	{
		for (int j = 0; j < temp_Head_fill.cols; j++)
		{
			temp_Head_fill.at<cv::Vec3b>(i, j)[0] = 255;
			temp_Head_fill.at<cv::Vec3b>(i, j)[1] = 255;
			temp_Head_fill.at<cv::Vec3b>(i, j)[2] = 255;
		}
	}
	for (int i = 0; i <Head_fill.rows; i++)
	{
		for (int j = 0; j < Head_fill.cols; j++)
		{
			if (i*rate_fill<Head_fill.rows && j*rate_fill<temp_Head_fill.cols)
				temp_Head_fill.at<cv::Vec3b>(i*rate_fill, j*rate_fill) = Head_fill.at<cv::Vec3b>(i, j); //확대or 축소해서 값 채웠음.
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	double rate_empty = (double)interval / (double)Head_empty.rows;
	for (int i = 0; i < temp_Head_empty.rows; i++)
	{
		for (int j = 0; j < temp_Head_empty.cols; j++)
		{
			temp_Head_empty.at<cv::Vec3b>(i, j)[0] = 255;
			temp_Head_empty.at<cv::Vec3b>(i, j)[1] = 255;
			temp_Head_empty.at<cv::Vec3b>(i, j)[2] = 255;
		}
	}
	for (int i = 0; i <Head_empty.rows; i++)
	{
		for (int j = 0; j < Head_empty.cols; j++)
		{
			if (i*rate_fill<Head_empty.rows && j*rate_fill<temp_Head_empty.cols)
				temp_Head_empty.at<cv::Vec3b>(i*rate_empty, j*rate_empty) = Head_empty.at<cv::Vec3b>(i, j); //확대or 축소해서 값 채웠음.
		}
	}

	cout << rate_fill << endl;
	cout << rate_empty << endl;

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	for (int j=0; j<image.cols; j++)
	{
		for (int i=0; i<image.rows-interval; i++)
		{
			if (image.at<cv::Vec3b>(i, j)[0] == 0)//검정점 만나면
			{
				if (image.at<cv::Vec3b>(i + 1, j)[0] == 0)
				{
					if (image.at<cv::Vec3b>(i + 2, j)[0] == 0)
					{
						if (image.at<cv::Vec3b>(i + 3, j)[0] == 0)
						{
							int Hofhead = -1;
							int BeatofRest = -1;

							if (isHead_fill_new(image, i, j, interval, Wofhead, Hofhead))
							{
								//detectedNoteR(detectBeat,Hofhead); //박자 찾아서 push;
							}
							/*
							else if (isHead_empty(image,temp_Head_empty, i, j, interval, Wofhead, Hofhead))
							{

							}/*
							else if (isRest(image,i,j,BeatofRest))
							{
							//Emptynote(BeatofRest);
							}
						}
					}
				}
			}
		}
	}*/
	cv::namedWindow("Score_Original", CV_WINDOW_AUTOSIZE);
	cv::imshow("Score_Original", image);
	cv::waitKey(0);
	cv::destroyWindow("Score_Original");
}
