/*
	httpserver.h - Header for the HTTP Server.
	
*/


#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServerParams.h>


class NCMS_HttpServer {
	Poco::Net::ServerSocket svs;
	
public:
	bool start(uint16_t port);
	bool stop();
};