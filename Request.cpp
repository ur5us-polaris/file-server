#include "Request.h"




Request::Request(HeaderHead& head)
{
	this->user_id = head.user_id;
	this->version = head.version;
	this->op = head.op;

}

Request::Request(HeaderHead& head, HeaderFoot& foot) : Request(head)
{
	this->name_len = foot.name_len;
	this->filename = std::string(foot.filename, this->name_len);
}

Request::Request(HeaderHead& head, HeaderFoot& foot, Payload& payload) : Request(head, foot)
{
	this->size = payload.size;
	this->payload = new char[this->size];
	memcpy(this->payload, payload.payload, this->size);

}

Request::~Request()
{
	if (this->payload != NULL)
	{
		delete this->payload;
	}
}