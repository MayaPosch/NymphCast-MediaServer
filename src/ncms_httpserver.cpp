/*
	httpserver.cpp - HTTP server implementation.
	
*/


#include "ncms_httpserver.h"

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include "dashboard_handler.h"
//#include "DataHandler.h"


// 
class RequestHandlerFactory: public Poco::Net::HTTPRequestHandlerFactory {
	//
public: 
	MyRequestHandlerFactory() { return 0; }
	Poco::Net::HTTPRequestHandler* createRequestHandler(
											const Poco::Net::HTTPServerRequest& request) {
		if (request.getURI() == "/") {
			return new DashboardHandler();
		}
		else {
			//return new DataHandler(); 
		}
		
		return 0;
	}
};



// --- START ---
bool NCMS_HttpServer::start(uint16_t port) {
	// 
	//Poco::UInt16 port = 9999;
	Poco::Net::HTTPServerParams* pParams = new Poco::Net::HTTPServerParams;
	pParams->setMaxQueued(100);
	pParams->setMaxThreads(16);
	//Poco::ServerSocket svs(port); // set-up a server socket
	svs.bind(port, true, true);
	Poco::Net::HTTPServer srv(new RequestHandlerFactory(), svs, pParams);
	
	// start the HTTPServer
	srv.start();
	//waitForTerminationRequest();
	
	return true;
}


// --- STOP ---
bool NCMS_HttpServer::stop() {
	// Stop the HTTPServer
	//srv.stop();	// FIXME: implement
	//ss.close();
	
	return true;
}
