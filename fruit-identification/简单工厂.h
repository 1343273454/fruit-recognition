#pragma once
#ifndef _�򵥹���_H_
#define _�򵥹���_H_

#include "ͼ����.h"
#include "�ҶȻ�.h"
#include "��ֵ��.h"
#include "canny��Ե���.h"
#include"sebel����.h"
#include"��ֵ�˲�.h"
#include "ֱ��ͼ���⻯.h"
#include "laplace����.h"
#include "��˹�˲�.h"
#include "�������Ա任.h"

void drawText(Mat & inputImg, string text);

class SimpleFactory
{
public:
	static ImgProcess * Create(const std::string & model);
};


#endif // !<GONGCHANG>