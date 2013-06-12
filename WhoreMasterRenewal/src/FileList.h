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
#ifndef FILELIST_H_INCLUDED_1507
#define FILELIST_H_INCLUDED_1507
#pragma once

#include "DirPath.h" // required DirPath

#include <string>
#include <vector>

namespace WhoreMasterRenewal
{

/*
 * Just a container class to hold the path, filename
 * and full path of a file.
 *
 * This is reinventing Boost, just a little. But then
 * we don't need to build boost, which is a good thing
 * on the whole.
 */
class FileListEntry
{
    std::string m_path; // the base path: ".\Resources\Characters"
    std::string m_leaf; // the file name: "Girls.girls"
    std::string m_full; // the full path: ".\Resources\Characters\Girls.girls"
public:
    FileListEntry();
    FileListEntry( const FileListEntry& fle );
    FileListEntry( std::string a_path, std::string a_leaf );
    
    std::string& leaf();
    std::string& path();
    std::string& full();
};

class FileList
{
public:
    /*
     *  initialise the list with the DirPath for a folder and
     *  a pattern string. So:
     *
     *      DirPath dp = DirPath() << "Resources" << "Characters";
     *      FileList(dp, "*.girls");
     *
     *  The folder is scanned, and any files that match the pattern
     *  are stored in the vector
     */
    FileList( DirPath dp, const char* pattern = "*" );
    FileList( DirPath dp, const char* pattern, bool no_load );
    
    virtual ~FileList();
    /*
     *  [] operators so you can subscript the list like an array
     */
    FileListEntry& operator[]( int index );
    /*
     *  returns the number of elements in the list
     *  (everything int STL uses size so we use that.)
     */
    int size() const;
    /*
     *  scan lets us run another scan on the same folder
     *  but using a different pattern
     */
    virtual void scan( const char* );
    /*
     *  I need to concatentate these babies...
     */
    FileList& operator+=( FileList& l );
    
protected:
    DirPath& folder_dp();
    
private:
    DirPath folder;
    /*
     *  std::vector - standard template library class.
     *  produces a typed dynamic array that grows as needed
     */
    std::vector<FileListEntry>  files;
};

/*
 * don't give this any file extensions: it'll scan for .jpg, .jpeg, .gif...
 */
class ImageFileList : public FileList
{
public:
    /*
     *  these are the extensions for which ImageFileList scans
     *
     *  made them a static class member so they can be overridden in software.
     *  if need be, can be a config file entry
     */
    static std::vector<std::string> file_extensions;
    ImageFileList( DirPath dp, const char* pattern = "*" );
    virtual void scan( const char* );
};

class XMLFileList
{
    DirPath folder;
    std::vector<FileListEntry>  files;
public:
    XMLFileList( DirPath dp, char const* pattern = "*" );
    FileListEntry& operator[]( int index );
    int size();
    void scan( const char* );
};

} // namespace WhoreMasterRenewal

#endif // FILELIST_H_INCLUDED_1507
