#include <iostream>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <stdio.h>
#include <time.h>
#include <cmath>
using namespace cv;
using namespace std;

//限定顏色範圍的參數
int iLow = 80;
int iHighH = 124;
int iLowS = 43;
int iHighS = 255;
int iLowV = 46;
int iHighV = 255;

Point2f center;																			 //裝甲板的中心點
Point2f predict_center;																	 //預測的中心點
double time1 = 0, time2 = 0;															 //兩幀間隔時間
Point2f pre_center1, pre_center2;														 //存儲前兩幀的裝甲板中心點
Point2f predict(Point2f pre1, Point2f pre2, Point2f center, double time1, double time2); //用來預測中心點移動位置的函數

int main()
{
	/****************先識別出裝甲板兩側亮着的燈條***************/
	//讀取視頻
	VideoCapture cap("/home/yevette/文档/opencv_program/test/预测效果.avi");
	if (!cap.isOpened())
	{
		cout << "無法播放視頻" << endl;
		return -1;
	}
	//  int i;
	//循環顯示獲取的每一幀圖像
	while (1)
	{
		clock_t start = clock();
		//	i++;
		Mat frame;	  //定義一個變量用來存儲圖像
		cap >> frame; //將獲取的圖像寫入變量Mat中
		if (frame.empty())
		{
			cout << "視頻已播放完" << endl;
			break;
		}

		//轉化到HSV空間
		Mat hsv_image, temp;
		cvtColor(frame, hsv_image, COLOR_BGR2HSV);
		inRange(hsv_image, Scalar(iLow, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), temp);
		//开操作
		Mat element = getStructuringElement(MORPH_RECT, Size(7, 7));
		morphologyEx(temp, temp, MORPH_OPEN, element);
		//闭操作
		morphologyEx(temp, temp, MORPH_CLOSE, element);
		//寻找最外层轮廓
		Canny(temp, temp, 20, 80, 3, false);
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;
		findContours(temp, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);
		//面积太小忽略
		for (int i = 0; i < contours.size(); i++)
		{
			if (contourArea(contours[i]) < 10)
				continue;
			//寻找最小外接矩形
			RotatedRect rect = minAreaRect(contours[i]);
			Mat background = Mat::zeros(frame.size(), CV_8UC1); //創建畫布
			for (int i = 0; i < contours.size(); i++)
			{
				//绘制轮廓
				drawContours(background, contours, i, Scalar(255), 1, 8, hierarchy);

				//畫出最小外接矩形
				RotatedRect rect = minAreaRect(contours[i]);
				Point2f P[4];
				rect.points(P);
				for (int j = 0; j <= 3; j++)
				{
					line(frame, P[j], P[(j + 1) % 4], Scalar(0, 0, 255), 1);
					line(background, P[j], P[(j + 1) % 4], Scalar(111), 2);
				}
			}
		}
		/**************************對裝甲板的位置進行預測**********************/
		//计算轮廓矩
		vector<Moments> mu(contours.size());
		for (int i = 0; i < contours.size(); i++)
		{
			mu[i] = moments(contours[i], false);
		}
		//计算轮廓中心
		vector<Point2i> mc(contours.size());
		for (int i = 0; i < contours.size(); i++)
		{
			mc[i] = Point2i(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
		}
		//通過左右燈柱的重心坐標得到裝甲板重心的位置
		center.x = (mc[0].x + mc[1].x) / 2;
		center.y = (mc[0].y + mc[1].y) / 2;
		circle(frame, center, 4, Scalar(0, 255, 225), 2);

		clock_t end = clock(); //計時器結束
		time1 = time2;
		time1 = end - start;
		time2 = end - start;
		//調用預測函數，返回預測點
		predict_center = predict(pre_center1, pre_center2, center, time1, time2);
		circle(frame, predict_center, 5, Scalar(0, 0, 225), 4);
		imshow("result", frame);
		//更新三個數據
		pre_center1 = pre_center2;
		pre_center1 = center;
		pre_center2 = center;

		waitKey(30);
	}
	return 0;
}

Point2f predict(Point2f pre1, Point2f pre2, Point2f center, double time1, double time2)
{
	Point2f Predict;
	double accelerate_x, accelerate_y;			   //加速度
	double speed1_x, speed2_x, speed1_y, speed2_y; //速度
	speed1_x = ((double)pre_center2.x - (double)pre_center1.x) * 10 / time1;
	speed2_x = ((double)center.x - (double)pre_center2.x) * 10 / time2;
	speed1_y = ((double)pre_center2.y - (double)pre_center1.y) * 10 / time1;
	speed2_y = ((double)center.y - (double)pre_center2.y) * 10 / time2;
	accelerate_x = (speed2_x - speed1_x) / time2;
	accelerate_y = (speed2_y - speed1_y) / time2;
	//計算預測點的坐標
	Predict.x = center.x + (speed2_x * time2 + (1 / 2) * accelerate_x * pow(time2, 2));
	Predict.y = center.y + (speed2_y * time2 + (1 / 2) * accelerate_x * pow(time2, 2));
	//返回預測點
	return Predict;
}