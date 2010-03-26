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

#include "xmpp_emoticons.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtGui/QImageReader>

#include <kpluginfactory.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kmimetype.h>

K_PLUGIN_FACTORY(XmppEmoticonsFactory, registerPlugin<XmppEmoticons>();)
K_EXPORT_PLUGIN(XmppEmoticonsFactory("XmppEmoticons"))

XmppEmoticons::XmppEmoticons(QObject *parent, const QVariantList &args)
        : KEmoticonsProvider(parent)
{
    Q_UNUSED(args);
}

bool XmppEmoticons::removeEmoticon(const QString &emo)
{
    QString emoticon = QFileInfo(emoticonsMap().key(emo.split(' '))).fileName();
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
                    removeEmoticonsMap(emoticonsMap().key(emo.split(' ')));
                    removeEmoticonIndex(emoticon, emo.split(' '));
                    return true;
                }
            }
        }
    }
    return false;
}

bool XmppEmoticons::addEmoticon(const QString &emo, const QString &text, AddEmoticonOption option)
{
    KEmoticonsProvider::addEmoticon(emo, text, option);

    const QStringList splitted = text.split(' ');
    QDomElement fce = m_themeXml.firstChildElement("icondef");

    if (fce.isNull()) {
        return false;
    }

    QDomElement emoticon = m_themeXml.createElement("icon");
    fce.appendChild(emoticon);
    QStringList::const_iterator constIterator;

    for (constIterator = splitted.begin(); constIterator != splitted.end(); ++constIterator) {
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

    addEmoticonIndex(emo, splitted);
    addEmoticonsMap(emo, splitted);
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
    emoStream.setCodec( "UTF-8" );
    emoStream << m_themeXml.toString(4);
    fp.close();
}

bool XmppEmoticons::loadTheme(const QString &path)
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

    clearEmoticonsMap();

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

            addEmoticonIndex(emo, sl);
            addEmoticonsMap(emo, sl);
        }
    }

    return true;
}

void XmppEmoticons::createNew()
{
    QString path = KGlobal::dirs()->saveLocation("emoticons", themeName());

    QFile fp(path + '/' + "icondef.xml");

    if (!fp.open(QIODevice::WriteOnly)) {
        kWarning() << fp.fileName() << "can't open WriteOnly!";
        return;
    }

    QDomDocument doc;
    doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
    doc.appendChild(doc.createElement("icondef"));

    QTextStream emoStream(&fp);
    emoStream.setCodec( "UTF-8" );
    emoStream << doc.toString(4);
    fp.close();
}

// kate: space-indent on; indent-width 4; replace-tabs on;
