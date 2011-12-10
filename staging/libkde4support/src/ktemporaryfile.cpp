/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
/* This file is part of the KDE libraries
 *  Copyright 2006 Jaison Lee <lee.jaison@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "ktemporaryfile.h"
#include "kcomponentdata.h"

#include <QDir>

class KTemporaryFilePrivate
{
    public:
        KTemporaryFilePrivate(const KComponentData &c)
            : componentData(c)
        {
        }

        inline QString defaultPrefix() const
        {
            // was: KStandardDirs::locateLocal("tmp", componentData.componentName(), componentData);
            return QDir::tempPath() + QLatin1Char('/') + componentData.componentName();
        }

        KComponentData componentData;
};

KTemporaryFile::KTemporaryFile(const KComponentData &componentData)
    : d(new KTemporaryFilePrivate(componentData))
{
    QString temp(d->defaultPrefix());
    setFileTemplate(temp + QLatin1String("XXXXXX.tmp"));
}

KTemporaryFile::~KTemporaryFile()
{
    delete d;
}

void KTemporaryFile::setPrefix(const QString &prefix)
{
    QString oldTemplate = fileTemplate();
    QString suffix = oldTemplate.mid(oldTemplate.lastIndexOf(QLatin1String("XXXXXX"))+6);
    QString newPrefix = prefix;

    if ( newPrefix.isEmpty() ) {
        newPrefix = d->defaultPrefix();
    } else {
        if ( !QDir::isAbsolutePath(newPrefix) ) {
            newPrefix.prepend(QDir::tempPath() + QLatin1Char('/'));
        }
    }

    setFileTemplate(newPrefix + QLatin1String("XXXXXX") + suffix);
}

void KTemporaryFile::setSuffix(const QString &suffix)
{
    QString oldTemplate = fileTemplate();
    QString prefix = oldTemplate.left(oldTemplate.indexOf(QLatin1String("XXXXXX")));

    setFileTemplate(prefix + QLatin1String("XXXXXX") + suffix);
}
