/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>

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
#ifndef KNEWSTUFF2_PROVIDER_H
#define KNEWSTUFF2_PROVIDER_H

#include <knewstuff2/core/feed.h>

#include <kurl.h>

#include <qdom.h>
#include <qobject.h>
#include <qlist.h>
#include <qstring.h>

namespace KIO { class Job; }

namespace KNS {

/**
 * @short KNewStuff provider container.
 *
 * This class provides accessors for the provider object.
 * as used by KNewStuff.
 * It should probably not be used directly by the application.
 *
 * @author Cornelius Schumacher (schumacher@kde.org)
 * \par Maintainer:
 * Josef Spillner (spillner@kde.org)
 */
class KDE_EXPORT Provider
{
  public:
    typedef QList<Provider*> List;

    /**
     * Constructor.
     */
    Provider();

    /**
     * Destructor.
     */
    ~Provider();

    /**
     * Sets the common name of the provider.
     */
    void setName( const KTranslatable& name );

    /**
     * Retrieves the common name of the provider.
     *
     * @return provider name
     */
    KTranslatable name() const;

    /**
     * Sets the download URL.
     */
    void setDownloadUrl( const KUrl & );

    /**
     * Retrieves the download URL.
     *
     * @return download URL
     */
    KUrl downloadUrl() const;

    /**
     * Adds a feed URL.
     */
    void addDownloadUrlFeed( QString feedtype, Feed *feed );

    /**
     * Feed to retrieve 'tagged' (variant) download URLs.
     * Feed can be one of 'score', 'downloads', 'latest'.
     *
     * @return download URL of a certain feed
     */
    Feed *downloadUrlFeed( QString feedtype ) const;

    /**
     * Returns a list of all feeds.
     */
    QStringList feeds() const;

    /**
     * Sets the upload URL.
     */
    void setUploadUrl( const KUrl & );

    /**
     * Retrieves the upload URL.
     *
     * @return upload URL
     */
    KUrl uploadUrl() const;

    /**
     * Sets the URL where a user is led if the provider does not support
     * uploads.
     */
    void setNoUploadUrl( const KUrl & );

    /**
     * Retrieves the URL where a user is led if the provider does not
     * support uploads.
     *
     * @return website URL
     */
    KUrl noUploadUrl() const;

    /**
     * Sets the URL of a web frontend for the provider.
     */
    void setWebAccess( const KUrl & );

    /**
     * Retrieves the web frontend URL.
     *
     * @return web frontend URL
     */
    KUrl webAccess() const;

    /**
     * Sets the URL of the DXS, if offered by the provider.
     */
    void setWebService( const KUrl & );

    /**
     * Retrieves the URL to the DXS Web Service.
     *
     * @return DXS Web Service URL
     */
    KUrl webService() const;

    /**
     * Sets the URL for an icon for this provider.
     * The icon should be in 32x32 format. If not set, the default icon
     * of KDialogBase is used.
     */
    void setIcon( const KUrl & );

    /**
     * Retrieves the icon URL for this provider.
     *
     * @return icon URL
     */
    KUrl icon() const;

  private:
    KTranslatable mName;
    KUrl mDownloadUrl;
    KUrl mUploadUrl;
    KUrl mNoUploadUrl;
    KUrl mWebAccess;
    KUrl mWebService;
    KUrl mIcon;
    QMap<QString, Feed*> mFeeds;

    class ProviderPrivate *d;
};

}

#endif
