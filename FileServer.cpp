#pragma comment(lib, "Ws2_32.lib")
#pragma pack(1)
#define WINDOWS_IGNORE_PACKING_MISMATCH 1
#include  <stdexcept>
#include <iostream>
#include <winsock2.h>
#include "FileServer.h"
#include "RequestHandler.h"
#include "ThreadedRequestHandler.h"
#include "Request.h"

FileServer::FileServer(unsigned int port, /*RequestHandler* request_handler ,*/ WindowsFileManager* file_manager)
{
	this->port = port;
	this->request_handler = new ThreadedRequestHandler([this](SOCKET s) -> void { return this->handleConnection(s); });
	this->file_manager = file_manager;
}

void FileServer::run()
{
	if (this->is_running == true) {
		throw std::runtime_error("Cannot run FileServer: server is already running");
	}

	this->serve();
}

void FileServer::stop()
{
	if (this->is_running == false) {
		throw std::runtime_error("Cannot stop FileServer: server is not running");
	}
	if (this->sock != NULL) {
		closesocket(this->sock);
	}
	WSACleanup();
}

bool FileServer::is_server_running()
{
	return this->is_running;
}

void FileServer::handleConnection(SOCKET clientsocket) 
{
	Request* req = loadRequest(clientsocket);
	
	// DEBUG PRINTING
	std::cout << std::to_string(req->user_id) << std::endl;
	std::cout << std::to_string(req->version) << std::endl;
	std::cout << std::to_string(req->op) << std::endl;
	std::cout << std::to_string(req->name_len) << std::endl;
	std::cout << req->filename << std::endl;

	// TESTER
	//char msg[65546] = { 0 };
	//for (int i = 0; i < 65545; i++)
	//{
	//	char c = 'a' + (i % (122 - 96));
	//	msg[i] = c;
	//}
	//msg[65535] = 0;

	//this->file_manager->saveFile(9999, "test_file.txt", sizeof(msg), msg);
	//
	//char msg2[] = "bbbb";
	//this->file_manager->saveFile(9999, "test_file2.txt", sizeof(msg2), msg2);
	//std::string files_in_dir;
	//this->file_manager->dirList(9999, &files_in_dir);
	//std::cout << files_in_dir << std::endl;
	//char* read;
	//size_t file_size;
	//this->file_manager->getFile(9999, "test_file2.txt", &file_size, &read);
	//std::cout << read << std::endl;
	//this->file_manager->deleteFile(9999, "test_file.txt");
	//this->file_manager->dirList(9999, &files_in_dir);
	//std::cout << files_in_dir << std::endl;

	closesocket(clientsocket);
}


Request* FileServer::loadRequest(SOCKET s)
{
	HeaderHead head = HeaderHead();
	recv(s, (char*)&head, sizeof(HeaderHead), 0);
	
	if (head.op == (unsigned char)RequestOp::dirList)
		return new Request(head);

	HeaderFoot foot = HeaderFoot();
	recv(s, (char*)(&(foot.name_len)), sizeof(foot.name_len), 0);
	foot.filename = new char[foot.name_len];
	recv(s, foot.filename, foot.name_len, 0);
	if (head.op == (unsigned char)RequestOp::getFile || head.op == (unsigned char)RequestOp::deleteFile)
		return new Request(head, foot);

	Payload payload = Payload();
	recv(s, (char*)&(payload.size), sizeof(payload.size), 0);
	payload.payload = new char[payload.size];
	recv(s, payload.payload, payload.size, 0);
	return new Request(head, foot, payload);
}

void FileServer::serve()
{
	WSADATA wsaData;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);

	this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in sa = { 0 };
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(this->port);

	bind(this->sock, (struct sockaddr*)&sa, sizeof(sa));
	listen(this->sock, SOMAXCONN);
	std::cout << "Server listening on port " << this->port << "..." << std::endl;

	while (true)
	{
		std::cout << "Waiting for requests..." << std::endl;
		SOCKET clientsocket = accept(this->sock, NULL, NULL);
		std::cout << "Incoming request" << std::endl;
		this->request_handler->handle(clientsocket);		
	}
}


FileServer::~FileServer()
{
	if (this->is_server_running() == true) {
		this->stop();
	}
	delete this->request_handler;

}