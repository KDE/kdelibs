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

#include "xmpp_emoticons.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtGui/QImageReader>

#include <KPluginFactory>
#include <KDebug>
#include <KStandardDirs>
#include <KMimeType>

K_PLUGIN_FACTORY(XmppEmoticonsFactory, registerPlugin<XmppEmoticons>();)
K_EXPORT_PLUGIN(XmppEmoticonsFactory("XmppEmoticons"))

XmppEmoticons::XmppEmoticons(QObject *parent, const QVariantList &args)
        : KEmoticonsTheme(parent, args)
{
}

XmppEmoticons::~XmppEmoticons()
{
}

bool XmppEmoticons::removeEmoticon(const QString &emo)
{
    QString emoticon = QFileInfo(emoticonsMap()->key(emo.split(" "))).fileName();
    QDomElement fce = m_themeXml.firstChildElement("icondef");

    if (fce.isNull())
        return false;

    QDomNodeList nl = fce.childNodes();
    for (uint i = 0; i < nl.length(); i++) {
        QDomElement de = nl.item(i).toElement();
        if (!de.isNull() && de.tagName() == "icon") {
            QDomNodeList snl = de.childNodes();
            QStringList sl;
            QStringList mime;

            for (uint k = 0; k < snl.length(); k++) {
                QDomElement sde = snl.item(k).toElement();

                if (!sde.isNull() && sde.tagName() == "object" && sde.text() == emoticon) {
                    fce.removeChild(de);
                    emoticonsMap()->remove(emoticonsMap()->key(emo.split(" ")));
                    return true;
                }
            }
        }
    }
    return false;
}

bool XmppEmoticons::addEmoticon(const QString &emo, const QString &text, bool copy)
{
    KEmoticonsTheme::addEmoticon(emo, text, copy);

    QStringList splitted = text.split(" ");
    QDomElement fce = m_themeXml.firstChildElement("icondef");

    if (fce.isNull()) {
        return false;
    }

    QDomElement emoticon = m_themeXml.createElement("icon");
    fce.appendChild(emoticon);
    QStringList::const_iterator constIterator;

    for (constIterator = splitted.begin(); constIterator != splitted.end(); constIterator++) {
        QDomElement emotext = m_themeXml.createElement("text");
        QDomText txt = m_themeXml.createTextNode((*constIterator).trimmed());
        emotext.appendChild(txt);
        emoticon.appendChild(emotext);
    }

    QDomElement emoElement = m_themeXml.createElement("object");
    KMimeType::Ptr mimePtr = KMimeType::findByPath(emo, 0, true);
    emoElement.setAttribute("mime", mimePtr->name());
    QDomText txt = m_themeXml.createTextNode(QFileInfo(emo).fileName());

    emoElement.appendChild(txt);
    emoticon.appendChild(emoElement);

    (*emoticonsMap())[emo] = splitted;
    return true;
}

void XmppEmoticons::save()
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
    emoStream << m_themeXml.toString(4);
    fp.close();
}

bool XmppEmoticons::loadTheme(const QString &path)
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

    QString error;
    int eli, eco;
    if (!m_themeXml.setContent(&fp, &error, &eli, &eco)) {
        kWarning() << fp.fileName() << "can't copy to xml!";
        kWarning() << error << "line:" << eli << "column:" << eco;
        fp.close();
        return false;
    }

    fp.close();

    QDomElement fce = m_themeXml.firstChildElement("icondef");

    if (fce.isNull()) {
        return false;
    }

    QDomNodeList nl = fce.childNodes();

    emoticonsMap()->clear();

    for (uint i = 0; i < nl.length(); i++) {
        QDomElement de = nl.item(i).toElement();

        if (!de.isNull() && de.tagName() == "icon") {
            QDomNodeList snl = de.childNodes();
            QStringList sl;
            QString emo;
            QStringList mime;
            mime << "image/png" << "image/gif" << "image/bmp" << "image/jpeg";

            for (uint k = 0; k < snl.length(); k++) {
                QDomElement sde = snl.item(k).toElement();

                if (!sde.isNull() && sde.tagName() == "text") {
                    sl << sde.text();
                } else if (!sde.isNull() && sde.tagName() == "object" && mime.contains(sde.attribute("mime"))) {
                    emo = sde.text();
                }
            }

            emo = KGlobal::dirs()->findResource("emoticons", themeName() + '/' + emo);

            if (emo.isNull()) {
                continue;
            }

            (*emoticonsMap())[emo] = sl;
        }
    }

    return true;
}

void XmppEmoticons::createNew()
{
    QString path = KGlobal::dirs()->saveLocation("emoticons", themeName(), false);

    QFile fp(path + '/' + "icondef.xml");

    if (!fp.open(QIODevice::WriteOnly)) {
        kWarning() << fp.fileName() << "can't open WriteOnly!";
        return;
    }

    QDomDocument doc;
    doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
    doc.appendChild(doc.createElement("icondef"));

    QTextStream emoStream(&fp);
    emoStream << doc.toString(4);
    fp.close();
}

// kate: space-indent on; indent-width 4; replace-tabs on;
