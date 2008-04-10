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

#include "kde_emoticons.h"

#include <QtCore/QFile>

#include <KPluginFactory>
#include <KDebug>
#include <KStandardDirs>

K_PLUGIN_FACTORY(KdeEmoticonsFactory, registerPlugin<KdeEmoticons>();)
K_EXPORT_PLUGIN(KdeEmoticonsFactory("KdeEmoticons"))

KdeEmoticons::KdeEmoticons(QObject *parent, const QVariantList &args)
    : KEmoticonsTheme(parent, args)
{
}

KdeEmoticons::~KdeEmoticons()
{
}

bool KdeEmoticons::removeEmoticon(const QString &emo)
{
}

bool KdeEmoticons::addEmoticon(const QString &emo, const QString &text, bool copy)
{
}

void KdeEmoticons::save()
{
}

bool KdeEmoticons::loadTheme(const QString &path)
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
        kWarning() << error << eli << eco;
        fp.close();
        return false;
    }
    
    fp.close();
    
    QDomElement fce = m_themeXml.firstChildElement("messaging-emoticon-map");
    
    if(fce.isNull())
        return false;
    
    QDomNodeList nl = fce.childNodes();
    
    d->m_themeMap.clear();
    
    for(uint i = 0; i < nl.length(); i++) {
        QDomElement de = nl.item(i).toElement();
        
        if(!de.isNull() && de.tagName() == "emoticon") {
            QDomNodeList snl = de.childNodes();
            QStringList sl;
            
            for(uint k = 0; k < snl.length(); k++) {
                QDomElement sde = snl.item(k).toElement();
                
                if(!sde.isNull() && sde.tagName() == "string") {
                    sl << sde.text();
                }
            }
            
            QString emo = KGlobal::dirs()->findResource( "emoticons", d->m_themeName + '/' + de.attribute("file"));
            
            //have to find something better than try all extensions
//             if( emo.isNull() )
//                 emo = KGlobal::dirs()->findResource( "emoticons", themeName + '/' + de.attribute("file") + ".mng" );
//             if ( emo.isNull() )
//                 emo = KGlobal::dirs()->findResource( "emoticons", themeName + '/' + de.attribute("file") + ".png" );
//             if ( emo.isNull() )
//                 emo = KGlobal::dirs()->findResource( "emoticons", themeName + '/' + de.attribute("file") + ".gif" );
//             if ( emo.isNull() )
//                 continue;
            
            d->m_themeMap[emo] = sl;
        }
    }
    
    return true;
}

// kate: space-indent on; indent-width 4; replace-tabs on;
