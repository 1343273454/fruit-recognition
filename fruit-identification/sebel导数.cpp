#include"sebel����.h"
int scale = 1;
int delta = 0;
int ddepth = CV_16S;

Mat EdgeSebel::Process (Mat inputImg)
{
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;
	Mat grad_sebel;
	/// �� X�����ݶ�
	//Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
	Sobel(inputImg, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_x, abs_grad_x);

	/// ��Y�����ݶ�
	//Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
	Sobel(inputImg, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_y, abs_grad_y);

	/// �ϲ��ݶ�(����)
	addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad_sebel);
	return grad_sebel;
}