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
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <KDebug>
#include <KStandardDirs>
#include <KArchive>
#include <KProgressDialog>
#include <KMessageBox>
#include <KMimeType>
#include <KZip>
#include <KTar>
#include <kio/netaccess.h>
#include <klocalizedstring.h>


KEmoticons::KEmoticons()
{
}

KEmoticons::~KEmoticons()
{
}

KEmoticonsTheme *KEmoticons::getTheme()
{
}

KEmoticonsTheme *KEmoticons::getTheme(const QString &name)
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
