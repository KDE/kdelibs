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

#include "kemoticonsprovider.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtGui/QTextDocument>

#include <kemoticons.h>
#include <kio/netaccess.h>
#include <KStandardDirs>
#include <KDebug>

class KEmoticonsProviderPrivate
{
public:
    KEmoticonsProviderPrivate();
    QString m_themeName;
    QString m_fileName;
    QString m_themePath;
    QHash<QString, QStringList> m_emoticonsMap;
    QHash<QChar, QList<KEmoticonsProvider::Emoticon> > m_emoticonsIndex;
};

KEmoticonsProviderPrivate::KEmoticonsProviderPrivate()
{
}

KEmoticonsProvider::KEmoticonsProvider(QObject *parent)
        : QObject(parent), d(new KEmoticonsProviderPrivate)
{
}

KEmoticonsProvider::~KEmoticonsProvider()
{
    delete d;
}

bool KEmoticonsProvider::loadTheme(const QString &path)
{
    QFileInfo info(path);
    d->m_fileName = info.fileName();
    d->m_themeName = info.dir().dirName();
    d->m_themePath = info.absolutePath();
    return true;
}

bool KEmoticonsProvider::removeEmoticon(const QString &emo)
{
    Q_UNUSED(emo);
    return false;
}

bool KEmoticonsProvider::addEmoticon(const QString &emo, const QString &text, bool copy)
{
    if (copy) {
        KIO::NetAccess::dircopy(KUrl(emo), KUrl(d->m_themePath));
    }

    Q_UNUSED(text);
    return false;
}

void KEmoticonsProvider::save()
{
}

QString KEmoticonsProvider::themeName() const
{
    return d->m_themeName;
}

void KEmoticonsProvider::setThemeName(const QString &name)
{
    d->m_themeName = name;
}

QString KEmoticonsProvider::themePath() const
{
    return d->m_themePath;
}

QString KEmoticonsProvider::fileName() const
{
    return d->m_fileName;
}

QHash<QString, QStringList> *KEmoticonsProvider::emoticonsMap()
{
    return &(d->m_emoticonsMap);
}

QHash<QString, QStringList> KEmoticonsProvider::constEmoticonsMap() const
{
    return d->m_emoticonsMap;
}

void KEmoticonsProvider::createNew()
{
}

QHash<QChar, QList<KEmoticonsProvider::Emoticon> > *KEmoticonsProvider::emoticonsIndex()
{
    return &(d->m_emoticonsIndex);
}

QHash<QChar, QList<KEmoticonsProvider::Emoticon> > KEmoticonsProvider::constEmoticonsIndex() const
{
    return d->m_emoticonsIndex;
}

void KEmoticonsProvider::addEmoticonIndex(const QString &path, const QStringList &emoList)
{
    foreach(const QString &s, emoList) {
        KEmoticonsProvider::Emoticon e;
        QPixmap p;

        QString escaped = Qt::escape(s);
        e.picPath = path;
        p.load(path);


        e.picHTMLCode = QString("<img align=\"center\" title=\"%1\" alt=\"%1\" src=\"%2\" width=\"%3\" height=\"%4\" />").arg(escaped).arg(path).arg(p.width()).arg(p.height());

        e.matchTextEscaped = escaped;
        e.matchText = s;

        (*emoticonsIndex())[escaped[0]].append(e);
        (*emoticonsIndex())[s[0]].append(e);
    }
}

void KEmoticonsProvider::removeEmoticonIndex(const QString &path, const QStringList &emoList)
{
    foreach(const QString &s, emoList) {
        QString escaped = Qt::escape(s);

        QList<Emoticon> ls = emoticonsIndex()->value(escaped[0]);

        for (int i = 0; i < ls.size(); ++i) {
            if (ls.at(i).picPath == path) {
                ls.removeAt(i);
            }
        }

        ls = emoticonsIndex()->value(s[0]);

        for (int i = 0; i < ls.size(); ++i) {
            if (ls.at(i).picPath == path) {
                ls.removeAt(i);
            }
        }
    }
}


// kate: space-indent on; indent-width 4; replace-tabs on;
