
#include "cTextItem.h"
#include "sConfig.h"
#include "cScrollBar.h"

cTextItem::cTextItem()
{
    m_Next = 0;
    m_ScrollBar = 0;
    m_Hide = false;
    m_AutoScrollBar = true;
    m_ForceScrollBar = false;
    m_ScrollChange = 0;
}

cTextItem::~cTextItem()
{
    if( m_Next )
        delete m_Next;
    m_Next = 0;
}

void cTextItem::CreateTextItem( int ID, int x, int y, int width, int height, std::string text, int size, bool auto_scrollbar, bool force_scrollbar )
{
    m_ID = ID;
    SetPosition( x, y, width, height );

    SetText( text );
    ChangeFontSize( size );

    m_AutoScrollBar = auto_scrollbar;
    m_ForceScrollBar = force_scrollbar;
}

void cTextItem::DisableAutoScroll( bool disable )
{
    m_AutoScrollBar = !disable;
}

void cTextItem::ForceScrollBar( bool force )
{
    m_ForceScrollBar = force;
}

// does scrollbar exist, but current text fits, and scrollbar isn't being forced?
bool cTextItem::NeedScrollBarHidden()
{
    return (m_ScrollBar && !m_ScrollBar->m_Hidden && HeightTotal() <= GetHeight() && !m_ForceScrollBar);
}

// does scrollbar exist but is hidden, and current text doesn't fit?
bool cTextItem::NeedScrollBarShown()
{
    return (m_ScrollBar && m_ScrollBar->m_Hidden && HeightTotal() > GetHeight());
}

// does a scrollbar need to be added?
bool cTextItem::NeedScrollBar()
{
    return (!m_ScrollBar && GetHeight() > 47 && (HeightTotal() > GetHeight() || m_ForceScrollBar));
}

int cTextItem::HeightTotal()
{
    return m_Font.GetHeight();
}

void cTextItem::MouseScrollWheel( int x, int y, bool ScrollDown )
{
    if(m_ScrollBar && !m_ScrollBar->m_Hidden && IsOver(x,y) )
    {
        int newpos = (ScrollDown) ? m_ScrollChange + m_Font.GetFontHeight() : m_ScrollChange - m_Font.GetFontHeight();
        if(newpos < 0)
            newpos = 0;
        else if(newpos > HeightTotal() - GetHeight())
            newpos = HeightTotal() - GetHeight();
        m_ScrollBar->SetTopValue(newpos);
        m_ScrollChange = newpos;
    }
}

bool cTextItem::IsOver( int x, int y )
{
    return ( x > m_XPos && y > m_YPos && x < m_XPos + m_Width - 15 && y < m_YPos + m_Height);
}

void cTextItem::ChangeFontSize(int FontSize)
{
    cConfig cfg;
    m_Font.LoadFont( cfg.fonts.normal(), FontSize );
    m_Font.SetText( m_Text );
    m_Font.SetColor( 0, 0, 0 );
    m_Font.SetMultiline( true, m_Width, m_Height );
}

void cTextItem::SetText( std::string text )
{
    m_Text = text;
    m_Font.SetText( m_Text );
}

void cTextItem::Draw()
{
    if( m_Hide )
    {
        return;
    }
    if( m_Text == "" )
        return;

    m_Font.DrawMultilineText( m_XPos, m_YPos, 0, m_ScrollChange );

    /*int position = 0;
    for(int i=0; i<m_LinesPerBox; i++)
    {
        char buffer[9000];
        int j;
        bool newline = false;
        bool end = false;
        int bufferPos = 0;

        // copy all characters accross
        for(j=0; j<m_CharsPerLine; j++)
        {
            if(position+j >= (signed int) m_Text.length())
            {
                buffer[bufferPos+j] = '\0';
                end = true;

                m_Font.SetText(buffer);
                m_Font.DrawText(m_XPos, m_YPos+(i*m_CharHeight)+1);

                break;
            }

            if(m_Text[position+j] == '\n')	// create new line when \n is encountered
            {
                position += j+1;

                buffer[bufferPos+j] = '\0';
                
                m_Font.SetText(buffer);
                m_Font.DrawText(m_XPos, m_YPos+(i*m_CharHeight)+1);
                
                newline = true;
                break;
            }

            buffer[bufferPos+j] = m_Text[position+j];
        }

        if(end)
            break;

        if(newline)
            continue;
            
        bufferPos = j;
        position += j;

        // check for any half done words
        if(position+1 < (signed int) m_Text.length())
        {
            if((m_Text[position] != '\n' || m_Text[position] != ' ') && (m_Text[position+1] != '\n' || m_Text[position+1] != ' '))
            {
                while(((position+1) && m_Text[position] != '\n' && m_Text[position] != ' ' ))
                {
                    buffer[bufferPos] = '\0';
                    bufferPos--;
                    position--;
                }
                buffer[bufferPos] = '\0';
            }
            else
                buffer[bufferPos] = '\0';
        }
        else
            buffer[bufferPos] = '\0';

        m_Font.SetText(buffer);
        m_Font.DrawText(m_XPos, m_YPos+(i*m_CharHeight)+1);
    }*/
}

void cTextItem::hide()
{
    m_Hide = true;
    if( m_ScrollBar && !m_ScrollBar->m_Hidden )
        m_ScrollBar->hide();
}

void cTextItem::unhide()
{
    m_Hide = false;
    if( m_ScrollBar && m_ScrollBar->m_Hidden )
        m_ScrollBar->unhide();
}
