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
#ifndef DIRPATH_H_INCLUDED_1507
#define DIRPATH_H_INCLUDED_1507
#pragma once

#include <string>
#include <iostream>
#include <fstream>

/*
 * little class to create pathnames in a cross platform way
 */
class DirPath
{
protected:
	std::string		path;
/*
 *	sep is the path element separator: "/" on unix and "\\"
 *	on windows. The #ifdef is hidden in the .cpp file
 */
static	const char	sep[];
public:
/*
 *	constructor takes a single path component,
 *	defaulting to "," since that seems to be the most common case
 *
 *	If you want an absolute pathanme (/foo/bar or \FOO\BAR)
 *	construct the path with an empty string
 */
	DirPath(const char *top_dir = ".") {
		path = top_dir;
		size_t pos = path.find('\r');
		if(pos != std::string::npos) {
			path[pos] = 0;
		}
	}
/*
 *	copy contstructor, probably not needed...
 */
	DirPath(DirPath& dp) {
		path = dp.path;
	}
	
	virtual ~DirPath() {}
/*
 *	to build the path we use the << operator
 *	since that lets us cope with char * and string
 *	types mixed into sequences of arbitrary length
 *
 *	this is the string operator
 */
	DirPath& operator << (std::string &s) {
		path += sep;
		path += s;
		return *this;
	}
/*
 *	and here's one for const char * values
 */
	DirPath& operator << (const char *pt) {
		path += sep;
		path += pt;
		return *this;
	}
/*
 *	same operator, but overloaded on ostream
 *	so you can use << to print the path on debug messages
 */
	std::ostream& operator << (std::ostream& os) {
		return os << path;
	}
/*
 *	a c_str() method so I can drop this sucker in where 
 *	the code used to use a string
 */
	const char *c_str()	{ return path.c_str(); }
/*
 *	and a couple of type coercion operators, so I shouldn't
 *	even need to do that
 */
	operator const char *()	{ return path.c_str(); }
	operator std::string()	{ return path; }
};

/*
 * little class to create pathnames in a cross platform way
 */
class DirPathR : public DirPath
{
	DirPath over;
public:
/*
 *	constructor takes a single path component,
 *	defaulting to "," since that seems to be the most common case
 *
 *	If you want an absolute pathanme (/foo/bar or \FOO\BAR)
 *	construct the path with an empty string
 */
	DirPathR();
/*
 *	copy contstructor, probably not needed...
 */
	DirPathR( DirPathR& dp )
        : DirPath( dp )
	{
		path = dp.path;
		over = dp.over;
	}

	std::ifstream &open(std::ifstream &ifs) {
		ifs.open(over.c_str());
		if(!ifs.good()) {
			ifs.open(c_str());
		}
		return ifs;
	}

	std::ofstream &open(std::ofstream &ofs) {
		ofs.open(over.c_str());
		if(!ofs.good()) {
			ofs.open(c_str());
		}
		return ofs;
	}
/*
 *	to build the path we use the << operator
 *	since that lets us cope with char * and string
 *	types mixed into sequences of arbitrary length
 *
 *	this is the string operator
 */
	DirPath& operator << (std::string &s) {
		over << s;
		path += sep;
		path += s;
		return *this;
	}
/*
 *	and here's one for const char * values
 */
	DirPath& operator << (const char *pt) {
		over << pt;
		path += sep;
		path += pt;
		return *this;
	}

};


class ImagePath : public DirPath {
public:
	ImagePath(std::string filename)
	: DirPath()
	{
		(*this) << "Resources" << "Images" << filename;
	}
};

class ButtonPath : public DirPath {
public:
	ButtonPath(std::string filename)
	: DirPath()
	{
		(*this) << "Resources" << "Buttons" << filename;
	}
};

class ScriptPath : public DirPath {
public:
	ScriptPath(std::string filename)
	: DirPath()
	{
		(*this) << "Resources" << "Scripts" << filename;
	}
};

#endif // DIRPATH_H_INCLUDED_1507
