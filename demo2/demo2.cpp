// demo2.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "demo2.h"


// ���ǵ���������һ��ʾ��
DEMO2_API int ndemo2=0;

// ���ǵ���������һ��ʾ����
DEMO2_API int fndemo2(void)
{
    return 42;
}

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� demo2.h
Cdemo2::Cdemo2()
{
    return;
}
