// FTPConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include"FTPClient.h"


int main()
{
	FTPClient ftpClient;
	char user[100], pass[100];
	cout << "Username: ";
	gets_s(user);
	cout << "\nPassword: ";
	gets_s(pass);
	ftpClient.connectServer(user, pass);
	ftpClient.disconnect(ftpClient.clientSocket);
    return 0;
}

