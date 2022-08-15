#pragma once
#include <cstdint>
#include <string>


enum class RequestOp {saveFile = 100, getFile = 200, deleteFile = 201, dirList = 202};

struct HeaderHead {
	uint32_t user_id = 0;
	unsigned char version;
	unsigned char op;
};

struct HeaderFoot {
	uint16_t name_len = 0;
	char* filename = NULL;
};

struct Payload {
	uint32_t size;
	char* payload = NULL;
};

class Request
{
public:
	Request(HeaderHead&);
	Request(HeaderHead&, HeaderFoot&);
	Request(HeaderHead&, HeaderFoot&, Payload&);
	~Request();
	//private:
	uint32_t user_id;
	unsigned char version;
	unsigned char op;
	uint16_t name_len = 0;
	std::string filename;
	uint32_t size = 0;
	char* payload = NULL;
};
