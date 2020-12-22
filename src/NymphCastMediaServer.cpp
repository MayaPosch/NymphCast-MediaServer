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
#include <nymphcast_client.h>

#include "config_parser.h"
#include "INIReader.h"
#include "sarge.h"
#include "nyansd.h"
#include "mimetype.h"

#include <Poco/Condition.h>
#include <Poco/Thread.h>
using namespace Poco;

#include <filesystem> 		// C++17
namespace fs = std::filesystem;


struct MediaFile {
	std::string section;
	fs::path path;
};


// Global objects.
Condition gCon;
Mutex gMutex;
NymphArray* media_files = new NymphArray();
std::vector<MediaFile> mediaFiles;
NymphCastClient client;
uint32_t handle = 0;
std::string serverip;
// ---


void signal_handler(int signal) {
	gCon.signal();
}


// array getFileList()
NymphMessage* getFileList(int session, NymphMessage* msg, void* data) {
	NymphMessage* returnMsg = msg->getReplyMessage();
	
	returnMsg->setResultValue(media_files);
	return returnMsg;
}


// uint8 playMedia(uint32 id, array receivers)
// Returns: 0 on success. 1 on error.
NymphMessage* playMedia(int session, NymphMessage* msg, void* data) {
	NymphMessage* returnMsg = msg->getReplyMessage();
	
	// Get the file ID to play back and the list of receivers to play it back on.
	uint32_t fileId = ((NymphUint32*) msg->parameters()[0])->getValue();
	std::vector<NymphType*> receivers = ((NymphArray*) msg->parameters()[1])->getValues();
	
	// Obtain the file record using its ID.
	if (fileId > mediaFiles.size()) {
		// Invalid file ID.
		returnMsg->setResultValue(new NymphUint8(1));
		return returnMsg;
	}
	
	MediaFile& mf = mediaFiles[fileId];
	
	// Connect to first receiver in the list, then send the remaining receivers as slave receivers.
	if (receivers.empty()) {
		// No receivers to connect to.
		returnMsg->setResultValue(new NymphUint8(1));
		return returnMsg;
	}
	
	NymphType* sip = 0;
	((NymphStruct*) receivers[0])->getValue("ipv4", sip);
	serverip = ((NymphString*) sip)->getValue();
	receivers.erase(receivers.begin());	// Erase server entry from the list, pass the rest as slaves.
	if (!client.connectServer(serverip, handle)) {
		std::cerr << "Failed to connect to server: " << serverip << std::endl;
		returnMsg->setResultValue(new NymphUint8(1));
		return returnMsg;
	}
	
	std::vector<NymphCastRemote> slaves;
	for (int i = 0; i < receivers.size(); ++i) {
		NymphCastRemote remote;
		NymphType* value = 0;
		((NymphStruct*) receivers[i])->getValue("name", value);
		remote.name = ((NymphString*) value)->getValue();
		((NymphStruct*) receivers[i])->getValue("ipv4", value);
		remote.ipv4 = ((NymphString*) value)->getValue();
		((NymphStruct*) receivers[i])->getValue("ipv6", value);
		remote.ipv6 = ((NymphString*) value)->getValue();
		
		slaves.push_back(remote);
	}
	
	// Set up slaves.
	if (!receivers.empty()) {
		client.addSlaves(handle, slaves);
	}
	
	// Initiate playback. We immediately return here if playback start is successful.
	if (!client.castFile(handle, mf.path.string())) {
		// Playback failed.
		std::cerr << "Playback failed for file: " << mf.path.string() << std::endl;
		returnMsg->setResultValue(new NymphUint8(1));
		return returnMsg;
	}
	
	returnMsg->setResultValue(new NymphUint8(0));
	return returnMsg;
}


// --- LOG FUNCTION ---
void serverLogFunction(int level, std::string logStr) {
	std::cout << level << " - " << logStr << std::endl;
}


int main(int argc, char** argv) {
	// Parse the command line arguments.
	Sarge sarge;
	sarge.setArgument("h", "help", "Get this help message.", false);
	sarge.setArgument("c", "configuration", "Path to configuration file.", true);
	sarge.setArgument("f", "folders", "Path to folder list file.", true);
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
	sarge.getFlag("configuration", config_file);
	
	std::string folders_file = "folders.ini";
	if (!sarge.getFlag("folders", folders_file)) {
		std::cerr << "Folder list file argument is required." << std::endl;
		sarge.printHelp();
		return 0;
	}
	
	// Read in the configuration.
	// TODO: no exposed configuration options yet.
	/* IniParser config;
	if (!config.load(config_file)) {
		std::cerr << "Unable to load configuration file: " << config_file << std::endl;
		return 1;
	} */
	//is_full_screen = config.getValue<bool>("fullscreen", false);
	
	// Obtain the list of directories to scan.
	std::cout << "Scanning directories..." << std::endl;
	INIReader folderList(folders_file);
	if (folderList.ParseError() != 0) {
		std::cerr << "Failed to parse the '" << folders_file << "' file." << std::endl;
		return false;
	}
	
	std::set<std::string> sections = folderList.Sections();
	std::cout << "Found " << sections.size() << " sections in the folder list." << std::endl;
	
	uint32_t index = 0;
	std::set<std::string>::const_iterator it;
	for (it = sections.cbegin(); it != sections.cend(); ++it) {
		// Read out each 'path' string and add the files in the folder (if it exists) to the
		// central list.
		std::cout << "Section: " << *it << std::endl;
		std::string path = folderList.Get(*it, "path", "");
		if (path.empty()) {
			std::cerr << "Path was missing or empty for entry: " << *it << std::endl;
			continue;
		}
		
		// Check that path is a valid directory.
		fs::path dir = path;
		if (!fs::is_directory(dir)) {
			std::cout << "Path is not a valid directory: " << path << ". Skipping." << std::endl;
			continue;
		}
		
		// Iterate through the directory to filter out the media files.
		NymphArray* file_list = new NymphArray();
		for (fs::recursive_directory_iterator next(dir); next != fs::end(next); next++) {
			fs::path fe = next->path();
			//std::cout << "Checking path: " << fe.string() << std::endl;
			if (!fs::is_regular_file(fe)) { 
				std::cout << "Not a regular file." << std::endl;
				continue; 
			}
			
			std::string ext = fe.extension().string();
			ext.erase(0, 1);	// Remove leading '.' character.
			//std::cout << "Checking extension: " << ext << std::endl;
			MediaFile mf;
			if (MimeType::hasExtension(ext)) {
				// Add to media file list.
				std::cout << "Adding file: " << fe << std::endl;
				
				NymphStruct* f = new NymphStruct();
				f->addPair("id", new NymphUint32(index++));
				f->addPair("section", new NymphString(*it));
				f->addPair("filename", new NymphString(fe.filename().string()));
				media_files->addValue(f);
				
				mf.path = fe;
				mf.section = *it;
				mediaFiles.push_back(mf);
			}
		}
	}
	
	// Initialise the server.
	std::cout << "Initialising server...\n";
	long timeout = 5000; // 5 seconds.
	//NymphRemoteClient::init(serverLogFunction, NYMPH_LOG_LEVEL_TRACE, timeout);
	NymphRemoteClient::init(serverLogFunction, NYMPH_LOG_LEVEL_INFO, timeout);
	
	
	// Define all of the RPC methods we want to export for clients.
	std::cout << "Registering methods...\n";
	std::vector<NymphTypes> parameters;
	
	// array getFileList()
	NymphMethod getFileListFunction("getFileList", parameters, NYMPH_ARRAY);
	getFileListFunction.setCallback(getFileList);
	NymphRemoteClient::registerMethod("getFileList", getFileListFunction);
	
	// uint8 playMedia(uint32 id, array receivers)
	parameters.clear();
	parameters.push_back(NYMPH_UINT32);
	parameters.push_back(NYMPH_ARRAY);
	NymphMethod playMediaFunction("playMedia", parameters, NYMPH_UINT8);
	playMediaFunction.setCallback(playMedia);
	NymphRemoteClient::registerMethod("playMedia", playMediaFunction);
	
	// Install signal handler to terminate the server.
	signal(SIGINT, signal_handler);
	
	// Start server on port 4004.
	NymphRemoteClient::start(4004);
	
	// Start NyanSD announcement server.
	NYSD_service sv;
	sv.port = 4004;
	sv.protocol = NYSD_PROTOCOL_TCP;
	sv.service = "nymphcast_mediaserver";
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
