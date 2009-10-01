// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/*
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
#ifndef SONNET_SETTINGS_P_H
#define SONNET_SETTINGS_P_H

#include <QtCore/QStringList>
#include <QtCore/QString>
#include <kdecore_export.h>

class KConfig;

namespace Sonnet
{
    class Loader;

    /**
     * Settings class
     */
    class KDECORE_EXPORT Settings
    {
    public:
        ~Settings();

        bool modified() const;
        void setModified(bool modified);

        void setDefaultLanguage(const QString &lang);
        QString defaultLanguage() const;

        void setDefaultClient(const QString &client);
        QString defaultClient() const;

        void setCheckUppercase(bool);
        bool checkUppercase() const;

        void setSkipRunTogether(bool);
        bool skipRunTogether() const;

        void setBackgroundCheckerEnabled(bool);
        bool backgroundCheckerEnabled() const;

        void setCheckerEnabledByDefault(bool);
        bool checkerEnabledByDefault() const;

        void setCurrentIgnoreList(const QStringList &ignores);
        void addWordToIgnore(const QString &word);
        QStringList currentIgnoreList() const;
        bool ignore(const QString &word);

        void save(KConfig *config);
        void restore(KConfig *config);

        int disablePercentageWordError() const;
        int disableWordErrorCount() const;

    private:
        void readIgnoreList(KConfig *config);
        void setQuietIgnoreList(const QStringList &ignores);

    private:
        friend class Loader;
        Settings(Loader *loader);
    private:
        class Private;
        Private *const d;
    };
}

#endif // SONNET_SETTINGS_P_H
