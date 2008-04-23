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

#include "pidgin_emoticons.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtGui/QImageReader>

#include <KPluginFactory>
#include <KDebug>
#include <KStandardDirs>

K_PLUGIN_FACTORY(PidginEmoticonsFactory, registerPlugin<PidginEmoticons>();)
K_EXPORT_PLUGIN(PidginEmoticonsFactory("PidginEmoticons"))

PidginEmoticons::PidginEmoticons(QObject *parent, const QVariantList &args)
    : KEmoticonsTheme(parent, args)
{
}

PidginEmoticons::~PidginEmoticons()
{
}

bool PidginEmoticons::removeEmoticon(const QString &emo)
{
    QString emoticon = QFileInfo(d->m_emoticonsMap.key(emo.split(" "))).fileName();
    QDomElement fce = m_themeXml.firstChildElement("messaging-emoticon-map");

    if (fce.isNull())
        return false;

    QDomNodeList nl = fce.childNodes();
    for (uint i = 0; i < nl.length(); i++) {
        QDomElement de = nl.item(i).toElement();
        if(!de.isNull() && de.tagName() == "emoticon" && (de.attribute("file") == emoticon || de.attribute("file") == QFileInfo(emoticon).baseName())) {
            fce.removeChild(de);
            d->m_emoticonsMap.remove(d->m_emoticonsMap.key(emo.split(" ")));
            return true;
        }
    }
    return false;
}

bool PidginEmoticons::addEmoticon(const QString &emo, const QString &text, bool copy)
{
    KEmoticonsTheme::addEmoticon(emo, text, copy);

    QStringList splitted = text.split(" ");
    QDomElement fce = m_themeXml.firstChildElement("messaging-emoticon-map");

    if (fce.isNull())
        return false;

    QDomElement emoticon = m_themeXml.createElement("emoticon");
    emoticon.setAttribute("file", QFileInfo(emo).fileName());
    fce.appendChild(emoticon);
    QStringList::const_iterator constIterator;
    for(constIterator = splitted.begin(); constIterator != splitted.end(); constIterator++)
    {
        QDomElement emoText = m_themeXml.createElement("string");
        QDomText txt = m_themeXml.createTextNode((*constIterator).trimmed());
        emoText.appendChild(txt);
        emoticon.appendChild(emoText);
    }
    d->m_emoticonsMap[emo] = splitted;
    return true;
}

void PidginEmoticons::save()
{
    QFile fp(d->m_themePath + '/' + d->m_fileName);

    if (!fp.exists()) {
        kWarning() << fp.fileName() << "doesn't exist!";
        return;
    }

    if (!fp.open( QIODevice::WriteOnly)) {
        kWarning() << fp.fileName() << "can't open WriteOnly!";
        return;
    }

    QTextStream emoStream(&fp);
    emoStream << m_themeXml.toString(4);
    fp.close();
}

bool PidginEmoticons::loadTheme(const QString &path)
{
    KEmoticonsTheme::loadTheme(path);

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
    while (!str.atEnd()) {
         QString line = str.readLine();
         
         if (line.startsWith('#') || line.isEmpty()) {
            continue;
         }
         
         QRegExp re("^\[(.*)\]$");
         int pos = re.indexIn(line.trim());
         if (pos > -1) {
            
         }
         
    }

    fp.close();

  

            d->m_emoticonsMap[emo] = sl;
    return true;
}

void PidginEmoticons::createNew()
{
    QString path = KGlobal::dirs()->saveLocation("emoticons", themeName(), false);
    
    QFile fp(path + '/' + "theme");
    
    if (!fp.open( QIODevice::WriteOnly)) {
        kWarning() << fp.fileName() << "can't open WriteOnly!";
        return;
    }
    
    QDomDocument doc;
    doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\""));
    doc.appendChild(doc.createElement("messaging-emoticon-map"));
    
    QTextStream emoStream(&fp);
    emoStream << doc.toString(4);
    fp.close();
}

// kate: space-indent on; indent-width 4; replace-tabs on;
