#include "��ֵ�˲�.h"
Size s = Size(3, 3);
/// Ӧ��ֱ��ͼ���⻯
Mat LvboAvg::Process (Mat inputImg)
{
	Mat lvbo_avg;
	blur(inputImg, lvbo_avg,s, Point(-1, -1));
	return lvbo_avg;
}