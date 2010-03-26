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

#include "kde_emoticons.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtGui/QImageReader>
#include <QtGui/QTextDocument>

#include <kpluginfactory.h>
#include <kdebug.h>
#include <kstandarddirs.h>

K_PLUGIN_FACTORY(KdeEmoticonsFactory, registerPlugin<KdeEmoticons>();)
K_EXPORT_PLUGIN(KdeEmoticonsFactory("KdeEmoticons"))

KdeEmoticons::KdeEmoticons(QObject *parent, const QVariantList &args)
        : KEmoticonsProvider(parent)
{
    Q_UNUSED(args);
}

bool KdeEmoticons::removeEmoticon(const QString &emo)
{
    QString emoticon = QFileInfo(emoticonsMap().key(emo.split(' '))).fileName();
    QDomElement fce = m_themeXml.firstChildElement("messaging-emoticon-map");

    if (fce.isNull())
        return false;

    QDomNodeList nl = fce.childNodes();
    for (uint i = 0; i < nl.length(); i++) {
        QDomElement de = nl.item(i).toElement();
        if (!de.isNull() && de.tagName() == "emoticon" && (de.attribute("file") == emoticon || de.attribute("file") == QFileInfo(emoticon).baseName())) {
            fce.removeChild(de);
            removeEmoticonsMap(emoticonsMap().key(emo.split(' ')));
            removeEmoticonIndex(emoticon, emo.split(' '));
            return true;
        }
    }
    return false;
}

bool KdeEmoticons::addEmoticon(const QString &emo, const QString &text, AddEmoticonOption option)
{
    KEmoticonsProvider::addEmoticon(emo, text, option);

    const QStringList splitted = text.split(' ');
    QDomElement fce = m_themeXml.firstChildElement("messaging-emoticon-map");

    if (fce.isNull())
        return false;

    QDomElement emoticon = m_themeXml.createElement("emoticon");
    emoticon.setAttribute("file", QFileInfo(emo).fileName());
    fce.appendChild(emoticon);
    QStringList::const_iterator constIterator;
    for (constIterator = splitted.begin(); constIterator != splitted.end(); ++constIterator) {
        QDomElement emoText = m_themeXml.createElement("string");
        QDomText txt = m_themeXml.createTextNode((*constIterator).trimmed());
        emoText.appendChild(txt);
        emoticon.appendChild(emoText);
    }

    addEmoticonIndex(emo, splitted);
    addEmoticonsMap(emo, splitted);
    return true;
}

void KdeEmoticons::save()
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

bool KdeEmoticons::loadTheme(const QString &path)
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

    QDomElement fce = m_themeXml.firstChildElement("messaging-emoticon-map");

    if (fce.isNull())
        return false;

    QDomNodeList nl = fce.childNodes();

    clearEmoticonsMap();

    for (uint i = 0; i < nl.length(); i++) {
        QDomElement de = nl.item(i).toElement();

        if (!de.isNull() && de.tagName() == "emoticon") {
            QDomNodeList snl = de.childNodes();
            QStringList sl;

            for (uint k = 0; k < snl.length(); k++) {
                QDomElement sde = snl.item(k).toElement();

                if (!sde.isNull() && sde.tagName() == "string") {
                    sl << sde.text();
                }
            }

            QString emo = KGlobal::dirs()->findResource("emoticons", themeName() + '/' + de.attribute("file"));

            if (emo.isEmpty()) {
                QList<QByteArray> ext = QImageReader::supportedImageFormats();

                for (int j = 0; j < ext.size(); ++j) {
                    emo = KGlobal::dirs()->findResource("emoticons", themeName() + '/' + de.attribute("file") + '.' + ext.at(j));
                    if (!emo.isEmpty()) {
                        break;
                    }
                }

                if (emo.isEmpty()) {
                    continue;
                }
            }

            addEmoticonIndex(emo, sl);
            addEmoticonsMap(emo, sl);
        }
    }

    return true;
}

void KdeEmoticons::createNew()
{
    QString path = KGlobal::dirs()->saveLocation("emoticons", themeName());

    QFile fp(path + '/' + "emoticons.xml");

    if (!fp.open(QIODevice::WriteOnly)) {
        kWarning() << fp.fileName() << "can't open WriteOnly!";
        return;
    }

    QDomDocument doc;
    doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\""));
    doc.appendChild(doc.createElement("messaging-emoticon-map"));

    QTextStream emoStream(&fp);
    emoStream.setCodec( "UTF-8" );
    emoStream << doc.toString(4);
    fp.close();
}

// kate: space-indent on; indent-width 4; replace-tabs on;
