#pragma once
#include <WinSock2.h>
#include <functional>


/*
* RequestHandler is in charge of pooling and handling incoming requests.
* For instance, it can handle them one-by-one, threaded or by priority.
* Actual handling is done by Protocol.
* Implement concrete RequestHandler with a Protocol, and pass them to FileServer in order to handle incoming requests.
*/
class RequestHandler
{
public:
	RequestHandler(std::function<void(SOCKET)>) {};
protected:
	std::function<void(SOCKET)> callback;
public:
	virtual void* handle(SOCKET) = 0;

};

