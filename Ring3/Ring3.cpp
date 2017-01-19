#include <windows.h>  
#include <stdio.h>  
#include "codemsg.h"

#define DosDEVICE L"\\\\DosDevices\\MyDosDevece.com"

BOOL CallDrive(char *ID,char *buffer)
{
	HANDLE device = 0;
	WCHAR ToSend[512];
	int code = -1;
	char ret[1024];
	DWORD bytes;


	device = CreateFile(DosDEVICE, GENERIC_ALL, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (!device || device == INVALID_HANDLE_VALUE)
	{
		printf("������ʧ�ܣ���������������");
		return FALSE;
	}

	//�ж��Ƿ��������-file
	if (!strcmp(ID, "-file"))
	{
		code = INIT_FILE_NAME;
	}
	
	//�ж������Ŀ��ƴ����Ƿ���Ч
	if (code == -1)
	{
		printf("��Ч��ID:%d\r\n", code);
		return FALSE;
	}

	//��ascii��lpbuffer�ַ���תunicode���ַ���
	memset(ToSend, 0, sizeof(ToSend));
	MultiByteToWideChar(CP_ACP, 0, buffer, -1, ToSend, sizeof(ToSend));

	//��ʼ��дͨѶ����
	DeviceIoControl(device,
		CODEMSG(code),
		ToSend,
		(wcslen(ToSend) + 1) * 2,
		&ret,
		sizeof(ret),
		&bytes,
		NULL
		);

	//�ر������ļ�
	CloseHandle(device);
	printf("���!\r\n");
	return TRUE;
}

void main(int argc, char *argv[])
{
	int result = 0;
	if (argv == NULL || argc != 3)
	{
		printf("����Ĳ�������ȷ!!\n");
		return;
	}

	CallDrive(argv[1], argv[2]);







}












