// OpenCVtest.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include"opencv2\opencv.hpp"
#include"opencv2\xfeatures2d.hpp"
#include"opencv2\ml.hpp"
#include < iostream > 
#include < string > 
#include <typeinfo>
#include <fstream>
#include <sstream>
#include <cmath>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>

using namespace cv;
using namespace std;

namespace KongYuanhang{
	enum GRADIENT_TYPE{ SOBEL = 0, PREWITT, ROBERTS, LAPLACIAN };
}

//*******ͼ�����γ�
//**��������ȡ
int harrisDetector(std::string path, std::vector<cv::Point3f> &featurePointLeft)
{
	cv::Mat imageRGB = cv::imread(path, cv::IMREAD_COLOR);
	if (imageRGB.empty())
	{
		std::cout << "Fail to read image:" << path << std::endl;
		return -1;
	}
	cv::imshow("Image", imageRGB);
	cv::waitKey(0);
	//creat grey-scale image for computation
	cv::Mat imageGray;
	cv::cvtColor(imageRGB, imageGray, cv::COLOR_RGB2GRAY);
	//creat result matrix
	cv::Mat result/*32-bit float*/, resultNorm/*0-255 32-bit float*/, resultNormUInt8/*0-255 uisigned char*/;
	result = cv::Mat::zeros(imageGray.size(), CV_32FC1);
	//Define harris detector
	int blockSize = 2;//size of neighbor window 2*blocksize+1
	int apertureSize = 1;//sobel window
	double k = 0.04;//harris responding cofficient-- 32-bit float
	cv::cornerHarris(imageGray, result, blockSize, apertureSize, k, cv::BORDER_DEFAULT);
	//Normalizing image to 0-255
	cv::normalize(result, resultNorm, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());
	cv::convertScaleAbs(resultNorm, resultNormUInt8);
	//drawing circles around corners
	bool bMarkCorners = true;
	if (bMarkCorners)
	{
		double thHarrisRes = 125;
		int radius = 5;
		for (int j = 0; j < resultNorm.rows; j++)
		{
			for (int i = 0; i < resultNorm.cols;i++)
			{
				if ((int)resultNorm.at<float>(j, i) > thHarrisRes)
				{
					cv::circle(resultNormUInt8, cv::Point(i, j), radius, cv::Scalar(255), 1, 8, 0);
					cv::circle(imageRGB, cv::Point(i, j), radius, cv::Scalar(0, 255, 255), 1, 4, 0);

					cv::line(imageRGB, cv::Point(i - radius - 2, j), cv::Point(i + radius + 2, j), cv::Scalar(0, 255, 255), 1, 8, 0);
					cv::line(imageRGB, cv::Point(i, j - radius - 2), cv::Point(i, j + radius + 2), cv::Scalar(0, 255, 255), 1, 8, 0);
					cv::Point3f tempP;
					tempP.x = j;
					tempP.y = i;
					tempP.z = resultNorm.at<float>(j, i);
					featurePointLeft.push_back(tempP);
				}
			}
			
		}
	}
	cv::imshow("Harris result", resultNormUInt8);
	cv::imshow("Image", imageRGB);

	cv::waitKey(0);

	return 0;
}
int siftDetector(std::string path)
{
	//image is created for computation; imageRGB is created to show;
	cv::Mat  image, imageRGB = cv::imread(path, cv::IMREAD_COLOR);
	if (imageRGB.empty()) {
		std::cout << "Fail to read the image: " << path << std::endl;
		return -1;
	}

	cv::cvtColor(imageRGB, image, cv::COLOR_RGB2GRAY);
	//cv::KeyPoint -- SIFT corner, SURF, FAST, ORB...
	std::vector<cv::KeyPoint> keypoints; //(x,y,scale, angle)

	float thContrast = 0.04; // С�ڴ���ֵ�������㱻�����ԱȶȲ���ĵ��˳�
	float thEdge = 10;		//���ڴ���ֵ�������㱻������Ե���˳�

	cv::Ptr<cv::Feature2D> f2d = cv::xfeatures2d::SIFT::create(0, 3, thContrast, thEdge, 1.6);
	//cv::Ptr<Feature2D> f2d = cv::xfeatures2d::SURF::create(10000);

	f2d->detect(image, keypoints);

	//drawing is not used in matching
	cv::drawKeypoints(imageRGB, keypoints, imageRGB, cv::Scalar(255, 255, 0), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);//DEFAULT

	cv::imshow("2D Features", imageRGB);
	cv::waitKey(0);
	return 0;
}
int siftMatch(std::string pathLeft, std::string pathRight)
{
	//image is created for computation; imageRGB is created to show;
	cv::Mat  imageLeft, imageLeftRGB = cv::imread(pathLeft, cv::IMREAD_COLOR);
	cv::Mat  imageRight, imageRightRGB = cv::imread(pathRight, cv::IMREAD_COLOR);
	if (imageLeftRGB.empty()) {
		std::cout << "Fail to read the image: " << pathLeft << std::endl;
		return -1;
	}
	if (imageRightRGB.empty()) {
		std::cout << "Fail to read the image: " << pathRight << std::endl;
		return -1;
	}
	cv::imshow("imageLeft",imageLeftRGB);
	cv::imshow("imageRight", imageRightRGB);
	cv::waitKey(0);

	cv::cvtColor(imageLeftRGB, imageLeft, cv::COLOR_RGB2GRAY);
	cv::cvtColor(imageRightRGB, imageRight, cv::COLOR_RGB2GRAY);

	std::vector<cv::KeyPoint> keypointLeft, keypointRight;

	float thContrast = 0.05; // С�ڴ���ֵ�������㱻�����ԱȶȲ���ĵ��˳�
	float thEdge = 10;		//���ڴ���ֵ�������㱻������Ե���˳�

	cv::Ptr<cv::Feature2D> f2d = cv::xfeatures2d::SIFT::create(0, 3, thContrast, thEdge, 1.6);

	f2d->detect(imageLeft, keypointLeft);
	f2d->detect(imageRight, keypointRight);

	cv::drawKeypoints(imageLeftRGB, keypointLeft, imageLeftRGB, cv::Scalar(255, 255, 0), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);//DEFAULT
	cv::drawKeypoints(imageRightRGB, keypointRight, imageRightRGB, cv::Scalar(255, 255, 0), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);//DEFAULT
	
	std::cout << std::endl;
	std::cout << "Feature Points Number In left:" << keypointLeft.size() << std::endl;
	std::cout << "Feature Points Number In Right:" << keypointRight.size() << std::endl;

	cv::namedWindow("LeftFeatures", 1);
	cv::namedWindow("RightFeatures", 1);
	cv::imshow("LeftFeatures", imageLeftRGB);
	cv::imshow("RightFeatures", imageRightRGB);
	cv::waitKey(0);

	cv::Mat descriptorsLeft, descriptorsRight;
	f2d->compute(imageLeft, keypointLeft, descriptorsLeft);
	f2d->compute(imageRight, keypointRight, descriptorsRight);
	//matching of the feature vector 
	cv::FlannBasedMatcher matcher;//Fast Library for Approximate Nearest Neighbors
	std::vector<cv::DMatch> matches;
	matcher.match(descriptorsLeft, descriptorsRight, matches);
	//draw the result
	cv::Mat rawMatcheImage;
	cv::drawMatches(imageLeft, keypointLeft, imageRight, keypointRight, matches, rawMatcheImage);
	cv::namedWindow("Row Matches", 1);
	cv::imshow("Raw Matches", rawMatcheImage);
	cv::waitKey(0);
   //remove the bad matches, compute the eulide distance of two matches 
	double maxDistance, minDistance;

	for (int i = 0; i < matches.size(); i++)
	{
		if (i == 0)
		{
			maxDistance = matches[i].distance;
			minDistance = matches[i].distance;
		}
		else
		{
			double dist = matches[i].distance;
			if (dist < minDistance) minDistance = dist;
			if (dist > maxDistance) maxDistance = dist;
		}
	}

	std::cout << std::endl;
	std::cout << "---The worst match distance:" << maxDistance << std::endl;
	std::cout << "---The best match distance:" << minDistance << std::endl;

	//draw the good matches, distance < 3*min_dist
	std::vector<cv::DMatch> goodMatches;

	for (int i = 0; i < matches.size(); i++)
	{
		if (matches[i].distance < 1.5 * minDistance)
			goodMatches.push_back(matches[i]);
	}

	std::cout << std::endl << "Number of good matches: " << goodMatches.size() << std::endl;

	cv::Mat goodMatcheImage;
	cv::drawMatches(imageLeft, keypointLeft, imageRight, keypointRight, goodMatches, goodMatcheImage, cv::Scalar::all(-1), cv::Scalar::all(-1),
		std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	cv::namedWindow("Good Matches", 1);
	cv::imshow("Good Matches", goodMatcheImage);
	cv::waitKey(0);

	if (goodMatches.size() < 8)
	{
		std::cout << "There are no enough good matches! HOMOGRAPHY MATRIX cannot be found!" << std::endl;
		return -1;
	}

	//==========compute the homography matrix and locate the corresponding area of left image==============
	std::vector<cv::Point2f> corrPointsLeft;
	std::vector<cv::Point2f> corrPointsRight;

	for (int i = 0; i < goodMatches.size(); i++)
	{
		corrPointsLeft.push_back(keypointLeft[goodMatches[i].queryIdx].pt);
		corrPointsRight.push_back(keypointRight[goodMatches[i].trainIdx].pt);
	}

	//find the homography matrix using RANSAC, only works for flann transformation
	cv::Mat perspectiveMat = findHomography(corrPointsLeft, corrPointsRight, cv::RANSAC);// CV_RANSAC);

	//===========draw the final result ======================================
	//The corners of the left image
	std::vector<cv::Point2f> cornersLeft(4);
	cornersLeft[0] = cv::Point2f(0, 0);
	cornersLeft[1] = cv::Point2f(imageLeft.cols, 0);
	cornersLeft[2] = cv::Point2f(imageLeft.cols, imageLeft.rows);
	cornersLeft[3] = cv::Point2f(0, imageLeft.rows);

	//Compute the corresponding corners of the left image in the right one
	std::vector<cv::Point2f> cornersLeftInRight(4);
	cv::perspectiveTransform(cornersLeft, cornersLeftInRight, perspectiveMat);

	//draw object
	cv::line(goodMatcheImage, cornersLeftInRight[0] + cv::Point2f(imageLeft.cols, 0), cornersLeftInRight[1] + cv::Point2f(imageLeft.cols, 0), cv::Scalar(0, 255, 0), 2);
	cv::line(goodMatcheImage, cornersLeftInRight[1] + cv::Point2f(imageLeft.cols, 0), cornersLeftInRight[2] + cv::Point2f(imageLeft.cols, 0), cv::Scalar(0, 255, 0), 2);
	cv::line(goodMatcheImage, cornersLeftInRight[2] + cv::Point2f(imageLeft.cols, 0), cornersLeftInRight[3] + cv::Point2f(imageLeft.cols, 0), cv::Scalar(0, 255, 0), 2);
	cv::line(goodMatcheImage, cornersLeftInRight[3] + cv::Point2f(imageLeft.cols, 0), cornersLeftInRight[0] + cv::Point2f(imageLeft.cols, 0), cv::Scalar(0, 255, 0), 2);

	cv::namedWindow("Match Result", 1);
	cv::imshow("Match Result", goodMatcheImage);
	cv::waitKey(0);

	return 0;
}
//����ת����ƽ�ƾ���
int estimateImageAttitude(std::string pathLeft, std::string pathRight)
{
	cv::Mat  imageLeft, imageLeftRGB = cv::imread(pathLeft, cv::IMREAD_COLOR);
	cv::Mat  imageRight, imageRightRGB = cv::imread(pathRight, cv::IMREAD_COLOR);
	if (imageLeftRGB.empty()) {
		std::cout << "Fail to read the image: " << pathLeft << std::endl;
		return -1;
	}
	if (imageRightRGB.empty()) {
		std::cout << "Fail to read the image: " << pathRight << std::endl;
		return -1;
	}
	cv::imshow("imageLeft", imageLeftRGB);
	cv::imshow("imageRight", imageRightRGB);
	cv::waitKey(0);

	cv::cvtColor(imageLeftRGB, imageLeft, cv::COLOR_RGB2GRAY);
	cv::cvtColor(imageRightRGB, imageRight, cv::COLOR_RGB2GRAY);

	std::vector<cv::KeyPoint> keypointLeft, keypointRight;
	int minHessian = 400;//������ֵ
	cv::Ptr<cv::Feature2D> f2d = cv::xfeatures2d::SURF::create(minHessian);	
	f2d->detect(imageLeft, keypointLeft);
	f2d->detect(imageLeftRGB,keypointRight);
	cv::Mat descriptorsLeft, descriptorsRight;
	f2d->compute(imageLeft, keypointLeft, descriptorsLeft);
	f2d->compute(imageRight, keypointRight, descriptorsRight);

	cv::FlannBasedMatcher matcher;
	std::vector< cv::DMatch > matches;
	matcher.match(descriptorsLeft, descriptorsRight, matches);
	double maxDistance, minDistance;
	for (int i = 0; i < matches.size(); i++)
	{
		if (i == 0)
		{
			maxDistance = matches[i].distance;
			minDistance = matches[i].distance;
		}
		else
		{
			double dist = matches[i].distance;
			if (dist < minDistance) minDistance = dist;
			if (dist > maxDistance) maxDistance = dist;
		}
	}
	std::vector<cv::DMatch> goodMatches;
	for (int i = 0; i < matches.size(); i++)
	{
		if (matches[i].distance < 3 * minDistance)
			goodMatches.push_back(matches[i]);
	}
	cv::Mat goodMatcheImage;
	cv::drawMatches(imageLeft, keypointLeft, imageRight, keypointRight, goodMatches, goodMatcheImage, cv::Scalar::all(-1), cv::Scalar::all(-1),
		std::vector<char>(),cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	cv::namedWindow("GOODMATCHES", 1);
	cv::imshow("GOOD MATCHES", goodMatcheImage);
	cv::waitKey(0);
	if (goodMatches.size() < 8)
	{
		std::cout << "There are no enough good matches! HOMOGRAPHY MATRIX cannot be found!" << std::endl;
		return -1;
	}
	//�洢ͬ����
	std::vector<cv::Point2f> corrPointLeft, corrPointRight;
	for (int i = 0; i < goodMatches.size(); i++)
	{
		corrPointLeft.push_back(keypointLeft[goodMatches[i].queryIdx].pt);
		corrPointRight.push_back(keypointRight[goodMatches[i].trainIdx].pt);
	}
	//��������F
	cv::Mat fundamental;
	std::vector <uchar> RANSACStatus;
	fundamental = cv::findFundamentalMat(corrPointLeft,corrPointRight, RANSACStatus, cv::RANSAC);
	std::cout << "��������" << std::endl << fundamental << std::endl;
	double fx, fy, cx, cy;
	//����
	fx = 1565.11505389804;
	fy = 1570.81668565518;
	//��������ԭ��
	cx = 751.786785642578;
	cy = 526.659304960075;
	//�궨������������Ծ�ת��
	cv::Mat K = cv::Mat::eye(3, 3, CV_64FC1);
	K.at<double>(0, 0) = fx;
	K.at<double>(1, 1) = fy;
	K.at<double>(0, 2) = cx;
	K.at<double>(1, 2) = cy;
	cv::Mat Kt = K.t();
	cv::Mat E = Kt*fundamental*K;//���ʾ���
	cv::SVD svd(E);
	cv::Mat W = cv::Mat::zeros(3, 3, CV_64FC1);
	W.at<double>(0, 1) = -1;
	W.at<double>(1, 0) = 1;
	W.at<double>(2, 2) = 1;
	cv::Mat_<double> R = svd.u*W*svd.vt;
	cv::Mat_<double> t = svd.u.col(2);
	std::cout << "R=" << R << std::endl;
	std::cout << "t=" << t << std::endl;
	system("pause");
}
//**��������ȡ
int Gradient(std::string filePath, int gradientType)
{
	cv::Mat image = cv::imread(filePath, cv::IMREAD_COLOR);
	if (image.empty())
	{
		std::cout << "Fail to read the image at: \" " << filePath << "\" " << std::endl;
		return -1;
	}

	imshow("Image", image);
	//cv::waitKey(0);

	int xWindowStart = 50;	int yWindowStart = 50;
	cv::moveWindow("Image", xWindowStart, yWindowStart);
	cv::Mat imageGray;
	cv::cvtColor(image, imageGray, cv::COLOR_RGB2GRAY);

	cv::Mat xResult, yResult;

	std::string xWindowName, yWindowName;

	switch (gradientType)
	{
	case KongYuanhang::SOBEL:
	{
		xWindowName = "xSobel"; yWindowName = "ySobel";

		cv::Sobel(imageGray, xResult, CV_8U/*image.type()*/, 1, 0); //, 3, 1, 0, BORDER_REFLECT_101);
		cv::Sobel(imageGray, yResult, CV_8U/*image.type()*/, 0, 1); //, 3, 1, 0, BORDER_REFLECT_101);
	}
		break;
	case KongYuanhang::PREWITT:
	{
		xWindowName = "xPrewitt"; yWindowName = "yPrewitt";

		cv::Mat kernelX = (cv::Mat_<char>(3, 3) <<
			-1, 0, 1,
			-1, 0, 1,
			-1, 0, 1);

		cv::Mat kernelY = (cv::Mat_<char>(3, 3) <<
			-1, -1, -1,
			0, 0, 0,
			1, 1, 1);

		cv::filter2D(imageGray, xResult, imageGray.type(), kernelX);
		cv::filter2D(imageGray, yResult, imageGray.type(), kernelY);
	}
		break;

	case KongYuanhang::ROBERTS:
	{
		xWindowName = "xRoberts"; yWindowName = "yRoberts";

		xResult = imageGray.clone();
		yResult = imageGray.clone();
		int nRows = imageGray.rows;
		int nCols = imageGray.cols;
		for (int i = 0; i < nRows - 1; i++){
			for (int j = 0; j < nCols - 1; j++){
				//���ݹ�ʽ����
				int t1 = (imageGray.at<uchar>(i, j) -
					imageGray.at<uchar>(i + 1, j + 1))*
					(imageGray.at<uchar>(i, j) -
					imageGray.at<uchar>(i + 1, j + 1));
				int t2 = (imageGray.at<uchar>(i + 1, j) -
					imageGray.at<uchar>(i, j + 1))*
					(imageGray.at<uchar>(i + 1, j) -
					imageGray.at<uchar>(i, j + 1));
				int t3 = (-imageGray.at<uchar>(i, j) +
					imageGray.at<uchar>(i + 1, j + 1))*
					(-imageGray.at<uchar>(i, j) +
					imageGray.at<uchar>(i + 1, j + 1));
				int t4 = (-imageGray.at<uchar>(i + 1, j) +
					imageGray.at<uchar>(i, j + 1))*
					(-imageGray.at<uchar>(i + 1, j) +
					imageGray.at<uchar>(i, j + 1));
				//����g��x,y��
				xResult.at<uchar>(i, j) = (uchar)sqrt(t1 + t2);
				yResult.at<uchar>(i, j) = (uchar)sqrt(t3 + t4);
			}
		}

	}
		break;
	case KongYuanhang::LAPLACIAN:
	{
		xWindowName = "laplacianEdge"; yWindowName = "laplacianEnhance";
		cv::Mat kernelLaplacian = (cv::Mat_<char>(3, 3) <<
		0, -1,  0,
		-1,  4, -1,
		0, -1,  0);
		cv::Mat kernelLaplacianEnhance = (cv::Mat_<char>(3, 3) <<
			0, -1, 0,
			-1, 5, -1,
			0, -1, 0);
		cv::filter2D(imageGray, xResult, imageGray.type(), kernelLaplacian);
		cv::filter2D(imageGray, yResult, imageGray.type(), kernelLaplacianEnhance);
		
	}
		break;
	default:
		xWindowName = "xNotSupported"; yWindowName = "yNotSupported";
		cv::cvtColor(imageGray, image, cv::COLOR_GRAY2BGR);
		cv::putText(image, "Not supported yet!", cv::Point(10, 20), 1, 1, cv::Scalar(0, 0, 255));
		xResult = yResult = image.clone();
		break;
	}

	int xWindow = xWindowStart;
	int yWindow = yWindowStart + image.rows + 36;
	imshow(xWindowName, xResult);
	cv::moveWindow(xWindowName, xWindow, yWindow);

	xWindow = xWindowStart + image.cols + 20;
	imshow(yWindowName, yResult);
	cv::moveWindow(yWindowName, xWindow, yWindow);

	cv::waitKey(0);

	return 0;
}

int CannyEdge(std::string filePath)
{
	cv::Mat image = cv::imread(filePath, cv::IMREAD_COLOR);
	if (image.empty())
	{
		std::cout << "Fail to read the image at: \" " << filePath << "\" " << std::endl;
		return -1;
	}

	imshow("Image", image);

	int xShowStart = 10;
	int yShowStart = 10;
	cv::moveWindow("Image", xShowStart, yShowStart);
	//��˹�˲�
	/*cv::GaussianBlur(image, image, cv::Size(5, 5), 1.6);*/
	const int countToCompare = 5;

	int widthWindow = image.cols + 10;
	int heightWindow = image.rows + 25;

	int tHigh, tLow;//�ߵ���ֵ
	//�̶�����ֵ
	tLow = 80;
	for (int i = 0; i < countToCompare; i++)
	{
		tHigh = 100 + 100 * i;
		cv::Mat result;
		cv::Canny(image, result, tHigh, tLow);

		std::ostringstream os;
		os << tHigh << '-' << tLow;
		std::string windowName = os.str();

		cv::imshow(windowName, result);

		int xWindow = xShowStart + i*widthWindow;
		int yWindow = yShowStart + heightWindow;
		cv::moveWindow(windowName, xWindow, yWindow);
	}

	cv::waitKey(0);

	//�̶�����ֵ
	tHigh = 300;
	for (int i = 0; i < countToCompare; i++)
	{
		tLow = 30 + 20 * i;
		cv::Mat result;
		cv::Canny(image, result, tHigh, tLow);

		std::ostringstream os;
		os << tHigh << '-' << tLow;
		std::string windowName = os.str();

		cv::imshow(windowName, result);

		int xWindow = xShowStart + i*widthWindow;
		int yWindow = yShowStart + heightWindow*2+10;
		cv::moveWindow(windowName, xWindow, yWindow);
	}
	cv::waitKey(0);
}

int LineHough(std::string filePath)
{
	//=================================================
	//Create data matrix, read and show image.
	//=================================================
	cv::Mat result, image;
	image = cv::imread(filePath, cv::IMREAD_COLOR);
	if (image.empty())
	{
		std::cout << "Fail to read the image at: \" " << filePath << "\" " << std::endl;
		return -1;
	}

	imshow("Image", image);

	int xShowStart = 150;
	int yShowStart = 200;

	int widthWindow = image.cols + 10;
	int heightWindow = image.rows + 25;

	cv::moveWindow("Image", xShowStart, yShowStart);

	//=======================================
	//Compute hough transfrom
	//=======================================
	//Create result matrix
	cv::Mat resultHough = image.clone(); //Classical hough transform
	cv::Mat resultHoughP = image.clone(); //Probabilistic hough transfrom

	cv::cvtColor(image, image, cv::COLOR_RGB2GRAY);

	//Run canny detector
	int tHigh = 300;	int tLow = 100;
	cv::Mat resultCanny, resultCannyHough,resultCannyHoughP;
	cv::Canny(image, resultCanny, tHigh, tLow);
	cv::cvtColor(resultCanny, resultCannyHough, cv::COLOR_GRAY2BGR);
	cv::cvtColor(resultCanny, resultCannyHoughP, cv::COLOR_GRAY2BGR);
	

	std::string windowName = "Canny";
	imshow(windowName, resultCanny);
	int xWindow = xShowStart + widthWindow;
	cv::moveWindow(windowName, xWindow, yShowStart);

	//Classical hough transfrom
	std::vector<cv::Vec2f> lines; //rho, theta
	int imageWidth = image.cols;
	int accThreshold = (float)imageWidth / 2.5;
	double thetaRes = CV_PI / 180.0; double rhoRes = 1;
	//cv::HoughLines(resultCanny, lines, rhoRes, thetaRes, accThreshold, 2, 2, CV_PI * 0.8, CV_PI*0.99);
	cv::HoughLines(resultCanny, lines, rhoRes, thetaRes, accThreshold, 2, 2, CV_PI * 0.0, CV_PI*0.15);

	std::cout << "I find " << lines.size() << " lines in this image." << std::endl;

	//Show hough transfrom result
	for (int i = 0; i < lines.size(); i++)
	{
		float rho = lines.at(i)[0];
		float theta = lines.at(i)[1];

		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;

		cv::Point ptStart(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * (a)));
		cv::Point ptEnd(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * (a)));

		cv::line(resultHough, ptStart, ptEnd, cv::Scalar(0, 0, 255), 1, 8);
		cv::line(resultCannyHough, ptStart, ptEnd, cv::Scalar(0, 0, 255), 1, 8);
		cv::line(resultHough, ptStart, ptEnd, cv::Scalar(0, 0, 255), 1, 8);
		cv::line(resultCannyHough, ptStart, ptEnd, cv::Scalar(0, 0, 255), 1, 8);
	}
	windowName = "Classical Hough Canny";
	cv::imshow(windowName, resultCannyHough);
	xWindow += widthWindow;
	cv::moveWindow(windowName, xWindow, yShowStart);

	windowName = "Classical Hough";
	cv::imshow(windowName, resultHough);
	xWindow += widthWindow;
	cv::moveWindow(windowName, xWindow, yShowStart);

	cv::waitKey(0);

	std::vector<cv::Vec4i> linesP;
	accThreshold = 80;
	double minLineLength = 50;
	double maxLineGap = 5;
	cv::HoughLinesP(resultCanny, linesP, rhoRes, thetaRes, accThreshold, minLineLength, maxLineGap);
	for (size_t i = 0; i < linesP.size(); i++)
	{
		cv::line(resultHoughP, cv::Point(linesP[i][0], linesP[i][1]), cv::Point(linesP[i][2], linesP[i][3]), cv::Scalar(0, 0, 255), 1, 8);
		cv::line(resultCannyHoughP, cv::Point(linesP[i][0], linesP[i][1]), cv::Point(linesP[i][2], linesP[i][3]), cv::Scalar(0, 0, 255), 1, 8);
	}
	windowName = "Probabilistic Hough";
	imshow(windowName, resultHoughP);
	int yWindow = yShowStart + heightWindow;
	cv::moveWindow(windowName, xWindow, yWindow);

	windowName = "Probabilistic Hough Canny";
	cv::imshow(windowName, resultCannyHoughP);
	xWindow += widthWindow;
	cv::moveWindow(windowName, xWindow, yWindow);

	cv::waitKey(0);
}

int LineLSD(std::string filePath)
{
	cv::Mat image;
	image = cv::imread(filePath, cv::IMREAD_COLOR);
	if (image.empty())
	{
		std::cout << "Fail to read the image at: \" " << filePath << "\" " << std::endl;
		return -1;
	}

	imshow("Image", image);

	int xShowStart = 150;
	int yShowStart = 200;

	int widthWindow = image.cols + 10;
	int heightWindow = image.rows + 25;
	cv::cvtColor(image, image, cv::COLOR_RGB2GRAY);

	std::vector<cv::Ptr<cv::LineSegmentDetector>> lsdList;
	std::vector<std::string> nameList;

	double scale = 0.8, sigmaSale = 0.6;
	double angleTh = 22.5;
	cv::Ptr<cv::LineSegmentDetector> lsd1 = cv::createLineSegmentDetector(
		cv::LSD_REFINE_NONE, scale, sigmaSale, 2, angleTh, 0.8, 0.7, 1024);
	lsdList.push_back(lsd1); nameList.push_back("Refine None");

	cv::Ptr<cv::LineSegmentDetector> lsd2 = cv::createLineSegmentDetector(
		cv::LSD_REFINE_STD, scale, sigmaSale, 2, angleTh/2.0, 0.8, 0.7, 1024);
	lsdList.push_back(lsd2); nameList.push_back("Refine STD");

	cv::Ptr<cv::LineSegmentDetector> lsd3 = cv::createLineSegmentDetector(
		cv::LSD_REFINE_ADV, scale, sigmaSale, 2, angleTh/4.0, 0.8, 0.7, 1024);
	lsdList.push_back(lsd3); nameList.push_back("Refine ADV");
	for (int i = 0; i < lsdList.size(); i++)
	{
		std::vector<cv::Vec4f> lines;
		lsdList.at(i)->detect(image, lines);

		cv::Mat result(image);
		lsdList.at(i)->drawSegments(result, lines);

		std::string windowName = nameList.at(i);
		cv::imshow(windowName, result);
		int xWindow = xShowStart + (i + 1)*widthWindow;
		cv::moveWindow(windowName, xWindow, yShowStart);
	}
	cv::waitKey(0);

}

//������Ӱ�����γ�

/*Moravec����*/
//Ѱ���������ֵ����Сֵ
void find(float a[], int m, float &max, float &min)
{
	min = a[0];
	max = a[0];
	for (int i = 0; i < m; i++)
	{
		if (a[i] > max)
		{
			max = a[i];
			continue;
		}
		else if (a[i] < min)
		{
			min = a[i];
			continue;
		}
	}
}
//����Ȥֵ������(ͼ�����Moravec���ڴ�С�������ص�x,y)
float getInterestValue(cv::Mat m_srcimg, int Moravecsize, int i_x, int j_y)
{
	int halfsize = (Moravecsize) / 2;//����С������ڴ�С
	float temp4[4];//�����ĸ����������������"*"�͵�ƽ����
	//�����ʼ��
	for (int i = 0; i < 4; i++)
	{
		temp4[i] = 0;
	}
	//�ۼ�*��ƽ����
	for (int i = 0; i < Moravecsize; i++)
	{
		float l = m_srcimg.at<uchar>(i_x - halfsize + i, j_y);//    | x���� ������
		temp4[0] += pow(m_srcimg.at<uchar>(i_x - halfsize + i, j_y) - m_srcimg.at<uchar>(i_x - halfsize + i + 1, j_y), 2);//    | x���� ������
		temp4[1] += pow(m_srcimg.at<uchar>(i_x, j_y - halfsize + i) - m_srcimg.at<uchar>(i_x, j_y - halfsize + i + 1), 2);//    - y����
		temp4[2] += pow(m_srcimg.at<uchar>(i_x - halfsize + i, j_y - halfsize + i) - m_srcimg.at<uchar>(i_x - halfsize + i + 1, j_y - halfsize + i + 1), 2);//   \ ����
		temp4[3] += pow(m_srcimg.at<uchar>(i_x - halfsize + i, j_y + halfsize - i) - m_srcimg.at<uchar>(i_x - halfsize + i + 1, j_y + halfsize - i - 1), 2);//   / ����
	}
	float min, max;//������������ֵ ��Сֵ
	find(temp4, 4, max, min);//����Сֵ��ֵ
	return min;//���ؼ�Сֵ
}
//�ó����
int  Moravec(std::string path,std::vector<cv::Point3f> &featurePointLeft)
{
	cv::Mat imageRGB = cv::imread(path, cv::IMREAD_COLOR);
	if (imageRGB.empty())
	{
		std::cout << "Fail to read the image!" << path << std::endl;
		return -1;
	}
	cv::imshow("image", imageRGB);//��ʾԭͼ
	cv::waitKey(0);
	cv::Mat imageGray;
	cv::cvtColor(imageRGB, imageGray, cv::COLOR_RGB2GRAY);
	std::vector<cv::Point3f> f;
	GaussianBlur(imageGray, imageGray, cv::Size(5, 5), 0, 0);//ʹ��opencv�Դ���˹�˲�Ԥ����
	cv::Mat result/*���32bit*/, resultNorm, resultNormUInt8;
	result = cv::Mat::zeros(imageGray.size(), CV_32FC1);

	int Moravecsize = 5;//����Moravec��Ȥֵ�Ĵ��ڴ�С
	int Inhibitionsize = 9;//���ƾֲ����Ĵ��ڴ�С

	float sum = 0;
	for (int i = 5; i < imageGray.rows - 5; i++)
	{
		for (int j = 5; j < imageGray.cols - 5; j++)
		{
			float min = getInterestValue(imageGray, Moravecsize, i, j);
			result.at<float>(i, j) = min;
			sum += min;
		}
	}
	//��С����ֵ�ĸ�������Ȥֵ��Ϊ��
	float mean = sum / (result.rows*result.cols);//������ֵ����
	if (mean < 60) { mean = 300; }
	mean = 700;
	for (int i = 0; i <result.rows; i++)
	{
		for (int j = 0; j < result.cols; j++)
		{
			if (result.at<float>(i, j) < mean)
			{
				result.at<float>(i, j) = 0;
			}
		}
	}
	int halfInhibitionsize = Inhibitionsize / 2;//����С������ڴ�С
	//���ƾֲ����
	for (int i = halfInhibitionsize; i < result.rows - 1 - halfInhibitionsize; i++)
	{
		for (int j = halfInhibitionsize; j < result.cols - 1 - halfInhibitionsize; j++)
		{
			float temp1 = result.at<float>(i, j);
			for (int m = 0; m < Inhibitionsize; m++)
			{
				for (int n = 0; n < Inhibitionsize; n++)
				{
					float temp2 = result.at<float>(i - halfInhibitionsize + m, j - halfInhibitionsize + n);
					if (temp1 < temp2)
					{
						result.at<float>(i, j) = 0;
						n = Inhibitionsize;
						m = Inhibitionsize;
					}
				}
			}
		}
	}

	//�洢������
	for (int i = halfInhibitionsize; i < result.rows - halfInhibitionsize - 1; i++)
	{
		for (int j = halfInhibitionsize; j < result.cols - halfInhibitionsize - 1; j++)
		{

			if (result.at<float>(i, j) > 0)
			{
				cv::Point3f temp;
				temp.x = i;
				temp.y = j;
				temp.z = result.at<float>(i, j);
				featurePointLeft.push_back(temp);
			}
		}
	}
	//��ʼɾ��ͬ��һ����ֵһ�����ظ��㣬���ܳ��ָ��ʽ�С�����ϴ��ͼ������ĳЩ�����л���ںü�����ֵ��ȵļ���ֵ
	for (int i = 0; i < featurePointLeft.size() - 1; i++)
	{
		for (int j = i + 1; j < featurePointLeft.size(); j++)
		{
			if ((featurePointLeft.at(i).z == featurePointLeft.at(j).z))
			{
				if (abs(featurePointLeft.at(i).x - featurePointLeft.at(j).x) < Inhibitionsize || abs(featurePointLeft.at(i).y - featurePointLeft.at(j).y) < Inhibitionsize)
				{
					featurePointLeft.erase(featurePointLeft.begin() + j);
					i = 0;
					break;
				}
			}
		}
	}
	//Normalizing image to 0-255
	cv::normalize(result, resultNorm, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());
	cv::convertScaleAbs(resultNorm, resultNormUInt8);
	//��ͼչʾ����
	int radius = 5;
	for (size_t n = 0; n < featurePointLeft.size(); n++)
	{
		int i = int(featurePointLeft.at(n).y);
		int j = int(featurePointLeft.at(n).x);
		cv::circle(resultNormUInt8, cv::Point(i, j), radius, cv::Scalar(255), 1, 8, 0);
		cv::circle(imageRGB, cv::Point(i, j), radius, cv::Scalar(0, 255, 255), 1, 4, 0);
		cv::line(imageRGB, cv::Point(i - radius - 2, j), cv::Point(i + radius + 2, j), cv::Scalar(0, 255, 255), 1, 8, 0);
		cv::line(imageRGB, cv::Point(i, j - radius - 2), cv::Point(i, j + radius + 2), cv::Scalar(0, 255, 255), 1, 8, 0);		
	}

	cv::imshow("MoravecResult", resultNormUInt8);
	cv::imshow("imageRGB", imageRGB);
	cv::waitKey(0);
}
//���ϵ��ͼ��ƥ��
float Get_coefficient(cv::Mat matchLeftWindow, cv::Mat imageRight, int x, int y)
{
	//��������������ȷ�����������ڵĴ�С
	cv::Mat Rmatchwindow;
	Rmatchwindow.create(matchLeftWindow.rows, matchLeftWindow.cols, CV_32FC1);
	float aveRImg = 0;
	for (int m = 0; m < matchLeftWindow.rows; m++)
	{
		for (int n = 0; n < matchLeftWindow.cols; n++)
		{
			aveRImg += imageRight.at<uchar>(x + m, y + n);
			Rmatchwindow.at<float>(m, n) = imageRight.at<uchar>(x + m, y + n);
		}
	}
	aveRImg = aveRImg / (matchLeftWindow.rows*matchLeftWindow.cols);
	for (int m = 0; m < matchLeftWindow.rows; m++)
	{
		for (int n = 0; n < matchLeftWindow.cols; n++)
		{
			Rmatchwindow.at<float>(m, n) -= aveRImg;
		}
	}
	//��ʼ�������ϵ��
	float cofficent1 = 0;
	float cofficent2 = 0;
	float cofficent3 = 0;
	for (int m = 0; m < matchLeftWindow.rows; m++)
	{
		for (int n = 0; n < matchLeftWindow.cols; n++)
		{
			cofficent1 += matchLeftWindow.at<float>(m, n)*Rmatchwindow.at<float>(m, n);
			cofficent2 += Rmatchwindow.at<float>(m, n)*Rmatchwindow.at<float>(m, n);
			cofficent3 += matchLeftWindow.at<float>(m, n)*matchLeftWindow.at<float>(m, n);
		}
	}
	double cofficent = cofficent1 / sqrt(cofficent2 * cofficent3);
	return cofficent;
}
void vectorsort(std::vector < cv::Point3f> &Temp_sort)
{
	for (int i = 0; i < Temp_sort.size() - 1; i++) {
		float tem = 0;
		float temx = 0;
		float temy = 0;
		// �ڲ�forѭ���������ڵ�����Ԫ�ؽ��бȽ�
		for (int j = i + 1; j < Temp_sort.size(); j++) {
			if (Temp_sort.at(i).z < Temp_sort.at(j).z) {
				tem = Temp_sort.at(j).z;
				Temp_sort.at(j).z = Temp_sort.at(i).z;
				Temp_sort.at(i).z = tem;

				temx = Temp_sort.at(j).x;
				Temp_sort.at(j).x = Temp_sort.at(i).x;
				Temp_sort.at(i).x = temx;

				temy = Temp_sort.at(j).y;
				Temp_sort.at(j).y = Temp_sort.at(i).y;
				Temp_sort.at(i).y = temy;
			}
		}
	}
}
void lastview(cv::Mat imageLeftRGB, cv::Mat imageRightRGB, std::vector<cv::Point3f> featurePointLeft, std::vector<cv::Point3f> featurePointRight)
{
	cv::Mat bothview;//���ͼ��
	bothview.create(imageLeftRGB.rows, imageLeftRGB.cols + imageRightRGB.cols, imageLeftRGB.type());
	for (int i = 0; i <imageLeftRGB.rows; i++)
	{
		for (int j = 0; j < imageLeftRGB.cols; j++)
		{
			bothview.at<cv::Vec3b>(i, j) = imageLeftRGB.at<cv::Vec3b>(i, j);
		}
	}
	
	for (int i = 0; i <imageRightRGB.rows; i++)
	{
		for (int j = imageLeftRGB.cols; j <imageLeftRGB.cols + imageRightRGB.cols; j++)
		{
			bothview.at<cv::Vec3b>(i, j) = imageRightRGB.at<cv::Vec3b>(i, j - imageLeftRGB.cols);
		}
	}//����Ӱ��϶�Ϊһ	
	for (int i = 0; i < featurePointRight.size(); i++)
	{
		int a = (rand() % 200);
		int b = (rand() % 200 + 99);
		int c = (rand() % 200) - 50;
		if (a > 100 || a < 0)
		{
			a = 255;
		}
		if (b > 255 || b < 0)
		{
			b = 88;
		}
		if (c > 255 || c < 0)
		{
			c = 188;
		}
		int radius = 5;
		//��Ƭ
		int lm = int(featurePointLeft.at(i).y);
		int ln = int(featurePointLeft.at(i).x);

		cv::circle(bothview, cv::Point(lm, ln), radius, cv::Scalar(0, 255, 255), 1, 4, 0);
		cv::line(bothview, cv::Point(lm - radius - 2, ln), cv::Point(lm + radius + 2, ln), cv::Scalar(0, 255, 255), 1, 8, 0);
		cv::line(bothview, cv::Point(lm, ln - radius - 2), cv::Point(lm, ln + radius + 2), cv::Scalar(0, 255, 255), 1, 8, 0);

		//��Ƭ
		int rm = int(featurePointRight.at(i).y + imageLeftRGB.cols);
		int rn = int(featurePointRight.at(i).x);

		cv::circle(bothview, cv::Point(rm, rn), radius,cv::Scalar(0, 255, 255), 1, 4, 0);
		cv::line(bothview, cv::Point(rm - radius - 2, rn), cv::Point(rm + radius + 2, rn), cv::Scalar(0, 255, 255), 1, 8, 0);
		cv::line(bothview, cv::Point(rm, rn - radius - 2), cv::Point(rm, rn + radius + 2), cv::Scalar(0, 255, 255), 1, 8, 0);
		//����
		cv::line(bothview, cv::Point(lm,ln), cv::Point(rm, rn),cv::Scalar(a, b, c), 1,8,0);
	}

	cv::imshow("����ƬӰ��ͬ����չʾ", bothview);
	cv::waitKey(0);
}
int kyhMatchingImg(std::string pathLeft, std::string pathRight, std::vector<cv::Point3f> featurePointLeft)
{
	cv::Mat imageLeft, imageLeftRGB = cv::imread(pathLeft, cv::IMREAD_COLOR);
	cv::Mat imageRight, imageRightRGB = cv::imread(pathRight, cv::IMREAD_COLOR);
	if (imageLeftRGB.empty())
	{
		std::cout << "Fail to read the image:" << pathLeft << std::endl;
		return -1;
	}
	if (imageRightRGB.empty())
	{
		std::cout << "Fail to read the image:" << pathRight << std::endl;
		return -1;
	}
	cv::imshow("imageLeftRGB", imageLeftRGB);
	cv::imshow("imageRightRGB", imageRightRGB);
	cv::waitKey(0);

	cv::cvtColor(imageLeftRGB, imageLeft, cv::COLOR_BGR2GRAY);
	cv::cvtColor(imageRightRGB, imageRight, cv::COLOR_BGR2GRAY);

	int matchsize = 9;//���ϵ���������δ��ڵı߳�
	int half_matchsize = matchsize / 2;//�߳���һ��

	std::vector<cv::Point3f> featurePointRight;//��Ƭƥ�䵽������

	float lowst_door = 0.7; //���ϵ����ƥ�����ֵ
	int dist_width = 850;//����Ƭ������Ƭ����Ծ��룬������ͨ���ֶ��۲�

	//����f���ݵ�Ԥ���� ɾ�������Ϲ淶������
	for (size_t i = 0; i < featurePointLeft.size(); i++)
	{
		//����� 5 = half_matchsize + 1
		if ((featurePointLeft.at(i).y + dist_width < imageLeft.cols) || (imageLeft.cols - featurePointLeft.at(i).y <5))
		{
			featurePointLeft.erase(featurePointLeft.begin() + i);
			i--;
			continue;
		}
		if ((featurePointLeft.at(i).x < 5) || (imageLeft.rows - featurePointLeft.at(i).x < 5))
		{
			featurePointLeft.erase(featurePointLeft.begin() + i);
			i--;
			continue;
		}

	}
	//�����󴰿ڵ�С����
	cv::Mat matchLeftWindow;
	matchLeftWindow.create(matchsize, matchsize, CV_32FC1);
	for (size_t i = 0; i < featurePointLeft.size(); i++)
	{
		float aveLImg = 0;
		for (int m = 0; m <matchsize; m++)
		{
			for (int n = 0; n < matchsize; n++)
			{
				aveLImg += imageLeft.at<uchar>(featurePointLeft.at(i).x - half_matchsize + m, featurePointLeft.at(i).y - half_matchsize + n);
				matchLeftWindow.at<float>(m, n) = imageLeft.at<uchar>(featurePointLeft.at(i).x - half_matchsize + m, featurePointLeft.at(i).y - half_matchsize + n);
			}
		}
		aveLImg = aveLImg / (matchsize* matchsize);//��ȡ�󴰿�ƽ��ֵ
		//����ĳ��ֵ
		for (int m = 0; m < matchsize; m++)
		{
			for (int n = 0; n < matchsize; n++)
			{
				matchLeftWindow.at<float>(m, n) = matchLeftWindow.at<float>(m, n) - aveLImg;
			}
		}
		//***************************���Ҵ��ڽ��м���
		//����Ԥ���Ҵ��ڵ�λ��
		int halflengthsize = 10; //�������İ뾶
		std::vector < cv::Point3f> tempfeatureRightPoint;
		//ȥ���ܵ�������ĵ�
		for (int ii = -halflengthsize; ii <= halflengthsize; ii++)
		{
			for (int jj = -halflengthsize; jj <= halflengthsize; jj++)
			{
				//Ϊ��ʡ�¡��� �ѱ�Ե���޵Ķ�����û��
				if ((featurePointLeft.at(i).x<(halflengthsize + 5)) || (imageRight.rows - featurePointLeft.at(i).x)<(halflengthsize + 5)
					|| (featurePointLeft.at(i).y + dist_width - imageLeft.cols)<(halflengthsize + 5))
				{
					cv::Point3f temphalflengthsize;
					temphalflengthsize.x = 0;
					temphalflengthsize.y = 0;
					temphalflengthsize.z = 0;
					tempfeatureRightPoint.push_back(temphalflengthsize);
				}
				else
				{
					cv::Point3f temphalflengthsize;
					int x = featurePointLeft.at(i).x + ii - half_matchsize;
					int y = featurePointLeft.at(i).y + dist_width - imageLeft.cols + jj - half_matchsize;
					float  coffee = Get_coefficient(matchLeftWindow, imageRight, x, y);
					temphalflengthsize.x = featurePointLeft.at(i).x + ii;
					temphalflengthsize.y = featurePointLeft.at(i).y + dist_width - imageLeft.cols + jj;
					temphalflengthsize.z = coffee;
					tempfeatureRightPoint.push_back(temphalflengthsize);
				}
				
			}
		}
		vectorsort(tempfeatureRightPoint);

		if (tempfeatureRightPoint.at(0).z > lowst_door&&tempfeatureRightPoint.at(0).z < 1)
		{
			cv::Point3f tempr;
			tempr.x = tempfeatureRightPoint.at(0).x;
			tempr.y = tempfeatureRightPoint.at(0).y;
			tempr.z = tempfeatureRightPoint.at(0).z;
			featurePointRight.push_back(tempr);
		}
		else
		{
			featurePointLeft.erase(featurePointLeft.begin() + i);
			i--;
			continue;
		}
	}
	/*int radius = 5;
	for (size_t n = 0; n < featurePointRight.size(); n++)
	{
		int i = int(featurePointRight.at(n).y);
		int j = int(featurePointRight.at(n).x);
		cv::circle(imageRightRGB, cv::Point(i, j), radius, cv::Scalar(0, 255, 255), 1, 4, 0);
		cv::line(imageRightRGB, cv::Point(i - radius - 2, j), cv::Point(i + radius + 2, j), cv::Scalar(0, 255, 255), 1, 8, 0);
		cv::line(imageRightRGB, cv::Point(i, j - radius - 2), cv::Point(i, j + radius + 2), cv::Scalar(0, 255, 255), 1, 8, 0);
	}
	cv::imshow("imageRightRGB", imageRightRGB);
	cv::waitKey(0);*/
	lastview(imageLeftRGB, imageRightRGB, featurePointLeft, featurePointRight);
	return 0;
}
//��С����ͼ��ƥ��
int kyhMatchImgbyLST(std::string pathLeft, std::string pathRight, std::vector<cv::Point3f> featurePointLeft)
{
	cv::Mat imageLeft, imageLeftRGB = cv::imread(pathLeft, cv::IMREAD_COLOR);
	cv::Mat imageRight, imageRightRGB = cv::imread(pathRight, cv::IMREAD_COLOR);
	if (imageLeftRGB.empty())
	{
		std::cout << "Fail to read the image:" << pathLeft << std::endl;
		return -1;
	}
	if (imageRightRGB.empty())
	{
		std::cout << "Fail to read the image:" << pathRight << std::endl;
		return -1;
	}
	cv::cvtColor(imageLeftRGB, imageLeft, cv::COLOR_BGR2GRAY);
	cv::cvtColor(imageRightRGB, imageRight, cv::COLOR_BGR2GRAY);

	int matchsize = 9;//���ϵ���������δ��ڵı߳�
	int half_matchsize = matchsize / 2;//�߳���һ��

	std::vector<cv::Point3f> featurePointRight;//��Ƭƥ�䵽������

	float lowst_door = 0.7; //���ϵ����ƥ�����ֵ
	int dist_width = 270;//����Ƭ������Ƭ����Ծ��룬������ͨ���ֶ��۲�

	//����f���ݵ�Ԥ���� ɾ�������Ϲ淶������
	for (size_t i = 0; i < featurePointLeft.size(); i++)
	{
		//����� 5 = half_matchsize + 1
		if ((featurePointLeft.at(i).y + dist_width < imageLeft.cols) || (imageLeft.cols - featurePointLeft.at(i).y <5))
		{
			featurePointLeft.erase(featurePointLeft.begin() + i);
			i--;
			continue;
		}
		if ((featurePointLeft.at(i).x < 5) || (imageLeft.rows - featurePointLeft.at(i).x < 5))
		{
			featurePointLeft.erase(featurePointLeft.begin() + i);
			i--;
			continue;
		}

	}
	//�����󴰿ڵ�С����
	cv::Mat matchLeftWindow;
	matchLeftWindow.create(matchsize, matchsize, CV_32FC1);
	for (size_t i = 0; i < featurePointLeft.size(); i++)
	{
		float aveLImg = 0;
		for (int m = 0; m <matchsize; m++)
		{
			for (int n = 0; n < matchsize; n++)
			{
				aveLImg += imageLeft.at<uchar>(featurePointLeft.at(i).x - half_matchsize + m, featurePointLeft.at(i).y - half_matchsize + n);
				matchLeftWindow.at<float>(m, n) = imageLeft.at<uchar>(featurePointLeft.at(i).x - half_matchsize + m, featurePointLeft.at(i).y - half_matchsize + n);
			}
		}
		aveLImg = aveLImg / (matchsize* matchsize);//��ȡ�󴰿�ƽ��ֵ
		//����ĳ��ֵ
		for (int m = 0; m < matchsize; m++)
		{
			for (int n = 0; n < matchsize; n++)
			{
				matchLeftWindow.at<float>(m, n) = matchLeftWindow.at<float>(m, n) - aveLImg;
			}
		}
		//***************************���Ҵ��ڽ��м���
		//����Ԥ���Ҵ��ڵ�λ��
		int halflengthsize = 10; //�������İ뾶
		std::vector < cv::Point3f> tempfeatureRightPoint;
		//ȥ���ܵ�������ĵ�
		for (int ii = -halflengthsize; ii <= halflengthsize; ii++)
		{
			for (int jj = -halflengthsize; jj <= halflengthsize; jj++)
			{
				//Ϊ��ʡ�¡��� �ѱ�Ե���޵Ķ�����û��
				if ((featurePointLeft.at(i).x<(halflengthsize + 5)) || (imageRight.rows - featurePointLeft.at(i).x)<(halflengthsize + 5)
					|| (featurePointLeft.at(i).y + dist_width - imageLeft.cols)<(halflengthsize + 5))
				{
					cv::Point3f temphalflengthsize;
					temphalflengthsize.x = 0;
					temphalflengthsize.y = 0;
					temphalflengthsize.z = 0;
					tempfeatureRightPoint.push_back(temphalflengthsize);
				}
				else
				{
					cv::Point3f temphalflengthsize;
					int x = featurePointLeft.at(i).x + ii - half_matchsize;
					int y = featurePointLeft.at(i).y + dist_width - imageLeft.cols + jj - half_matchsize;
					float  coffee = Get_coefficient(matchLeftWindow, imageRight, x, y);
					temphalflengthsize.x = featurePointLeft.at(i).x + ii;
					temphalflengthsize.y = featurePointLeft.at(i).y + dist_width - imageLeft.cols + jj;
					temphalflengthsize.z = coffee;
					tempfeatureRightPoint.push_back(temphalflengthsize);
				}

			}
		}
		vectorsort(tempfeatureRightPoint);

		if (tempfeatureRightPoint.at(0).z > lowst_door&&tempfeatureRightPoint.at(0).z < 1)
		{
			cv::Point3f tempr;
			tempr.x = tempfeatureRightPoint.at(0).x;
			tempr.y = tempfeatureRightPoint.at(0).y;
			tempr.z = tempfeatureRightPoint.at(0).z;
			featurePointRight.push_back(tempr);
		}
		else
		{
			featurePointLeft.erase(featurePointLeft.begin() + i);
			i--;
			continue;
		}
	}
	//�õ�������Ƭ��ͬ�����ʼֵ

	/*��ʽ��ʼ��С����ƥ��*/
	std::vector<cv::Point3f> featureRightPointLST;//�洢��С����ƥ�䵽�ĵ�
	//�󼸺λ���ĳ�ʼֵ
	cv::Mat formerP = cv::Mat::eye(2 * featurePointLeft.size(), 2 * featurePointLeft.size(), CV_32F)/*Ȩ����*/,
		formerL = cv::Mat::zeros(2 * featurePointLeft.size(), 1, CV_32F)/*������*/,
		formerA = cv::Mat::zeros(2 * featurePointLeft.size(), 6, CV_32F)/*ϵ������*/;
	for (int i = 0; i < featurePointLeft.size(); i++)
	{
		float x1 = featurePointLeft.at(i).x;
		float y1 = featurePointLeft.at(i).y;
		float x2 = featurePointRight.at(i).x;
		float y2 = featurePointRight.at(i).y;
		float coef = featurePointRight.at(i).z;//��ʼͬ��������ϵ����ΪȨ��
		formerP.at<float>(2 * i, 2 * i) = coef;
		formerP.at<float>(2 * i + 1, 2 * i + 1) = coef;
		formerL.at<float>(2 * i, 0) = x2;
		formerL.at<float>(2 * i + 1, 0) = y2;
		formerA.at<float>(2 * i, 0) = 1; formerA.at<float>(2 * i, 1) = x1; formerA.at<float>(2 * i, 2) = y1;
		formerA.at<float>(2 * i + 1, 3) = 1; formerA.at<float>(2 * i + 1, 4) = x1; formerA.at<float>(2 * i + 1, 5) = y1;
	}
	cv::Mat Nbb = formerA.t()*formerP*formerA, U = formerA.t()*formerP*formerL;
	cv::Mat formerR = Nbb.inv()*U;
	//��ʼ������С����ƥ��
	for (int i = 0; i < featurePointLeft.size(); i++)
	{
		//����ĵ�����ʼֵ
		float x1 = featurePointLeft.at(i).x;
		float y1 = featurePointLeft.at(i).y;
		float x2 = featurePointRight.at(i).x;
		float y2 = featurePointRight.at(i).y;
		//���λ������������ʼֵ
		float a0 = formerR.at<float>(0, 0); float a1 = formerR.at<float>(1, 0); float a2 = formerR.at<float>(2, 0);
		float b0 = formerR.at<float>(3, 0); float b1 = formerR.at<float>(4, 0); float b2 = formerR.at<float>(5, 0);
		//������������ʼֵ
		float h0 = 0, h1 = 1;
		
		//����һ�����ϵ��С��ǰһ�Σ�����ֹͣ
		float beforeCorrelationCoe = 0/*ǰһ�����ϵ��*/, CorrelationCoe = 0;
		float xs = 0,ys = 0;

		while (beforeCorrelationCoe <= CorrelationCoe)
		{
			beforeCorrelationCoe = CorrelationCoe;
			cv::Mat C = cv::Mat::zeros(matchsize*matchsize, 8, CV_32F);//ϵ������matchsizeΪ��ƬĿ�괰�ڴ�С
			cv::Mat L = cv::Mat::zeros(matchsize*matchsize, 1, CV_32F);//������
			cv::Mat P = cv::Mat::eye(matchsize*matchsize, matchsize*matchsize, CV_32F);//Ȩ����
			float sumgxSquare = 0, sumgySquare = 0, sumXgxSquare = 0, sumYgySquare = 0;
			int dimension = 0;//���ھ���ֵ
			float sumLImg = 0, sumLImgSquare = 0, sumRImg = 0, sumRImgSquare = 0, sumLR = 0;

			for (int m = x1 - half_matchsize; m <= x1 + half_matchsize; m++)
			{
				for (int n = y1 - half_matchsize; n <= y1 + half_matchsize; n++)
				{
					float x2 = a0 + a1*m + a2*n;
					float y2 = b0 + b1*m + b2*n;
					int I = std::floor(x2); int J = std::floor(y2);//�������Ա������������
					if (I <= 1 || I >= imageRight.rows - 1 || J <= 1 || J >= imageRight.cols - 1)
					{
						I = 2; J = 2; P.at<float>((m - (y1 -5) - 1)*(2 * 4+ 1) + n - (x1 - 5), (m - (y1 - 5) - 1)*(2 * 4 + 1) + n - (x1 - 5)) = 0;
					}

						
					//˫�����ڲ��ز���
					float linerGray = (J + 1 - y2)*((I + 1 - x2)*imageRight.at<uchar>(I, J) + (x2-I)*imageRight.at<uchar>(I+1,J))
						+(y2 - J)*((I+1-x2)*imageRight.at<uchar>(I,J+1)+(x2-I)*imageRight.at<uchar>(I+1,J+1));
					//����У��
					float radioGray = h0 + h1*linerGray;//�õ���Ӧ�Ҷ�

					sumRImg += radioGray;
					sumRImgSquare += radioGray*radioGray;
					//ȷ��ϵ������
					float gy = 0.5*(imageRight.at<uchar>(I, J + 1) - imageRight.at<uchar>(I, J - 1));
					float gx = 0.5*(imageRight.at<uchar>(I + 1, J) - imageRight.at<uchar>(I - 1, J));
					C.at<float>(dimension, 0) = 1; C.at<float>(dimension, 1) = linerGray;
					C.at<float>(dimension, 2) = gx; C.at<float>(dimension, 3) = x2*gx;
					C.at<float>(dimension, 4) = y2*gx; C.at<float>(dimension, 5) = gy;
					C.at<float>(dimension, 6) = x2*gy; C.at<float>(dimension, 7) = y2*gy;				
					//�����ֵ
					L.at<float>(dimension,0) = imageLeft.at<uchar>(m,n)-radioGray;
					dimension =dimension + 1;
					//�󴰿ڼ�Ȩƽ��
					float gyLeft = 0.5*(imageLeft.at<uchar>(m, n + 1) - imageLeft.at<uchar>(m, n - 1));
					float gxLeft = 0.5*(imageLeft.at<uchar>(m + 1, n) - imageLeft.at<uchar>(m - 1, n));
					sumgxSquare += gxLeft*gxLeft;
					sumgySquare += gyLeft*gyLeft;
					sumXgxSquare += m*gxLeft*gxLeft;
					sumYgySquare += n*gyLeft*gyLeft;
					//��Ƭ�Ҷ�������������ϵ��
					sumLImg += imageLeft.at<uchar>(m, n);
					sumLImgSquare += imageLeft.at<uchar>(m, n)*imageLeft.at<uchar>(m, n);
					sumLR += radioGray*imageLeft.at<uchar>(m, n);
				}
			}
			//�������ϵ��
			float coefficent1 = sumLR - sumLImg*sumRImg / (matchsize*matchsize);
			float coefficent2 = sumLImgSquare - sumLImg*sumLImg / (matchsize*matchsize);
			float coefficent3 = sumRImgSquare - sumRImg*sumRImg / (matchsize*matchsize);
			CorrelationCoe = coefficent1 / sqrt(coefficent2*coefficent3);
			//����������ͼ��α��εĲ���
			cv::Mat Nb = C.t()*P*C, Ub = C.t()*P*L;
			cv::Mat parameter = Nb.inv()*Ub;
			float dh0 = parameter.at<float>(0,0); float dh1 = parameter.at<float>(1,0);
			float da0 = parameter.at<float>(2,0); float da1 = parameter.at<float>(3,0);float da2 = parameter.at<float>(4,0); 
			float db0 = parameter.at<float>(5,0); float db1 = parameter.at<float>(6,0);float db2 = parameter.at<float>(7,0);

			a0 = a0 + da0 + a0*da1 + b0*da2;
			a1 = a1 + a1*da1 + b1*da2;
			a2 = a2 + a2*da1 + b2*da2;
			b0 = b0 + db0 + a0*db1 + b0*db2;
			b1 = b1 + a1*db1 + b1*db2;
			b2 = b2 + a2*db1 + b2*db2;
			h0 = h0 + dh0 + h0*dh1;
			h1 = h1 + h1*dh1;
			
			float xt = sumXgxSquare / sumgxSquare;
			float yt = sumYgySquare / sumgySquare;
			xs = a0 + a1*xt + a2*yt;
			ys = b0 + b1*xt + b2*yt;
		}
		cv::Point3f tempPoint;
		tempPoint.x = xs;
		tempPoint.y = ys;
		tempPoint.z = CorrelationCoe;
		featureRightPointLST.push_back(tempPoint);
	}
	lastview(imageLeftRGB, imageRightRGB, featurePointLeft, featureRightPointLST);
	//�������ƥ����ԵĽ�����۲������Ƿ����仯
	std::ofstream outputfile;
	outputfile.open("FeturePointOutput.txt");
	if (outputfile.is_open()) {
		for (size_t i = 0; i < featurePointRight.size(); i++)
		{
			outputfile << featurePointRight.at(i).x << ", " << featurePointRight.at(i).y << ", " << featurePointRight.at(i).z << "��" <<
				featureRightPointLST.at(i).x << ", " << featureRightPointLST.at(i).y << ", " << featureRightPointLST.at(i).z << std::endl;
		}
	}
	outputfile.close();
	return 0;
}


/*ͼ��������ҵ_Ŀ���������*/
//��ñ���ͼ��
cv::Mat calculateLightPattern(cv::Mat img)
{
	cv::Mat pattern;
	//ͨ��ʹ�������ͼ���С�Ĵ��ں˳ߴ�ģ���õ�����ͼ
	blur(img, pattern, cv::Size(img.cols / 3, img.cols / 3));
	return pattern;
}
//�Ƴ�����
cv::Mat removeLight(cv::Mat img, cv::Mat pattern,int tag = 0)
{
	cv::Mat result;
	cv::Mat img32, pattern32;
	img.convertTo(img32, CV_32F);
	pattern.convertTo(pattern32, CV_32F);
	//ͨ������ͼ���Ƴ�����
	if (tag == 1)
		result = 1 - (img32 / pattern32);
	else
		result = 1 - (pattern32 / img32);
	result = result * 255;
	result.convertTo(result, CV_8U);
	/*result = pattern-img;*/
	return result;
}
//����α���ɫ
static cv::Scalar randomColor(cv::RNG& rng)
{
	int icolor = (unsigned)rng;
	return cv::Scalar(
		icolor & 255, (icolor >> 8) & 255, 
		(icolor >> 16) & 255);
}
//Ŀ����
int targetDectation(std::string path)
{
	//��ȡͼ��
	cv::Mat imageGray,imageRGB = cv::imread(path, cv::IMREAD_COLOR);
	if (imageRGB.empty())
	{
		std::cout << "Fail to read image:" << path << std::endl;
		return -1;
	}
	cv::cvtColor(imageRGB, imageGray, cv::COLOR_RGB2GRAY);
	cv::imshow("ImageGray", imageGray);
	cv::waitKey(0);

	//ȥ������
	GaussianBlur(imageGray, imageGray, cv::Size(5, 5), 0, 0);//ʹ��opencv�Դ���˹�˲�Ԥ����
	//��ñ���
	cv::Mat imgPattern = calculateLightPattern(imageGray);
	//�Ƴ�����
	imageGray = removeLight(imageGray, imgPattern);
	cv::imshow("ImageGray", imageGray);
	cv::waitKey(0);
	//��ֵ��
	cv::Mat imageThr;
	cv::threshold(imageGray, imageThr, 50, 255, cv::THRESH_BINARY);
	cv::imshow("thr", imageThr);
	cv::waitKey(0);

	//ͨ����ͨ����㷨�ָ�
	cv::Mat labels, stats, centroids;
	int num_objects = connectedComponentsWithStats(imageThr, labels, stats, centroids);
	//����������Ŀ����
	if (num_objects < 2){
		std::cout << "No objects detected" << std::endl;
		return -1;
	}
	else{
		std::cout << "Number of objects detected: " << num_objects - 1 << std::endl;
	}
	//չʾͼ��ָ���
	cv::Mat output = cv::Mat::zeros(imageThr.rows, imageThr.cols, CV_8UC3);
	cv::RNG randomNumGenerator(0xFFFFFFFF);
	for (int i = 1; i<num_objects; i++)
	{
		cv::Mat mask = labels == i;
		output.setTo(randomColor(randomNumGenerator), mask);
		//��Ŀ�긳����ɫ�Լ������ǩ
		std::stringstream ss;
		ss << "area: " << stats.at<int>(i, cv::CC_STAT_AREA);

		putText(output, ss.str(), centroids.at<cv::Point2d>(i),
			cv::FONT_HERSHEY_SIMPLEX,
			0.4,
			cv::Scalar(255, 255, 255));
	}
	imshow("Result", output);
	cv::waitKey(0);

	//���������㷨����
	/*cv::Mat output = cv::Mat::zeros(imageThr.rows, imageThr.cols, CV_8UC3);
	cv::RNG randomNumGenerator(0xFFFFFFFF);*/

	//std::vector<std::vector<cv::Point> > contours;
	//findContours(imageThr, contours, 
	//	cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
	//output = cv::Mat::zeros(imageThr.rows, imageThr.cols, CV_8UC3);
	//// ������������
	//if (contours.size() == 0)
	//{
	//	std::cout << "No objects detected" << std::endl;
	//	return -1;
	//}
	//else
	//{
	//	std::cout << "Number of objects detected: " << contours.size() << std::endl;
	//}
	//for (int i = 0; i<contours.size(); i++)
	//	drawContours(output, contours, i, randomColor(randomNumGenerator));
	//cv::imshow("Result", output);
	//cv::waitKey(0);

	return 0;
}

//��ȡ��������
std::vector< std::vector<float> > extractFeatures(cv::Mat img, 
std::vector<int>* left = NULL, std::vector<int>* top = NULL)
{
	std::vector< std::vector<float> > output;
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	//findContours��ı�����ͼ���ȱ���
	cv::Mat input = img.clone();
	//������
	findContours(input, contours, hierarchy, CV_RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
	//���������
	if (contours.size() == 0){
		return output;
	}
	cv::RNG rng(0xFFFFFFFF);
	for (int i = 0; i<contours.size(); i++)
	{
		cv::Mat mask = cv::Mat::zeros(img.rows, img.cols, CV_8UC1);
		//ʹ��ֵ1������״���������ͳ�����
		drawContours(mask, contours, i, cv::Scalar(1), cv::FILLED, cv::LINE_8, hierarchy, 1);
		cv::Scalar area_s = sum(mask);
		float area = area_s[0];//�õ���һ���������
		//������������С��ֵ
		float MIN_AREA = 500;
		if (area>MIN_AREA)
		{ 
			cv::RotatedRect r = minAreaRect(contours[i]);
			float width = r.size.width;
			float height = r.size.height;
			//�ڶ��������ݺ��
			float ar = (width<height) ? height / width : width / height;
			std::vector<float> row;
			row.push_back(area);
			row.push_back(ar);
			output.push_back(row);
			if (left != NULL){
				left->push_back((int)r.center.x);
			}
			if (top != NULL){
				top->push_back((int)r.center.y);
			}
		}
	}
	return output;
}


//Ԥ����
cv::Mat preprocessImage(cv::Mat input)
{
	cv::Mat result;
	//ȥ��
	cv::Mat img_noise;
	GaussianBlur(input, img_noise, cv::Size(5, 5), 0, 0);

	//ȥ������
	cv::Mat img_no_light;
	cv::Mat light_pattern = calculateLightPattern(img_noise);
	img_no_light = removeLight(img_noise, light_pattern);

	//��ֵ������
	threshold(img_no_light, result, 30, 255, cv::THRESH_BINARY);

	return result;
}
//��ȡѵ����������ѵ�����������ļ���
bool readFolderAndExtractFeatures(std::string folder, int label, int num_for_test,
	std::vector<float> &trainingData, std::vector<int> &responsesData,
	std::vector<float> &testData, std::vector<float> &testResponsesData)
{
	std::vector<cv::String> image_files;
	cv::glob(folder, image_files);
	int img_index = 0;
	for (int i = 0; i<image_files.size(); i++)
	{
		cv::Mat frame = imread(image_files[i],cv::IMREAD_COLOR);
		if (frame.empty())
		{
			std::cout << "δ�ܶ�ȡͼ��" << std::endl;
			return false;
		}
		cv::cvtColor(frame, frame, cv::COLOR_RGB2GRAY);
		//Ԥ��������
		cv::Mat pre = preprocessImage(frame);
		//��ȡ����
		std::vector<int> pos_top, pos_left;
		std::vector< std::vector<float> > features = extractFeatures(pre);
		for (int i = 0; i< features.size(); i++)
		{
			if (img_index >= num_for_test)
			{
				trainingData.push_back(features[i][0]);
				trainingData.push_back(features[i][1]);
				responsesData.push_back(label);
			}
			else
			{
				testData.push_back(features[i][0]);
				testData.push_back(features[i][1]);
				testResponsesData.push_back((float)label);
			}
		}
		img_index++;
	}
	return true;
}

//���������ռ�ͼ
void plotTrainData(cv::Mat trainData, cv::Mat labels, float *error = NULL)
{
	float area_max, ar_max, area_min, ar_min;
	area_max = ar_max = 0;
	area_min = ar_min = 99999999;
	//��ȡ�����Сֵ��׼������
	for (int i = 0; i< trainData.rows; i++){
		float area = trainData.at<float>(i, 0);
		float ar = trainData.at<float>(i, 1);
		if (area > area_max)
			area_max = area;
		if (ar > ar_max)
			ar_max = ar;
		if (area < area_min)
			area_min = area;
		if (ar < ar_min)
			ar_min = ar;
	}
	//��������ͼƬ�������
	cv::Scalar green(0, 255, 0), blue(255, 0, 0), red(0, 0, 255);
	cv::Mat plot = cv::Mat::zeros(512, 512, CV_8UC3);
	for (int i = 0; i< trainData.rows; i++){
		float area = trainData.at<float>(i, 0);
		float ar = trainData.at<float>(i, 1);
		int x = (int)(512.0f*((area - area_min) / (area_max - area_min)));
		int y = (int)(512.0f*((ar - ar_min) / (ar_max - ar_min)));

		// Get label
		int label = labels.at<int>(i);
		// Set color depend of label
		cv::Scalar color;
		if (label == 0)
			color = green; // ��ĸ
		else if (label == 1)
			color = blue; // Բ��
		else if (label == 2)
			color = red; // ��˿

		circle(plot, cv::Point(x, y), 3, color, -1, 8);
	}

	if (error != NULL){
		std::stringstream ss;
		ss << "Error: " << *error << "\%";
		putText(plot, ss.str().c_str(), cv::Point(20, 512 - 40), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(200, 200, 200), 1, cv::LINE_AA);
	}
	cv::imshow("Plot", plot);
	cv::waitKey(0);
}

//����ģ��
cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();
//ѵ��ģ�Ͳ�����
int trainAndTest()
{
	//�����洢ѵ���Ͳ�������
	std::vector< float > trainingData;
	std::vector< int > responsesData;
	std::vector< float > testData;
	std::vector< float > testResponsesData;
	//��ȡѵ��������
	int num_for_test = 20;//���Լ���Ŀ
	readFolderAndExtractFeatures("./nut/*.pgm", 0, num_for_test, trainingData, responsesData, testData, testResponsesData);
	readFolderAndExtractFeatures("./ring/*.pgm", 1, num_for_test, trainingData, responsesData, testData, testResponsesData);
	readFolderAndExtractFeatures("./screw/*.pgm", 2, num_for_test, trainingData, responsesData, testData, testResponsesData);
	std::cout << "Num of train samples: " << responsesData.size() << std::endl;
	std::cout << "Num of test samples: " << testResponsesData.size() << std::endl;
	//�ϲ���������
	cv::Mat trainingDataMat(trainingData.size() / 2, 2, CV_32FC1, &trainingData[0]);
	cv::Mat responses(responsesData.size(), 1, CV_32SC1, &responsesData[0]);
	cv::Mat testDataMat(testData.size() / 2, 2, CV_32FC1, &testData[0]);
	cv::Mat testResponses(testResponsesData.size(), 1, CV_32FC1, &testResponsesData[0]);

	//���ò���
	svm->setKernel(cv::ml::SVM::KernelTypes::CHI2);
	svm->setType(cv::ml::SVM::Types::C_SVC);
	svm->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER + cv::TermCriteria::EPS, 1000, FLT_EPSILON));
	//ѵ��ģ��
	svm->train(trainingDataMat, cv::ml::ROW_SAMPLE, responses);
	if (testResponsesData.size()>0)
	{
		//����ģ��
		cv::Mat testPredict;
		svm->predict(testDataMat, testPredict);
		cv::Mat errorMat = testPredict != testResponses;
		float error = 100.0f * countNonZero(errorMat) / testResponsesData.size();
		std::cout << "Error: " << error << "\%" << std::endl;
		//���������ռ�
		plotTrainData(trainingDataMat, responses, &error);

	}
	else
	{
		plotTrainData(trainingDataMat, responses);
	}
	return 0;

}
//����ͼ��Ԥ��
int recognitationAndClassify(std::string path)
{
	//��ȡͼ��
	cv::Mat imageGray, imageRGB = cv::imread(path, cv::IMREAD_COLOR);
	if (imageRGB.empty())
	{
		std::cout << "Fail to read image:" << path << std::endl;
		return -1;
	}
	cv::cvtColor(imageRGB, imageGray, cv::COLOR_RGB2GRAY);
	cv::imshow("ImageGray", imageGray);
	cv::waitKey(0);
	cv::Mat img_output = imageRGB.clone();
	//Ԥ����

	//ȥ������
	GaussianBlur(imageGray, imageGray, cv::Size(5, 5), 0, 0);//ʹ��opencv�Դ���˹�˲�Ԥ����
	//��ñ���
	cv::Mat imgPattern = calculateLightPattern(imageGray);
	//�Ƴ�����
	imageGray = removeLight(imageGray, imgPattern);
	//��ֵ��
	cv::Mat pre;
	cv::threshold(imageGray, pre, 50, 255, cv::THRESH_BINARY);
	cv::imshow("Binary image", pre);
	cv::waitKey(0);
	//��ȡ����
	std::vector<int> pos_top, pos_left;
	std::vector< std::vector<float> > features = extractFeatures(pre, &pos_left, &pos_top);
	//ѵ��ģ��

	trainAndTest();
	cv::Scalar green(0, 255, 0), blue(255, 0, 0), red(0, 0, 255);
	for (int i = 0; i< features.size(); i++)
	{
		cv::Mat trainingDataMat(1, 2, CV_32FC1, &features[i][0]);
		float result = svm->predict(trainingDataMat);
		std::stringstream ss;
		cv::Scalar color;
		if (result == 0)
		{
			color = red; 
			ss << "NUT";
		}
		else if (result == 1){
			color = red; 
			ss << "RING";
		}
		else if (result == 2){
			color = red;
			ss << "SCREW";
		}
		putText(img_output,
			ss.str(),
			cv::Point2d(pos_left[i], pos_top[i]),
			cv::FONT_HERSHEY_SIMPLEX,
			0.4,
			color);
	}
	
	cv::imshow("Result", img_output);
	cv::waitKey(0);
}


void triangulation(
	const vector<KeyPoint> &keypoint_1,
	const vector<KeyPoint> &keypoint_2,
	const std::vector<DMatch> &matches,
	const Mat &R, const Mat &t,
	vector<Point3d> &points
	);
/// ��ͼ��
inline cv::Scalar get_color(float depth) {
	float up_th = 50, low_th = 10, th_range = up_th - low_th;
	if (depth > up_th) depth = up_th;
	if (depth < low_th) depth = low_th;
	return cv::Scalar(255 * depth / th_range, 0, 255 * (1 - depth / th_range));
}

//��Ӱ�����γ� 2d-2d��̬����
void find_feature_matches(
	const Mat &img_1, const Mat &img_2,
	std::vector<KeyPoint> &keypoints_1,
	std::vector<KeyPoint> &keypoints_2,
	std::vector<DMatch> &matches);

void pose_estimation_2d2d(
	std::vector<KeyPoint> keypoints_1,
	std::vector<KeyPoint> keypoints_2,
	std::vector<DMatch> matches,
	Mat &R, Mat &t);

// ��������ת�����һ������
Point2d pixel2cam(const Point2d &p, const Mat &K);

void find_feature_matches(const Mat &img_1, const Mat &img_2,
	std::vector<KeyPoint> &keypoints_1,
	std::vector<KeyPoint> &keypoints_2,
	std::vector<DMatch> &matches) {
	//-- ��ʼ��
	Mat descriptors_1, descriptors_2;
	// used in OpenCV3
	Ptr<FeatureDetector> detector = ORB::create();
	Ptr<DescriptorExtractor> descriptor = ORB::create();
	// use this if you are in OpenCV2
	// Ptr<FeatureDetector> detector = FeatureDetector::create ( "ORB" );
	// Ptr<DescriptorExtractor> descriptor = DescriptorExtractor::create ( "ORB" );
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
	//-- ��һ��:��� Oriented FAST �ǵ�λ��
	detector->detect(img_1, keypoints_1);
	detector->detect(img_2, keypoints_2);

	//-- �ڶ���:���ݽǵ�λ�ü��� BRIEF ������
	descriptor->compute(img_1, keypoints_1, descriptors_1);
	descriptor->compute(img_2, keypoints_2, descriptors_2);

	//-- ������:������ͼ���е�BRIEF�����ӽ���ƥ�䣬ʹ�� Hamming ����
	vector<DMatch> match;
	//BFMatcher matcher ( NORM_HAMMING );
	matcher->match(descriptors_1, descriptors_2, match);

	//-- ���Ĳ�:ƥ����ɸѡ
	double min_dist = 10000, max_dist = 0;

	//�ҳ�����ƥ��֮�����С�����������, ���������Ƶĺ�����Ƶ������֮��ľ���
	for (int i = 0; i < descriptors_1.rows; i++) {
		double dist = match[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}

	printf("-- Max dist : %f \n", max_dist);
	printf("-- Min dist : %f \n", min_dist);

	//��������֮��ľ��������������С����ʱ,����Ϊƥ������.����ʱ����С�����ǳ�С,����һ������ֵ30��Ϊ����.
	for (int i = 0; i < descriptors_1.rows; i++) {
		if (match[i].distance <= max(2 * min_dist, 30.0)) {
			matches.push_back(match[i]);
		}
	}
}

void triangulation(
	const vector<KeyPoint> &keypoint_1,
	const vector<KeyPoint> &keypoint_2,
	const std::vector<DMatch> &matches,
	const Mat &R, const Mat &t,
	vector<Point3d> &points) {
	Mat T1 = (Mat_<float>(3, 4) <<
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0);
	Mat T2 = (Mat_<float>(3, 4) <<
		R.at<double>(0, 0), R.at<double>(0, 1), R.at<double>(0, 2), t.at<double>(0, 0),
		R.at<double>(1, 0), R.at<double>(1, 1), R.at<double>(1, 2), t.at<double>(1, 0),
		R.at<double>(2, 0), R.at<double>(2, 1), R.at<double>(2, 2), t.at<double>(2, 0)
		);

	Mat K = (Mat_<double>(3, 3) << 520.9, 0, 325.1, 0, 521.0, 249.7, 0, 0, 1);
	vector<Point2f> pts_1, pts_2;
	for (DMatch m : matches) {
		// ����������ת�����������
		pts_1.push_back(pixel2cam(keypoint_1[m.queryIdx].pt, K));
		pts_2.push_back(pixel2cam(keypoint_2[m.trainIdx].pt, K));
	}

	Mat pts_4d;
	cv::triangulatePoints(T1, T2, pts_1, pts_2, pts_4d);

	// ת���ɷ��������
	for (int i = 0; i < pts_4d.cols; i++) {
		Mat x = pts_4d.col(i);
		x /= x.at<float>(3, 0); // ��һ��
		Point3d p(
			x.at<float>(0, 0),
			x.at<float>(1, 0),
			x.at<float>(2, 0)
			);
		points.push_back(p);
	}
}

Point2d pixel2cam(const Point2d &p, const Mat &K) {
	return Point2d
		(
		(p.x - K.at<double>(0, 2)) / K.at<double>(0, 0),
		(p.y - K.at<double>(1, 2)) / K.at<double>(1, 1)
		);
}

void pose_estimation_2d2d(std::vector<KeyPoint> keypoints_1,
	std::vector<KeyPoint> keypoints_2,
	std::vector<DMatch> matches,
	Mat &R, Mat &t) {
	// ����ڲ�,TUM Freiburg2
	Mat K = (Mat_<double>(3, 3) << 520.9, 0, 325.1, 0, 521.0, 249.7, 0, 0, 1);

	//-- ��ƥ���ת��Ϊvector<Point2f>����ʽ
	vector<Point2f> points1;
	vector<Point2f> points2;

	for (int i = 0; i < (int)matches.size(); i++) {
		points1.push_back(keypoints_1[matches[i].queryIdx].pt);
		points2.push_back(keypoints_2[matches[i].trainIdx].pt);
	}

	//-- �����������
	Mat fundamental_matrix;
	fundamental_matrix = findFundamentalMat(points1, points2, CV_FM_8POINT);
	cout << "fundamental_matrix is " << endl << fundamental_matrix << endl;

	//-- ���㱾�ʾ���
	Point2d principal_point(325.1, 249.7);  //�������, TUM dataset�궨ֵ
	double focal_length = 521;      //�������, TUM dataset�궨ֵ
	Mat essential_matrix;
	essential_matrix = findEssentialMat(points1, points2, focal_length, principal_point);
	cout << "essential_matrix is " << endl << essential_matrix << endl;

	//-- ���㵥Ӧ����
	//-- ���Ǳ����г�������ƽ�棬��Ӧ�������岻��
	Mat homography_matrix;
	homography_matrix = findHomography(points1, points2, RANSAC, 3);
	cout << "homography_matrix is " << endl << homography_matrix << endl;

	//-- �ӱ��ʾ����лָ���ת��ƽ����Ϣ.
	// �˺�������Opencv3���ṩ
	recoverPose(essential_matrix, points1, points2, R, t, focal_length, principal_point);
	cout << "R is " << endl << R << endl;
	cout << "t is " << endl << t << endl;

}




int main(int argc, _TCHAR* argv[])
{
	/*std::string pathLeft = "C:\\Users\\58381\\Desktop\\ComputerVision\\Data\\LOR49.bmp";
	std::string pathRight = "C:\\Users\\58381\\Desktop\\ComputerVision\\Data\\LOR50.bmp";
	std::vector<cv::Point3f> featurePointLeft;
	return siftMatch(pathLeft, pathRight);*/
	/*int kkk = harrisDetector(pathLeft, featurePointLeft);
	int kkk = Moravec(pathLeft, featurePointLeft);
	return kyhMatchingImg(pathLeft, pathRight, featurePointLeft);*/

	/*std::string pathLeft = "C:\\Users\\58381\\Desktop\\ComputerVision\\Data\\LOR49.bmp";
	std::string pathRight = "C:\\Users\\58381\\Desktop\\ComputerVision\\Data\\LOR50.bmp";*/
	/*std::string path = "C:\\Users\\58381\\Desktop\\ComputerVision\\Data\\fen.jpg";
	std::string pathL = "C:\\Users\\58381\\Desktop\\ComputerVision\\Data\\cheku1.jpg";
	std::string pathR = "C:\\Users\\58381\\Desktop\\ComputerVision\\Data\\cheku2.jpg";
	return estimateImageAttitude(pathL, pathR);
	return LineHough(path);
	return LineLSD(path);*/
	/*std::vector<cv::Point3f> featurePointLeft;
	int kkk = harrisDetector(pathLeft,featurePointLeft);
	return kyhMatchingImg(pathLeft, pathRight, featurePointLeft);*/
	/*return kyhMatchImgbyLST(pathLeft, pathRight, featurePointLeft);
	return CannyEdge(pathLeft);
	return  Gradient(pathLeft, KongYuanhang::LAPLACIAN);
	int kkk =  Moravec(pathLeft, featurePointLeft);
	return siftDetector(pathLeft);*/
	/*return siftMatch(pathLeft, pathRight);*/
	////Ŀ����
	//int res = targetDectation(path);
	////Ŀ�����
	/*int sec = recognitationAndClassify(path);*/

	//-- ��ȡͼ��
	std::string pathLeft = "C:\\Users\\58381\\Desktop\\ComputerVision\\Data\\1.png";
	std::string pathRight = "C:\\Users\\58381\\Desktop\\ComputerVision\\Data\\2.png";
	Mat img_1 = imread(pathLeft, CV_LOAD_IMAGE_COLOR);
	Mat img_2 = imread(pathRight, CV_LOAD_IMAGE_COLOR);
	assert(img_1.data && img_2.data && "Can not load images!");

	vector<KeyPoint> keypoints_1, keypoints_2;
	vector<DMatch> matches;
	find_feature_matches(img_1, img_2, keypoints_1, keypoints_2, matches);
	cout << "һ���ҵ���" << matches.size() << "��ƥ���" << endl;

	//-- ��������ͼ����˶�
	Mat R, t;
	pose_estimation_2d2d(keypoints_1, keypoints_2, matches, R, t);

	////-- ��֤E=t^R*scale
	//Mat t_x =
	//	(Mat_<double>(3, 3) << 0, -t.at<double>(2, 0), t.at<double>(1, 0),
	//	t.at<double>(2, 0), 0, -t.at<double>(0, 0),
	//	-t.at<double>(1, 0), t.at<double>(0, 0), 0);

	//cout << "t^R=" << endl << t_x * R << endl;

	////-- ��֤�Լ�Լ��
	//Mat K = (Mat_<double>(3, 3) << 520.9, 0, 325.1, 0, 521.0, 249.7, 0, 0, 1);
	//for (DMatch m : matches) {
	//	Point2d pt1 = pixel2cam(keypoints_1[m.queryIdx].pt, K);
	//	Mat y1 = (Mat_<double>(3, 1) << pt1.x, pt1.y, 1);
	//	Point2d pt2 = pixel2cam(keypoints_2[m.trainIdx].pt, K);
	//	Mat y2 = (Mat_<double>(3, 1) << pt2.x, pt2.y, 1);
	//	Mat d = y2.t() * t_x * R * y1;
	//	cout << "epipolar constraint = " << d << endl;
	//}

	//-- ���ǻ�
	vector<Point3d> points;
	triangulation(keypoints_1, keypoints_2, matches, R, t, points);

	//-- ��֤���ǻ��������������ͶӰ��ϵ
	Mat K = (Mat_<double>(3, 3) << 520.9, 0, 325.1, 0, 521.0, 249.7, 0, 0, 1);
	Mat img1_plot = img_1.clone();
	Mat img2_plot = img_2.clone();
	for (int i = 0; i < matches.size(); i++) {
		// ��һ��ͼ
		float depth1 = points[i].z;
		cout << "depth: " << depth1 << endl;
		Point2d pt1_cam = pixel2cam(keypoints_1[matches[i].queryIdx].pt, K);
		cv::circle(img1_plot, keypoints_1[matches[i].queryIdx].pt, 2, get_color(depth1), 2);

		// �ڶ���ͼ
		Mat pt2_trans = R * (Mat_<double>(3, 1) << points[i].x, points[i].y, points[i].z) + t;
		float depth2 = pt2_trans.at<double>(2, 0);
		cv::circle(img2_plot, keypoints_2[matches[i].trainIdx].pt, 2, get_color(depth2), 2);
	}
	cv::imshow("img 1", img1_plot);
	cv::imshow("img 2", img2_plot);
	cv::waitKey();
	system("pause");
	return 0;
	
}
