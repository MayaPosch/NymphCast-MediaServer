/*
	dashboard_handler.h - 
	
*/


#include <Poco/Net/HTTPRequestHandler.h>

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>


class DashboardHandler: public Poco::Net::HTTPRequestHandler { 
	//
	
public: 
	void handleRequest(Poco::Net::HTTPServerRequest& request, 
								Poco::Net::HTTPServerResponse& response);
};

