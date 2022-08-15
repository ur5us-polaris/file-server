#pragma once
#include <functional>
#include "RequestHandler.h"


class ThreadedRequestHandler : public RequestHandler
{
public:
    ThreadedRequestHandler(std::function<void(SOCKET)>);
    void* handle(SOCKET);
};

