/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2006 Josef Spillner <spillner@kde.org>

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
#ifndef KNEWSTUFF2_PROVIDER_LOADER_H
#define KNEWSTUFF2_PROVIDER_LOADER_H

#include "provider.h"

#include <qcstring.h>
#include <qdom.h>
#include <qobject.h>
#include <qptrlist.h>
#include <qstring.h>

#include <kurl.h>

namespace KIO { class Job; }

namespace KNS {

//class Provider;
//class Provider::List;

/**
 * KNewStuff provider loader.
 * This class sets up a list of all possible providers by querying
 * the main provider database for this specific application.
 * It should probably not be used directly by the application.
 */
class KDE_EXPORT ProviderLoader : public QObject
{
    Q_OBJECT
  public:
    /**
     * Constructor.
     *
     * @param parentWidget the parent widget
     */
    ProviderLoader( QWidget *parentWidget );

    /**
     * Starts asynchronously loading the list of providers of the
     * specified type.
     *
     * @param type data type such as 'kdesktop/wallpaper'.
     * @param providerList the URl to the list of providers; if empty
     *    we first try the ProvidersUrl from KGlobal::config, then we
     *    fall back to a hardcoded value.
     */
    void load( const QString &type, const QString &providerList = QString::null );

  signals:
    /**
     * Indicates that the list of providers has been successfully loaded.
     */
    void providersLoaded( Provider::List * );

  protected slots:
    void slotJobData( KIO::Job *, const QByteArray & );
    void slotJobResult( KIO::Job * );

  private:
    QWidget *mParentWidget;

    QString mJobData;

    Provider::List mProviders;
};

}

#endif
