/**
 * kspell_aspelldict.cpp
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 */
#include "kspell_aspelldict.h"

#include <kdebug.h>

using namespace KSpell2;

ASpellDict::ASpellDict( const QString& lang )
    : Dictionary( lang )
{
    m_config = new_aspell_config();
    aspell_config_replace( m_config, "lang", lang.latin1() );

    AspellCanHaveError * possible_err = new_aspell_speller( m_config );

    if ( aspell_error_number( possible_err ) != 0 )
        kdDebug()<< "Error : "<< aspell_error_message( possible_err ) <<endl;
    else
        m_speller = to_aspell_speller( possible_err );

}

ASpellDict::~ASpellDict()
{
    delete_aspell_speller( m_speller );
    delete_aspell_config( m_config );
}

bool ASpellDict::check( const QString& word )
{
    int correct = aspell_speller_check( m_speller, word.utf8(), word.length() );
    return correct;
}

QStringList ASpellDict::suggest( const QString& word )
{
    const AspellWordList * suggestions = aspell_speller_suggest( m_speller,
                                                                 word.utf8(),
                                                                 word.length() );

    AspellStringEnumeration * elements = aspell_word_list_elements( suggestions );

    QStringList qsug;
    const char * cword;

    while ( (cword = aspell_string_enumeration_next( elements )) ) {
        qsug.append( cword );
    }

    delete_aspell_string_enumeration( elements );
    return qsug;
}

bool ASpellDict::checkAndSuggest( const QString& word,
                                  QStringList& suggestions )
{
    bool c = check( word );
    if ( c )
        suggestions = suggest( word );
    return c;
}

bool ASpellDict::storeReplacement( const QString& bad,
                                   const QString& good )
{
    return aspell_speller_store_replacement( m_speller,
                                             bad.utf8(), bad.length(),
                                             good.utf8(), good.length() );
}

bool ASpellDict::addToPersonal( const QString& word )
{
    return aspell_speller_add_to_personal( m_speller, word.utf8(),
                                           word.length() );
}

bool ASpellDict::addToSession( const QString& word )
{
    return aspell_speller_add_to_session( m_speller, word.utf8(),
                                          word.length() );
}
