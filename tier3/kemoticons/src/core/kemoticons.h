/**********************************************************************************
 *   Copyright (C) 2008 by Carlo Segato <brandon.ml@gmail.com>                    *
 *                                                                                *
 *   This library is free software; you can redistribute it and/or                *
 *   modify it under the terms of the GNU Lesser General Public                   *
 *   License as published by the Free Software Foundation; either                 *
 *   version 2.1 of the License, or (at your option) any later version.           *
 *                                                                                *
 *   This library is distributed in the hope that it will be useful,              *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of               *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU            *
 *   Lesser General Public License for more details.                              *
 *                                                                                *
 *   You should have received a copy of the GNU Lesser General Public             *
 *   License along with this library.  If not, see <http://www.gnu.org/licenses/>.*
 *                                                                                *
 **********************************************************************************/

#ifndef KEMOTICONS_H
#define KEMOTICONS_H

#include "kemoticons_export.h"
#include "kemoticonstheme.h"

#include <QtCore/QObject>

#include <kservicetypetrader.h>

class KEmoticonsPrivate;

/**
 * This class can be used to retrieve, install, create emoticons theme.
 * For example, if you want to get the current emoticon theme
 * @code
 * KEmoticons ke;
 * KEmoticonsTheme et = ke.theme();
 * //do whatever you want with the theme
 * @endcode
 * It can also be used to set the emoticon theme and the parse mode in the config file
 * @author Carlo Segato (brandon.ml@gmail.com)
 */

class KEMOTICONS_EXPORT KEmoticons : public QObject
{
    Q_OBJECT
public:

    /**
     * Default constructor
     */
    KEmoticons();

    /**
     * Destructor
     */
    ~KEmoticons();

    /**
     * Retrieves the current emoticon theme
     * @return the current KEmoticonsTheme
     */
    KEmoticonsTheme theme() const;

    /**
    * Retrieves the emoticon theme with name @p name
    * @param name name of the theme
    * @return the KEmoticonsTheme with name @p name
    */
    KEmoticonsTheme theme(const QString &name) const;

    /**
     * Retrieves the current emoticon theme name
     */
    static QString currentThemeName();

    /**
     * Returns a list of installed emoticon themes
     */
    static QStringList themeList();

    /**
     * Sets @p theme as the current emoticon theme
     * @param theme a reference to a KEmoticonsTheme object
     */
    static void setTheme(const KEmoticonsTheme &theme);

    /**
     * Sets @p theme as the current emoticon theme
     * @param theme the name of a theme
     */
    static void setTheme(const QString &theme);

    /**
     * Creates a new emoticon theme
     * @code
     * KEmoticonsTheme theme;
     * KService::List srv = KServiceTypeTrader::self()->query("KEmoticons");
     * for (int i = 0; i < srv.size(); ++i) {
     *     // we want to create a kde emoticons theme
     *     if (srv.at(i)->property("X-KDE-EmoticonsFileName").toString() == "emoticons.xml") {
     *         theme = KEmoticons().newTheme("test", srv.at(i));
     *     }
     * }
     * @endcode
     * @param name the name of the new emoticon theme
     * @param service the kind of emoticon theme to create
     */
    KEmoticonsTheme newTheme(const QString &name, const KService::Ptr &service);

    /**
     * Installs all emoticon themes inside the archive @p archiveName
     * @param archiveName path to the archive
     * @return a list of installed themes
     */
    QStringList installTheme(const QString &archiveName);

    /**
     * Sets the parse mode to @p mode
     */
    static void setParseMode(KEmoticonsTheme::ParseMode mode);

    /**
     * Returns the current parse mode
     */
    static KEmoticonsTheme::ParseMode parseMode();

private:
    /**
     * Private implementation class
     */
    const QScopedPointer<KEmoticonsPrivate> d;

    Q_PRIVATE_SLOT(d, void changeTheme(const QString &path))
};

#endif /* KEMOTICONS_H */

// kate: space-indent on; indent-width 4; replace-tabs on;
