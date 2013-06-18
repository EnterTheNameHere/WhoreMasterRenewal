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
#ifndef CXMLWIDGET_H_INCLUDED_1508
#define CXMLWIDGET_H_INCLUDED_1508
#pragma once

#include <vector>
#include <string>

namespace WhoreMasterRenewal
{

struct sXmlWidgetPart
{
public:
	int x = {0};
	int y = {0};
	int w = {0};
	int h = {0};
	int r = {0};
	int g = {0};
	int b = {0};
	
	int fontsize = {0};
	int bordersize = {0};
	
	bool stat = {false};
	bool alpha = {false};
	bool scale = {false};
	bool hidden = {false};
	bool events = {false};
	bool multi = {false};
	bool hide = {false};
	bool cache = {false};
	
    std::string base = {"Default value"};
    std::string on = {"Default value"};
    std::string off = {"Default value"};
    std::string disabled = {"Default value"};
    std::string type = {"Default value"};
    std::string name = {"Default value"};
    std::string text = {"Default value"};
    std::string file = {"Default value"};
    std::string seq = {"Default value"};
};

class cXmlWidget
{
public:
	cXmlWidget();
	
	int size();
	sXmlWidgetPart& operator[](int i);
	void add(sXmlWidgetPart &part);
	
private:
    std::vector<sXmlWidgetPart> m_List;
};

} // namespace WhoreMasterRenewal

#endif // CXMLWIDGET_H_INCLUDED_1508
