// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� DEMO2_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// DEMO2_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef DEMO2_EXPORTS
#define DEMO2_API __declspec(dllexport)
#else
#define DEMO2_API __declspec(dllimport)
#endif

// �����Ǵ� demo2.dll ������
class DEMO2_API Cdemo2 {
public:
	Cdemo2(void);
	// TODO:  �ڴ�������ķ�����
};

extern DEMO2_API int ndemo2;

DEMO2_API int fndemo2(void);
