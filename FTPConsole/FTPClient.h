#include<iostream>
#include<stdio.h>
#include<conio.h>
#include <cstdio>
#include<Winsock2.h>
#include<string>
#pragma comment (lib,"ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


using namespace std;
#pragma once

class FTPClient {	
public:
	SOCKET clientSocket;
	SOCKET dataSocket;
	int connectServer(char user[100],char pass[100]);
	int etablishDataChanel();
	void list(void);
	void printCurrentDirectory(void);
	void changeDirectory(char path[100]);
	int sendBuff(char * buff);
	int recvBuff(char * buff);
	void disconnect(SOCKET clientSocket);
};

int dataLen;
unsigned int port;
char tr[1024], code[3];
char receive[1024], buff[1024],command[1024],cmd[10],path[100];
DWORD threadId;

inline int FTPClient::sendBuff(char * buff) {
	send(clientSocket, buff, strlen(buff), 0);
	recvBuff(receive);
	strncpy(code, receive, 3);
	return atoi(code);
}

inline int FTPClient :: recvBuff(char * buff) {
	dataLen = recv(clientSocket, receive, sizeof(receive), 0);
	if (dataLen == SOCKET_ERROR) {
		cout << "\n loi connect toi server";
		disconnect(clientSocket);
		return 0;
	};
	receive[dataLen] = 0;
	puts(receive);
}

int FTPClient::connectServer(char user[100], char pass[100]) {
	FTPClient ftpClient;
	WORD wversion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(wversion, &wsaData) == SOCKET_ERROR) {
		cout << "\n loi khoi tao winsock";

	};
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//dinh ip
	SOCKADDR_IN addrServer,addrData;
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(21);
	addrServer.sin_addr.s_addr = inet_addr("127.0.0.1");
	//connect 
	if (connect(clientSocket, (SOCKADDR*)&addrServer, sizeof(addrServer)) == SOCKET_ERROR) {
		cout << "\n loi connect toi server";
		ftpClient.disconnect(clientSocket);
		return 0;
	};
	recvBuff(receive);
	
	sprintf(buff, "%s %s\n", "USER", user);
	sendBuff(buff);
	sprintf(buff, "%s %s\n", "PASS", pass);
	puts(buff);
	if (sendBuff(buff) == 230) {
		cout << "Logged on \n" << endl;
	}
	else return 0;
	sprintf(buff, "%s\n", "TYPE I");
	sendBuff(buff);
	while (1) {
		cout << "Enter your cmd:" << endl;
		gets_s(command);
		sscanf(command, "%s %s", cmd, path);
		sprintf(cmd, "%s\n", cmd);
		if (strcmp(cmd,"LIST\n")==0) {
			list();
			closesocket(dataSocket);
		}
		else if (strcmp(cmd, "PWD\n") == 0) {
			printCurrentDirectory();
		}
		else if (strcmp(cmd, "CWD\n") == 0) {
			changeDirectory(command);
		}
		else if (strcmp(cmd, "RNFR\n")==0) {
			sprintf(command, "%s\n", command);
			if (sendBuff(command) == 350) {
				cout << "Rename to: ";
				gets_s(path);
				sprintf(command, "%s %s\n", "RNTO",path);
				sendBuff(command);
			};
		}
		else if (strcmp(cmd, "MKD\n") == 0) {
			sprintf(command, "%s\n", command);
			sendBuff(command);
		}
		else if (strcmp(cmd, "DELE\n") == 0) {
			sprintf(command, "%s\n", command);
			sendBuff(command);
		}
		else if (strcmp(cmd, "RMD\n") == 0) {
			sprintf(command, "%s\n", command);
			sendBuff(command);
		}
		else {
			sendBuff(buff);
		}
	}
	system("pause");
}

int FTPClient::etablishDataChanel() {
	sprintf(buff, "%s\n", "PASV");
	if (sendBuff(buff) == 227) {
		//tien hanh boc tach de lay port
		string receive1(receive);
		int pos1 = receive1.find('(');
		int pos2 = receive1.find(')');
		string c1 = receive1.substr(pos1 + 1, pos2 - pos1 - 1);
		for (int i = 1; i <= 4; i++) {
			pos1 = c1.find(',');
			c1 = c1.substr(pos1 + 1, c1.length() - pos1 - 1);
		}
		pos1 = c1.find(',');
		int h1 = atoi(c1.substr(0, pos1).c_str());
		int h2 = atoi(c1.substr(pos1 + 1, c1.length() - pos1 - 1).c_str());
		port = h1 * 256 + h2;
	}
	//sau khi lay dc port tao socket data connection
	dataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//dinh ip
	SOCKADDR_IN addrServer2;
	addrServer2.sin_family = AF_INET;
	addrServer2.sin_port = htons(port);
	addrServer2.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (connect(dataSocket, (SOCKADDR*)&addrServer2, sizeof(addrServer2)) == SOCKET_ERROR) {
		cout << "\n khong tao duoc socket data connection";
		WSACleanup();
		return 0;
	}
}

void FTPClient::list(void) {
	//CreateThread(0, 0, Thread, this, 0, 0);
	etablishDataChanel();
	sprintf(buff, "%s\n", "MLSD");
	if (sendBuff(buff) == 150) {
		recvBuff(receive);
	};
	int iResult = recv(dataSocket, receive, 1024, 0);
	receive[iResult] = 0;
	cout << "directory:\n"<< receive;
}

void FTPClient::printCurrentDirectory(void) {
	sprintf(buff, "%s\n", "PWD");
	sendBuff(buff);
}

void FTPClient::changeDirectory(char path[100]) {
	sprintf(buff, "%s\n", path);
	sendBuff(buff);
}

inline void FTPClient::disconnect(SOCKET clientSocket)
{
	closesocket(clientSocket);
	closesocket(dataSocket);
	WSACleanup();
	cout << "\nDisconnected...";
	system("pause");
}

