
#include "MasterFile.h"
#include "DirPath.h"
#include "XmlMisc.h"

#include <string>
#include <fstream>

void MasterFile::LoadLegacy( std::string filename )
{
    files.clear();
    std::ifstream ifs;
    char buffer[1024];	// power of 2 makes better use of memory
/*
*		format the path for the master file.
*/
    std::string mfile = filename + ".mast";
    DirPath mastfile = DirPath() << "Saves" << mfile;
    filename = mastfile.c_str();
/*
*		open the file
*/
    ifs.open( mastfile.c_str() );
/*
*		problem opening the file?
*/
    if( !ifs.good() )
    {
        ifs.close();
        return;
    }
/*
*		loop through the file, one line at a time
*/
    while( ifs.good() )
    {
/*
*			Using "sizeof()" means that the size is right even if the
*			buffer size is later changed.
*/
        ifs.getline(buffer, sizeof(buffer)-1, '\n');

        if( std::string(buffer).empty() == false )
        {
/*
*				add the file to the map
*/
            files[std::string(buffer)] = 1;
        }
    }
    ifs.close();
}

bool MasterFile::exists( std::string name )
{
    return( files.find(name) != files.end() );
}

size_t MasterFile::size()
{
    return files.size();
}

void MasterFile::add( std::string str )
{
    files[str] = 1;
}

bool MasterFile::LoadXML( TiXmlHandle hLoadedFiles )
{
    files.clear();
    TiXmlElement* pLoadedFiles = hLoadedFiles.ToElement();
    if( pLoadedFiles == 0 )
    {
        return false;
    }

    TiXmlElement* pGirlsFiles = pLoadedFiles->FirstChildElement("Girls_Files");
    if( pGirlsFiles == 0 )
    {
        return false;
    }

    for( TiXmlElement* pFile = pGirlsFiles->FirstChildElement("File");
        pFile != 0;
        pFile = pFile->NextSiblingElement("File") )
    {
        if( pFile->Attribute("Filename") )
        {
            add( pFile->Attribute("Filename") );
        }
    }

    return true;
}

TiXmlElement* MasterFile::SaveXML( TiXmlElement* pRoot )
{
    TiXmlElement* pLoadedFiles = new TiXmlElement("Loaded_Files");
    pRoot->LinkEndChild( pLoadedFiles );

    TiXmlElement* pGirlsFiles = new TiXmlElement("Girls_Files");
    pLoadedFiles->LinkEndChild( pGirlsFiles );

    FileFlags::const_iterator it;
    for( it = files.begin(); it != files.end(); ++it )
    {
        TiXmlElement* pFile = new TiXmlElement("File");
        pGirlsFiles->LinkEndChild( pFile );
        pFile->SetAttribute( "Filename", it->first );
    }
    return pLoadedFiles;
}

