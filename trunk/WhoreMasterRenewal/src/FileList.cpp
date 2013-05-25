/*
 * Copyright 2009, 2010, The Pink Petal Development Team.
 * The Pink Petal Devloment Team are defined as the game's coders
 * who meet on http://pinkpetal.co.cc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "FileList.h"

#include <map>

static std::string clobber_extension(std::string s);

#ifdef LINUX

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>
#include <regex.h>
#include <string.h>

static std::string& gsub(std::string &str, const char *pat_pt, const char *repl_pt)
{
	size_t pat_len = strlen(pat_pt);
	size_t repl_len = strlen(repl_pt);

	size_t siz = 0;
 	for(int i = 0 ; i < 10; i++) {
		siz = str.find(pat_pt, siz);
		if(siz == std::string::npos) {
			break;
		}
		str.replace(siz, pat_len, repl_pt);
		siz += repl_len;
	}
	return str;
}

/*
 * scan the directory, dp, looking for files that match the
 * pattern, pattern.
 *
 * This is the LINUX version of this func. The windows one
 * is underneath
 */
FileList::FileList(DirPath dp, const char *pattern)
{
	folder = dp;
	scan(pattern);
}

void FileList::scan(const char *pattern)
{
	DIR		*dpt;
	struct dirent	*dent;
	const char	*base_path = folder.c_str();
    std::string 	s_bp(folder.c_str());
    std::string 	s_pat(pattern);
	s_pat += "$";
/*
 *	clear the file vector
 */
	files.clear();
/*
 *	we'll need to match regular expressions against the file name
 *	to do that we need to turn the "." into "\\." sequences and similarly
 *	"*" into ".*"
 */
	gsub(s_pat, ".", "\\.");
	gsub(s_pat, "*", ".*");
/*
 *	now make a regex
 */
	regex_t r;
 	regcomp(&r, s_pat.c_str(), 0);
/*
 *	open the directory. Print an error to the console if it fails
 */
	if((dpt = opendir(base_path)) == NULL) {
		std::cerr __FILE__ << " (" << __LINE__ << "): " << "Error(" << errno << ") opening " << base_path << std::endl;
		return;
	}
/*
 *	loop through the files
 */
	while ((dent = readdir(dpt)) != NULL) {
		if(regexec(&r, dent->d_name, 0, 0, 0) != 0) {
			continue;
		}
		files.push_back(
			FileListEntry(s_bp, std::string(dent->d_name))
		);
	}
	closedir(dpt);
}

#else
#include"cInterfaceWindow.h"
#include<windows.h>
#include"interfaceIDs.h"
extern cInterfaceWindow g_LoadGame;



FileList::FileList(DirPath dp, const char *pattern)
{
	//files = 0;
	//n_files = 0;

	folder=dp;
	scan(pattern);

}

 void FileList::scan(const char * pattern )
{
	files.clear();
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;
	DirPath loc=folder;
	loc<<pattern;
    std::string base=folder.c_str();
    std::string filename;
	hFind = FindFirstFileA(loc.c_str(), &FindFileData);

	/*int i = 0;*/
	while(hFind != INVALID_HANDLE_VALUE) {
		 filename=FindFileData.cFileName;
		 		 FileListEntry tempfile(base,filename);
				 files.push_back(tempfile);
		if(FindNextFileA(hFind, &FindFileData) == 0) {
			break;
		}
	}
	FindClose(hFind);
}

#endif



XMLFileList::XMLFileList(DirPath dp, char const *pattern)
{
	folder = dp;
	scan(pattern);
}

void XMLFileList::scan(const char *pattern)
{
	std::map<std::string,FileListEntry> lookup;
	FileList fl(folder, pattern);
/*
 *	OK: do a scan with the non xml file name
 *	and store the results in a map keyed by filename
 *	minus extension
 */
	for(int i = 0; i < fl.size(); i++) {
	    std::string str = fl[i].full();
	    std::string key = clobber_extension(str);
		lookup[key] = fl[i];
		//std::cout << "       adding " << str << std::endl;
		//std::cout << "       under " << key << std::endl;
		//std::cout << "       result " << lookup[key].full() << std::endl;
	}
/*
 *	Repeat with "x" added to the end of the pattern.
 *	If an xml file shadows a non-XML version, the XML
 *	pathname will overwrite the non-XML one
 */
    std::string newpat = pattern; newpat += "x";
	fl.scan(newpat.c_str());
	for(int i = 0; i < fl.size(); i++) {
	    std::string str = fl[i].full();
	    std::string key = clobber_extension(str);
		lookup[key] = fl[i];
		//std::cout << "       adding " << str << std::endl;
		//std::cout << "       under " << key << std::endl;
		//std::cout << "       result " << lookup[key].full() << std::endl;
	}
/*
 *	We now have a map of files with the desired extensions
 *	and where ".foox" takes precedence over ",foo"
 *
 *	now walk the map, and populate the vector
 */
	files.clear();
	for(std::map<std::string,FileListEntry>::const_iterator it = lookup.begin(); it != lookup.end(); ++it) {
		files.push_back(it->second);
	}
}

static std::string clobber_extension(std::string s)
{
	//std::cout << "clobber_extension: s = " << s << std::endl;
	size_t pos = s.rfind(".");

	//std::cout << "clobber_extension: pos = " << pos << std::endl;
	std::string base = s.substr(0, pos);

	//std::cout << "clobber_extension: s = " << s << std::endl;
	//std::cout << "clobber_extension: base = " << base << std::endl;
	return base;
}

std::vector<std::string> ImageFileList::file_extensions;

void ImageFileList::scan(const char *base)
{
	DirPath &dp = folder_dp();
/*
 *	loop over all the file types
 */
	for(u_int i = 0; i < file_extensions.size(); i++) {
/*
 *		build the pattern from the base plus extension
 */
	    std::string pat = base + file_extensions[i];
/*
 *		get a FileList for the extension,
 */
		FileList l(dp, pat.c_str());
/*
 *		add its files to this list
 */
		(*this) += l;
	}
}
