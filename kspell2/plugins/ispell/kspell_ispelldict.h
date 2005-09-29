/**
 * kspell_ispelldict.h
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
#ifndef KSPELL_ASPELLDICT_H
#define KSPELL_ASPELLDICT_H

#include "dictionary.h"

class ISpellChecker;

class ISpellDict : public KSpell2::Dictionary
{
public:
    ISpellDict( const QString& lang );
    ~ISpellDict();
    virtual bool check( const QString& word );

    virtual QStringList suggest( const QString& word );

    virtual bool checkAndSuggest( const QString& word,
                                  QStringList& suggestions ) ;

    virtual bool storeReplacement( const QString& bad,
                                   const QString& good );

    virtual bool addToPersonal( const QString& word );
    virtual bool addToSession( const QString& word );
private:
    ISpellChecker *m_checker;
};

#endif
