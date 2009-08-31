/* This file is part of the KDE libraries
    Copyright 2007 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KFILEMODULE_H
#define KFILEMODULE_H

#include <kabstractfilemodule.h>

class KFileModule : public KAbstractFileModule
{
    Q_OBJECT
public:
    KFileModule(QObject* parent, const QVariantList&);

    virtual QWidget* createFileWidget(const KUrl& startDir, QWidget *parent);

    virtual KUrl getStartUrl( const KUrl& startDir, QString& recentDirClass );

    virtual void setStartDir( const KUrl& directory );

    virtual KUrl selectDirectory( const KUrl& startDir,
                                  bool localOnly, QWidget *parent,
                                  const QString& caption );
};

#endif /* KFILEMODULE_H */

