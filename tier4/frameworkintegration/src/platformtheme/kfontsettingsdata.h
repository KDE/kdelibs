/* This file is part of the KDE libraries
   Copyright (C) 2000, 2006 David Faure <faure@kde.org>
   Copyright 2008 Friedrich W. H. Kossebau <kossebau@kde.org>
   Copyright 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KGLOBALSETTINGSDATA_H
#define KGLOBALSETTINGSDATA_H

#include <QObject>
#include <QFont>

struct KFontData
{
    const char *ConfigGroupKey;
    const char *ConfigKey;
    const char *FontName;
    int Size;
    int Weight;
    QFont::StyleHint StyleHint;
};

class KFontSettingsData : public QObject
{
  Q_OBJECT
  public:
    // if adding a new type here also add an entry to DefaultFontData
    enum FontTypes
    {
        GeneralFont = 0,
        FixedFont,
        ToolbarFont,
        MenuFont,
        WindowTitleFont,
        TaskbarFont ,
        SmallestReadableFont,
        FontTypesCount
    };

  public:
    KFontSettingsData();
    ~KFontSettingsData();

  public Q_SLOTS:
    void dropFontSettingsCache();

  private Q_SLOTS:
    void delayedDBusConnects();

  public: // access, is not const due to caching
    QFont *font( FontTypes fontType );

  protected:
    QFont *mFonts[FontTypesCount];
};

#endif
