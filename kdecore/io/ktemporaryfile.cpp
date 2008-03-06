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
#include "kstandarddirs.h"

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
            return KStandardDirs::locateLocal("tmp", componentData.componentName(), componentData);
        }

        KComponentData componentData;
};

KTemporaryFile::KTemporaryFile(const KComponentData &componentData)
    : d(new KTemporaryFilePrivate(componentData))
{
    QString temp(d->defaultPrefix());
    setFileTemplate(temp + "XXXXXX.tmp");
}

KTemporaryFile::~KTemporaryFile()
{
    delete d;
}

void KTemporaryFile::setPrefix(const QString &prefix)
{
    QString oldTemplate = fileTemplate();
    QString suffix = oldTemplate.mid(oldTemplate.lastIndexOf("XXXXXX")+6);
    QString newPrefix = prefix;

    if ( newPrefix.isEmpty() ) {
        newPrefix = d->defaultPrefix();
    } else {
        if ( !QDir::isAbsolutePath(newPrefix) ) {
            newPrefix.prepend ( KStandardDirs::locateLocal("tmp", "") );
        }
    }

    setFileTemplate(newPrefix + "XXXXXX" + suffix);
}

void KTemporaryFile::setSuffix(const QString &suffix)
{
    QString oldTemplate = fileTemplate();
    QString prefix = oldTemplate.left(oldTemplate.indexOf("XXXXXX"));

    setFileTemplate(prefix + "XXXXXX" + suffix);
}
