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

#include "kfontsettingsdata.h"
#include <QCoreApplication>
#include <QString>
#include <QVariant>
#include <QApplication>
#include <QDBusMessage>
#include <QDBusConnection>
#include <qpa/qwindowsysteminterface.h>

#include <ksharedconfig.h>
#include <kconfiggroup.h>

KFontSettingsData::KFontSettingsData()
    : QObject(0)
{
    QMetaObject::invokeMethod(this, "delayedDBusConnects", Qt::QueuedConnection);

    for( int i=0; i<FontTypesCount; ++i )
        mFonts[i] = 0;
}

KFontSettingsData::~KFontSettingsData()
{
    for( int i=0; i<FontTypesCount; ++i )
        delete mFonts[i];
}

// NOTE: keep in sync with kdebase/workspace/kcontrol/fonts/fonts.cpp
static const char GeneralId[] =      "General";
static const char DefaultFont[] =    "Sans Serif";

static const KFontData DefaultFontData[KFontSettingsData::FontTypesCount] =
{
    { GeneralId, "font",                 DefaultFont,  9, -1, QFont::SansSerif },
    { GeneralId, "fixed",                "Monospace",  9, -1, QFont::TypeWriter },
    { GeneralId, "toolBarFont",          DefaultFont,  8, -1, QFont::SansSerif },
    { GeneralId, "menuFont",             DefaultFont,  9, -1, QFont::SansSerif },
    { "WM",      "activeFont",           DefaultFont,  8, -1, QFont::SansSerif },
    { GeneralId, "taskbarFont",          DefaultFont,  9, -1, QFont::SansSerif },
    { GeneralId, "smallestReadableFont", DefaultFont,  8, -1, QFont::SansSerif }
};

QFont *KFontSettingsData::font( FontTypes fontType )
{
    QFont *cachedFont = mFonts[fontType];

    if (!cachedFont) {
        const KFontData &fontData = DefaultFontData[fontType];
        cachedFont = new QFont( fontData.FontName, fontData.Size, fontData.Weight );
        cachedFont->setStyleHint( fontData.StyleHint );

        const KConfigGroup configGroup(KSharedConfig::openConfig("kdeglobals"), fontData.ConfigGroupKey);
        QString fontInfo = configGroup.readEntry( fontData.ConfigKey, QString() );

        //If we have serialized information for this font, restore it
        //NOTE: We are not using KConfig directly because we can't call QFont::QFont from here
        if (!fontInfo.isEmpty()) {
            cachedFont->fromString(fontInfo);
        }

        mFonts[fontType] = cachedFont;
    }

    return cachedFont;
}

void KFontSettingsData::dropFontSettingsCache()
{
    for( int i=0; i<FontTypesCount; ++i )
    {
        delete mFonts[i];
        mFonts[i] = 0;
    }

    QWindowSystemInterface::handleThemeChange(0);

    if (qobject_cast<QApplication*>(QCoreApplication::instance())) {
        QApplication::setFont(*font(KFontSettingsData::GeneralFont));
    } else {
        QGuiApplication::setFont(*font(KFontSettingsData::GeneralFont));
    }
}

void KFontSettingsData::delayedDBusConnects()
{
    QDBusConnection::sessionBus().connect( QString(), "/KDEPlatformTheme", "org.kde.KDEPlatformTheme",
                                                      "refreshFonts", this, SLOT(dropFontSettingsCache()) );
}
