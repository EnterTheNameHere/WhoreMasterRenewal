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

#include "cInterfaceWindow.h"
#include "DirPath.h"
#include "CLog.h"
#include "XmlMisc.h"
#include "cButton.h"
#include "cSlider.h"
#include "cEditBox.h"
#include "cImageItem.h"
#include "cTextItem.h"
#include "cListBox.h"
#include "cScrollBar.h"
#include "cCheckBox.h"
#include "cXmlWidget.h"
#include "CGraphics.h"
#include "CSurface.h"
#include "cInterfaceEvent.h"
#include "InterfaceGlobals.h"
#include "InterfaceProcesses.h"

#include <string>
#include <cctype>
#include <ctime>

namespace WhoreMasterRenewal
{

typedef unsigned int u_int;

cInterfaceWindow::cInterfaceWindow() : cInterfaceObject()
{
    ;
}

cInterfaceWindow::~cInterfaceWindow()
{
	Free();
}

cInterfaceWindow& cInterfaceWindow::operator = ( const cInterfaceWindow& rhs )
{
    // TODO: proper implementation
    
    if( this != &rhs )
    {
        m_Buttons = rhs.m_Buttons;
        m_Images = rhs.m_Images;
        m_ListBoxes = rhs.m_ListBoxes;
        m_ScrollBars = rhs.m_ScrollBars;
        m_EditBoxes = rhs.m_EditBoxes;
        m_TextItems = rhs.m_TextItems;
        m_CheckBoxes = rhs.m_CheckBoxes;
        m_Sliders = rhs.m_Sliders;
        m_BackgroundSurface = rhs.m_BackgroundSurface;
        m_Background = rhs.m_Background;
        m_Border = rhs.m_Border;
        m_BorderSize = rhs.m_BorderSize;
        m_xRatio = rhs.m_xRatio;
        m_yRatio = rhs.m_yRatio;
    }
    
    return *this;
}

void cInterfaceWindow::Free()
{
	for(unsigned int i=0;i< m_Buttons.size();i++)
		delete m_Buttons[i];
	m_Buttons.clear();

	for(unsigned int i=0;i< m_ScrollBars.size();i++)
		delete m_ScrollBars[i];
	m_ScrollBars.clear();

	for(unsigned int i=0;i<m_EditBoxes.size();i++)
		delete m_EditBoxes[i];
	m_EditBoxes.clear();

	//there is a special case where a picture may have already been deleted by something prior
	//and it seems like way too much effort to include smart pointers
	//so just detect any pointers that have been deleted already
	for(unsigned int i=0;i<m_Images.size();i++)
	{
		if (m_Images[i])
		{
		    // FIXME: In Release the 0xfeeefeee check almost certainly won't work...
			if (m_Images[i]->m_Image &&
				*(m_Images[i]->m_Image->GetSurface()) == (SDL_Surface*)0xfeeefeee)
			{
			    g_LogFile.ss() << "ERROR: trying to delete deleted SDL_Surface";
			    g_LogFile.ssend();
				//error, it's been deleted, do nothing
			}
			else
			{
				delete m_Images[i];
			}
		}
	}
	m_Images.clear();

	for(unsigned int i=0;i<m_CheckBoxes.size();i++)
		delete m_CheckBoxes[i];
	m_CheckBoxes.clear();

	for(unsigned int i=0;i<m_TextItems.size();i++)
		delete m_TextItems[i];
	m_TextItems.clear();

	for(unsigned int i=0;i<m_ListBoxes.size();i++)
		delete m_ListBoxes[i];
	m_ListBoxes.clear();

	for(unsigned int i=0;i<m_Sliders.size();i++)
		delete m_Sliders[i];
	m_Sliders.clear();

	if(m_Background)
		SDL_FreeSurface(m_Background);
	m_Background = nullptr;

	if(m_Border)
		SDL_FreeSurface(m_Border);
	m_Border = nullptr;
}

void cInterfaceWindow::UpdateWindow(int x, int y)
{
	// check buttons
	for(unsigned int i=0;i< m_Buttons.size();i++)
		m_Buttons[i]->IsOver(x,y);

	// check listbox scroll bars
	for(unsigned int i=0;i< m_ScrollBars.size();i++)
		m_ScrollBars[i]->IsOver(x,y);

	// check sliders
	for(unsigned int i=0;i< m_Sliders.size();i++)
		m_Sliders[i]->IsOver(x,y);
}

void cInterfaceWindow::MouseDown(int x, int y)
{  // this method added to handle draggable objects
	// check listbox scroll bars
	for(unsigned int i=0;i< m_ScrollBars.size();i++)
		m_ScrollBars[i]->MouseDown(x,y);

	// check sliders
	for(unsigned int i=0;i< m_Sliders.size();i++)
		m_Sliders[i]->MouseDown(x,y);
}

void cInterfaceWindow::Click(int x, int y, bool mouseWheelDown, bool mouseWheelUp)
{
	// things which shouldn't respond to scroll wheel up/down as clicks
	if(!mouseWheelDown && !mouseWheelUp)
	{
		// check buttons
		for(unsigned int i=0;i< m_Buttons.size();i++)
			m_Buttons[i]->ButtonClicked(x,y);
		// Check EditBoxes
		for(unsigned int i=0;i<m_EditBoxes.size();i++)
			m_EditBoxes[i]->OnClicked(x,y);
		// check check boxes
		for(unsigned int i=0;i<m_CheckBoxes.size();i++)
			m_CheckBoxes[i]->ButtonClicked(x,y);
	}
	// things which should only respond to mouse scroll wheel up/down
	else if(mouseWheelDown || mouseWheelUp)
	{
		for(unsigned int i=0;i<m_TextItems.size();i++)
			m_TextItems[i]->MouseScrollWheel(x,y,mouseWheelDown);
	}

	// check listbox scroll bars
	for(unsigned int i=0;i< m_ScrollBars.size();i++)
		m_ScrollBars[i]->ButtonClicked(x,y,mouseWheelDown,mouseWheelUp);

	// check sliders
	for(unsigned int i=0;i<m_Sliders.size();i++)
		m_Sliders[i]->ButtonClicked(x,y,mouseWheelDown,mouseWheelUp);
	// Check list boxes
	for(unsigned int i=0;i<m_ListBoxes.size();i++)
		m_ListBoxes[i]->OnClicked(x,y,mouseWheelDown,mouseWheelUp);
}

void cInterfaceWindow::Reset()
{
	for(unsigned int i=0;i<m_Images.size();i++)
	{
		if(m_Images[i]->m_loaded == false)
		{
			m_Images[i]->m_Image = nullptr;
		}
	}

	for(unsigned int i=0;i<m_ListBoxes.size();i++)
	{
		m_ListBoxes[i]->ClearList();
	}
}

void cInterfaceWindow::Draw()
{
	if(m_Background && m_Border)
	{
		// Draw the window
		SDL_Rect offset;
		offset.x = m_XPos;
		offset.y = m_YPos;

		// blit to the screen
		SDL_BlitSurface(m_Border, nullptr, g_Graphics.GetScreen(), &offset);

		offset.x = m_XPos+m_BorderSize;
		offset.y = m_YPos+m_BorderSize;
		SDL_BlitSurface(m_Background, nullptr, g_Graphics.GetScreen(), &offset);
	}

	if(m_BackgroundSurface)
	{
		SDL_Rect clip;
		clip.x = m_XPos+m_BorderSize;
		clip.y = m_YPos+m_BorderSize;
		clip.w = m_Width-(m_BorderSize*2);
		clip.h = m_Height-(m_BorderSize*2);

		m_BackgroundSurface->DrawSurface(clip.x,clip.y, nullptr,&clip,true);
	}

	// draw Images
	for(unsigned int i=0;i< m_Images.size();i++)
		m_Images[i]->Draw();


	// Draw Editboxes
	for(unsigned int i=0;i<m_EditBoxes.size();i++)

		m_EditBoxes[i]->Draw();

	// Draw Text item boxes
	for(unsigned int i=0;i<m_TextItems.size();i++)

		m_TextItems[i]->Draw();


	// Draw list boxes
	for(unsigned int i=0;i<m_ListBoxes.size();i++)
		m_ListBoxes[i]->Draw();

	// Draw sliders
	for(unsigned int i=0;i<m_Sliders.size();i++)
		m_Sliders[i]->Draw();


	// draw buttons
	for(unsigned int i=0;i<m_Buttons.size();i++)
		m_Buttons[i]->Draw();


	// draw listbox scroll bars
	for(unsigned int i=0;i<m_ScrollBars.size();i++)
		m_ScrollBars[i]->Draw();


	// draw check boxes
	for(unsigned int i=0;i<m_CheckBoxes.size();i++)
		m_CheckBoxes[i]->Draw();
}

void cInterfaceWindow::AddButton(const char *img_name, int & ID,  int x, int y, int width, int height, bool transparency, bool scale,bool cached)
{
	DirPath dp = ButtonPath(img_name);
    std::string on = std::string(dp.c_str()) + "On.png";
    std::string off = std::string(dp.c_str()) + "Off.png";
    std::string disabled;
	disabled = std::string(dp.c_str()) + "Disabled.png";
	AddButton(off, disabled, on, ID, x, y, width, height, transparency, scale,cached);
}

void cInterfaceWindow::AddButtonND(const char *img_name, int & ID,  int x, int y, int width, int height, bool transparency, bool scale,bool cached)
{
	DirPath dp = ButtonPath(img_name);
    std::string on = std::string(dp.c_str()) + "On.png";
    std::string off = std::string(dp.c_str()) + "Off.png";
    std::string disabled;
	AddButton(off, disabled, on, ID, x, y, width, height, transparency, scale, cached);
}

void cInterfaceWindow::AddButton(std::string image_name, int & ID,  int x, int y, int width, int height, bool transparency, bool scale,bool cached)
{
	DirPath dp = ButtonPath(image_name);
    std::string on = std::string(dp.c_str()) + "On.png";
    std::string off = std::string(dp.c_str()) + "Off.png";
    std::string disabled = std::string(dp.c_str()) + "Disabled.png";
	AddButton(off, disabled, on, ID, x, y, width, height, transparency, scale,cached);
}

void cInterfaceWindow::AddButtonND(std::string image_name, int & ID,  int x, int y, int width, int height, bool transparency, bool scale,bool cached)
{
	DirPath dp = ButtonPath(image_name);
    std::string on = std::string(dp.c_str()) + "On.png";
    std::string off = std::string(dp.c_str()) + "Off.png";
    std::string disabled = "";
	AddButton(off, disabled, on, ID, x, y, width, height, transparency, scale,cached);
}

void cInterfaceWindow::AddButton(std::string OffImage, std::string DisabledImage, std::string OnImage, int & ID,  int x, int y, int width, int height, bool transparency, bool scale,bool cached)
{
	if(scale)
	{
		width = (int)((float)width*m_xRatio);
		height = (int)((float)height*m_yRatio);
		x = (int)((float)x*m_xRatio);
		y = (int)((float)y*m_yRatio);
	}
	ID=m_Buttons.size();
	// create button
	cButton* newButton = new cButton();
	newButton->CreateButton(OffImage,DisabledImage,OnImage,ID, x+m_XPos, y+m_YPos, width, height, transparency,cached);

	// Store button
	m_Buttons.push_back(newButton);
}

void cInterfaceWindow::AddScrollBar(int & ID, int x, int y, int width, int height, int visibleitems)
{
	ID=m_ScrollBars.size();
	// create scroll bar
	g_LogFile.write("initializing scrollbar");
	cScrollBar* newScrollBar = new cScrollBar();
	g_LogFile.write("creating scrollbar");
	newScrollBar->CreateScrollBar(ID, x+m_XPos, y+m_YPos, width, height, visibleitems);

	// Store scroll bar
	g_LogFile.write("storing scrollbar");
	m_ScrollBars.push_back(newScrollBar);
}

void cInterfaceWindow::AddTextItemScrollBar(int id)
{	// adding scrollbar to a TextItem
	int x = m_TextItems[id]->GetXPos();
	int y = m_TextItems[id]->GetYPos();
	int width = m_TextItems[id]->GetWidth();
	int height = m_TextItems[id]->GetHeight();
	int newID = m_ScrollBars.size();
	AddScrollBar(newID, x+width-m_XPos-15, y-m_YPos, 16, height, height);
	m_TextItems[id]->m_ScrollBar = m_ScrollBars[newID];  // give TextItem pointer to scrollbar
	m_ScrollBars[newID]->ParentPosition = &m_TextItems[id]->m_ScrollChange;  // give scrollbar pointer to value it should update
	m_ScrollBars[newID]->m_ScrollAmount = m_TextItems[id]->m_Font.GetFontHeight();
	m_ScrollBars[newID]->m_PageAmount = height - m_TextItems[id]->m_Font.GetFontHeight();
}

void cInterfaceWindow::HideImage(int id, bool hide)
{
	if(id == -1) return;
	if(hide) {
		//g_LogFile.ss() << "hiding image ID " << id << std::endl;
		//g_LogFile.ssend();
		m_Images[id]->hide();
	}
	else {
		//g_LogFile.ss() << "unhiding image ID " << id << std::endl;
		//g_LogFile.ssend();
		m_Images[id]->unhide();
	}
}

void cInterfaceWindow::HideButton(int id, bool hide)
{
	if(id == -1) return;
	if(hide) {
		m_Buttons[id]->hide();
	}
	else {
		m_Buttons[id]->unhide();
	}
}

void cInterfaceWindow::AddImage(int & id, std::string filename, int x, int y, int width, int height, bool statImage, int R, int G, int B)
{
	width = (int)((float)width*m_xRatio);
	height = (int)((float)height*m_yRatio);
	x = (int)((float)x*m_xRatio);
	y = (int)((float)y*m_yRatio);

	// create image
	id=m_Images.size();
	cImageItem* newImage = new cImageItem();
	newImage->CreateImage(id, filename, x+m_XPos, y+m_YPos, width, height, statImage, R, G, B);

	// Store button
	m_Images.push_back(newImage);
}

void cInterfaceWindow::SetImage(int id, std::shared_ptr<CSurface> image)
{
	m_Images[id]->m_Image = image;
	m_Images[id]->m_AnimatedImage = nullptr;
}

void cInterfaceWindow::SetImage(int id, cAnimatedSurface* image)
{
	m_Images[id]->m_AnimatedImage = image;
}

void cInterfaceWindow::AddEditBox(int & ID, int x, int y, int width, int height, int BorderSize)
{
	width = (int)((float)width*m_xRatio);
	height = (int)((float)height*m_yRatio);
	x = (int)((float)x*m_xRatio);
	y = (int)((float)y*m_yRatio);

	// create button
	ID=m_EditBoxes.size();
	cEditBox* newEditBox = new cEditBox();
	newEditBox->CreateEditBox(ID, x+m_XPos, y+m_YPos, width, height, BorderSize);

	// Store button
	m_EditBoxes.push_back(newEditBox);
}

void cInterfaceWindow::DisableButton(int id, bool disable)
{
	// diable button
	 m_Buttons[id]->SetDisabled(disable);

}

void cInterfaceWindow::CreateWindow(int x, int y, int width, int height, int BorderSize)
{
	m_xRatio = 1.0f;
	m_yRatio = 1.0f;

	if(g_ScreenWidth != 800)
		m_xRatio = ((float)g_ScreenWidth/(float)800);
	if(g_ScreenHeight != 600)
		m_yRatio = ((float)g_ScreenHeight/(float)600);

	width = (int)((float)width*m_xRatio);
	height = (int)((float)height*m_yRatio);
	x = (int)((float)x*m_xRatio);
	y = (int)((float)y*m_yRatio);

	m_BorderSize = BorderSize;
	SetPosition(x,y,width,height);
	m_Border = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0,0,0,0);
	SDL_FillRect(m_Border, nullptr,SDL_MapRGB(m_Border->format,g_WindowBorderR,g_WindowBorderG,g_WindowBorderB));

	m_Background = SDL_CreateRGBSurface(SDL_SWSURFACE, width-(BorderSize*2), height-(BorderSize*2), 32, 0,0,0,0);
	SDL_FillRect(m_Background, nullptr,SDL_MapRGB(m_Background->format,g_WindowBackgroundR,g_WindowBackgroundG,g_WindowBackgroundB));
}

void cInterfaceWindow::SetBackgroundImage(std::string file)
{
	m_BackgroundSurface.reset( new CSurface(file) );
}

std::string cInterfaceWindow::GetEditBoxText(int ID)
{
	return m_EditBoxes[ID]->GetText();
}

void cInterfaceWindow::UpdateEditBoxes(char key, bool upper)
{
	for(unsigned int i=0;i<m_EditBoxes.size();i++)
		if(m_EditBoxes[i]->m_HasFocus)
		{
			m_EditBoxes[i]->UpdateText(key, upper);
		}
}

void cInterfaceWindow::Focused()
{
	// clear any events
	g_InterfaceEvents.ClearEvents();

	// clear edit boxes and set the first one as focused
for(unsigned int i=0;i<m_EditBoxes.size();i++)
m_EditBoxes[i]->ClearText();
	if(!m_EditBoxes.empty())
		m_EditBoxes[0]->m_HasFocus = true;
}

void cInterfaceWindow::SetCheckBox(int ID, bool on)
{
	 m_CheckBoxes[ID]->SetState(on);
}

void cInterfaceWindow::DisableCheckBox(int ID, bool disable)
{
	m_CheckBoxes[ID]->m_Disabled = disable;
}

bool cInterfaceWindow::IsCheckboxOn(int ID)
{
	return m_CheckBoxes[ID]->m_StateOn;
}

void cInterfaceWindow::AddSlider(int & ID, int x, int y, int width, int min, int max, int increment, int value, bool live_update)
{
	width = (int)((float)width*m_xRatio);
	float height = m_yRatio;
	x = (int)((float)x*m_xRatio);
	y = (int)((float)y*m_yRatio);

	ID=m_Sliders.size();
	cSlider* newSlider = new cSlider();
	newSlider->CreateSlider(ID, x+m_XPos, y+m_YPos, width, min, max, increment, value, height);
	newSlider->LiveUpdate(live_update);

	m_Sliders.push_back(newSlider);
}

void cInterfaceWindow::DisableSlider(int ID, bool disable)
{
	if(ID == -1) return;
	m_Sliders[ID]->Disable(disable);
}

void cInterfaceWindow::HideSlider(int ID, bool hide)
{
	if(ID == -1) return;
	m_Sliders[ID]->Hide(hide);
}

void cInterfaceWindow::SliderLiveUpdate(int ID, bool live_update)
{
	if(ID == -1) return;
	m_Sliders[ID]->LiveUpdate(live_update);
}

int cInterfaceWindow::SliderRange(int ID, int min, int max, int value, int increment)
{
	if(ID == -1) return 0;
	return m_Sliders[ID]->SetRange(min, max, value, increment);
}

int cInterfaceWindow::SliderValue(int ID)
{
	if(ID == -1) return 0;
	return m_Sliders[ID]->Value();
}

int cInterfaceWindow::SliderValue(int ID, int value)
{
	if(ID == -1) return 0;
	return m_Sliders[ID]->Value(value);
}

void cInterfaceWindow::AddCheckbox(int & ID, int x, int y, int width, int height, std::string text, int size)
{
	width = (int)((float)width*m_xRatio);
	height = (int)((float)height*m_yRatio);
	x = (int)((float)x*m_xRatio);
	y = (int)((float)y*m_yRatio);

	// create checkbox item
	ID=m_CheckBoxes.size();
	cCheckBox* newCheckBox = new cCheckBox();
	newCheckBox->CreateCheckBox(ID, x+m_XPos, y+m_YPos, width, height, text, size);

	// Store text item
	m_CheckBoxes.push_back(newCheckBox);
}

void cInterfaceWindow::AddTextItem(int & ID, int x, int y, int width, int height, std::string text, int size, bool auto_scrollbar, bool force_scrollbar)
{
	width = (int)((float)width*m_xRatio);
	height = (int)((float)height*m_yRatio);
	x = (int)((float)x*m_xRatio);
	y = (int)((float)y*m_yRatio);

	// create text item
	ID=m_TextItems.size();
	cTextItem* newTextItem = new cTextItem();
	newTextItem->CreateTextItem(ID, x+m_XPos, y+m_YPos, width, height, text, size, auto_scrollbar, force_scrollbar);

	// Store text item
	m_TextItems.push_back(newTextItem);
}

void cInterfaceWindow::HideText(int id, bool hide)
{
	if(id == -1) return;
	if(hide) {
		m_TextItems[id]->hide();
	}
	else {
		m_TextItems[id]->unhide();
	}
}

void cInterfaceWindow::EditTextItem(std::string text, int ID)
{
	if(ID == -1) return;

	// force width to full (no scrollbar) to properly detect if scrollbar is needed for new text
	m_TextItems[ID]->m_Font.SetMultiline(true, m_TextItems[ID]->GetWidth(), m_TextItems[ID]->GetHeight());
	m_TextItems[ID]->SetText(text);

	if(m_TextItems[ID]->GetHeight() == 0 || text == "")
	{
		if(m_TextItems[ID]->m_ScrollBar)
		{  // has scrollbar but doesn't need one since there doesn't seem to be any text at the moment; hide scrollbar
			m_TextItems[ID]->m_ScrollBar->SetTopValue(0);
			m_TextItems[ID]->m_ScrollBar->hide();
		}
		return;
	}

	m_TextItems[ID]->m_ScrollChange = 0;

	if(m_TextItems[ID]->NeedScrollBarHidden())  // hide scrollbar
	{
		m_TextItems[ID]->m_ScrollBar->SetTopValue(0);
		m_TextItems[ID]->m_ScrollBar->hide();
	}
	else if(m_TextItems[ID]->NeedScrollBarShown())  // un-hide existing scrollbar
		m_TextItems[ID]->m_ScrollBar->unhide();
	else if(m_TextItems[ID]->NeedScrollBar())  // add scrollbar
		AddTextItemScrollBar(ID);

	// update scrollbar if it exists
	if(m_TextItems[ID]->m_ScrollBar)
	{
		if(!m_TextItems[ID]->m_ScrollBar->m_Hidden)
		{  // also, re-render text in narrower space to accommodate scrollbar width
			m_TextItems[ID]->m_Font.SetMultiline(true, m_TextItems[ID]->GetWidth()-17, m_TextItems[ID]->GetHeight());
			m_TextItems[ID]->SetText(text);
		}
		m_TextItems[ID]->m_ScrollBar->m_ItemsTotal = m_TextItems[ID]->HeightTotal();
	}
}

void cInterfaceWindow::AddListBox(int & ID, int x, int y, int width, int height, int BorderSize, bool enableEvents, bool MultiSelect, bool ShowHeaders, bool HeaderDiv, bool HeaderSort)
{
	width = (int)((float)width*m_xRatio);
	height = (int)((float)height*m_yRatio);
	x = (int)((float)x*m_xRatio);
	y = (int)((float)y*m_yRatio);

	g_LogFile.write("Adding listbox...");
	// create listbox item
	ID=m_ListBoxes.size();
	g_LogFile.write("initializing listbox");
	cListBox* newListBox = new cListBox();
	g_LogFile.write("creating listbox");
	newListBox->CreateListbox(ID, x+m_XPos, y+m_YPos, width, height, BorderSize, MultiSelect, ShowHeaders, HeaderDiv, HeaderSort);
	g_LogFile.write("enabling events");
	newListBox->m_EnableEvents = enableEvents;

	// Store listbox item
	DirPath up,down;
	up = ButtonPath("Up");
	down = ButtonPath("Down");

	// if showing headers and allowing header clicks to sort list, offset scrollbar and scroll up button
	int header_offset = (ShowHeaders && HeaderSort) ? 21 : 0;

	g_LogFile.write("getting scrollbar ID");
	newListBox->m_ScrollDragID = m_ScrollBars.size();
	g_LogFile.write("adding scrollbar");
	AddScrollBar(newListBox->m_ScrollDragID, x+width-16, y+header_offset+1, 16, height-header_offset-2, newListBox->m_NumDrawnElements);
	newListBox->m_ScrollBar = m_ScrollBars[newListBox->m_ScrollDragID];
	m_ScrollBars[newListBox->m_ScrollDragID]->ParentPosition = &newListBox->m_ScrollChange;
	g_LogFile.write("pushing listbox onto stack");
	m_ListBoxes.push_back(newListBox);
}

int cInterfaceWindow::GetListBoxSize(int ID)
{
	return m_ListBoxes[ID]->GetSize();
}

void cInterfaceWindow::ScrollListBoxDown(int ID)
{
	m_ListBoxes[ID]->ScrollDown();
}

void cInterfaceWindow::ScrollListBoxUp(int ID)
{
	m_ListBoxes[ID]->ScrollUp();
}

void cInterfaceWindow::SetSelectedItemText(int listBoxID, int itemID, std::string data)
{
	m_ListBoxes[listBoxID]->SetElementText(itemID, data);
}

void cInterfaceWindow::AddToListBox(int listBoxID, int dataID, std::string data, int color)
{
    std::string datarray[] = {data};
	AddToListBox(listBoxID, dataID, datarray, 1, color);
}

void cInterfaceWindow::SetSelectedItemText(int listBoxID, int itemID, std::string data[], int columns)
{
	m_ListBoxes[listBoxID]->SetElementText(itemID, data, columns);
}

void cInterfaceWindow::SetSelectedItemColumnText(int listBoxID, int itemID, std::string data, int column)
{
	m_ListBoxes[listBoxID]->SetElementColumnText(itemID, data, column);
}

void cInterfaceWindow::SetSelectedItemTextColor(int listBoxID, int itemID, SDL_Color* text_color)
{
	m_ListBoxes[listBoxID]->SetElementTextColor(itemID, text_color);
}

void cInterfaceWindow::FillSortedIDList(int listBoxID, std::vector<int> *id_vec, int *vec_pos)
{
	m_ListBoxes[listBoxID]->GetSortedIDList(id_vec, vec_pos);
}

void cInterfaceWindow::AddToListBox(int listBoxID, int dataID, std::string data[], int columns, int color)
{
	m_ListBoxes[listBoxID]->AddElement(dataID, data, columns, color);

	// update "item total" reference for scroll bar
	m_ScrollBars[m_ListBoxes[listBoxID]->m_ScrollDragID]->m_ItemsTotal = m_ListBoxes[listBoxID]->m_NumElements;
}

void cInterfaceWindow::SortColumns(int listBoxID, std::string column_name[], int columns)
{
	m_ListBoxes[listBoxID]->SetColumnSort(column_name, columns);
}

void cInterfaceWindow::DefineColumns(int listBoxID, std::string name[], std::string header[], int offset[], bool skip[], int columns)
{
	m_ListBoxes[listBoxID]->DefineColumns(name, header, offset, skip, columns);
}

void cInterfaceWindow::SortListItems(int listBoxID, std::string column_name, bool Desc)
{
	m_ListBoxes[listBoxID]->SortByColumn(column_name, Desc);
}

std::string cInterfaceWindow::HeaderClicked(int listBoxID)
{
    std::string clicked = m_ListBoxes[listBoxID]->m_HeaderClicked;
	m_ListBoxes[listBoxID]->m_HeaderClicked = "";
	return clicked;
}

int cInterfaceWindow::GetNextSelectedItemFromList(int listBoxID, int from, int& pos)
{
	return m_ListBoxes[listBoxID]->GetNextSelected(from, pos);
}

int cInterfaceWindow::GetLastSelectedItemFromList(int listBoxID)
{
	return m_ListBoxes[listBoxID]->GetLastSelected();
}

int cInterfaceWindow::GetSelectedItemFromList(int listBoxID)
{
	return m_ListBoxes[listBoxID]->GetSelected();
}

std::string cInterfaceWindow::GetSelectedTextFromList(int listBoxID)
{
	return m_ListBoxes[listBoxID]->GetSelectedText();
}

int cInterfaceWindow::GetAfterSelectedItemFromList(int listBoxID)
{
	return m_ListBoxes[listBoxID]->GetAfterSelected();
}

bool cInterfaceWindow::ListDoubleClicked(int listBoxID)
{
	return m_ListBoxes[listBoxID]->DoubleClicked();
}

void cInterfaceWindow::SetSelectedItemInList(int listBoxID, int itemID, bool ev, bool DeselectOthers)
{
	if(itemID == -1)
		return;

	m_ListBoxes[listBoxID]->SetSelected(itemID, ev, DeselectOthers);
}

int cInterfaceWindow::ArrowDownListBox(int ID)
{
	if(m_ListBoxes.empty())
		return -1;

	return m_ListBoxes[ID]->ArrowDownList();

}

bool cInterfaceWindow::IsMultiSelected(int ID)
{
	if(m_ListBoxes.empty())
		return 0;

	return m_ListBoxes[ID]->HasMultiSelected();
}

int cInterfaceWindow::ArrowUpListBox(int ID)
{
if(m_ListBoxes.empty())
		return -1;

	return m_ListBoxes[ID]->ArrowUpList();
}

void cInterfaceWindow::ClearListBox(int ID)
{
	if(m_ListBoxes.empty())
		return;

	m_ListBoxes[ID]->ClearList();

	// update "item total" reference for scroll bar
	m_ScrollBars[m_ListBoxes[ID]->m_ScrollDragID]->m_ItemsTotal = 0;
	m_ScrollBars[m_ListBoxes[ID]->m_ScrollDragID]->SetTopValue(0);
}

void cInterfaceWindow::SetListBoxPosition(int ID, int pos)
{
	if(m_ListBoxes.empty())
		return;

	m_ListBoxes[ID]->m_Position=pos;

	m_ScrollBars[m_ListBoxes[ID]->m_ScrollDragID]->SetTopValue(pos);
}

cSelector::cSelector( cInterfaceWindow& a_win, int a_id )
    : m_Id( a_id ),
    m_Win( a_win )
{
    ;
}

cSelector& cSelector::operator =( const cSelector& other )
{
    if( this != &other )
    {
        m_Id = other.m_Id;
        m_Pos = other.m_Pos;
        m_Win = other.m_Win;
    }
    return *this;
}

int cSelector::first()
{
    m_Pos = 0;
    return m_Win.GetNextSelectedItemFromList( m_Id, 0, m_Pos );
}

int cSelector::next()
{
    return m_Win.GetNextSelectedItemFromList( m_Id, m_Pos + 1, m_Pos );
}

} // namespace WhoreMasterRenewal
