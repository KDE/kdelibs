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
    void setName( const QString & );

    /**
     * Retrieves the common name of the provider.
     *
     * @return provider name
     */
    QString name() const;

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
     * Variant to retrieve 'tagged' download URLs.
     * Variant can be one of 'score', 'downloads', 'latest'.
     *
     * @return download specific URL
     */
    KUrl downloadUrlVariant( QString variant ) const;

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
     *
     * @see setNoUpload
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
     * Indicate whether provider supports uploads.
     */
    void setNoUpload( bool );

    /**
     * Query whether provider supports uploads.
     *
     * @return upload support status
     */
    bool noUpload() const;

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
    QString mName;
    KUrl mDownloadUrl;
    KUrl mUploadUrl;
    KUrl mNoUploadUrl;
    KUrl mIcon;
    bool mNoUpload;
    KUrl mDownloadUrlLatest;
    KUrl mDownloadUrlScore;
    KUrl mDownloadUrlDownloads;

    class ProviderPrivate *d;
};

}

#endif
