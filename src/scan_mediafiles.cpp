/*
	scan_mediafiles.cpp - Implements the scanning for media files.
	
	Revision 0
	
*/


#include "types.h"

#include "INIReader.h"
#include "mimetype.h"


extern std::vector<MediaFile> mediaFiles;


bool scan_mediafiles(std::string folders_file) {
	// Obtain the list of directories to scan.
	std::cout << "Scanning directories..." << std::endl;
	INIReader folderList(folders_file);
	if (folderList.ParseError() != 0) {
		std::cerr << "Failed to parse the '" << folders_file << "' file." << std::endl;
		return 1;
	}
	
	std::set<std::string> sections = folderList.Sections();
	std::cout << "Found " << sections.size() << " sections in the folder list." << std::endl;
	
	uint32_t index = 0;
	uint32_t id = 0;
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
		for (fs::recursive_directory_iterator next(dir); next != fs::end(next); next++) {
			fs::path fe = next->path();
			//std::cout << "Checking path: " << fe.string() << std::endl;
			if (!fs::is_regular_file(fe)) {
				continue; 
			}
			
			std::string ext = fe.extension().string();
			ext.erase(0, 1);	// Remove leading '.' character.
			//std::cout << "Checking extension: " << ext << std::endl;
			MediaFile mf;
			uint8_t type;
			if (MimeType::hasExtension(ext, type)) {
				// Add to media file list.
				std::cout << "Adding file: " << fe << std::endl;
				
				mf.path = fe;
				mf.rel_path = fe.parent_path().string();
				mf.rel_path.erase(0, dir.string().size());
				mf.section = *it;
				mf.filename = fe.filename().string();
				mf.type = type;
				mediaFiles.push_back(mf);
				
				std::cout << "Relative path: " << mf.rel_path << std::endl;
				
				// Add to serialised list.
				// This is the data we will be sending to a client as the file list. It contains
				// the filenames and their IDs, sorted by the type and containing folder.
				
			}
		}
	
		// Register a DirectoryWatcher for the media folder.
		Poco::DirectoryWatcher* dw = new Poco::DirectoryWatcher(dir.string());
		dw->itemModified	+= Poco::delegate(&onFileModified);
		dw->itemAdded		+= Poco::delegate(&onFileAdded);
		dw->itemRemoved		+= Poco::delegate(&onFileRemoved);
		dirwatchers.push_back(dw);
	}
	
	return true;
}
