#pragma once
#include <winsock2.h>
#include "RequestHandler.h"
#include "WindowsFileManager.h"
#include "Request.h"

#define BUFFER_SIZE 1024

class FileServer
{
private:
	void serve();
	void handleConnection(SOCKET);
	Request* loadRequest(SOCKET s);
	unsigned int port;
	SOCKET sock = NULL;
	bool is_running = false;
	RequestHandler* request_handler;
	WindowsFileManager* file_manager;

public:
	FileServer(unsigned int, /*RequestHandler*,*/ WindowsFileManager*);
	~FileServer();
	void run();
	void stop();
	bool is_server_running();
};

