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

#ifndef KEMOTICONS_PROVIDER_H
#define KEMOTICONS_PROVIDER_H

#include "kemoticons_export.h"
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QStringList>
#include <QtCore/QPair>

class QString;
class KEmoticonsProviderPrivate;

/**
 * This class contains the emoticons theme
 * this is also the parent class for the theme parser plugins
 */
class KEMOTICONS_EXPORT KEmoticonsProvider
{
public:

    /**
     * Default constructor, you should never use this, instead use KEmoticons::theme()
     */
    KEmoticonsProvider();

    /**
     * Destructor
     */
    virtual ~KEmoticonsProvider();

    /**
     * Load the theme inside the directory @p path
     * @param path path to the directory
     */
    virtual bool loadTheme(const QString &path);

    /**
     * Remove the emoticon @p emo, this will not delete the image file too
     * @code
     * KEmoticonsTheme *theme = KEmoticons().theme();
     * theme->removeEmoticon(":)");
     * @endcode
     * @param emo the emoticon text to remove
     * @return @c true if it can delete the emoticon
     */
    virtual bool removeEmoticon(const QString &emo);
    
    /**
     * Add the emoticon @p emo with text @p text
     * @code
     * KEmoticonsTheme *theme = KEmoticons().theme();
     * theme->addEmoticon("/path/to/smiley.png", ":) :-)");
     * @endcode
     * @param emo path to the emoticon image
     * @param text the text of the emoticon separated by space for multiple text
     * @param copy whether or not copy @p emo into the theme directory
     * @return @c true if it can add the emoticon
     */
    virtual bool addEmoticon(const QString &emo, const QString &text, bool copy=false);

    /**
     * Save the emoticon theme
     */
    virtual void save();

    /**
     * Returns the theme name
     */
    QString themeName();

    /**
     * Set the theme name
     * @param name name of the theme
     */
    void setThemeName(const QString &name);

    /**
     * Returns the theme path
     */
    QString themePath();

    /**
     * Returns the file name of the theme
     */
    QString fileName();

    /**
     * Returns a pointer to a QMap that contains the emoticons path as keys and the text as values
     */
    QMap<QString, QStringList> *emoticonsMap();

    /**
     * Create a new theme
     */
    virtual void createNew();

protected:
    /**
     * Private class
     */
    KEmoticonsProviderPrivate * const d;
};

#endif /* KEMOTICONS_PROVIDER_H */

// kate: space-indent on; indent-width 4; replace-tabs on;
