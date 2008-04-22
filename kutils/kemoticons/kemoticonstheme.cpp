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

#include "kemoticonstheme.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtGui/QTextDocument>

#include <kemoticons.h>
#include <kio/netaccess.h>
#include <KStandardDirs>
#include <KDebug>

KEmoticonsThemePrivate::KEmoticonsThemePrivate()
{
}

KEmoticonsTheme::KEmoticonsTheme(QObject *parent, const QVariantList &args)
    : QObject(parent), d(new KEmoticonsThemePrivate)
{
    Q_UNUSED(args);
}

KEmoticonsTheme::~KEmoticonsTheme()
{
    delete d;
}

bool KEmoticonsTheme::loadTheme(const QString &path)
{
    QFileInfo info(path);
    d->m_fileName = info.fileName();
    d->m_themeName = info.dir().dirName();
    d->m_themePath = info.absolutePath();
    return true;
}

bool KEmoticonsTheme::removeEmoticon(const QString &emo)
{
    Q_UNUSED(emo);
    return false;
}

bool KEmoticonsTheme::addEmoticon(const QString &emo, const QString &text, bool copy)
{
    if(copy) {
        KIO::NetAccess::dircopy(KUrl(emo), KUrl(d->m_themePath));
    }

    Q_UNUSED(text);
    return false;
}

void KEmoticonsTheme::save()
{
}

QString KEmoticonsTheme::themeName()
{
    return d->m_themeName;
}

void KEmoticonsTheme::setThemeName(const QString &name)
{
    d->m_themeName = name;
}

QMap<QString, QStringList> KEmoticonsTheme::emoticonsMap()
{
    return d->m_emoticonsMap;
}

void KEmoticonsTheme::createNew()
{
}

QString KEmoticonsTheme::parseEmoticons(const QString &text, ParseMode mode, const QStringList &exclude)
{
    QList<Token> tokens = tokenize(text, mode);
    QString result;
    
    foreach (Token token , tokens ) {
        switch (token.type) {
            case Text:
                result += token.text;
                kDebug()<<"TEXT:"<<token.text;
                break;
            case Image:
                if (!exclude.contains(token.text)) {
                    result += token.picHTMLCode;
                    kDebug()<<"IMG:"<<token.picHTMLCode;
                } else {
                    result += token.text;
                    kDebug()<<"TEXT:"<<token.text;
                }
                break;
            default:
                kWarning() << "Unknown token type. Something's broken.";
                break;
        }
    }
    return result;
}

QList<KEmoticonsTheme::Token> KEmoticonsTheme::tokenize(const QString &message, ParseMode mode)
{
    if (!(mode & (StrictParse|RelaxedParse))) {
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
    QList<QPair<QString, EmoticonNode> > foundEmoticons;
    /* First-pass, store the matched emoticon locations in foundEmoticons */
    QMap<QString, QStringList> emoticonList;
    QMap<QString, QStringList>::const_iterator it;
    int pos;

    bool inHTMLTag = false;
    bool inHTMLLink = false;
    bool inHTMLEntity = false;
    QString needle; // search for this
    
    for (pos = 0; pos < message.length(); pos++) {
        c = message[pos];

        if (mode & SkipHTML) { // Shall we skip HTML ?
            if (!inHTMLTag) { // Are we already in an HTML tag ?
                if (c == '<') { // If not check if are going into one
                    inHTMLTag = true; // If we are, change the state to inHTML
                    p = c;
                    continue;
                }
            }
            else { // We are already in a HTML tag
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

            if(!inHTMLEntity) { // are we
                if(c == '&') {
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
        

        bool found = false;
        for (it = d->m_emoticonsMap.constBegin(); it != d->m_emoticonsMap.constEnd(); ++it) {
            // If this is an HTML, then search for the HTML form of the emoticon.
            // For instance <o) => &gt;o)
            QStringList needles = it.value();
            for (int k = 0; k < needles.size(); ++k) {
                needle = (mode & SkipHTML) ? Qt::escape(needles.at(k)) : needles.at(k);
                if ((pos == message.indexOf(needle, pos))) {
                    if (mode & StrictParse) {
                        /* check if the character after this match is space or end of string*/
                        if (message.length() > pos + needle.length()) {
                            n = message[pos + needle.length()];
                            //<br/> marks the end of a line
                            if(n != '<' && !n.isSpace() &&  !n.isNull() && n!= '&') {
                                break;
                            }
                        }
                    }
                    /* Perfect match */
                    foundEmoticons.append(QPair<QString, EmoticonNode>(it.key(), EmoticonNode(needle, pos)));
                    found = true;
                    /* Skip the matched emoticon's matchText */
                    pos += needle.length() - 1;
                    break;
                }
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
        QPair<QString, EmoticonNode> itFound = foundEmoticons.at(i);
        needle = itFound.second.first;
        
        QPixmap p(itFound.first);
        QString htmlCode = QString("<img align=\"center\" src=\"%1\" width=\"%2\" height=\"%3\" />").arg(itFound.first).arg(p.width()).arg(p.height());
 
        if ((length = (itFound.second.second - pos))) {
            result.append(Token(Text, message.mid(pos, length)));
            result.append(Token(Image, needle, itFound.first, htmlCode));
            pos += length + needle.length();
        } else {
            result.append(Token(Image, needle, itFound.first, htmlCode));
            pos += needle.length();
        }
    }

    if (message.length() - pos) { // if there is remaining regular text
        result.append(Token(Text, message.mid(pos)));
    }

    return result;
}

// kate: space-indent on; indent-width 4; replace-tabs on;
