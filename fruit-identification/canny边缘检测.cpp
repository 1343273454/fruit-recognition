#include "canny��Ե���.h"
/// ȫ�ֱ���
int edgeThresh = 1;
int lowThreshold;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;
Mat EdgeCanny::Process(Mat inputImg)
{
	Mat gard_canny;
	/// ʹ�� 3x3�ں˽���
	blur(inputImg, gard_canny, Size(3, 3));
	/// ����Canny����
	Canny(gard_canny, gard_canny, lowThreshold, lowThreshold*ratio, kernel_size);

	return gard_canny;
}
