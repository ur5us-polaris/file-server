#include <iostream>
#include "ThreadedRequestHandler.h"
#include "FileServer.h"
#include "WindowsFileManager.h"

#define SERVER_PORT 8080


int main()
{
    // Create FileManager
    WindowsFileManager* file_manager = new WindowsFileManager();
    std::cout << file_manager->getRoot() << std::endl;
    // Create server
    FileServer* server = new FileServer(8080, file_manager);
    // Run server
    server->run();

    return 0;
}
