/*=========================================================================

                         >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

                            -----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                            -----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/


#include <wg_caretinstance.h>

#ifndef WG_TEXTFIELD_DOT_H
#	include <wg_textfield.h>
#endif

#ifndef WG_TEXTTOOL_DOT_H
#	include <wg_texttool.h>
#endif

#ifndef WG_FONT_DOT_H
#	include <wg_font.h>
#endif

#include <wg_widget.h>

//____ Constructor() __________________________________________________________

WgCaretInstance::WgCaretInstance( WgTextField& text )
{
	m_pText 		= &text;
	m_bHidden		= true;
	m_bInsert		= true;
	m_bSelectMode	= false;
	m_wantedOfsX	= -1;

	m_time 			= 0;

	m_line 			= text.Lines()-1;
	m_column 		= text.getLine(m_line)->nChars;
	m_selStartLine	= m_line;
	m_selStartColumn= m_column;
}

//____ incTime() _______________________________________________________________

bool WgCaretInstance::incTime( int ms )
{
	m_time += ms;
	return true; //TODO: Should only return true if cursor needs to be re-rendered.
}

//____ hide() _________________________________________________________________

void WgCaretInstance::hide()
{
	m_bHidden = true;
}

//____ show() _________________________________________________________________

void WgCaretInstance::show()
{
	m_bHidden = false;
}

//____ isHidden() _____________________________________________________________

bool WgCaretInstance::isHidden() const
{
	return m_bHidden;
}



//____ insertMode() ____________________________________________________________

void WgCaretInstance::insertMode( bool bInsert )
{
	m_bInsert = bInsert;
}


//____ gotoSoftLine() _________________________________________________________

void WgCaretInstance::gotoSoftLine( int line, const WgRect& container )
{
	// First we need to update m_wantedOfsX...

	if( m_wantedOfsX == -1 )
	{
		int	ln = m_line;
		int	col = m_column;
		m_pText->posHard2Soft( ln, col );

		m_wantedOfsX = m_pText->getSoftLineWidthPart( ln, 0, col );
	}

	// Set our line and convert to hard

	int ln = line;
	int col = m_pText->CoordToColumn( line, WgCoord( m_wantedOfsX+container.x, 0), container, true );


	m_pText->posSoft2Hard( ln, col );
	_updateLocation(ln, col);
}


//____ gotoColumn() ____________________________________________________________

void WgCaretInstance::gotoColumn( int col )
{
	const int minLine = 0;
	const int maxLine = m_pText->Lines()-1;

	int line = m_line;

	if( line > maxLine )
		line = maxLine;

	int maxCol = m_pText->getLine(line)->nChars;

	// Check whether to hop down/up on lines
	if(col < 0)
	{
		line = line - 1;

		// Have we reached the top?
		if(line < minLine)
		{
			// Yes
			line = minLine;
			col = 0;
		}
		else
		{
			// No, set cursor column to end of line
			col = m_pText->getLine(line)->nChars;
		}
	}
	else if( col > (int)maxCol )
	{
		line = line + 1;

		// Have we reached the bottom?
		if(line > maxLine)
		{
			// Yes, set cursor on end of line
			line = maxLine;
			col = m_pText->getLine(line)->nChars;
		}
		else
		{
			// No, set cursor column to 0
			col = 0;
		}
	}

	_updateLocation(line, col);

	m_wantedOfsX = -1;
}

//____ gotoPrevWord() _________________________________________________________

void WgCaretInstance::gotoPrevWord()
{
	gotoBeginningOfWord();

	if(m_line == 0 && m_column == 0)
		return;

	int line = m_line;
	int col = m_column;

	if(line > m_pText->Lines() - 1)
		line = m_pText->Lines() - 1;

	if(col > m_pText->getLine(line)->nChars)
		col = m_pText->getLine(line)->nChars;

	const WgChar* pText = m_pText->getLineText(line) + col - 1;

	while( !isspace(pText->Glyph()) && !ispunct(pText->Glyph()) )
	{
		if(col == 0)
		{
			if(line == 0)
				break;
			line--;
			col = m_pText->getLine(line)->nChars;
		}
		else
		{
			col--;
			if(line == 0 && col == 0)
				break;
		}
		pText--;
	}

	_updateLocation(line, col);
	m_wantedOfsX = -1;
}

//_____________________________________________________________________________
void WgCaretInstance::gotoBeginningOfWord()
{
	if(m_line == 0 && m_column == 0)
		return;

	int line = m_line;
	int col = m_column;

	if(line > m_pText->Lines() - 1)
		line = m_pText->Lines() - 1;

	if(col > m_pText->getLine(line)->nChars)
		col = m_pText->getLine(line)->nChars;

	const WgChar* pText = m_pText->getLineText(line) + col - 1;

	while( isspace(pText->Glyph()) || ispunct(pText->Glyph()) )
	{
		if(col == 0)
		{
			if(line == 0)
				break;
			line--;
			col = m_pText->getLine(line)->nChars;
		}
		else
		{
			col--;
			if(line == 0 && col == 0)
				break;
		}
		pText--;
	}

	_updateLocation(line, col);
	m_wantedOfsX = -1;
}

//____ gotoNextWord() _________________________________________________________

void WgCaretInstance::gotoNextWord()
{
	gotoEndOfWord();

	int line = m_line;
	int col = m_column;

	if(line > m_pText->Lines() - 1)
		line = m_pText->Lines() - 1;

	if(col > m_pText->getLine(line)->nChars)
		col = m_pText->getLine(line)->nChars;

	const WgChar* pText = m_pText->getLineText(line) + col;

	while( !pText->IsEndOfText() && (isspace(pText->Glyph()) || ispunct(pText->Glyph())) )
	{
		if(pText->IsEndOfLine())
			line++, col = 0;
		else
			col++;
		pText++;
	}

	_updateLocation(line, col);
	m_wantedOfsX = -1;
}

//_____________________________________________________________________________
void WgCaretInstance::gotoEndOfWord()
{
	int line = m_line;
	int col = m_column;

	if(line > m_pText->Lines() - 1)
		line = m_pText->Lines() - 1;

	if(col > m_pText->getLine(line)->nChars)
		col = m_pText->getLine(line)->nChars;

	const WgChar* pText = m_pText->getLineText(line) + col;

	while( !pText->IsEndOfText() && !isspace(pText->Glyph()) && !ispunct(pText->Glyph()) )
	{
		if(pText->IsEndOfLine())
			line++, col = 0;
		else
			col++;
		pText++;
	}

	_updateLocation(line, col);
	m_wantedOfsX = -1;
}

//____ gotoHardPos() __________________________________________________________

void WgCaretInstance::gotoHardPos( int line, int col )
{
	_gotoPos( line, col );
	m_wantedOfsX = -1;
}


//____ _gotoPos() ______________________________________________________________

void WgCaretInstance::_gotoPos( int line, int col )
{
	int maxLine = m_pText->Lines()-1;
	if( line > maxLine )
		line = maxLine;

	int maxCol = m_pText->getLine(line)->nChars;
	if( col > maxCol )
		col = maxCol;

	_updateLocation(line, col);
}

//____ gotoSoftPos() __________________________________________________________

void WgCaretInstance::gotoSoftPos( int line, int col )
{
	m_pText->posSoft2Hard( line, col );

	_updateLocation(line, col);

	m_wantedOfsX = -1;
}

//____ getSoftPos() ___________________________________________________________

void WgCaretInstance::getSoftPos( int &line, int &col ) const
{
	line	= m_line;
	col		= m_column;

	m_pText->posHard2Soft( line, col );
}



//____ putChar() ______________________________________________________________

bool WgCaretInstance::putChar( Uint16 character )
{
	m_wantedOfsX = -1;

	int ret;

	int line = m_line;
	int column = m_column;

	int ofs = m_pText->LineColToOffset( line, column );

	if( m_bInsert )
		ret = m_pText->insertChar( ofs, character );

	else
		ret = m_pText->replaceChar( ofs, character );


	if( character == '\n' )
	{
		line++;
		column = 0;
	}
	else if( ret == 1 )
		column++;


	_updateLocation(line, column);

	return ret==1?true:false;
}

//____ putText() ______________________________________________________________

int	WgCaretInstance::putText( const WgCharSeq& seq )
{
	m_wantedOfsX = -1;

	int nInserted;
	int nLines = m_pText->Lines();

	int ofs = m_pText->LineColToOffset( m_line, m_column );

	if( m_bInsert )
		nInserted = m_pText->Insert( ofs, seq );
	else
		nInserted = m_pText->Replace( ofs, seq.Length(), seq );

	nLines = m_pText->Lines() - nLines;

	_updateLocation(m_line + nLines, m_column + nInserted);

	return nInserted;
}

//______________________________________________________________
void WgCaretInstance::unputText( int nChar )
{
	if(nChar < 0)
		return;

	// TODO: FIX!
	for(; nChar; nChar--)
		delPrevChar();
}

//_____________________________________________________________________
void WgCaretInstance::delPrevWord()
{
	int	ofs1 = m_pText->LineColToOffset( m_line, m_column );
	gotoPrevWord();
	int ofs2 = m_pText->LineColToOffset( m_line, m_column );
	m_pText->Delete( ofs2, ofs1-ofs2 );
}

//_____________________________________________________________________
void WgCaretInstance::delNextWord()
{
	int	ofs1 = m_pText->LineColToOffset( m_line, m_column );
	int line = m_line;
	int column = m_column;
	gotoNextWord();
	int ofs2 = m_pText->LineColToOffset( m_line, m_column );
	m_pText->Delete(ofs1, ofs2-ofs1);
	_updateLocation(line, column);
}

//____ delPrevChar() __________________________________________________________

bool WgCaretInstance::delPrevChar()
{
	m_wantedOfsX = -1;

	int column= m_column;
	int line = m_line;

	if( column == 0 )
	{
		if( line == 0 )
			return false;

		line -= 1;
		column = m_pText->getLine(line)->nChars;
	}
	else
		column--;


	int ret = m_pText->deleteChar( m_pText->LineColToOffset( m_line, m_column ) -1 );
	_updateLocation(line, column);
	return ret==1?true:false;
}

//____ delNextChar() __________________________________________________________

bool WgCaretInstance::delNextChar()
{
	m_wantedOfsX = -1;

	int ofs = m_pText->LineColToOffset( m_line, m_column );
	int nDel = m_pText->deleteChar( ofs );

	if( nDel )
		return true;
	else
		return false;
}

//____ ofsX() __________________________________________________________________

int WgCaretInstance::ofsX() const
{
	int	ln = m_line;
	int	col = m_column;
	m_pText->posHard2Soft( ln, col );

	return m_pText->getSoftLineWidthPart( ln, 0, col );
}

//____ ofsY() __________________________________________________________________

int WgCaretInstance::ofsY() const
{
	//TODO: Need a better way...

	int ofs = 0;

	for( int i = 0 ; i < (int) m_line ; i++ )
		ofs += m_pText->softLineSpacing(i);

	return ofs;
}

//____ mode() __________________________________________________________________

WgCaret::Mode WgCaretInstance::cursorMode() const
{
	if( m_line >= m_pText->Lines() || m_pText->getLine(m_line)->nChars == m_column )
		return WgCaret::EOL;

	if( m_bInsert )
		return WgCaret::INS;

	return WgCaret::OVR;
}

//____ goBOL() ________________________________________________________________

void WgCaretInstance::goBOL()
{
	int	ln = m_line;
	int	col = m_column;
	m_pText->posHard2Soft( ln, col );

	col = 0;
	m_pText->posSoft2Hard( ln, col );
	gotoHardPos( ln, col );

	m_wantedOfsX = -1;
}

//____ goEOL() ________________________________________________________________

void WgCaretInstance::goEOL()
{
	int	ln = m_line;
	int	col = m_column;
	m_pText->posHard2Soft( ln, col );

	col = INT_MAX;

	m_pText->posSoft2Hard( ln, col );
	gotoHardPos( ln, col );

	m_wantedOfsX = -1;
}

//////////////////////////////////////////////////////////////////////////
void WgCaretInstance::_updateLocation(int line, int column)
{
	m_line = line;
	m_column = column;

	if(m_bSelectMode)
	{
		m_pText->selectText(m_selStartLine, m_selStartColumn, line, column);
	}
	else
	{
		clearSelection();
	}
}

//////////////////////////////////////////////////////////////////////////
void WgCaretInstance::setSelectionMode(bool bOn)
{
	m_bSelectMode = bOn;
}

bool WgCaretInstance::hasSelection()
{
	if(m_selStartLine < 0)
		return false;

	return m_selStartLine != m_line || (m_selStartLine == m_line && m_selStartColumn != m_column);
}

void WgCaretInstance::delSelection()
{
	if(!hasSelection())
		return;

	m_wantedOfsX = -1;

	int line = m_line;
	int column = m_column;
	if(m_selStartLine < (int)m_line || (m_selStartLine == (int)m_line && m_selStartColumn < (int)m_column))
	{
		std::swap(line, m_selStartLine);
		std::swap(column, m_selStartColumn);
	}

	int ofs1 = m_pText->LineColToOffset(line, column);
	int ofs2 = m_pText->LineColToOffset(m_selStartLine, m_selStartColumn );
	m_pText->Delete(ofs1, ofs2-ofs1);

	m_selStartLine = line;
	m_selStartColumn = column;

	_updateLocation(line, column);
	clearSelection();
}

void WgCaretInstance::clearSelection()
{
	m_pText->ClearSelection();
	m_selStartLine = m_line;
	m_selStartColumn = m_column;
}

void WgCaretInstance::selectRange( WgRange range )
{
	WgTextPos beg = m_pText->OfsToPos( range.ofs );
	WgTextPos end = m_pText->OfsToPos( range.ofs + range.len );

	m_pText->ClearSelection();
	setSelectionMode(true);
	m_selStartLine = beg.line;
	m_selStartColumn = beg.col;
	gotoHardPos( end.line, end.col );
	setSelectionMode(false);
}

void WgCaretInstance::selectAll()
{
	m_pText->ClearSelection();
	setSelectionMode(true);
	m_selStartLine = 0;
	m_selStartColumn = 0;
	gotoHardPos(m_pText->Lines(), INT_MAX);
	setSelectionMode(false);
}