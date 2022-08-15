#include <functional>
#include <thread>
#include "RequestHandler.h"
#include "ThreadedRequestHandler.h"



ThreadedRequestHandler::ThreadedRequestHandler(std::function<void(SOCKET)> callback) : RequestHandler(callback) {
	this->callback = callback;
}

void* ThreadedRequestHandler::handle(SOCKET connection_socket)
{
	std::thread t(this->callback, connection_socket);
	t.detach();
	void* a = NULL;
	return a;
}