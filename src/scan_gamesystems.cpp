/*
	scan_gamesystems.cpp - Implements the scanning for game systems & save files.
	
	Revision 0.
	
*/


#include "types.h"


extern std::vector<GameSystem> gameSystems;


bool scan_gamesystems(std::string gameFolder) {
	// Check that path is a valid directory.
	fs::path gamedir = gameFolder;
	if (!fs::is_directory(gamedir)) {
		std::cout << "Path is not a valid directory: " << gameFolder << ". Skipping." << std::endl;
		return false;
	}
	
	// TODO: Scan the available systems in the 'games' folder.
	// Use the 'system.ini' file in each sub-folder to compile a list of available systems
	// and their games.
	// Start by getting the list of sub-folders.
	
	
	// Attempt to open the 'system.ini' file per sub-folder. If found, read in
	// system details and scan the 'games' sub-sub-folder for game filenames to add.

	// First iterate through the directory to filter out the game folders.
	for (fs::recursive_directory_iterator next(gamedir); next != fs::end(next); next++) {
		fs::path sysc = next->path();
		sysc /= "system.ini";
		fs::path sysdir = next->path();
		fs::path romdir = sysdir;
		romdir /= "roms";
		fs::path savedir = sysdir;
		savedir /= "saves";
		if (!fs::is_regular_file(sysc)) {
			continue; 
		}
		
		// Open the INI file and process it.
		INIReader syscfg(sysc.string());
		
		GameSystem gs;
		gs.name = syscfg.Get("", "name", "");
		gs.long_name = syscfg.Get("", "long_name", "");
		gs.extensions = syscfg.Get("", "extensions", "");
		gs.launch_cmd = syscfg.Get("", "launch_cmd", "");
		gs.theme = syscfg.Get("", "theme", "");
		
		if (gs.name.empty() || gs.long_name.empty() || gs.launch_cmd.empty() || gs.theme.empty()) {
			// Issue with the INI file. Report and skip this folder.
			std::cout << "Missing value in system.ini file. Skipping folder..." << std::endl;
			continue;
		}
		
		// Unpack extensions and scan for ROMs.
		Poco::StringTokenizer tokens(sysc.string(), ",", Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);
		uint32_t tc = tokens.count();
		if (tc < 1) {
			std::cerr << "Zero system extensions found. Skipping system..." << std::endl;
			continue;
		}
		
		if (!fs::is_directory(romdir)) {
			std::cout << "Path is not a valid directory: " << romdir << ". Skipping." << std::endl;
			continue;
		}
		
		// Iterate through the directory to filter out the media files.
		for (fs::recursive_directory_iterator rd(romdir); rd != fs::end(rd); rd++) {
			fs::path fe = rd->path();
			//std::cout << "Checking path: " << fe.string() << std::endl;
			if (!fs::is_regular_file(fe)) {
				continue; 
			}
			
			std::string ext = fe.extension().string();
			ext.erase(0, 1);	// Remove leading '.' character.
			
			// Check we have this extension in the filter.
			if (tokens.has(ext)) {
				// Add to list.
				Game g;
				g.name = fe.filename().string();
				gs.games.push_back(g);
			}
		}
		
		// Scan for any save files.
		for (fs::recursive_directory_iterator dit(savedir); dit != fs::end(dit); dit++) {
			fs::path fe = dit->path();
			if (!fs::is_regular_file(fe)) {
				continue;
			}
			
			// TODO: need to filter save file extensions too?
			// Save files with libretro are apparently all *.srm extensions.
			std::string ext = fe.extension().string();
			ext.erase(0, 1);	// Remove leading '.' character.
			
			// Check we have this extension in the filter.
			//if (tokens.has(ext)) {
			if (ext == "srm") {
				// Add to list.
				Save s;
				s.name = fe.filename().string();
				gs.saves.push_back(s);
			}
		}
	
		gameSystems.push_back(gs);
	}
	
	return true;
}
