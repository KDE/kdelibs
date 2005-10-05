/**
 * kspell_hspelldict.cpp
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
 * Copyright (C)  2005  Mashrab Kuvatov <kmashrab@uni-bremen.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#include "kspell_hspelldict.h"
#include <kdebug.h>

#include <qtextcodec.h>

using namespace KSpell2;

HSpellDict::HSpellDict( const QString& lang )
    : Dictionary( lang )
{
   int int_error = hspell_init( &m_speller, HSPELL_OPT_DEFAULT );
   if ( int_error == -1 )
      kdDebug() << "HSpellDict::HSpellDict: Init failed" << endl;
    /* hspell understans only iso8859-8-i            */
    codec = QTextCodec::codecForName( "iso8859-8-i" );
}

HSpellDict::~HSpellDict()
{
    /* It exists in =< hspell-0.8 */
    hspell_uninit( m_speller );
}

bool HSpellDict::check( const QString& word )
{
    kdDebug() << "HSpellDict::check word = " << word <<endl;
    int preflen;
    QByteArray wordISO = codec->fromUnicode( word );
    /* returns 1 if the word is correct, 0 otherwise */
    int correct = hspell_check_word ( m_speller,
		                      wordISO,
                                      &preflen); //this going to be removed
                                                 //in next hspell releases
    /* I do not really understand what gimatria is   */
    if( correct != 1 ){
        if( hspell_is_canonic_gimatria( wordISO ) != 0 )
	correct = 1;
    }
    return correct == 1;
}

QStringList HSpellDict::suggest( const QString& word )
{
    QStringList qsug;
    struct corlist cl;
    int n_sugg;
    corlist_init( &cl );
    hspell_trycorrect( m_speller, codec->fromUnicode( word ), &cl );
    for( n_sugg = 0; n_sugg < corlist_n( &cl ); n_sugg++){
	    qsug.append( codec->toUnicode( corlist_str( &cl, n_sugg) ) );
    }
    corlist_free( &cl );
    return qsug;
}

bool HSpellDict::checkAndSuggest( const QString& word,
                                  QStringList& suggestions )
{
    bool c = check( word );
    if( c )
        suggestions = suggest( word );
    return c;
}

bool HSpellDict::storeReplacement( const QString& bad,
                                   const QString& good )
{
    // hspell-0.9 cannot do this
    kdDebug() << "HSpellDict::storeReplacement: Sorry, cannot." << endl; 
    return false;
}

bool HSpellDict::addToPersonal( const QString& word )
{
    // hspell-0.9 cannot do this
    kdDebug() << "HSpellDict::addToPersonal: Sorry, cannot." << endl; 
    return false;
}

bool HSpellDict::addToSession( const QString& word )
{
    // hspell-0.9 cannot do this
    kdDebug() << "HSpellDict::addToSession: Sorry, cannot." << endl; 
    return false;
}
