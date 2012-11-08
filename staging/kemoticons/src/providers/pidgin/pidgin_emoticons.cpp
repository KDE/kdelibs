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

#include "pidgin_emoticons.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include <kpluginfactory.h>
#include <kdebug.h>
#include <kstandarddirs.h>

K_PLUGIN_FACTORY(PidginEmoticonsFactory, registerPlugin<PidginEmoticons>();)
K_EXPORT_PLUGIN(PidginEmoticonsFactory("PidginEmoticons"))

PidginEmoticons::PidginEmoticons(QObject *parent, const QVariantList &args)
        : KEmoticonsProvider(parent)
{
    Q_UNUSED(args);
}

bool PidginEmoticons::removeEmoticon(const QString &emo)
{
    QString emoticon = QFileInfo(emoticonsMap().key(emo.split(' '))).fileName();

    bool start = false;
    for (int i = 0; i < m_text.size(); ++i) {
        QString line = m_text.at(i);

        if (line.startsWith('#') || line.isEmpty()) {
            continue;
        }

        QRegExp re("^\\[(.*)\\]$");
        int pos = re.indexIn(line.trimmed());
        if (pos > -1) {
            if (!re.cap(1).compare("default", Qt::CaseInsensitive)) {
                start = true;
            } else {
                start = false;
            }
            continue;
        }

        if (!start) {
            continue;
        }

        QStringList splitted = line.split(' ');
        QString emoName;

        if (splitted.at(0) == "!") {
            emoName = splitted.at(1);
        } else {
            emoName = splitted.at(0);
        }

        if (emoName == emoticon) {
            m_text.removeAt(i);
            removeEmoticonIndex(emoticon, emo.split(' '));
            return true;
        }
    }

    return false;
}

bool PidginEmoticons::addEmoticon(const QString &emo, const QString &text, AddEmoticonOption option)
{
    KEmoticonsProvider::addEmoticon(emo, text, option);

    const QStringList splitted = text.split(' ');
    int i = m_text.indexOf(QRegExp("^\\[default\\]$", Qt::CaseInsensitive));

    if (i == -1) {
        return false;
    }

    QString emoticon = QString("%1 %2").arg(QFileInfo(emo).fileName()).arg(text);
    m_text.insert(i + 1, emoticon);

    addEmoticonIndex(emo, splitted);
    addEmoticonsMap(emo, splitted);
    return true;
}

void PidginEmoticons::save()
{
    QFile fp(themePath() + '/' + fileName());

    if (!fp.exists()) {
        kWarning() << fp.fileName() << "doesn't exist!";
        return;
    }

    if (!fp.open(QIODevice::WriteOnly)) {
        kWarning() << fp.fileName() << "can't open WriteOnly!";
        return;
    }

    QTextStream emoStream(&fp);

    if (m_text.indexOf(QRegExp("^Icon=.*", Qt::CaseInsensitive)) == -1) {
        int i = m_text.indexOf(QRegExp("^Description=.*", Qt::CaseInsensitive));
        QString file = QFileInfo(emoticonsMap().keys().value(0)).fileName();
        m_text.insert(i + 1, "Icon=" + file);
    }

    emoStream << m_text.join("\n");
    fp.close();
}

bool PidginEmoticons::loadTheme(const QString &path)
{
    KEmoticonsProvider::loadTheme(path);

    QFile fp(path);

    if (!fp.exists()) {
        kWarning() << path << "doesn't exist!";
        return false;
    }

    if (!fp.open(QIODevice::ReadOnly)) {
        kWarning() << fp.fileName() << "can't open ReadOnly!";
        return false;
    }

    QTextStream str(&fp);
    bool start = false;
    m_text.clear();
    while (!str.atEnd()) {
        QString line = str.readLine();
        m_text << line;

        if (line.startsWith('#') || line.isEmpty()) {
            continue;
        }

        QRegExp re("^\\[(.*)\\]$");
        int pos = re.indexIn(line.trimmed());
        if (pos > -1) {
            if (!re.cap(1).compare("default", Qt::CaseInsensitive)) {
                start = true;
            } else {
                start = false;
            }
            continue;
        }

        if (!start) {
            continue;
        }

        QStringList splitted = line.split(QRegExp("\\s+"));
        QString emo;
        int i = 1;
        if (splitted.at(0) == "!") {
            i = 2;
            emo = KGlobal::dirs()->findResource("emoticons", themeName() + '/' + splitted.at(1));
        } else {
            emo = KGlobal::dirs()->findResource("emoticons", themeName() + '/' + splitted.at(0));
        }

        QStringList sl;
        for (; i < splitted.size(); ++i) {
            if (!splitted.at(i).isEmpty() && splitted.at(i) != " ") {
                sl << splitted.at(i);
            }
        }

        addEmoticonIndex(emo, sl);
        addEmoticonsMap(emo, sl);
    }

    fp.close();

    return true;
}

void PidginEmoticons::createNew()
{
    QString path = KGlobal::dirs()->saveLocation("emoticons", themeName());

    QFile fp(path + '/' + "theme");

    if (!fp.open(QIODevice::WriteOnly)) {
        kWarning() << fp.fileName() << "can't open WriteOnly!";
        return;
    }

    QTextStream out(&fp);
    out.setCodec( "UTF-8" );

    out << "Name=" + themeName() << endl;
    out << "Description=" + themeName() << endl;
    out << "Author=" << endl;
    out << endl;
    out << "[default]" << endl;

    fp.close();
}

// kate: space-indent on; indent-width 4; replace-tabs on;
