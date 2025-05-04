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

#include "types.h"

#include <nymph/nymph.h>
#include <nymphcast_client.h>

#include "config_parser.h"
#include "sarge.h"
#include "nyansd.h"
#include "mimetype.h"

#include <Poco/Condition.h>
#include <Poco/Thread.h>
using namespace Poco;

#include <map>
#include <csignal>
#include <fstream>
#include <mutex>
#include <filesystem> 		// C++17
namespace fs = std::filesystem;


// Types:
// 0	Folder
// 1	File
/* struct MediaFolderEntry {
	std::string section;
	std::string path;	// Full folder path.
	uint32_t id;
	std::vector<MediaFolder> folders;
	std::vector<MediaFile*> files;
}; */


struct RemoteServerStatus {
	NymphPlaybackStatus status;
	bool init = false;	// True after first connection update.
	bool list = false;	// Do we have a playlist?
	std::vector<std::string> playlist;
	uint32_t playlistId = 0;
};


// Global objects.
Condition gCon;
Mutex gMutex;
std::vector<MediaFile> mediaFiles;
std::vector<NymphType*>* tArr = 0;
bool mediaFilesCurrent = false;
/* std::vector<MediaFolder> audioFolders;
std::vector<MediaFolder> videoFolders;
std::vector<MediaFolder> imageFolders;
std::vector<MediaFolder> playlistFolders; */
std::vector<GameSystem> gameSystems;
static NymphCastClient client;
uint32_t handle = 0;
std::string serverip;
std::map<uint32_t, bool> receiverStatus;
std::map<uint32_t, RemoteServerStatus> remoteStatus;
std::mutex remoteMutex;
std::vector<Poco::DirectoryWatcher*> dirwatchers;
// ---


void signal_handler(int signal) {
	gCon.signal();
}


// array getFileList()
NymphMessage* getFileList(int session, NymphMessage* msg, void* data) {
	NymphMessage* returnMsg = msg->getReplyMessage();
	
	if (!mediaFilesCurrent) {
		// We need to update the serialised list of media files.
		// First check whether we need to delete an old list.
		if (tArr != 0) { delete tArr; tArr = 0; }
		
		// Copy values from the media file array into the new array.
		tArr = new std::vector<NymphType*>();
		for (uint32_t i = 0; i < mediaFiles.size(); ++i) {
			std::map<std::string, NymphPair>* pairs = new std::map<std::string, NymphPair>;
			
			NymphPair pair;
			std::string* key = new std::string("id");
			pair.key = new NymphType(key, true);
			pair.value = new NymphType(i);
			pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
		
			key = new std::string("section");
			pair.key = new NymphType(key, true);
			pair.value = new NymphType(&mediaFiles[i].section);
			pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
			
			key = new std::string("filename");
			pair.key = new NymphType(key, true);
			pair.value = new NymphType(&mediaFiles[i].filename);
			pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
			
			key = new std::string("rel_path");
			pair.key = new NymphType(key, true);
			pair.value = new NymphType(&mediaFiles[i].rel_path);
			pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
			
			key = new std::string("type");
			pair.key = new NymphType(key, true);
			pair.value = new NymphType(mediaFiles[i].type);
			pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
			
			tArr->push_back(new NymphType(pairs, true));
		}
	}
	
	returnMsg->setResultValue(new NymphType(tArr, true));
	msg->discard();
	return returnMsg;
}


// uint8 playMedia(uint32 id, string path, array receivers)
// Returns: 0 on success. 1 on outdated client list, 2 on error.
NymphMessage* playMedia(int session, NymphMessage* msg, void* data) {
	NymphMessage* returnMsg = msg->getReplyMessage();
	
	// Get the file ID to play back and the list of receivers to play it back on.
	uint32_t fileId = msg->parameters()[0]->getUint32();
	std::string filePath = msg->parameters()[1]->getString();
	std::vector<NymphType*>* receivers = msg->parameters()[2]->getArray();
	
	// Obtain the file record using its ID.
	if (fileId > mediaFiles.size()) {
		// Invalid file ID.
		returnMsg->setResultValue(new NymphType((uint8_t) 2));
		msg->discard();
		return returnMsg;
	}
	
	MediaFile& mf = mediaFiles[fileId];
	
	// TODO: Compare the paths of the fileId in our file list with the provided file path.
	//if (filePath == mf.
	
	// Connect to first receiver in the list, then send the remaining receivers as slave receivers.
	if (receivers->empty()) {
		// No receivers to connect to.
		returnMsg->setResultValue(new NymphType((uint8_t) 1));
		msg->discard();
		return returnMsg;
	}
	
	// Lock access to the remotes map for synchronisation reasons.
	remoteMutex.lock();
	
	NymphType* sip = 0;
	(*receivers)[0]->getStructValue("ipv4", sip);
	serverip = sip->getString();
	receivers->erase(receivers->begin());	// Erase server entry from the list, pass the rest as slaves.
	if (!client.connectServer(serverip, 0, handle)) {
		std::cerr << "Failed to connect to server: " << serverip << std::endl;
		returnMsg->setResultValue(new NymphType((uint8_t) 1));
		msg->discard();
		remoteMutex.unlock();
		return returnMsg;
	}
	
	// Create new entry for this remote if we don't have it registered yet.
	// TODO: handle case where we're already playing on this remote.
	if (remoteStatus.find(handle) == remoteStatus.end()) {
		// Insert new entry.
		RemoteServerStatus rs;
		rs.list = false;
		rs.playlistId = 0;
		remoteStatus.insert(std::pair<uint32_t, RemoteServerStatus>(handle, rs));
	}
	
	remoteMutex.unlock();
	
	std::vector<NymphCastRemote> slaves;
	for (int i = 0; i < receivers->size(); ++i) {
		NymphCastRemote remote;
		NymphType* value = 0;
		(*receivers)[i]->getStructValue("name", value);
		remote.name = value->getString();
		(*receivers)[i]->getStructValue("ipv4", value);
		remote.ipv4 = value->getString();
		(*receivers)[i]->getStructValue("ipv6", value);
		remote.ipv6 = value->getString();
		
		slaves.push_back(remote);
	}
	
	// Set up slaves.
	if (!receivers->empty()) {
		client.addSlaves(handle, slaves);
	}
	
	// If the item is a playlist, we want to play back each individual item.
	if (mf.type == 3) {
		// Get the remote status reference.
		remoteMutex.lock();
		std::map<uint32_t, RemoteServerStatus>::iterator rit;
		rit = remoteStatus.find(handle);
		if (rit == remoteStatus.end()) {
			// Failed to find remote somehow. Panic.
			returnMsg->setResultValue(new NymphType((uint8_t) 1));
			msg->discard();
			remoteMutex.unlock();
			return returnMsg;
		}
		
		// Clear the playlist.
		std::vector<std::string>& playlist = rit->second.playlist;
		playlist.clear();
		rit->second.playlistId = 0;
		rit->second.list = true;
		
		// Open playlist file, try to parse it into a local playlist for use later.
		std::ifstream pl(mf.path.string());
		if (!pl.is_open()) {
			std::cerr << "Failed to open playlist file." << std::endl;
			returnMsg->setResultValue(new NymphType((uint8_t) 1));
			msg->discard();
			remoteMutex.unlock();
			return returnMsg;
		}
		
		// Parse file.
		std::string line;
		while (std::getline(pl, line)) {
			// Skip extended M3U lines as we don't need them.
			if (line[0] == '#') { continue; }
			
			// Check that the file exists.
			fs::path mf = fs::u8path(line);	// Convert from Unicode for cross-platform support.
			if (!fs::is_regular_file(mf)) {
				std::cerr << "Skipping non-existing playlist file: " << line << std::endl;
				continue;
			}
			
			playlist.push_back(line);
		}
		
		pl.close();
		remoteMutex.unlock();
		
		if (playlist.empty()) {
			// Empty playlist. Abort.
			std::cerr << "Found empty playlist. Aborting playback." << std::endl;
			returnMsg->setResultValue(new NymphType((uint8_t) 1));
			msg->discard();
			return returnMsg;
		}
		
		// Play back first file.
		if (!client.castFile(handle, playlist[0])) {
			// Playback failed.
			std::cerr << "Playback failed for file: " << playlist[0] << std::endl;
			returnMsg->setResultValue(new NymphType((uint8_t) 1));
			msg->discard();
			return returnMsg;
		}
		
		rit->second.playlistId++;
	}
	else {
		// Initiate playback. We immediately return here if playback start is successful.
		if (!client.castFile(handle, mf.path.string())) {
			// Playback failed.
			std::cerr << "Playback failed for file: " << mf.path.string() << std::endl;
			returnMsg->setResultValue(new NymphType((uint8_t) 1));
			msg->discard();
			return returnMsg;
		}
	}
	
	returnMsg->setResultValue(new NymphType((uint8_t) 0));
	msg->discard();
	return returnMsg;
}


// array getGameList()
NymphMessage* getGameList(int session, NymphMessage* msg, void* data) {
	NymphMessage* returnMsg = msg->getReplyMessage();
	
	// Copy values from the game systems array into the new array.
	std::vector<NymphType*>* tArr = new std::vector<NymphType*>();
	for (uint32_t i = 0; i < mediaFiles.size(); ++i) {
		std::map<std::string, NymphPair>* pairs = new std::map<std::string, NymphPair>;
		
		// 
		
		NymphPair pair;
		std::string* key = new std::string("id");
		pair.key = new NymphType(key, true);
		pair.value = new NymphType(i);
		pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
	
		key = new std::string("section");
		pair.key = new NymphType(key, true);
		pair.value = new NymphType(&mediaFiles[i].section);
		pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
		
		key = new std::string("filename");
		pair.key = new NymphType(key, true);
		pair.value = new NymphType(&mediaFiles[i].filename);
		pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
		
		key = new std::string("type");
		pair.key = new NymphType(key, true);
		pair.value = new NymphType(mediaFiles[i].type);
		pairs->insert(std::pair<std::string, NymphPair>(*key, pair));
		
		tArr->push_back(new NymphType(pairs, true));
	}
	
	returnMsg->setResultValue(new NymphType(tArr, true));
	msg->discard();
	return returnMsg;
}


// --- LOG FUNCTION ---
void serverLogFunction(int level, std::string logStr) {
	std::cout << level << " - " << logStr << std::endl;
}


// --- STATUS UPDATE CALLBACK ---
void statusUpdateCallback(uint32_t handle, NymphPlaybackStatus status) {
	// Debug
	std::cout << "Received remote status update. Status: " << status.status << std::endl;
	
	// If we get a 'stopped' status from the remote while we're playing, that means playback has stopped.
	// In this case we have to shutdown communications for the provided handle.
	if (status.status == NYMPH_PLAYBACK_STATUS_PLAYING) {
		// Set as playing.
		if (receiverStatus.find(handle) == receiverStatus.end()) {
			// Insert new entry.
			receiverStatus.insert(std::pair<uint32_t, bool>(handle, true));
		}
	}
	else if (status.status == NYMPH_PLAYBACK_STATUS_STOPPED) {
		// If we're playing back a playlist, we may want to play the next item. Make sure this
		// is desired behaviour by the user.
		if (!status.stopped) {
			// Play back next track in the playlist, if any.
			remoteMutex.lock();
			if (remoteStatus.find(handle) == remoteStatus.end()) {
				// Entry not found. Proceed to cleaning up.
				remoteMutex.unlock();
			}
			else {
				// Play next entry in the playlist, if any.
				// Get the remote status reference.
				std::map<uint32_t, RemoteServerStatus>::iterator rit;
				rit = remoteStatus.find(handle);
				if (rit == remoteStatus.end()) {
					// Failed to find remote somehow. Panic.
					std::cerr << "Failed to find remote in map. Abort." << std::endl;
					remoteMutex.unlock();
					return;
				}
				
				// If this is the first time we get called, skip playback.
				// It means this is the status update right after connecting.
				if (!(rit->second.init)) {
					std::cout << "Initial status update. Skipping update." << std::endl;
					rit->second.init = true;
					remoteMutex.unlock();
					return;
				}
				
				if (!(rit->second.list)) {
					std::cout << "Not a playlist. Do nothing." << std::endl;
					remoteMutex.unlock();
					return;
				}
				
				remoteMutex.unlock();
				
				// Start the playback.
				std::vector<std::string>& playlist = rit->second.playlist;
				uint32_t& playlistId = rit->second.playlistId;
				
				std::cout << "Playlist ID: " << playlistId << std::endl;
				
				if (playlist.size() > playlistId) {
					std::cout << "Playing back next track in playlist..." << std::endl;
					if (!client.castFile(handle, playlist[playlistId])) {
						// Playback failed.
						std::cerr << "Playback failed for file: " << playlist[playlistId] << std::endl;
						return;
					}
					
					rit->second.playlistId++;
					return;
				}
				
				// No more items to play back.
				std::cout << "Finished playlist. Shutting down connection with receiver." << std::endl;
			}
		}
		else {
			// Remote playback was stopped by a user. Stop playback & end session.
		}
		
		// Remove local references to this former handle.
		remoteMutex.lock();
		std::map<uint32_t, RemoteServerStatus>::iterator rit = remoteStatus.find(handle);
		if (rit == remoteStatus.end()) {
			// Unknown handle. Abort.
			remoteMutex.unlock();
			return;
		}
		
		remoteStatus.erase(rit);
		remoteMutex.unlock();
		
		std::map<uint32_t, bool>::iterator it = receiverStatus.find(handle);
		if (it == receiverStatus.end()) {
			// Unknown handle. Abort.
			return;
		}
		
		client.disconnectServer(handle);
		receiverStatus.erase(it);
	}
}


// --- ON FILE ADDED ---
void onFileAdded(const Poco::DirectoryWatcher::DirectoryEvent& addEvent) {
	std::cout << "Added: " << addEvent.item.path();
	
	// TODO: Handle.
	// Update list version to indicate change. 
	
}


// --- ON FILE MODIFIED ---
void onFileModified(const Poco::DirectoryWatcher::DirectoryEvent& changeEvent) {
	std::cout << "Modified: " << changeEvent.item.path();
	
	// TODO: Handle.
}


// --- ON FILE REMOVED ---
void onFileRemoved(const Poco::DirectoryWatcher::DirectoryEvent& removeEvent) {
	std::cout << "Removed: " << removeEvent.item.path();
	
	// TODO: Handle.
	// Update list version to indicate change. 
	
}


// Function declarations
bool scan_mediafiles(std::string folders_file);
bool scan_gamesystems(std::string gameFolder);


int main(int argc, char** argv) {
	// Parse the command line arguments.
	Sarge sarge;
	sarge.setArgument("h", "help", "Get this help message.", false);
	sarge.setArgument("c", "configuration", "Path to configuration file.", true);
	sarge.setArgument("f", "folders", "Path to folder list file.", true);
	sarge.setArgument("v", "version", "Output the NymphCast Media Server version and exit.", false);
	sarge.setDescription("NymphCast Media Server. Shares files with NymphCast clients. More details: http://nyanko.ws/nymphcast.php.");
	sarge.setUsage("nymphcast_mediaserver <options>");
	
	sarge.parseArguments(argc, argv);
	
	if (sarge.exists("help")) {
		sarge.printHelp();
		return 0;
	}
	
	if (sarge.exists("version")) {
		std::cout << "NymphCast Media Server version: " << __VERSION << std::endl;
		return 0;
	}
	
	std::string folders_file = "folders.ini";
	if (!sarge.getFlag("folders", folders_file)) {
		std::cerr << "Folder list file argument is required." << std::endl;
		sarge.printHelp();
		return 0;
	}
	
	// Read in the configuration.
	std::string config_file;
	std::string gameFolder;
	bool nc_gamesync = false;
	if (sarge.exists("configuration")) {
		sarge.getFlag("configuration", config_file);
	
		INIReader config(config_file);
		if (config.ParseError() != 0) {
			/*std::cerr << "Unable to load configuration file: " << config_file << std::endl;
			return 1;*/
			nc_gamesync = false;
		}
		else {
			// Get 'games' section and read the path.
			std::set<std::string> cfg_sections = config.Sections();
			std::cout << "Found " << cfg_sections.size() << " sections in the config file." << std::endl;
			nc_gamesync = config.GetBoolean("games", "enable", true);
			gameFolder = config.Get("games", "path", "games");
		}
	}
	
	if (nc_gamesync) {
		if (!scan_gamesystems(gameFolder)) {
			// TODO: handle error.
			std::cerr << "Scanning for game systems failed." << std::endl;
			return 1;
		}
	}
	
	if (!scan_mediafiles(folders_file)) {
		// TODO: handle error.
		std::cerr << "Scanning for media files failed." << std::endl;
		return 1;
	}
	
	// Initialise the server.
	std::cout << "Initialising server...\n";
	long timeout = 5000; // 5 seconds.
	//NymphRemoteClient::init(serverLogFunction, NYMPH_LOG_LEVEL_TRACE, timeout);
	NymphRemoteClient::init(serverLogFunction, NYMPH_LOG_LEVEL_INFO, timeout);
	
	// Configure client.
	client.setStatusUpdateCallback(statusUpdateCallback);
	
	// Define all of the RPC methods we want to export for clients.
	std::cout << "Registering methods...\n";
	std::vector<NymphTypes> parameters;
	
	// array getFileList()
	NymphMethod getFileListFunction("getFileList", parameters, NYMPH_ARRAY, getFileList);
	NymphRemoteClient::registerMethod("getFileList", getFileListFunction);
	
	// uint8 playMedia(uint32 id, uint32 version, array receivers)
	parameters.clear();
	parameters.push_back(NYMPH_UINT32);
	parameters.push_back(NYMPH_UINT32);
	parameters.push_back(NYMPH_ARRAY);
	NymphMethod playMediaFunction("playMedia", parameters, NYMPH_UINT8, playMedia);
	NymphRemoteClient::registerMethod("playMedia", playMediaFunction);
	
	// array getGameList()
	NymphMethod getGameListFunction("getGameList", parameters, NYMPH_ARRAY, getGameList);
	NymphRemoteClient::registerMethod("getGameList", getGameListFunction);
	
	// ?? addGame()
	
	// ?? updateSave()
	
	// Install signal handler to terminate the server.
	signal(SIGINT, signal_handler);
	
	// Start server on port 4005.
	NymphRemoteClient::start(4005);
	
	// Start NyanSD announcement server.
	NYSD_service sv;
	sv.port = 4005;
	sv.protocol = NYSD_PROTOCOL_TCP;
	sv.service = "nymphcast_mediaserver";
	NyanSD::addService(sv);
	
	std::cout << "Starting NyanSD on port 4005 UDP..." << std::endl;
	NyanSD::startListener(4005);
	
	// TODO: Announce the NCMS on the network if game synchronisation is enabled.
	// TODO: Handle synchronisation with other NCMS instances if found.
	// TODO: Perform regular polling of remote file/NCMS content changes, as needed.
	
	
	// TODO: Start webserver with dashboard & file management functionality.
	
	
	// Wait for the condition to be signalled.
	gMutex.lock();
	gCon.wait(gMutex);
	
	std::cout << "Stopping NymphCast Media Server..." << std::endl;
	
	// Clean-up
	NyanSD::stopListener();
	NymphRemoteClient::shutdown();
	
	for (uint32_t i = 0; i < dirwatchers.size(); i++) {
		delete dirwatchers[i];
	}
	
	// Wait before exiting, giving threads time to exit.
	Thread::sleep(2000); // 2 seconds.
	
	return 0;
}
