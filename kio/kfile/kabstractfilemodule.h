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

#ifndef KABSTRACTFILEMODULE_H
#define KABSTRACTFILEMODULE_H

#include <kio/kio_export.h>
#include <QtCore/QObject>
#include <kurl.h>
class QWidget;

/**
 * Interface to the module that contains the file widget.
 *
 * libkfilemodule returns an object that derives from KAbstractFileModule.
 *
 * This is internal to KFileDialog.
 */
class KIO_EXPORT KAbstractFileModule : public QObject
{
    Q_OBJECT
public:
    KAbstractFileModule(QObject* parent);

    /// the returned widget implements the KAbstractFileWidget interface
    virtual QWidget* createFileWidget(const KUrl& startDir, QWidget *parent) = 0;

    virtual KUrl getStartUrl( const KUrl& startDir, QString& recentDirClass ) = 0;

    virtual void setStartDir( const KUrl& directory ) = 0;

    /**
     * Creates a modal file dialog and returns the selected
     * directory or an empty string if none was chosen.
     */
    virtual KUrl selectDirectory( const KUrl& startDir,
                                  bool localOnly, QWidget *parent,
                                  const QString& caption ) = 0;

    // TODO QFileDialog-like methods
};

#endif /* KABSTRACTFILEMODULE_H */
