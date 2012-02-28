/**
 * kspell_hunspellclient.cpp
 *
 * Copyright (C)  2009  Montel Laurent <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "kspell_hunspellclient.h"
#include <QDir>

#include "kspell_hunspelldict.h"

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kdebug.h>

K_PLUGIN_FACTORY( HunspellClientFactory, registerPlugin<HunspellClient>(); )
K_EXPORT_PLUGIN( HunspellClientFactory( "kspell_hunspell" ) )

using namespace Sonnet;

HunspellClient::HunspellClient( QObject *parent, const QVariantList& /* args */  )
    : Client( parent )
{
    kDebug()<<" HunspellClient::HunspellClient";
}

HunspellClient::~HunspellClient()
{
}

SpellerPlugin *HunspellClient::createSpeller(const QString &language)
{
    kDebug()<<" SpellerPlugin *HunspellClient::createSpeller(const QString &language) ;"<<language;
    HunspellDict *ad = new HunspellDict( language );
    return ad;
}

QStringList HunspellClient::languages() const
{
    QStringList lst;
    QDir dir("/usr/share/myspell/dicts/");
    if(dir.exists())
    {
        QStringList lstDic = dir.entryList(QStringList("*.dic"), QDir::Files );
        foreach(const QString &tmp, lstDic)
        {
            lst<< tmp.remove(".dic");
        }
    }
    kDebug()<<" lst !!!!!!!!!!!!!!!!!!!!!!!! :"<<lst;
    return lst;
}

#include "kspell_hunspellclient.moc"
