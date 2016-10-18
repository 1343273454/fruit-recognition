#include"�򵥹���.h"

ImgProcess * SimpleFactory::Create(const std::string & model)
{
	if (model == "��ֵ��")
	{
		return new Value2();
	}
	else if (model == "�ҶȻ�")
	{
		return new Huidu();
	}
	else if (model == "��ֵ�˲�")
	{
		return new LvboAvg();
	}
	else if (model == "sebel����")
	{
		return new EdgeSebel();
	}
	else if (model == "canny��Ե���")
	{
		return new EdgeCanny();
	}
	else if (model == "ֱ��ͼ���⻯")
	{
		return new HistogramAvg();
	}
	else
	{
		return NULL;
	}

}