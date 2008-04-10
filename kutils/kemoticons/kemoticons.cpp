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

#include <QFile>
#include <QDir>

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

KEmoticonsTheme *KEmoticonsPrivate::loadThemeLibrary(const KService::Ptr &service)
{
    KPluginFactory *factory = KPluginLoader(service->library()).factory();
    if (!factory)
    {
        kWarning()<<"Invalid plugin factory for"<<service->library();
        return 0;
    }
    KEmoticonsTheme *theme = factory->create<KEmoticonsTheme>(0);
    return theme;
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

KEmoticonsTheme *KEmoticons::getTheme(const QString &name)
{
    for (int i = 0; i < d->m_loaded.size(); ++i) {
        QString fName = d->m_loaded.at(i)->property("X-KDE-EmoticonsFileName").toString();
        QString path = KGlobal::dirs()->findResource("emoticons", name + '/' + fName);
        
        if (QFile::exists(path)) {
            KEmoticonsTheme *theme = d->loadThemeLibrary(d->m_loaded.at(i));
            theme->loadTheme(path);
            return theme;
        }
    }
    return 0;
}

QStringList KEmoticons::getThemeList()
{
    QStringList ls;
    QStringList themeDirs = KGlobal::dirs()->findDirs("emoticons", "");
    
    for (int i = 0; i < themeDirs.count(); ++i) {
        QDir themeQDir(themeDirs[i]);
        themeQDir.setFilter(QDir::Dirs|QDir::NoDotAndDotDot);
        themeQDir.setSorting(QDir::Name);
        ls << themeQDir.entryList();
    }
    return ls;
}

void KEmoticons::setTheme(const KEmoticonsTheme &theme)
{
}

void KEmoticons::setTheme(const QString &theme)
{
}

KEmoticonsTheme *KEmoticons::newTheme(const QString &name, const KService::Ptr &service)
{
    KEmoticonsTheme *theme = d->loadThemeLibrary(service);
    theme->setThemeName(name);
    return theme;
}

QList<KService::Ptr> KEmoticons::loadedServices()
{
    return d->m_loaded;
}

// kate: space-indent on; indent-width 4; replace-tabs on;
