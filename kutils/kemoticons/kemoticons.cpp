/***************************************************************************
 *   Copyright (C) 2007 by Carlo Segato <brandon.ml@gmail.com>             *
 *   Copyright (C) 2008 Montel Laurent <montel@kde.org>                    *
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

#include "kemoticons.h"

#include <KPluginLoader>
#include <KDebug>
#include <KStandardDirs>


KEmoticonsPrivate::KEmoticonsPrivate()
{
}

void KEmoticonsPrivate::loadServiceList()
{
    QString constraint("(exist Library)");
    KService::List services = KServiceTypeTrader::self()->query("KEmoticons", constraint);
    foreach (KService::Ptr service, services) {
        kDebug()<<"NAME:"<<service->name();
        m_loaded.append(service);
    }    
}

void KEmoticonsPrivate::loadThemeLibrary(const KService::Ptr &service)
{
    /*
    KPluginFactory *factory = KPluginLoader(service->library()).factory();
    if (!factory)
    {
        kWarning()<<"Invalid plugin factory for"<<service->library();
        return;
    }
    KEmoticonsTheme *theme = factory->create<KEmoticonsTheme>(0);
    m_loaded.insert(service->name(), theme);
    */
}

KEmoticons::KEmoticons()
    : d(new KEmoticonsPrivate)
{
    d->loadServiceList();
}

KEmoticons::~KEmoticons()
{
    delete d;
}

KEmoticonsTheme KEmoticons::getTheme()
{
}

QList<KEmoticonsTheme *> KEmoticons::getTheme(const QString &name)
{
    QStringList themeDirs = KGlobal::dirs()->findDirs("emoticons", name);
    QList<KEmoticonsTheme *> ls;
    
    for (int i = 0; i < ls.size(); ++i) {
        
    }
    
    return ls;
}

QList<KEmoticonsTheme> KEmoticons::getThemeList()
{
}

void KEmoticons::setTheme(const KEmoticonsTheme &theme)
{
}

void KEmoticons::setTheme(const QString &theme)
{
}

KEmoticonsTheme *KEmoticons::newTheme(const QString &name)
{
}

// kate: space-indent on; indent-width 4; replace-tabs on;
