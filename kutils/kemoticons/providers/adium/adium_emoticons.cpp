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

#include "adium_emoticons.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtGui/QImageReader>

#include <kpluginfactory.h>
#include <kdebug.h>
#include <kstandarddirs.h>

K_PLUGIN_FACTORY(AdiumEmoticonsFactory, registerPlugin<AdiumEmoticons>();)
K_EXPORT_PLUGIN(AdiumEmoticonsFactory("AdiumEmoticons"))

AdiumEmoticons::AdiumEmoticons(QObject *parent, const QVariantList &args)
        : KEmoticonsProvider(parent)
{
    Q_UNUSED(args)
}

bool AdiumEmoticons::removeEmoticon(const QString &emo)
{
    QString emoticon = QFileInfo(emoticonsMap().key(emo.split(' '))).fileName();
    QDomElement fce = m_themeXml.firstChildElement("plist").firstChildElement("dict").firstChildElement("dict");

    if (fce.isNull()) {
        return false;
    }

    QDomNodeList nl = fce.childNodes();
    for (uint i = 0; i < nl.length(); i++) {
        QDomElement de = nl.item(i).toElement();
        if (!de.isNull() && de.tagName() == "key" && (de.text() == emoticon)) {
            QDomElement dict = de.nextSiblingElement();
            if (!dict.isNull() && dict.tagName() == "dict") {
                fce.removeChild(dict);
            }

            fce.removeChild(de);
            removeEmoticonsMap(emoticonsMap().key(emo.split(' ')));
            removeEmoticonIndex(emoticon, emo.split(' '));
            return true;
        }
    }
    return false;
}

bool AdiumEmoticons::addEmoticon(const QString &emo, const QString &text, AddEmoticonOption option)
{
    KEmoticonsProvider::addEmoticon(emo, text, option);

    const QStringList splitted = text.split(' ');
    QDomElement fce = m_themeXml.firstChildElement("plist").firstChildElement("dict").firstChildElement("dict");

    if (fce.isNull()) {
        return false;
    }

    QDomElement emoticon = m_themeXml.createElement("key");
    emoticon.appendChild(m_themeXml.createTextNode(QFileInfo(emo).fileName()));
    fce.appendChild(emoticon);

    QDomElement dict = m_themeXml.createElement("dict");
    QDomElement el = m_themeXml.createElement("key");
    el.appendChild(m_themeXml.createTextNode("Equivalents"));
    dict.appendChild(el);

    QDomElement arr = m_themeXml.createElement("array");

    QStringList::const_iterator constIterator;
    for (constIterator = splitted.begin(); constIterator != splitted.end(); ++constIterator) {
        QDomElement emoText = m_themeXml.createElement("string");
        QDomText txt = m_themeXml.createTextNode((*constIterator).trimmed());
        emoText.appendChild(txt);
        arr.appendChild(emoText);
    }

    dict.appendChild(arr);

    el = m_themeXml.createElement("key");
    el.appendChild(m_themeXml.createTextNode("Name"));
    dict.appendChild(el);

    el = m_themeXml.createElement("string");
    el.appendChild(m_themeXml.createTextNode(QFileInfo(emo).baseName()));
    dict.appendChild(el);

    fce.appendChild(dict);

    addEmoticonIndex(emo, splitted);
    addEmoticonsMap(emo, splitted);
    return true;
}

void AdiumEmoticons::save()
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

bool AdiumEmoticons::loadTheme(const QString &path)
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

    QDomElement fce = m_themeXml.firstChildElement("plist").firstChildElement("dict").firstChildElement("dict");

    if (fce.isNull()) {
        return false;
    }

    QDomNodeList nl = fce.childNodes();

    clearEmoticonsMap();
    QString name;
    for (uint i = 0; i < nl.length(); i++) {
        QDomElement de = nl.item(i).toElement();

        if (!de.isNull() && de.tagName() == "key") {
            name = KGlobal::dirs()->findResource("emoticons", themeName() + '/' + de.text());
            continue;
        } else if (!de.isNull() && de.tagName() == "dict") {
            QDomElement arr = de.firstChildElement("array");
            QDomNodeList snl = arr.childNodes();
            QStringList sl;

            for (uint k = 0; k < snl.length(); k++) {
                QDomElement sde = snl.item(k).toElement();

                if (!sde.isNull() && sde.tagName() == "string") {
                    sl << sde.text();
                }
            }
            if (!name.isEmpty()) {
                addEmoticonIndex(name, sl);
                addEmoticonsMap(name, sl);
                name.clear();
            }
        }
    }

    return true;
}

void AdiumEmoticons::createNew()
{
    QString path = KGlobal::dirs()->saveLocation("emoticons", themeName());

    QFile fp(path + '/' + "Emoticons.plist");

    if (!fp.open(QIODevice::WriteOnly)) {
        kWarning() << fp.fileName() << "can't open WriteOnly!";
        return;
    }

    QDomDocumentType ty = QDomImplementation().createDocumentType("plist", "-//Apple Computer//DTD PLIST 1.0//EN", "http://www.apple.com/DTDs/PropertyList-1.0.dtd");
    QDomDocument doc(ty);
    doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));

    QDomElement plist = doc.createElement("plist");
    plist.setAttribute("version", "1.0");
    doc.appendChild(plist);

    QDomElement dict = doc.createElement("dict");
    plist.appendChild(dict);

    QDomElement el = doc.createElement("key");
    el.appendChild(doc.createTextNode("AdiumSetVersion"));
    dict.appendChild(el);

    el = doc.createElement("integer");
    el.appendChild(doc.createTextNode("1"));
    dict.appendChild(el);

    el = doc.createElement("key");
    el.appendChild(doc.createTextNode("Emoticons"));
    dict.appendChild(el);

    dict.appendChild(doc.createElement("dict"));


    QTextStream emoStream(&fp);
    emoStream.setCodec( "UTF-8" );
    emoStream << doc.toString(4);
    fp.close();
}

// kate: space-indent on; indent-width 4; replace-tabs on;
