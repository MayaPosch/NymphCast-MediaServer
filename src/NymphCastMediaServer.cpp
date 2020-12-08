/*
	NymphCastFileServer - Server for making media content available via the network.
	
	Revision 0.
	
	Features:
			- Starts server instance with auto-discovery enabled.
			- Allows adding of media file folders.
			- Provides list of available media to connecting clients.
			- Clients can start playback of media content on a specific NC receiver.
			
	Notes:
			- 
			
	2020/12/08, Maya Posch
*/


#include <nymph/nymph.h>

#include "config_parser.h"
#include "sarge.h"

#include "nyansd.h"


#include <Poco/Condition.h>
#include <Poco/Thread.h>

using namespace Poco;




// Global objects.
Condition gCon;
Mutex gMutex;
// ---


void signal_handler(int signal) {
	gCon.signal();
}


// Callback for the connect function.
NymphMessage* connectClient(int session, NymphMessage* msg, void* data) {
	std::cout << "Received message for session: " << session << ", msg ID: " << msg->getMessageId() << "\n";
	
	std::string clientStr = ((NymphString*) msg->parameters()[0])->getValue();
	std::cout << "Client string: " << clientStr << "\n";
	
	//
	
	// Register this client with its ID. Return error if the client ID already exists.
	NymphMessage* returnMsg = msg->getReplyMessage();
	
	returnMsg->setResultValue(new NymphBoolean(true));
	return returnMsg;
}


// Client disconnects from server.
// bool disconnect()
NymphMessage* disconnect(int session, NymphMessage* msg, void* data) {
	
	// Remove the client ID from the list.
	/* std::map<int, CastClient>::iterator it;
	it = clients.find(session);
	if (it != clients.end()) {
		clients.erase(it);
	} */
	
	NymphMessage* returnMsg = msg->getReplyMessage();
	returnMsg->setResultValue(new NymphBoolean(true));
	return returnMsg;
}


// --- LOG FUNCTION ---
void logFunction(int level, std::string logStr) {
	std::cout << level << " - " << logStr << std::endl;
}


int main(int argc, char** argv) {
	// Parse the command line arguments.
	Sarge sarge;
	sarge.setArgument("h", "help", "Get this help message.", false);
	sarge.setArgument("c", "configuration", "Path to configuration file.", true);
	sarge.setArgument("v", "version", "Output the NymphCast version and exit.", false);
	sarge.setDescription("NymphCast Media Server. Shares files with NymphCast clients. More details: http://nyanko.ws/nymphcast.php.");
	sarge.setUsage("nc_mediaserver <options>");
	
	sarge.parseArguments(argc, argv);
	
	if (sarge.exists("help")) {
		sarge.printHelp();
		return 0;
	}
	
	if (sarge.exists("version")) {
		std::cout << "NymphCast version: " << __VERSION << std::endl;
		return 0;
	}
	
	std::string config_file;
	if (!sarge.getFlag("configuration", config_file)) {
		std::cerr << "No configuration file provided in command line arguments." << std::endl;
		return 1;
	}
	
	// Read in the configuration.
	IniParser config;
	if (!config.load(config_file)) {
		std::cerr << "Unable to load configuration file: " << config_file << std::endl;
		return 1;
	}
	
	//is_full_screen = config.getValue<bool>("fullscreen", false);
	
	// Initialise the server.
	std::cout << "Initialising server...\n";
	long timeout = 5000; // 5 seconds.
	//NymphRemoteClient::init(logFunction, NYMPH_LOG_LEVEL_TRACE, timeout);
	NymphRemoteClient::init(logFunction, NYMPH_LOG_LEVEL_INFO, timeout);
	
	
	// Define all of the RPC methods we want to export for clients.
	std::cout << "Registering methods...\n";
	std::vector<NymphTypes> parameters;
	
	// Client connects to server.
	// bool connect(string client_id)
	parameters.push_back(NYMPH_STRING);
	NymphMethod connectFunction("connect", parameters, NYMPH_BOOL);
	connectFunction.setCallback(connectClient);
	NymphRemoteClient::registerMethod("connect", connectFunction);
	
	// Client disconnects from server.
	// bool disconnect()
	parameters.clear();
	NymphMethod disconnectFunction("disconnect", parameters, NYMPH_BOOL);
	disconnectFunction.setCallback(disconnect);
	NymphRemoteClient::registerMethod("disconnect", disconnectFunction);
	
	// Install signal handler to terminate the server.
	signal(SIGINT, signal_handler);
	
	// Start server on port 4004.
	NymphRemoteClient::start(4004);
	
	// Start NyanSD announcement server.
	NYSD_service sv;
	sv.port = 4004;
	sv.protocol = NYSD_PROTOCOL_TCP;
	sv.service = "nymphcast";
	NyanSD::addService(sv);
	
	std::cout << "Starting NyanSD on port 4004 UDP..." << std::endl;
	NyanSD::startListener(4004);
	
	// Wait for the condition to be signalled.
	gMutex.lock();
	gCon.wait(gMutex);
	
	// Clean-up
	NyanSD::stopListener();
	NymphRemoteClient::shutdown();
	
	// Wait before exiting, giving threads time to exit.
	Thread::sleep(2000); // 2 seconds.
	
	return 0;
}
