#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv2/core/core.hpp>  
#include<iostream>
#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<opencv2/core/core.hpp>  //OpenCV2鏍囧噯澶存枃浠?   
#include <vector>
#include <cv.h>  
#include <algorithm>
using namespace cv;
using namespace std;
static const float eps = 0.001f;
void Converter(const cv::Mat& src, cv::Mat& dst)
{
	dst = cv::Mat_<Vec3f>(src.rows, src.cols);
	int i, j;
	float rv, gv, bv;
	int ri, gi, bi;
	double hv, sv, iv, minv;
	for (i = 0; i < src.rows; i++)
	{
		const uchar* datIn = src.ptr<const uchar>(i);
		float* datOut = dst.ptr<float>(i);
		for (j = 0; j < src.cols; j++)
		{
			//转化成0.0 ~ 1.0
			ri = *(datIn++);
			gi = *(datIn++);
			bi = *(datIn++);
			rv = ri / 255.0f;
			gv = gi / 255.0f;
			bv = bi / 255.0f;
			minv = min(rv, min(gv, bv));
			iv = (rv + gv + bv) / 3.0f;
			sv = 1.0f - minv / (iv + eps);
			//这里h采用近似算法
			if (minv == rv)
			{
				if ((gv + bv - 2 * rv) <= 0.001)
				{
					hv = 0.0;
				}
				if (rv <= 0.001&&gv <= 0.001&&bv <= 0.001)
				{
					hv = 0;
				}
				else
				{
					hv = (bv - rv) / 3 / (gv + bv - 2 * rv) + 1.0 / 3.0;
				}
			}
			else if (minv == gv)
			{
				if ((rv + bv - 2 * gv) <= 0.001)
				{
					hv = 0.0;
				}
				if (rv <= 0.001&&gv <= 0.001&&bv <= 0.001)
				{
					hv = 0;
				}
				else
				{
					hv = (rv - gv) / 3 / (rv + bv - 2 * gv) + 2.0 / 3.0;
				}
			}
			else
			{
				if ((rv + gv - 2 * bv) <= 0.001)
				{
					hv = 0.0;
				}
				else
				{
					if (rv <= 0.001&&gv <= 0.001&&bv <= 0.001)
					{
						hv = 0;
					}
					hv = (gv - bv) / 3 / (rv + gv - 2 * bv) + 0.0001;
				}
			}



			////cout << "F" << endl;
			//输出
			*(datOut++) = hv * 360;
			*(datOut++) = sv * 255;
			*(datOut++) = iv * 255;
		}
	}
	//cout << hv << " " << sv << " " << iv << endl;
	//cv::imshow("output_converter_hsi", dst);  
	cv::waitKey(5);
}

void Threshold_Ball(const cv::Mat& src, cv::Mat& dst, int max_h, int min_h, int max_s, int min_s, int max_i, int min_i)
{
	/*coun += 1;
	if (end1 - start1 >= 1)
	{
	cout << "fps= " << coun << endl;
	start1 = end1;
	coun = 0;
	}*/
	float hv, sv, iv;
	int i, j;
	for (i = 0; i < src.rows; i++)
	{
		float* datIn = dst.ptr<float>(i);
		float* datOut = dst.ptr<float>(i);
		for (j = 0; j < src.cols; j++)
		{
			hv = *(datIn++);
			sv = *(datIn++);
			iv = *(datIn++);
			int a = src.at<Vec3b>(i, j)[0];
			int b = src.at<Vec3b>(i, j)[1];
			int c = src.at<Vec3b>(i, j)[2];
			if (a == b || b == c)
			{
				hv = 0;
			}
			if (hv >= min_h&&hv <= max_h&&sv >= min_s&&sv <= max_s&& iv >= min_i&&iv <= max_i)
			{
				hv = 1.0;
				sv = 1.0;
				iv = 1.0;
			}
			else if (min_h>max_h && (hv <= min_h || hv >= max_h) && sv >= min_s&&sv <= max_s&& iv >= min_i&&iv <= max_i)
			{
				hv = 1.0;
				sv = 1.0;
				iv = 1.0;
			}

			else
			{
				hv = 0.0;
				sv = 0.0;
				iv = 0.0;

			}
			*(datOut++) = hv;
			*(datOut++) = sv;
			*(datOut++) = iv;
		}
	}
	//cv::imshow("threshold_ball", dst);  
	cv::waitKey(1);
}
int main(int argc, char *argv[])
{
	int rup = 360, rlow = 0, gup = 255, glow = 0, bup = 255, blow = 0;
	Mat imgChange;
	VideoCapture cap(0);//打开默认的摄像头
	if (!cap.isOpened())
	{
		return -1;
	}
	Mat frame; //接收视频输入流 
	cap >> frame; //  或cap>>frame;

	namedWindow("control", 1);      //新建窗口
	imgChange.create(frame.rows, frame.cols, frame.type());
	createTrackbar("R值_上限", "control", &rup, 360, 0);//trackbar name window name  start value  biggest value  thresholed(默认0，自动调用回掉函数）
	createTrackbar("R值_下限", "control", &rlow, 360, 0);
	createTrackbar("G值_上限", "control", &gup, 255, 0);
	createTrackbar("G值_下限", "control", &glow, 255, 0);
	createTrackbar("B值_上限", "control", &bup, 255, 0);
	createTrackbar("B值_下限", "control", &blow, 255, 0);
	int hight = frame.rows;
	int width = frame.cols;
	int count = 0;
	double t;
	bool stop = false;
	while (!stop)
	{
		count += 1;
		double start = (double)cvGetTickCount();
		cap >> frame; //  或cap>>frame;
		int iRows = frame.rows;
		Converter(frame, imgChange);//将frame RGB转换成imgChange GRAY
		Threshold_Ball(frame, imgChange, rup, rlow, gup, glow, bup, blow);
		imshow("Video", imgChange);
		if (waitKey(30) == 27) //Esc键退出
		{
			stop = true;
		}
		double end = (double)cvGetTickCount();
		t = end - start;
		printf("time = %g\n", t / (cvGetTickFrequency() * 1000000));
		if ((t / (cvGetTickFrequency() * 1000000)) >= 1) { printf("fps=%d\n", count); count = 0; }
	}
	return 0;
}