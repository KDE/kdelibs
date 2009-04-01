/**********************************************************************************
 *   Copyright (C) 2008 by Carlo Segato <brandon.ml@gmail.com>                    *
 *   Copyright (c) 2002-2003 by Stefan Gehn            <metz@gehn.net>            *
 *   Kopete    (c) 2002-2008 by the Kopete developers  <kopete-devel@kde.org>     *
 *   Copyright (c) 2005      by Engin AYDOGAN          <engin@bzzzt.biz>          *
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

#include "kemoticonstheme.h"
#include "kemoticons.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtGui/QTextDocument>
#include <QtCore/QtAlgorithms>

#include <kio/netaccess.h>
#include <kstandarddirs.h>
#include <kdebug.h>

class KEmoticonsTheme::KEmoticonsThemeData : public QSharedData
{
public:
    KEmoticonsThemeData();
    ~KEmoticonsThemeData();
    KEmoticonsProvider *provider;
};


KEmoticonsTheme::KEmoticonsThemeData::KEmoticonsThemeData()
{
    provider = 0;
}

KEmoticonsTheme::KEmoticonsThemeData::~KEmoticonsThemeData()
{
//     delete provider;
}

KEmoticonsTheme::KEmoticonsTheme()
{
    d = new KEmoticonsThemeData;
}

KEmoticonsTheme::KEmoticonsTheme(const KEmoticonsTheme &ket)
{
    d = ket.d;
}

KEmoticonsTheme::KEmoticonsTheme(KEmoticonsProvider *p)
{
    d = new KEmoticonsThemeData;
    d->provider = p;
}

KEmoticonsTheme::~KEmoticonsTheme()
{
}

bool KEmoticonsTheme::loadTheme(const QString &path)
{
    if (!d->provider) {
        return false;
    }

    return d->provider->loadTheme(path);
}

bool KEmoticonsTheme::removeEmoticon(const QString &emo)
{
    if (!d->provider) {
        return false;
    }

    return d->provider->removeEmoticon(emo);
}

bool KEmoticonsTheme::addEmoticon(const QString &emo, const QString &text, KEmoticonsProvider::AddEmoticonOption option)
{
    if (!d->provider) {
        return false;
    }

    return d->provider->addEmoticon(emo, text, option);
}

void KEmoticonsTheme::save()
{
    if (!d->provider) {
        return;
    }

    d->provider->save();
}

QString KEmoticonsTheme::themeName() const
{
    if (!d->provider) {
        return QString();
    }

    return d->provider->themeName();
}

void KEmoticonsTheme::setThemeName(const QString &name)
{
    if (!d->provider) {
        return;
    }

    d->provider->setThemeName(name);
}

QString KEmoticonsTheme::themePath() const
{
    if (!d->provider) {
        return QString();
    }

    return d->provider->themePath();
}

QString KEmoticonsTheme::fileName() const
{
    if (!d->provider) {
        return QString();
    }

    return d->provider->fileName();
}

QHash<QString, QStringList> KEmoticonsTheme::emoticonsMap() const
{
    if (!d->provider) {
        return QHash<QString, QStringList>();
    }

    return d->provider->emoticonsMap();
}

void KEmoticonsTheme::createNew()
{
    if (!d->provider) {
        return;
    }

    d->provider->createNew();
}

QString KEmoticonsTheme::parseEmoticons(const QString &text, ParseMode mode, const QStringList &exclude) const
{
    QList<Token> tokens = tokenize(text, mode | SkipHTML);
    if (tokens.isEmpty() && !text.isEmpty())
        return text;

    QString result;

    foreach(const Token &token , tokens) {
        switch (token.type) {
        case Text:
            result += token.text;
            break;
        case Image:
            if (!exclude.contains(token.text)) {
                result += token.picHTMLCode;
            } else {
                result += token.text;
            }
            break;
        default:
            kWarning() << "Unknown token type. Something's broken.";
            break;
        }
    }
    return result;
}

bool EmoticonCompareEscaped( const KEmoticonsProvider::Emoticon &s1, const KEmoticonsProvider::Emoticon &s2)
{
	return s1.matchTextEscaped.length()>s2.matchTextEscaped.length();
}
bool EmoticonCompare( const KEmoticonsProvider::Emoticon &s1, const KEmoticonsProvider::Emoticon &s2)
{
	return s1.matchText.length()>s2.matchText.length();
}


QList<KEmoticonsTheme::Token> KEmoticonsTheme::tokenize(const QString &message, ParseMode mode) const
{
    if (!d->provider) {
        return QList<KEmoticonsTheme::Token>();
    }

    if (!(mode & (StrictParse | RelaxedParse))) {
        //if none of theses two mode are selected, use the mode from the config
        mode |=  KEmoticons::parseMode();
    }

    QList<Token> result;

    /* previous char, in the firs iteration assume that it is space since we want
     * to let emoticons at the beginning, the very first previous QChar must be a space. */
    QChar p = ' ';
    QChar c; /* current char */
    QChar n;

    /* This is the EmoticonNode container, it will represent each matched emoticon */
    typedef QPair<KEmoticonsProvider::Emoticon, int> EmoticonNode;
    QList<EmoticonNode> foundEmoticons;
    /* First-pass, store the matched emoticon locations in foundEmoticons */
    QList<KEmoticonsProvider::Emoticon> emoticonList;
    QList<KEmoticonsProvider::Emoticon>::const_iterator it;
    int pos;

    bool inHTMLTag = false;
    bool inHTMLLink = false;
    bool inHTMLEntity = false;
    QString needle; // search for this

    for (pos = 0; pos < message.length(); ++pos) {
        c = message[pos];

        if (mode & SkipHTML) { // Shall we skip HTML ?
            if (!inHTMLTag) { // Are we already in an HTML tag ?
                if (c == '<') { // If not check if are going into one
                    inHTMLTag = true; // If we are, change the state to inHTML
                    p = c;
                    continue;
                }
            } else { // We are already in a HTML tag
                if (c == '>') { // Check if it ends
                    inHTMLTag = false;   // If so, change the state

                    if (p == 'a') {
                        inHTMLLink = false;
                    }
                } else if (c == 'a' && p == '<') { // check if we just entered an achor tag
                    inHTMLLink = true; // don't put smileys in urls
                }
                p = c;
                continue;
            }

            if (!inHTMLEntity) { // are we
                if (c == '&') {
                    inHTMLEntity = true;
                }
            }
        }

        if (inHTMLLink) { // i can't think of any situation where a link address might need emoticons
            p = c;
            continue;
        }

        if ((mode & StrictParse)  &&  !p.isSpace() && p != '>') {  // '>' may mark the end of an html tag
            p = c;
            continue;
        } /* strict requires space before the emoticon */

        if (d->provider->emoticonsIndex().contains(c)) {
            emoticonList = d->provider->emoticonsIndex().value(c);
	    if (mode & SkipHTML)
		qSort(emoticonList.begin(),emoticonList.end(),EmoticonCompareEscaped);
	    else
		qSort(emoticonList.begin(),emoticonList.end(),EmoticonCompare);
            bool found = false;
            for (it = emoticonList.constBegin(); it != emoticonList.constEnd(); ++it) {
                // If this is an HTML, then search for the HTML form of the emoticon.
                // For instance <o) => &gt;o)
                needle = (mode & SkipHTML) ? (*it).matchTextEscaped : (*it).matchText;
                if ((pos == message.indexOf(needle, pos))) {
                    if (mode & StrictParse) {
                        /* check if the character after this match is space or end of string*/
                        if (message.length() > pos + needle.length()) {
                            n = message[pos + needle.length()];
                            //<br/> marks the end of a line
                            if (n != '<' && !n.isSpace() &&  !n.isNull() && n != '&') {
                                break;
                            }
                        }
                    }
                    /* Perfect match */
                    foundEmoticons.append(EmoticonNode((*it), pos));
                    found = true;
                    /* Skip the matched emoticon's matchText */
                    pos += needle.length() - 1;
                    break;
                }

                if (found) {
                    break;
                }
            }

            if (!found) {
                if (inHTMLEntity) {
                    // If we are in an HTML entitiy such as &gt;
                    int htmlEnd = message.indexOf(';', pos);
                    // Search for where it ends
                    if (htmlEnd == -1) {
                        // Apparently this HTML entity isn't ended, something is wrong, try skip the '&'
                        // and continue
                        kDebug() << "Broken HTML entity, trying to recover.";
                        inHTMLEntity = false;
                        pos++;
                    } else {
                        pos = htmlEnd;
                        inHTMLEntity = false;
                    }
                }
            }
        } /* else no emoticons begin with this character, so don't do anything */
        p = c;
    }

    /* if no emoticons found just return the text */
    if (foundEmoticons.isEmpty()) {
        result.append(Token(Text, message));
        return result;
    }

    /* Second-pass, generate tokens based on the matches */

    pos = 0;
    int length;

    for (int i = 0; i < foundEmoticons.size(); ++i) {
        EmoticonNode itFound = foundEmoticons.at(i);
        needle = (mode & SkipHTML) ? itFound.first.matchTextEscaped : itFound.first.matchText;

        if ((length = (itFound.second - pos))) {
            result.append(Token(Text, message.mid(pos, length)));
            result.append(Token(Image, itFound.first.matchTextEscaped, itFound.first.picPath, itFound.first.picHTMLCode));
            pos += length + needle.length();
        } else {
            result.append(Token(Image, itFound.first.matchTextEscaped, itFound.first.picPath, itFound.first.picHTMLCode));
            pos += needle.length();
        }
    }

    if (message.length() - pos) { // if there is remaining regular text
        result.append(Token(Text, message.mid(pos)));
    }

    return result;
}

bool KEmoticonsTheme::isNull() const
{
    return d->provider ? false : true;
}

KEmoticonsTheme& KEmoticonsTheme::operator=(const KEmoticonsTheme &ket)
{
    if (d == ket.d) {
        return *this;
    }

    d = ket.d;
    return *this;
}

// kate: space-indent on; indent-width 4; replace-tabs on;
