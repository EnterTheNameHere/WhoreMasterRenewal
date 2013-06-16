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
#ifndef CINTERFACEWINDOW_H_INCLUDED_1526
#define CINTERFACEWINDOW_H_INCLUDED_1526
#pragma once

#include "Constants.h"
#include "cInterfaceObject.h" // required inheritance

#include <vector>
#include <map>
#include <string>
#include <memory>

class TiXmlElement;
class SDL_Color;
class SDL_Surface;

namespace WhoreMasterRenewal
{

class cInterfaceWindow;
extern cInterfaceWindow g_MainMenu;
extern cInterfaceWindow g_GetString;
extern cInterfaceWindow g_BrothelManagement;
extern cInterfaceWindow g_ChangeJobs;
extern cInterfaceWindow g_Turnsummary;
extern cInterfaceWindow g_GetInt;
extern cInterfaceWindow g_LoadGame;
extern cInterfaceWindow g_Gallery;
extern cInterfaceWindow g_TransferGirls;

class cImageItem;
class CSurface;
class cButton;
class cEditBox;
class cTextItem;
class cListBox;
class cCheckBox;
class cScrollBar;
class cSlider;
class cAnimatedSurface;
class cXmlWidget;

class cInterfaceWindow : public cInterfaceObject
{
public:
	cInterfaceWindow();
	virtual ~cInterfaceWindow();
    
    cInterfaceWindow( const cInterfaceWindow& ) = delete;
	//cInterfaceWindow& operator = ( const cInterfaceWindow& rhs );
    
	virtual void Free();

	void CreateWindow(int x, int y, int width, int height, int BorderSize);	// and color options latter
	void UpdateWindow(int x, int y);
	void MouseDown(int x, int y);
	void Click(int x, int y, bool mouseWheelDown = false, bool mouseWheelUp = false);
	void Draw();

	void Reset();

	// MOD: added a couple of overloads for the cases where we
	// can deduce the image names from the stem&
	void AddButton(const char* image_name, int& ID,  int x, int y, int width, int height, bool transparency = false, bool scale = true,bool cached=false);
	void AddButton(std::string image, int& ID, int x, int y, int width, int height, bool transparency = false, bool scale = true,bool cached=false);
/*
 *	the ND version stands for "No Disabled" meaning that it supplies
 *	an empty string in place a disabled icon
 */
	void AddButtonND(std::string image, int& ID, int x, int y, int width, int height, bool transparency = false, bool scale = true,bool cached=false);
	void AddButtonND(const char* image, int& ID, int x, int y, int width, int height, bool transparency = false, bool scale = true,bool cached=false);
	// END MOD
	void AddButton(std::string OffImage, std::string DisabledImage, std::string OnImage, int& ID, int x, int y, int width, int height, bool transparency = false, bool scale = true,bool cached=false);
	void AddScrollBar(int& ID, int x, int y, int width, int height, int visibleitems);

	void HideButton(int id, bool hide);
	void HideButton(int id) { HideButton(id, true); }
	void UnhideButton(int id) { HideButton(id, false); }
	void DisableButton(int id, bool disable);
	void DisableButton(int id){DisableButton(id,true);}
	void EnableButton(int id){DisableButton(id,false);}

	void HideImage(int id, bool hide);
	void HideImage(int id) { HideImage(id, true); }
	void UnhideImage(int id) { HideImage(id, false); }
	void AddImage(int & id, std::string filename, int x, int y, int width, int height, bool statImage = false, int R = 0, int G = 0, int B = 0);
	void SetImage(int id, std::shared_ptr<CSurface> image);
	void SetImage(int id, cAnimatedSurface* image);

	void AddEditBox(int& ID, int x, int y, int width, int height, int BorderSize);

	void EditTextItem(std::string text, int ID);
	void HideText(int id, bool hide);
	void HideText(int id) { HideText(id, true); }
	void UnhideText(int id) { HideText(id, false); }
	void AddTextItem(int& ID, int x, int y, int width, int height, std::string text, int size = 16, bool auto_scrollbar = true, bool force_scrollbar = false);
	void AddTextItemScrollBar(int id);

	void AddSlider(int& ID, int x, int y, int width, int min = 0, int max = 100, int increment = 5, int value = 0, bool live_update = true);
	void DisableSlider(int ID, bool disable = true);
	void HideSlider(int ID, bool hide = true);
	void SliderLiveUpdate(int ID, bool live_update = true);
	int SliderRange(int ID, int min, int max, int value, int increment);  // set min and max values; returns slider value
	int SliderValue(int ID);  // get slider value
	int SliderValue(int ID, int value);  // set slider value, get result (might be different than requested due to out-of-bounds or whatever)

	void DisableCheckBox(int ID, bool disable);
	void AddCheckbox(int& ID, int x, int y, int width, int height, std::string text, int size = 16);
	bool IsCheckboxOn(int ID);
	void SetCheckBox(int ID, bool on);

	void AddListBox(int& ID, int x, int y, int width, int height, int BorderSize, bool enableEvents, bool MultiSelect = false, bool ShowHeaders = false, bool HeaderDiv = true, bool HeaderSort = true);
	void ScrollListBoxDown(int ID);
	int GetListBoxSize(int ID);
	void ScrollListBoxUp(int ID);
	void AddToListBox(int listBoxID, int dataID, std::string data, int color = LISTBOX_BLUE);
//	void RemoveFromListBox(int listBoxID, int dataID);
	int GetSelectedItemFromList(int listBoxID);
    std::string GetSelectedTextFromList(int listBoxID); // MYR: For new message summary display in InterfaceProcesses.cpp
	int GetLastSelectedItemFromList(int listBoxID);
	int GetNextSelectedItemFromList(int listBoxID, int from, int& pos);
	int GetAfterSelectedItemFromList(int listBoxID);
	void SetSelectedItemInList(int listBoxID, int itemID, bool ev = true, bool DeselectOthers = true);
	void SetSelectedItemText(int listBoxID, int itemID, std::string data);
	void ClearListBox(int ID);
	void SetListBoxPosition(int ID, int pos = 0);
	int ArrowDownListBox(int ID);
	int ArrowUpListBox(int ID);
	bool IsMultiSelected(int ID);	// returns true if list has more than one item selected
	void AddToListBox(int listBoxID, int dataID, std::string data[], int columns, int color = LISTBOX_BLUE);
	void SetSelectedItemText(int listBoxID, int itemID, std::string data[], int columns);
	void SetSelectedItemColumnText(int listBoxID, int itemID, std::string data, int column);
	void SortColumns(int listBoxID, std::string column_name[], int columns);
	void DefineColumns(int listBoxID, std::string name[], std::string header[], int offset[], bool skip[], int columns);
	void SortListItems(int listBoxID, std::string column_name, bool Desc = false);
    std::string HeaderClicked(int listBoxID);
	bool ListDoubleClicked(int listBoxID);
	void SetSelectedItemTextColor(int listBoxID, int itemID, SDL_Color* text_color);
	void FillSortedIDList(int listBoxID, std::vector<int>* id_vec, int* vec_pos);

	void Focused();

	void SetBackgroundImage(std::string file);
	void UpdateEditBoxes(char key, bool upper = false);

    std::string GetEditBoxText(int ID);

	bool HasEditBox() {return !m_EditBoxes.empty();}

protected:
	std::vector<cButton*> m_Buttons;	// buttons
	std::vector<cImageItem*> m_Images; // Images
	std::vector<cListBox*> m_ListBoxes;	// List boxes
	std::vector<cScrollBar*> m_ScrollBars;	// Scroll bars
	// check boxes
	std::vector<cEditBox*> m_EditBoxes; // Edit boxes;
	std::vector<cTextItem*> m_TextItems; // Text Items
	std::vector<cCheckBox*>m_CheckBoxes;	// check boxes
	std::vector<cSlider*>m_Sliders;	// Sliders

	// the windows properties
	std::shared_ptr<CSurface> m_BackgroundSurface;
	SDL_Surface* m_Background;
	SDL_Surface* m_Border;
	int m_BorderSize;

	float m_xRatio, m_yRatio;	// for storing the scaling ratio of the interface
};

class cInterfaceWindowXML : public cInterfaceWindow
{
protected:
    std::string m_filename;
	std::map<std::string,int> name_to_id;
	std::map<int,std::string> id_to_name;

	std::map<std::string,cXmlWidget*> widgets;
public:
	enum AttributeNecessity {
		Mandatory = 0,
		Optional = 1
	};
	
	cInterfaceWindowXML();
	virtual ~cInterfaceWindowXML() {};
	
	void load();
/*
 *	this has static linkage so we can pass it
 *	to the window manager's Push method
 */
static	void handler_func(cInterfaceWindowXML* wpt);
/*
 * 	the handler func just calls the virtual process
 *	method, which can process calls as it likes
 */
virtual	void process()=0;
/*
 *	populates the maps so we can get the IDs from strings
 */
	void register_id(int id, std::string name);
/*
 *	XML reading stuff
 */
	void read_text_item(TiXmlElement*);
	void read_window_definition(TiXmlElement*);
	void read_button_definition(TiXmlElement*);
	void read_image_definition(TiXmlElement*);
	void read_listbox_definition(TiXmlElement*);
	void read_checkbox_definition(TiXmlElement*);
	void read_slider_definition(TiXmlElement*);
	void define_widget(TiXmlElement*);
	void place_widget(TiXmlElement*, std::string suffix = "");
	void widget_text_item(TiXmlElement*, cXmlWidget&);
	void widget_button_item(TiXmlElement*, cXmlWidget&);
	void widget_listbox_item(TiXmlElement*, cXmlWidget&);
	void widget_checkbox_item(TiXmlElement*, cXmlWidget&);
	void widget_widget(TiXmlElement*, cXmlWidget&);
	void widget_image_item(TiXmlElement*, cXmlWidget&);
	void widget_slider_item(TiXmlElement*, cXmlWidget&);
	int get_id(std::string name, bool essential=false);

	cXmlWidget* new_widget(std::string name);
	cXmlWidget* find_widget(std::string name);
	void add_widget(std::string widget_name,int x,int y,std::string seq);
};

class cSelector
{
public:
	cSelector( cInterfaceWindow& a_win, int a_id );
	cSelector& operator =( const cSelector& other );

	int first();
	int next();

private:
	int id;			// list ID
	int pos;		// position variable
	cInterfaceWindow& win;	// window reference
};

} // namespace WhoreMasterRenewal

#endif // CINTERFACEWINDOW_H_INCLUDED_1526
