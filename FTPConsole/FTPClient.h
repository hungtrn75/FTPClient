#undef UNICODE

#include<iostream>
#include<stdio.h>
#include<conio.h>
#include<cstdio>
#include<Winsock2.h>
#include<windows.h>
#include<string>
#include <stdlib.h>
#pragma comment (lib,"ws2_32.lib")


#define MAX_BUFF 10240
#define DEFAULT_BUFLEN 0x10240	
#define COMMAND(lcmd) strcmp(cmd, lcmd)==0

using namespace std;
#pragma once
class DownloadInformation {
public:
	char *destination;
	int start;
	int numberBlocks;
	SOCKET dataSocket;
	class FTPClient *client;

	DownloadInformation(char *destination, int start, int numberBlocks, SOCKET dataSocket, class FTPClient *client) : destination(destination),
		start(start), numberBlocks(numberBlocks), dataSocket(dataSocket), client(client) {}
};

class FTPClient {
public:
	SOCKET clientSocket;
	SOCKET dataSocket;
	static const int BLOCK_SIZE = 32;
	void help();
	int connectServer();
	int etablishDataChanel(SOCKET &dataSocket);
	void list(void);
	void printCurrentDirectory(void);
	void changeDirectory(char path[100]);
	long getFileSize(char filename[100]);
	void downloadVer1(char path[100], long fileSize);
	static DWORD WINAPI downloadThread(const DownloadInformation * const inf);
	void downloadSegment(HANDLE *h, DownloadInformation **inf, int index, char *source);
	void downloadVer2(char *destination, char *source, const int numberThread);
	void uploadFile(char path[100]);
	int sendBuff(const char * buff);
	int recvBuff(char * buff);
	void disconnect(SOCKET clientSocket);
};

int dataLen;
unsigned int port;
char user[100], pass[100];
char tr[1024], code[3];
char sendCmd[100];
char receive[MAX_BUFF], buff[1024], command[1024], cmd[10], path[100], buffFile[MAX_BUFF], numberThreads[2];
char pathDir[524];
char localDir[524];
char fileP[100];

//****SEND and RECV****//
inline int FTPClient::sendBuff(const char * buff) {
	send(clientSocket, buff, strlen(buff), 0);
	return recvBuff(receive);
}

//****RECV****//
inline int FTPClient::recvBuff(char * buff) {
	dataLen = recv(clientSocket, receive, sizeof(receive), 0);
	if (dataLen == SOCKET_ERROR) {
		cout << "\n loi connect toi server";
		disconnect(clientSocket);
		return 0;
	};
	receive[dataLen] = 0;
	cout << "Server: " << receive;
	strncpy(code, receive, 3);
	return atoi(code);
}

int FTPClient::connectServer() {
	FTPClient ftpClient;
	WORD wversion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(wversion, &wsaData) == SOCKET_ERROR) {
		cout << "\n loi khoi tao winsock";

	};
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addrServer, addrData;
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(21);
	addrServer.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (connect(clientSocket, (SOCKADDR*)&addrServer, sizeof(addrServer)) == SOCKET_ERROR) {
		cout << "\nCan't to server...";
		ftpClient.disconnect(clientSocket);
		return 0;
	};
	recvBuff(receive);
	bool login = false;
	while (login == false) {
		cout << "Username: ";
		gets_s(user);
		cout << "Password: ";
		gets_s(pass);

		sprintf(buff, "%s %s\n", "USER", user);
		sendBuff(buff);
		sprintf(buff, "%s %s\n", "PASS", pass);
		puts(buff);

		if (sendBuff(buff) == 230) {
			cout << "Logged on \n" << endl;
			login = true;
		}
	}
	sprintf(buff, "%s\n", "TYPE I");
	sendBuff(buff);
	cout << "Local directory: ";
	gets_s(localDir);
	while (1) {
		cout << "\nftp>";
		gets_s(command);
		sscanf(command, "%s %s", cmd, path);
		if (COMMAND("ls")) {
			list();
			closesocket(dataSocket);
		}
		else if (COMMAND("pwd")) {
			printCurrentDirectory();
		}
		else if (COMMAND("cd")) {
			changeDirectory(&command[3]);
		}
		else if (COMMAND("rename")) {
			sprintf(sendCmd, "%s %s\n", "RNFR", &command[7]);
			if (sendBuff(sendCmd) == 350) {
				cout << "Rename to: ";
				gets_s(path);
				sprintf(sendCmd, "%s %s\n", "RNTO", path);
				sendBuff(sendCmd);
			};
		}
		else if (COMMAND("mkdir")) {
			sprintf(sendCmd, "%s %s\n", "MKD", &command[6]);
			sendBuff(sendCmd);
		}
		else if (COMMAND("del")) {
			sprintf(sendCmd, "%s %s\n", "DELE", &command[4]);
			sendBuff(sendCmd);
		}
		else if (COMMAND("rmdir")) {
			sprintf(sendCmd, "%s %s\n", "RMD", &command[6]);
			sendBuff(sendCmd);
		}
		else if (COMMAND("get")) {
			sprintf(pathDir, "%s\\%s", localDir, &command[4]);
			long sizeFile = getFileSize(&command[4]);
			if (sizeFile > 0) {
				downloadVer1(pathDir, sizeFile);
			}
		}
		else if (COMMAND("put")) {
			sprintf(pathDir, "%s\\%s", localDir, &command[4]);
			uploadFile(pathDir);
		}
		else if (COMMAND("size")) {
			cout << "SIZE: " << getFileSize(&command[5]) << endl;
		}
		else if (COMMAND("sget")) {
			cout << "Number threads: ";
			gets_s(numberThreads);
			downloadVer2(pathDir, &command[5], atoi(numberThreads));
		}
		else if (COMMAND("quit")) {
			return 0;
		}
		else if (COMMAND("change")) {
			cout << "Local directory: ";
			gets_s(localDir);
		}
		else {
			cout << "\'" << command << "\'" << " is not recognized as an internal or external command\n";
		}
	}
	system("pause");
}

int FTPClient::etablishDataChanel(SOCKET &dataSocket) {
	sprintf(buff, "%s\n", "PASV");
	if (sendBuff(buff) == 227) {
		//get port
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

		dataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		SOCKADDR_IN addrServer2;
		addrServer2.sin_family = AF_INET;
		addrServer2.sin_port = htons(port);
		addrServer2.sin_addr.s_addr = inet_addr("127.0.0.1");
		if (connect(dataSocket, (SOCKADDR*)&addrServer2, sizeof(addrServer2)) == SOCKET_ERROR) {
			cout << "\Can't create socket data connection";
			WSACleanup();
			return 0;
		}
		return 1;
	}	
}

void FTPClient::list(void) {
	/*Thiết lập data chanel*/
	if (etablishDataChanel(dataSocket) == 1) {
		if (sendBuff("MLSD\n") == 150) {
			recvBuff(receive);
		};
		int iResult = recv(dataSocket, receive, MAX_BUFF, 0);
		receive[iResult] = 0;
		cout << "directory:\n" << receive;
	}
}

void FTPClient::printCurrentDirectory(void) {
	sendBuff("PWD\n");
}

void FTPClient::changeDirectory(char path[100]) {
	sprintf(buff, "%s %s\n", "CWD", path);
	sendBuff(buff);
}

long FTPClient::getFileSize(char fileName[100]) {
	char getSize[100];
	sprintf(getSize, "%s %s\n", "SIZE", fileName);
	sendBuff(getSize);
	long result = atol(&receive[4]);
	return result;
}

void FTPClient::downloadVer1(char pathDir[100], long fileSize) {
	cout << "Save in: " << pathDir << endl;
	FILE *file = fopen(pathDir, "wb");
	if (!file) {
		cout << "File unavaible\n";
		closesocket(dataSocket);
	}
	else {
		/*Thiết lập data chanel*/
		if (etablishDataChanel(dataSocket) == 1) {
			sprintf(sendCmd, "%s %s\n", "RETR", &command[4]);
			if (sendBuff(sendCmd) == 150) {

				int n = 0, sum = 0;
				char *recvbuf = new char[MAX_BUFF];

				while (sum < fileSize) {
					n = recv(dataSocket, recvbuf, MAX_BUFF, 0);
					fwrite(recvbuf, sizeof(char), n, file);
					sum += n;
				}
				recvBuff(receive);
				closesocket(dataSocket);
				fclose(file);
			};
		}
	}
}

void FTPClient::uploadFile(char pathDir[100]) {
	FILE *file = fopen(pathDir, "rb");
	if (!file) {
		cout << "File unavaible\n";
		closesocket(dataSocket);
	}
	else {
		/*Thiết lập data chanel*/
		if (etablishDataChanel(dataSocket) == 1) {
			sprintf(sendCmd, "%s %s\n", "STOR", &command[4]);
			if (sendBuff(sendCmd) == 150) {
				int n;
				while ((n = fread(buffFile, sizeof(char), MAX_BUFF, file)) > 0) {
					send(dataSocket, buffFile, n, 0);
				}
				closesocket(dataSocket);
				fclose(file);
				recvBuff(receive);
			}
		}
	}
}

DWORD WINAPI FTPClient::downloadThread(const DownloadInformation * const inf) {
	HANDLE file = CreateFile(inf->destination, GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_ALWAYS, 0, 0);
	if (file == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();
		printf("err %d\n", err);
		return 1;
	}
	SetFilePointer(file, inf->start, NULL, FILE_BEGIN);
	int n = 0, count = 0;
	char *recvbuf = new char[FTPClient::BLOCK_SIZE];

	while (count < inf->numberBlocks) {
		n = recv(inf->dataSocket, recvbuf, FTPClient::BLOCK_SIZE, 0);
		WriteFile(file, recvbuf, n, NULL, NULL);
		count++;
	}
	delete[] recvbuf;
	closesocket(inf->dataSocket);
	CloseHandle(file);
	return 0;
}


void FTPClient::downloadSegment(HANDLE *h, DownloadInformation **inf, int index, char* source) {
	SOCKET dataSocket = INVALID_SOCKET;
	/*Thiết lập data chanel*/
	if (etablishDataChanel(dataSocket) == 1) {
		if (dataSocket != INVALID_SOCKET) {
			inf[index]->dataSocket = dataSocket;

			char *recvbuf = new char[DEFAULT_BUFLEN];
			char *command = new char[strlen(source) + 6];

			sprintf(command, "%s %d\n", "REST", inf[index]->start);
			if (sendBuff(command) == 350) {

				sprintf(command, "%s %s\n", "RETR", source);
				if (sendBuff(command) == 150) {

					DWORD id;
					h[index] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)downloadThread, inf[index], FALSE, &id);
					recvBuff(receive);
				};
			}
		};
	}
}

void FTPClient::downloadVer2(char *destination, char *source, const int numberThread)
{
	sprintf(pathDir, "%s\\%s", localDir, source);
	int numberBlocks = getFileSize(source) / BLOCK_SIZE + 1;
	if (numberBlocks > 1) {
		HANDLE *h = new HANDLE[numberThread];
		DownloadInformation **inf = new DownloadInformation*[numberThread];
		int startPosition = 0, i = 0;
		int numberBlocksPerThread = numberBlocks / numberThread;
		int offset = numberBlocksPerThread * BLOCK_SIZE;

		for (i = 0; i < numberThread - 1; i++) {
			inf[i] = new DownloadInformation(destination, startPosition, numberBlocksPerThread, INVALID_SOCKET, this);
			downloadSegment(h, inf, i, source);
			startPosition += offset;
		}

		numberBlocksPerThread = numberBlocks - numberBlocksPerThread * (numberThread - 1);
		if (numberBlocksPerThread > 0) {
			inf[i] = new DownloadInformation(destination, startPosition, numberBlocksPerThread, INVALID_SOCKET, this);
			downloadSegment(h, inf, i, source);
		};

		WaitForMultipleObjects(numberThread, h, TRUE, INFINITE);
		for (int i = 0; i < numberThread; i++) {
			delete inf[i];
		}
		delete[] inf;
		for (int i = 0; i < numberThread; i++) {
			if (h[i] != NULL) CloseHandle(h[i]);
		}
		delete[] h;
		cout << "Download completed" << endl;
	}
}

inline void FTPClient::disconnect(SOCKET clientSocket)
{
	closesocket(clientSocket);
	closesocket(dataSocket);
	WSACleanup();
	cout << "\nDisconnected...\n";
	system("pause");
}

void FTPClient::help() {
	cout << "---------------------------------------FTPClient-------------------------------------------\r\n";
	cout << "-------------------------------------------------------------------------------------------\r\n";
	cout << "Command\r\n\n";
	cout << "ls    : List directory\r\n";
	cout << "pwd   : Directory location\r\n";
	cout << "cd    : Change directory\r\n";
	cout << "mkdir : Make directory\r\n";
	cout << "rmdir : Remove directory\r\n";
	cout << "rename: Rename file\r\n";
	cout << "del   : Delete file\r\n";
	cout << "size  : Size of file\r\n";
	cout << "get   : Get/download file\r\n";
	cout << "sget  : Segmented download\r\n";
	cout << "put   : Put/upload file\r\n";
	cout << "quit  : Disconnection\r\n";
	cout << "-------------------------------------------------------------------------------------------\r\n";
	cout << "-------------------------------------------------------------------------------------------\r\n";
}