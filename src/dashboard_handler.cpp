/*
	dashboard_handler.cpp - Generates the dashboard view for the NCMS web interface.
*/


#include "dashboard_handler.h"


void DashboardHandler::handleRequest(Poco::Net::HTTPServerRequest& request, 
										Poco::Net::HTTPServerResponse& response) { 
	//Application& app = Application::instance(); 
	//app.logger().information("Request from " + request.clientAddress().toString());
	response.setChunkedTransferEncoding(true);
	response.setContentType("text/html");
	std::ostream& ostr = response.send();
	ostr << "<html><head><title>HTTP Server powered by POCO C++ Libraries</title></head>";
	ostr << "<body>";
	//[...]
	ostr << "</body></html>";
}
