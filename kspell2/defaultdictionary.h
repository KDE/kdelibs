/**
 * defaultdictionary.h
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
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
#ifndef KSPELL_DEFAULTDICTIONARY_H
#define KSPELL_DEFAULTDICTIONARY_H

#include "dictionary.h"

#include <qobject.h>
#include <qstringlist.h>

namespace KSpell2
{
    class Broker;
    class DefaultDictionary : public QObject,
                              public Dictionary
    {
        Q_OBJECT
    public:
        DefaultDictionary( const QString& lang, Broker *broker );
        ~DefaultDictionary();
    public:
        //Dictionary interface
        virtual bool check( const QString& word );
        virtual QStringList suggest( const QString& word );
        virtual bool checkAndSuggest( const QString& word,
                                      QStringList& suggestions );
        virtual bool storeReplacement( const QString& bad,
                                       const QString& good );
        virtual bool addToPersonal( const QString& word );
        virtual bool addToSession( const QString& word );
        //end of Dictionary interfaces

    signals:
        void changed();

    protected slots:
        void defaultSettingsurationChanged();

    private:
        class Private;
        Private *d;
    };
}

#endif
