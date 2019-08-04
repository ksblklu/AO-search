// codev2.3.cpp : �������̨Ӧ�ó������ڵ㡣
//

// codev2.1.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "TestSelect.h"
#include <WINSOCK2.H>
#include <iostream>
#define MAXLEN 110
#pragma  comment(lib,"ws2_32.lib")
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
int main()
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return 0;
	}

	SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sclient == INVALID_SOCKET)
	{
		printf("invalid socket !");
		return 0;
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(8888);
	serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("connect error !");
		closesocket(sclient);
		return 0;
	}
	//recieve data from server
	char szBuff[MAXLEN];
	int i, j;
	unsigned char tsp;  // ���Բ�
	unsigned char tst;  // ����ͨ��״̬

	unsigned char* Tselect = NULL;
	unsigned char* Tsequence = NULL;
	unsigned char* Tnum = NULL;
	unsigned char* Fids = NULL;
	unsigned char* Flevels = NULL;

	Tselect = new unsigned char[101];
	Tsequence = new unsigned char[101];
	Tnum = new unsigned char[101];
	Fids = new unsigned char[101];
	Flevels = new unsigned char[101];

	while (true) // ����
	{
		if(recv(sclient, szBuff, MAXLEN, 0) < 0) // �������˶Ͽ����ӣ��˳�����
			break;
		else
		{
			switch (szBuff[0])
			{
			case 1:                               // �����������
				for (i = 0; szBuff[i + 1] > 0; i++) {
					Tselect[i] = szBuff[i + 1];
				}
				Tselect[i] = AE;

				szBuff[1] = test_init(Tselect, Tsequence);
				i = 0;
				while (Tsequence[i] != AE) {
					szBuff[i + 2] = Tsequence[i];
					i++;
				}
				szBuff[i + 2] = AE;

				send(sclient, szBuff, MAXLEN, 0);
				break;
			case 2:                                // ���ݴ�����Բ��Ͳ���״̬�����غ�����������
				tsp = szBuff[1];
				tst = szBuff[2];
				
				szBuff[1] = get_test_next_step(tsp, tst, Tnum);

				i = 0;
				while (Tnum[i] != AE)
				{
					szBuff[i + 2] = Tnum[i];
					i++;
				}
				szBuff[i + 2] = AE;

				send(sclient, szBuff, MAXLEN, 0);

				break;
			case 3:                             // ���ظ�����Ĺ���ģʽ
				j = get_test_result(Fids, Flevels);
				szBuff[1] = j;

				i = 0;
				j = 2;
				while (Fids[i] != AE)
				{
					szBuff[j] = Fids[i];
					szBuff[j+1] = Flevels[i];
					i++;
					j += 2;
				}
				szBuff[j] = AE;

				send(sclient, szBuff, MAXLEN, 0);
				
				break;
			}
		}
	}
	WSACleanup();
	closesocket(sclient);
	return 0;
}

