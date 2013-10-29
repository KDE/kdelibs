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
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QImageReader>
#include <QMimeDatabase>

#include <kpluginfactory.h>

K_PLUGIN_FACTORY(XmppEmoticonsFactory, registerPlugin<XmppEmoticons>();)

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
    for (int i = 0; i < nl.length(); i++) {
        QDomElement de = nl.item(i).toElement();
        if (!de.isNull() && de.tagName() == "icon") {
            QDomNodeList snl = de.childNodes();
            QStringList sl;
            QStringList mime;

            for (int k = 0; k < snl.length(); k++) {
                QDomElement sde = snl.item(k).toElement();

                if (!sde.isNull() && sde.tagName() == "object" && sde.text() == emoticon) {
                    fce.removeChild(de);
                    removeMapItem(emoticonsMap().key(emo.split(' ')));
                    removeIndexItem(emoticon, emo.split(' '));
                    return true;
                }
            }
        }
    }
    return false;
}

bool XmppEmoticons::addEmoticon(const QString &emo, const QString &text, AddEmoticonOption option)
{
    if (option == Copy) {
        bool result = copyEmoticon(emo);
        if (!result) {
            qWarning() << "There was a problem copying the emoticon";
            return false;
        }
    }

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
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(emo, QMimeDatabase::MatchExtension);
    emoElement.setAttribute("mime", mime.name());
    QDomText txt = m_themeXml.createTextNode(QFileInfo(emo).fileName());

    emoElement.appendChild(txt);
    emoticon.appendChild(emoElement);

    addIndexItem(emo, splitted);
    addMapItem(emo, splitted);
    return true;
}

void XmppEmoticons::saveTheme()
{
    QFile fp(themePath() + '/' + fileName());

    if (!fp.exists()) {
        qWarning() << fp.fileName() << "doesn't exist!";
        return;
    }

    if (!fp.open(QIODevice::WriteOnly)) {
        qWarning() << fp.fileName() << "can't open WriteOnly!";
        return;
    }

    QTextStream emoStream(&fp);
    emoStream.setCodec( "UTF-8" );
    emoStream << m_themeXml.toString(4);
    fp.close();
}

bool XmppEmoticons::loadTheme(const QString &path)
{
    QFile file(path);

    if (!file.exists()) {
        qWarning() << path << "doesn't exist!";
        return false;
    }

    setThemePath(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << file.fileName() << "can't be open ReadOnly!";
        return false;
    }

    QString error;
    int eli, eco;
    if (!m_themeXml.setContent(&file, &error, &eli, &eco)) {
        qWarning() << file.fileName() << "can't copy to xml!";
        qWarning() << error << "line:" << eli << "column:" << eco;
        file.close();
        return false;
    }

    file.close();

    QDomElement fce = m_themeXml.firstChildElement("icondef");

    if (fce.isNull()) {
        return false;
    }

    QDomNodeList nl = fce.childNodes();

    clearEmoticonsMap();

    for (int i = 0; i < nl.length(); i++) {
        QDomElement de = nl.item(i).toElement();

        if (!de.isNull() && de.tagName() == "icon") {
            QDomNodeList snl = de.childNodes();
            QStringList sl;
            QString emo;
            QStringList mime;
            mime << "image/png" << "image/gif" << "image/bmp" << "image/jpeg";

            for (int k = 0; k < snl.length(); k++) {
                QDomElement sde = snl.item(k).toElement();

                if (!sde.isNull() && sde.tagName() == "text") {
                    sl << sde.text();
                } else if (!sde.isNull() && sde.tagName() == "object" && mime.contains(sde.attribute("mime"))) {
                    emo = sde.text();
                }
            }

            emo = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "emoticons/" + themeName() + '/' + emo);

            if (emo.isEmpty()) {
                continue;
            }

            addIndexItem(emo, sl);
            addMapItem(emo, sl);
        }
    }

    return true;
}

void XmppEmoticons::newTheme()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/emoticons/" + themeName();
    QDir().mkpath(path);

    QFile fp(path + '/' + "icondef.xml");

    if (!fp.open(QIODevice::WriteOnly)) {
        qWarning() << fp.fileName() << "can't open WriteOnly!";
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

#include "xmpp_emoticons.moc"

// kate: space-indent on; indent-width 4; replace-tabs on;
