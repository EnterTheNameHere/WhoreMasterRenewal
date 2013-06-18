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
#ifndef CLISTBOX_H_INCLUDED_1524
#define CLISTBOX_H_INCLUDED_1524
#pragma once

#include "Constants.h"
#include "cInterfaceObject.h" // required inheritance
#include "cFont.h" // required cFont

#include <SDL_video.h> // required SDL_Rect

#include <vector>
#include <string>

namespace WhoreMasterRenewal
{

class cScrollBar;

class cListItem
{
public:
    cListItem();
    ~cListItem();
    
    cListItem( const cListItem& ) = delete;
    cListItem& operator = ( const cListItem& ) = delete;
    
    int m_Color = 0;
    bool m_Selected = false;
    std::string m_Data[LISTBOX_COLUMNS + 1]; // the text to display, up to LISTBOX_COLUMNS number of columns (+1 is used for "original sort" slot)
    int m_ID = 0;   // the id for the item
    SDL_Color* m_TextColor = nullptr;
    cListItem* m_Next = nullptr;    // pointer to the next object in the list
};

class cListBox : public cInterfaceObject
{
public:
    cListBox();
    virtual ~cListBox();
    
    cListBox( const cListBox& ) = delete;
    cListBox& operator = ( const cListBox& ) = delete;
    
    bool IsOver( int x, int y );
    void OnClicked( int x, int y, bool mouseWheelDown = false, bool mouseWheelUp = false );
    
    void CreateListbox( int ID, int x, int y, int width, int height, int BorderSize, bool MultiSelect, bool ShowHeaders = false, bool HeaderDiv = true, bool HeaderSort = true );
    
    void Draw();
    
    void AddElement( int ID, std::string data, int color = LISTBOX_BLUE );
    void SetElementText( int ID, std::string data );
//  void RemoveElement(int ID);

    void ClearList();
    
    int ArrowDownList();
    int ArrowUpList();
    
    bool HasMultiSelected();
    
    void ScrollDown( int amount = 1, bool updatebar = true );
    void ScrollUp( int amount = 1, bool updatebar = true );
    
    int GetSelected();  // returns the id for the first selected element
    std::string GetSelectedText();
    int GetAfterSelected(); // returns the id for the element after the last selected element
    int GetNextSelected( int from, int& pos );  // returns the id for the next selected element and sets pos to its position
    int GetLastSelected();  // gets the last item selected
    bool IsSelected();  // returns true if an element is selected
    void SetSelected( int ID, bool ev = true, bool deselect_others = true ); // sets the selected item
    int GetSize() { return m_NumElements; }
    bool IsMultiSelect() { return m_MultiSelect; }
    void GetSortedIDList( std::vector<int> *id_vec, int *vec_pos ); // fills up a vector with the list of item IDs, sorted
    void GetColumnNames( std::vector<std::string>& columnNames );
    
    void DefineColumns( std::string name[], std::string header[], int offset[], bool skip[], int columns ); // define column layout
    void SetColumnSort( std::string column_name[], int columns ); // Update column sorting based on expected default order
    void AddElement( int ID, std::string data[], int columns, int color = LISTBOX_BLUE );
    void SetElementText( int ID, std::string data[], int columns );
    void SetElementColumnText( int ID, std::string data, int column );
    void SetElementTextColor( int ID, SDL_Color* text_color );
    
    void SortByColumn( std::string ColumnName, bool Descending = false ); // re-sort list items based on specified column
    void ReSortList(); // re-sort list again, if needed
    void UnSortList(); // un-sort list back to the order the elements were originally added in
    cListItem* BubbleSortList( cListItem *head, int count, int col_id, bool Descending );
    bool StrCmpIn( const std::string &left, const std::string &right );
    
    // Double-click detection
    bool DoubleClicked();
    
    int m_ScrollChange = -1; // scrollbar changes will update this value
    int m_ID = 0;
    cListItem* m_Items = nullptr; // The list of elements in the list
    cListItem* m_LastItem = nullptr;  // The last elements of the list
    cListItem* m_LastSelected = nullptr;
    int m_Position = 0; // What element is at position 0 on the list
    int m_NumElements = 0;  // number of elements in the list
    int m_NumDrawnElements = 0; // how many elements can be rendered at a time
    int m_eHeight = 0; // the height of element images
    int m_eWidth = 0; // the width of element images
    int m_ScrollDragID = 0; // the id for the scroll bar
    
    // Multi-Column Support
    int m_ColumnCount = 1; // how many columns to display
    int m_ColumnOffset[LISTBOX_COLUMNS]; // x offset for each column
    bool m_ShowHeaders = false; // whether to show column headers
    bool m_HeaderDividers = true; // whether to show dividers between column headers
    std::string m_Header[LISTBOX_COLUMNS]; // text of column headers
    std::string m_ColumnName[LISTBOX_COLUMNS];  // reference names of columns, for sorting
    int m_ColumnSort[LISTBOX_COLUMNS]; // reference table used for custom sort of columns
    bool m_SkipColumn[LISTBOX_COLUMNS]; // used to skip a column which would normally be shown
    
    std::string m_HeaderClicked = {""}; // set to m_ColumnName value of a header that has just been clicked; otherwise empty
    
    SDL_Surface* m_HeaderBackground = nullptr; // the background and border for the multi-column header box
    static SDL_Surface* m_HeaderSortAsc; // image used on a column header when sorting "ascending" on that column
    static SDL_Surface* m_HeaderSortDesc; // image used on a column header when sorting "descending" on that column
    SDL_Surface* m_HeaderSortBack = nullptr; // the above two images are copied resized and stored here for actual use
    static SDL_Surface* m_HeaderUnSort; // image used for the extra "un-sort" header which removes any custom sort
    
    SDL_Rect m_Divider = {0, 0, 0, 0};
    
    //sorting of list, normally based on header clicks
    bool m_HeaderClicksSort = false; // whether clicks on column headers should sort data accordingly
    std::string m_SortedColumn = "Default cListBox::m_SortedColumn value"; // m_ColumnName of column which sort is currently based on
    bool m_SortedDescending = false; // descending or ascending sort
    Uint32 m_CurrentClickTime = 0;
    int m_CurrentClickX = 0;
    int m_CurrentClickY = 0;
    Uint32 m_LastClickTime = 0;
    int m_LastClickX = 0;
    int m_LastClickY = 0;
    
    cFont m_Font = {};
    
    int m_BorderSize = 0;
    SDL_Surface* m_Background = nullptr; // the background and border for the list item
    SDL_Surface* m_RedBackground = nullptr; // the background used for important things
    SDL_Surface* m_DarkBlueBackground = nullptr; // the background used for important things
    SDL_Surface* m_Border = nullptr;
    
    SDL_Surface* m_SelectedRedBackground = nullptr; // the background used for selected important things
    SDL_Surface* m_SelectedDarkBlueBackground = nullptr; // the background used for selected important things
    
    SDL_Surface* m_ElementBackground = nullptr; // the background and border for the list elements
    SDL_Surface* m_ElementSelectedBackground = nullptr; // the background and border for the list elements
    SDL_Surface* m_ElementBorder = nullptr;
    bool m_EnableEvents = false; // are events enabled
    bool m_MultiSelect = false;
    bool m_HasMultiSelect = false;
    
    cScrollBar* m_ScrollBar = nullptr; // pointer to the associated scrollbar
    
    cListBox* m_Next = nullptr;
};

} // namespace WhoreMasterRenewal

#endif // CLISTBOX_H_INCLUDED_1524
