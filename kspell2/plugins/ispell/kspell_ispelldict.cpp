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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "kspell_ispelldict.h"

#include <kdebug.h>

#include "ispell_checker.h"

using namespace KSpell2;

ISpellDict::ISpellDict( const QString& lang )
    : Dictionary( lang )
{
    m_checker = new ISpellChecker();

    if ( !m_checker->requestDictionary( lang.latin1() ) ) {
        kError()<<"Language \""<< lang << "\" doesn't exist for Ispell"<<endl;
    }
}

ISpellDict::~ISpellDict()
{
}

bool ISpellDict::check( const QString& word )
{
    return m_checker->checkWord( word );
}

QStringList ISpellDict::suggest( const QString& word )
{
    return m_checker->suggestWord( word );
}

bool ISpellDict::checkAndSuggest( const QString& word,
                                  QStringList& suggestions )
{
    bool c = check( word );
    if ( c )
        suggestions = suggest( word );
    return c;
}

bool ISpellDict::storeReplacement( const QString& ,
                                   const QString& )
{
    return false;
}

bool ISpellDict::addToPersonal( const QString& )
{
    return false;
}

bool ISpellDict::addToSession( const QString& )
{
    return false;
}
