// FTPConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include"FTPClient.h"


int main()
{
	FTPClient ftpClient;
	ftpClient.help();
	ftpClient.connectServer();
	ftpClient.disconnect(ftpClient.clientSocket);
    return 0;
}

