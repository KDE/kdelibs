// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * settings.h
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
#ifndef KSPELL_SETTINGS_H
#define KSPELL_SETTINGS_H

#include <qstringlist.h>
#include <qstring.h>

class KSharedConfig;

namespace KSpell2
{
    class Broker;

    class Settings
    {
    public:
        ~Settings();

        void setDefaultLanguage( const QString& lang );
        QString defaultLanguage() const;

        void setDefaultClient( const QString& client );
        QString defaultClient() const;

        void setCheckUppercase( bool );
        bool checkUppercase() const;

        void setSkipRunTogether( bool );
        bool skipRunTogether() const;

        void setCurrentIgnoreList( const QStringList& ignores );
        void addWordToIgnore( const QString& word );
        QStringList currentIgnoreList() const;
        bool ignore( const QString& word );

        void save();

        KSharedConfig *sharedConfig() const;

    private:
        void loadConfig();
        void readIgnoreList();
        void setQuietIgnoreList( const QStringList& ignores );

    private:
        friend class Broker;
        Settings( Broker *broker, KSharedConfig *config );
    private:
        class Private;
        Private *d;
    };
}

#endif
