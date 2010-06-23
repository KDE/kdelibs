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

#include "kemoticonsprovider.h"
#include "kemoticons.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtGui/QTextDocument>

#include <kio/netaccess.h>
#include "kstandarddirs.h"
#include "kdebug.h"

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

bool KEmoticonsProvider::addEmoticon(const QString &emo, const QString &text, AddEmoticonOption option)
{
    if (option == Copy) {
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

void KEmoticonsProvider::clearEmoticonsMap()
{
    d->m_emoticonsMap.clear();
}

void KEmoticonsProvider::addEmoticonsMap(QString key, QStringList value)
{
    if (!value.isEmpty()) {
        d->m_emoticonsMap.insert(key, value);
    }
}

void KEmoticonsProvider::removeEmoticonsMap(QString key)
{
    d->m_emoticonsMap.remove(key);
}

QHash<QString, QStringList> KEmoticonsProvider::emoticonsMap() const
{
    return d->m_emoticonsMap;
}

void KEmoticonsProvider::createNew()
{
}

QHash<QChar, QList<KEmoticonsProvider::Emoticon> > KEmoticonsProvider::emoticonsIndex() const
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

        e.picHTMLCode = QString("<img align=\"center\" title=\"%1\" alt=\"%1\" src=\"%2\" width=\"%3\" height=\"%4\" />").arg(escaped).arg(KUrl(path).url()).arg(p.width()).arg(p.height());
        e.matchTextEscaped = escaped;
        e.matchText = s;

        if (!s.isEmpty() && !escaped.isEmpty())
        {
            d->m_emoticonsIndex[escaped[0]].append(e);
            d->m_emoticonsIndex[s[0]].append(e);
        }
    }
}

void KEmoticonsProvider::removeEmoticonIndex(const QString &path, const QStringList &emoList)
{
    foreach(const QString &s, emoList) {
        QString escaped = Qt::escape(s);

        if (s.isEmpty() || escaped.isEmpty())
        {
            continue;
        }

        QList<Emoticon> ls = d->m_emoticonsIndex.value(escaped[0]);

        for (int i = 0; i < ls.size(); ++i) {
            if (ls.at(i).picPath == path) {
                ls.removeAt(i);
            }
        }

        ls = d->m_emoticonsIndex.value(s[0]);

        for (int i = 0; i < ls.size(); ++i) {
            if (ls.at(i).picPath == path) {
                ls.removeAt(i);
            }
        }
    }
}


// kate: space-indent on; indent-width 4; replace-tabs on;
