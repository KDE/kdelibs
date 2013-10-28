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

#include <kemoticons_export.h>

#include <QtCore/QObject>
#include <QtCore/QStringList>

class KEmoticonsProviderPrivate;
struct Emoticon;

/**
 * This is the base abstract class for the emoticon provider plugins.
 */
class KEMOTICONS_EXPORT KEmoticonsProvider : public QObject
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
     * Loads the emoticon theme inside the directory @p path
     * @param path path to the directory
     * @return @c true if the emoticon theme is successfully loaded
     */
    virtual bool loadTheme(const QString &path) = 0;

    /**
     * Removes the emoticon @p emo. This doesn't delete the image file.
     * @param emo the emoticon text to remove
     * @return @c true if the emoticon theme is successfully removed
     */
    virtual bool removeEmoticon(const QString &emo) = 0;

    /**
     * Adds the emoticon @p emo with text @p text
     * @param emo path to the emoticon image
     * @param text the emoticon text. If alternative texts are to be added,
     * use spaces to separate them.
     * @param copy whether or not to copy @p emo into the theme directory
     * @return @c true if the emoticon is successfully added
     */
    virtual bool addEmoticon(const QString &emo, const QString &text, AddEmoticonOption option = DoNotCopy) = 0;

    /**
     * Saves the emoticon theme
     *
     * @deprecated since 5.0, use saveTheme() instead
     */
#ifndef KDE_NO_DEPRECATED
    KEMOTICONS_DEPRECATED void save();
#endif

    /**
     * Saves the emoticon theme
     * @since 5.0
     */
    virtual void saveTheme() = 0;

    /**
     * Returns the theme name
     */
    QString themeName() const;

    /**
     * Sets the emoticon theme name
     * @param name name of the theme
     */
    void setThemeName(const QString &name);

    /**
     * Returns the emoticon theme path
     */
    QString themePath() const;

    /**
     * Returns the file name of the emoticon theme
     */
    QString fileName() const;

    /**
    * Returns a QHash that contains the emoticon path as keys and the text as values
    */
    QHash<QString, QStringList> emoticonsMap() const;

    /**
     * Returns a QHash that contains emoticons indexed by the first char
     */
    QHash<QChar, QList<Emoticon> > emoticonsIndex() const;

    /**
     * Creates a new theme
     */
    virtual void createNew() = 0;

protected:
    /**
     * Sets the theme inside the directory @p path
     * @param path path to the directory
     * @since 5.0
     */
    void setThemePath(const QString &path);

    /**
     * Copies the emoticon image to the theme directory
     * @param emo path to the emoticon image
     * @return true if the emoticon is successfully copied
     * @since 5.0
     */
    bool copyEmoticon(const QString &emo);

    /**
     * Clears the emoticons map
     */
    void clearEmoticonsMap();

    /**
     * Inserts a new item in the emoticon map
     *
     * @deprecated since 5.0, use addMapItem() instead
     */
#ifndef KDE_NO_DEPRECATED
    KEMOTICONS_DEPRECATED void addEmoticonsMap(QString key, QStringList value);
#endif

    /**
     * Inserts a new item in the emoticon map
     * @since 5.0
     * @see emoticonsMap()
     */
    void addMapItem(QString key, QStringList value);

    /**
     * Removes an item from the emoticon map
     *
     * @deprecated since 5.0, use removeMapItem() instead
     */
#ifndef KDE_NO_DEPRECATED
    KEMOTICONS_DEPRECATED void removeEmoticonsMap(QString key);
#endif
    /**
     * Removes an item from the emoticon map
     * @since 5.0
     * @see emoticonsMap()
     */
    void removeMapItem(QString key);

    /**
     * Adds an emoticon to the index
     * @param path path to the emoticon
     * @param emoList list of text associated with this emoticon
     *
     * @deprecated since 5.0, use addIndexItem() instead
     */
#ifndef KDE_NO_DEPRECATED
    KEMOTICONS_DEPRECATED void addEmoticonIndex(const QString &path, const QStringList &emoList);
#endif
    /**
     * Adds an emoticon to the index
     * @param path path to the emoticon
     * @param emoList list of text associated with this emoticon
     * @since 5.0
     * @see emoticonsIndex()
     */
    void addIndexItem(const QString &path, const QStringList &emoList);

    /**
     * Removes an emoticon from the index
     * @param path path to the emoticon
     * @param emoList list of text associated with this emoticon
     *
     * @deprecated since 5.0, use removeIndexItem() instead
     */
#ifndef KDE_NO_DEPRECATED
    KEMOTICONS_DEPRECATED void removeEmoticonIndex(const QString &path, const QStringList &emoList);
#endif
    /**
     * Removes an emoticon from the index
     * @param path path to the emoticon
     * @param emoList list of text associated with this emoticon
     * @since 5.0
     * @see emoticonsIndex()
     */
    void removeIndexItem(const QString &path, const QStringList &emoList);

private:
    /**
     * Private implementation class
     */
    const QScopedPointer<KEmoticonsProviderPrivate> d;
};

#endif /* KEMOTICONS_PROVIDER_H */

// kate: space-indent on; indent-width 4; replace-tabs on;
