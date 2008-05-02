/***************************************************************************
 *   Copyright (C) 2008 by Carlo Segato <brandon.ml@gmail.com>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef KEMOTICONS_H
#define KEMOTICONS_H

#include "kemoticons_export.h"
#include "kemoticonstheme.h"

#include <QtCore/QObject>
#include <QtCore/QHash>

#include <KDE/KServiceTypeTrader>

class KEmoticonsPrivate;

/**
 * This class can be used to retrieve, install, create emoticons theme.
 * For example if you want to get the current emoticon theme
 * @code
 * KEmoticons ke;
 * KEMoticonsTheme *et = ke.theme();
 * //do whatever you want with the theme
 * delete et;
 * @endcode
 * it can also be used to set the emoticon theme and the parse mode in the config file
 * @note Remember to the delete the KEmoticonsTheme objects that you get with theme and newTheme
 * @author Carlo Segato (brandon.ml@gmail.com)
 */

class KEMOTICONS_EXPORT KEmoticons : public QObject
{
    Q_OBJECT
public:
    KEmoticons();
    ~KEmoticons();

    KEmoticonsTheme *theme();
    KEmoticonsTheme *theme(const QString &name);
    static QString currentThemeName();

    static QStringList themeList();

    static void setTheme(KEmoticonsTheme *theme);
    static void setTheme(const QString &theme);

    KEmoticonsTheme *newTheme(const QString &name, const KService::Ptr &service);

    QStringList installTheme(const QString &archiveName);

    static void setParseMode(KEmoticonsTheme::ParseMode);
    static KEmoticonsTheme::ParseMode parseMode();

private:
    KEmoticonsPrivate * const d;

};

#endif /* KEMOTICONS_H */

// kate: space-indent on; indent-width 4; replace-tabs on;
