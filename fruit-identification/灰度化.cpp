#include "�ҶȻ�.h"

/// Ӧ��ֱ��ͼ���⻯
Mat Huidu::Process(Mat inputImg)
{
	Mat huidu;
	cvtColor(inputImg, huidu, CV_BGR2GRAY);
	return huidu;
}