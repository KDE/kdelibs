/* This file is part of the KDE libraries
    Copyright (C) 2000 David Smith <dsmith@algonet.se>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <stdlib.h>
#include <kdebug.h>
#include <qstring.h>
#include <qstringlist.h>
#include <kcompletion.h>

#include "kshellcompletion.h"

class KShellCompletionPrivate
{
};

KShellCompletion::KShellCompletion() : KURLCompletion()
{
	m_word_break_char = ' ';
	m_quote_char1 = '\"';
	m_quote_char2 = '\'';
	m_escape_char = '\\';
}

/*
 * makeCompletion()
 *
 * Entry point for file name completion 
 */
QString KShellCompletion::makeCompletion(const QString &text)
{
	// Split text at the last unquoted space 
	//
	splitText(text, m_text_start, m_text_compl);
	
	//kDebugInfo("KShellCompletion: split: '%s' - '%s'", 
	//	m_text_start.latin1(), m_text_compl.latin1());

	// Do exe-completion if there was no unquoted space
	//
	bool is_exe_completion = true;
	
	for ( uint i = 0; i < m_text_start.length(); i++ ) {
		if ( m_text_start[i] != m_word_break_char ) {
			is_exe_completion = false;
			break;
		}
	}
	
	Mode mode = (is_exe_completion ? ExeCompletion : FileCompletion );

	setMode(mode);

	// Make completion on the last part of text
	//
	return KURLCompletion::makeCompletion( m_text_compl );
}

/*
 * postProcessMatch, postProcessMatches
 *
 * Called by KCompletion before emitting match() and matches()
 *
 * Add add the part of the text that was not completed
 */
void KShellCompletion::postProcessMatch( QString *match )
{
	//kDebugInfo("KShellCompletion::postProcessMatch() in: '%s'",
	//	match->latin1());

	KURLCompletion::postProcessMatch( match );

	if ( *match == QString::null )
		return;
		
	match->prepend( m_text_start );

	//kDebugInfo("KShellCompletion::postProcessMatch() ut: '%s'",
	//	match->latin1());
}

void KShellCompletion::postProcessMatches( QStringList *matches )
{
	KURLCompletion::postProcessMatches( matches );

	for ( QStringList::Iterator it = matches->begin();
		  it != matches->end(); it++ )
	{
		if ( (*it) != QString::null ) 
			(*it).prepend( m_text_start );
	}
}

/*
 * splitText
 *
 * Split text at the last unquoted space 
 *
 * text_start = [out] text at the left, including the space
 * text_compl = [out] text at the right
 */
void KShellCompletion::splitText(const QString &text, QString &text_start, 
		QString &text_compl)
{
	bool in_quote = false;
	bool escaped = false;
	QChar p_last_quote_char;
	int last_unquoted_space = -1;
	int end_space_len = 0;
	
	for (uint pos = 0; pos < text.length(); pos++) {
		
		end_space_len = 0;
		
		if ( escaped ) {
			escaped = false;
		}
		else if ( in_quote && text[pos] == p_last_quote_char ) {
			in_quote = false;
		}
		else if ( !in_quote && text[pos] == m_quote_char1 ) {
			p_last_quote_char = m_quote_char1;
			in_quote = true;
		}
		else if ( !in_quote && text[pos] == m_quote_char2 ) {
			p_last_quote_char = m_quote_char2;
			in_quote = true;
		}
		else if ( text[pos] == m_escape_char ) {
			escaped = true;
		}
		else if ( !in_quote && text[pos] == m_word_break_char ) {

			end_space_len = 1;
			
			while ( pos+1 < text.length() && text[pos+1] == m_word_break_char ) {
				end_space_len++;
				pos++;
			}

			if ( pos+1 == text.length() ) 
				break; 

			last_unquoted_space = pos;
		}
	}

	text_start = text.left( last_unquoted_space + 1 );

	// the last part without trailing blanks
	text_compl = text.mid( last_unquoted_space + 1 ); 

//	text_compl = text.mid( last_unquoted_space + 1, 
//						   text.length() - end_space_len - (last_unquoted_space + 1) );

	//kDebugInfo("split right = '%s'", text_compl.latin1());
}


#include "kshellcompletion.moc"

