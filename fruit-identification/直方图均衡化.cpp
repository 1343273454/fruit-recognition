#include "ֱ��ͼ���⻯.h"

/// Ӧ��ֱ��ͼ���⻯
Mat HistogramAvg::Process (Mat inputImg)
{
	Mat avg;
	equalizeHist(inputImg, avg);
	return avg;
}