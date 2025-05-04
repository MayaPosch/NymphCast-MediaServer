/*
	types.h - Common NCMS types.
	
*/


#include "INIReader.h"

#include <string>
#include <iostream>
#include <vector>
#include <filesystem> 		// C++17
namespace fs = std::filesystem;

#include <Poco/DirectoryWatcher.h>
#include <Poco/Delegate.h>
#include <Poco/StringTokenizer.h>

extern std::vector<Poco::DirectoryWatcher*> dirwatchers; // in NymphCastMediaServer.cpp

void onFileAdded(const Poco::DirectoryWatcher::DirectoryEvent& addEvent);
void onFileModified(const Poco::DirectoryWatcher::DirectoryEvent& changeEvent);
void onFileRemoved(const Poco::DirectoryWatcher::DirectoryEvent& removeEvent);


// Types:
// 0	Audio
// 1	Video
// 2	Image
// 3	Playlist
struct MediaFile {
	std::string section;
	std::string filename;
	uint8_t type;
	fs::path path;
	std::string rel_path;
};


struct Game {
	std::string name;
};


struct Save {
	std::string name;
};


struct GameSystem {
	std::string name;		// Short name is also folder name.
	std::string long_name;
	std::string extensions;
	std::string launch_cmd;
	std::string theme;
	std::vector<Game> games;
	std::vector<Save> saves;
};
