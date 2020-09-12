#include<iostream>
#include<opencv4/opencv2/opencv.hpp>
#include<opencv4/opencv2/highgui.hpp>
using namespace cv;
using namespace std;
int iLow=100;
int iHighH=124;
int iLowS=43;
int iHighS=255;
int iLowV=46;
int iHighV=255;
int main()
{
    /****************先識別出裝甲板兩側亮着的燈條***************/
//讀取視頻
/************無法讀取視頻，暫時用攝像頭取代***********/
    //VideoCapture cap("source.avi");
   VideoCapture cap;
   cap.open(0);
   if(!cap.isOpened())
{
    cout<<"無法播放視頻"<<endl;
    return -1;
   }

    //循環顯示獲取的每一幀圖像
    while(1)
    {
        Mat frame;//定義一個變量用來存儲圖像
        cap>>frame;//將獲取的圖像寫入變量Mat中
        if(frame.empty())
        {
           cout<<"視頻已播放完"<<endl;
            break;
        }
        //轉化到HSV空間
        Mat hsv_image,temp;
        cvtColor(frame,hsv_image,COLOR_BGR2HSV);
        inRange(hsv_image, Scalar(iLow, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV),temp);
        //开操作 
			Mat element = getStructuringElement(MORPH_RECT, Size(7, 7));
			morphologyEx(temp,temp, MORPH_OPEN, element);
			//闭操作 
			morphologyEx(temp,temp,MORPH_CLOSE, element);
        //寻找最外层轮廓  
           Canny(temp, temp, 20, 80, 3, false);
			vector<vector<Point>> contours;
			vector<Vec4i> hierarchy;
			findContours(temp, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);

            //面积太小忽略	
			for (int i = 0; i < contours.size(); i++)
			{
				if (contourArea(contours[i]) < 80)		
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
		imshow("result", frame);
				
        waitKey(30);
    }
   
}
 return 0;
}


