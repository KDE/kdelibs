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

#ifndef KEMOTICONS_PROVIDER_H
#define KEMOTICONS_PROVIDER_H

#include "kutils_export.h"
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QStringList>
#include <QtCore/QPair>

class QString;
class KEmoticonsProviderPrivate;
struct Emoticon;

/**
 * This is the base class for the emoticons provider plugins
 */
class KUTILS_EXPORT KEmoticonsProvider : public QObject
{
    Q_OBJECT
public:
    struct Emoticon
    {
        Emoticon(){}
        /* sort by longest to shortest matchText */
        bool operator < (const Emoticon &e) const { return matchText.length() > e.matchText.length(); }
        QString matchText;
        QString matchTextEscaped;
        QString picPath;
        QString picHTMLCode;
    };

    /**
     * Options to pass to addEmoticon
     */
    enum AddEmoticonOption {
        DoNotCopy, /**<< Don't copy the emoticon file into the theme directory */
        Copy /**<< Copy the emoticon file into the theme directory */
    };

    /**
     * Default constructor
     */
    explicit KEmoticonsProvider(QObject *parent = 0);

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
     * @param emo the emoticon text to remove
     * @return @c true if it can delete the emoticon
     */
    virtual bool removeEmoticon(const QString &emo);

    /**
     * Add the emoticon @p emo with text @p text
     * @param emo path to the emoticon image
     * @param text the text of the emoticon separated by space for multiple text
     * @param copy whether or not copy @p emo into the theme directory
     * @return @c true if it can add the emoticon
     */
    virtual bool addEmoticon(const QString &emo, const QString &text, AddEmoticonOption option = DoNotCopy);

    /**
     * Save the emoticon theme
     */
    virtual void save();

    /**
     * Returns the theme name
     */
    QString themeName() const;

    /**
     * Set the theme name
     * @param name name of the theme
     */
    void setThemeName(const QString &name);

    /**
     * Returns the theme path
     */
    QString themePath() const;

    /**
     * Returns the file name of the theme
     */
    QString fileName() const;

    /**
    * Returns a QHash that contains the emoticons path as keys and the text as values
    */
    QHash<QString, QStringList> emoticonsMap() const;

    /**
     * Returns a QHash that contains emoticons indexed by the first char
     */
    QHash<QChar, QList<Emoticon> > emoticonsIndex() const;

    /**
     * Create a new theme
     */
    virtual void createNew();

protected:
    /**
     * Clears the emoticons map
     */
    void clearEmoticonsMap();

    /**
     * Insert a new item in the emoticons map
     */
    void addEmoticonsMap(QString key, QStringList value);

    /**
     * Remove an item from the emoticons map
     */
    void removeEmoticonsMap(QString key);

    /**
     * Add an emoticon to the index
     * @param path path to the emoticon
     * @param emoList list of text associated with this emoticon
     */
    void addEmoticonIndex(const QString &path, const QStringList &emoList);

    /**
     * Remove an emoticon from the index
     * @param path path to the emoticon
     * @param emoList list of text associated with this emoticon
     */
    void removeEmoticonIndex(const QString &path, const QStringList &emoList);

    /**
     * Private class
     */
    KEmoticonsProviderPrivate * const d;
};

#endif /* KEMOTICONS_PROVIDER_H */

// kate: space-indent on; indent-width 4; replace-tabs on;
