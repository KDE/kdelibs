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

#include "kemoticonstheme.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include <kio/netaccess.h>
#include <KStandardDirs>

KEmoticonsThemePrivate::KEmoticonsThemePrivate()
{
}

KEmoticonsTheme::KEmoticonsTheme(QObject *parent, const QVariantList &args)
    : QObject(parent), d(new KEmoticonsThemePrivate)
{
    Q_UNUSED(args);
}

KEmoticonsTheme::~KEmoticonsTheme()
{
    delete d;
}

bool KEmoticonsTheme::loadTheme(const QString &path)
{
    QFileInfo info(path);
    d->m_fileName = info.fileName();
    d->m_themeName = info.dir().dirName();
    d->m_themePath = info.absolutePath();
    return true;
}

bool KEmoticonsTheme::removeEmoticon(const QString &emo)
{
    Q_UNUSED(emo);
    return false;
}

bool KEmoticonsTheme::addEmoticon(const QString &emo, const QString &text, bool copy)
{
    if(copy) {
        KIO::NetAccess::dircopy(KUrl(emo), KUrl(d->m_themePath));
    }

    Q_UNUSED(text);
    return false;
}

void KEmoticonsTheme::save()
{
}

QString KEmoticonsTheme::themeName()
{
    return d->m_themeName;
}

void KEmoticonsTheme::setThemeName(const QString &name)
{
    d->m_themeName = name;
}

QMap<QString, QStringList> KEmoticonsTheme::themeMap()
{
    return d->m_themeMap;
}

// kate: space-indent on; indent-width 4; replace-tabs on;
