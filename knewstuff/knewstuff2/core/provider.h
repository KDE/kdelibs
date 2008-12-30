/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef KNEWSTUFF2_PROVIDER_H
#define KNEWSTUFF2_PROVIDER_H

#include <knewstuff2/core/feed.h>

#include <kurl.h>

#include <QtXml/qdom.h>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>

namespace KIO
{
class Job;
}

namespace KNS
{

/**
 * @short KNewStuff provider container.
 *
 * This class provides accessors for the provider object.
 * It should probably not be used directly by the application.
 *
 * @author Cornelius Schumacher (schumacher@kde.org)
 * \par Maintainer:
 * Josef Spillner (spillner@kde.org)
 *
 * @internal
 */
class KNEWSTUFF_EXPORT Provider
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
    void setName(const KTranslatable& name);

    /**
     * Retrieves the common name of the provider.
     *
     * @return provider name
     */
    KTranslatable name() const;

    /**
     * Adds a feed URL.
     *
     * Each feed is assigned an internal identifier string.
     * The empty string represents the default feed.
     *
     * @param feedtype String representing the feed type
     * @param feed Feed to add to this provider
     */
    void addDownloadUrlFeed(const QString& feedtype, Feed *feed);

    /**
     * Feed to retrieve for the given feed type.
     *
     * The feed can be one of 'score', 'downloads', 'latest'.
     * It can also be the empty string for the default feed.
     * For all feeds not present here, \b null is returned.
     *
     * @return download feed of a certain feed type
     */
    Feed *downloadUrlFeed(const QString& feedtype) const;

    /**
     * Returns a list of all feeds.
     *
     * @return List of identifier names for the feeds of this provider
     */
    QStringList feeds() const;

    /**
     * Sets the upload URL.
     */
    void setUploadUrl(const KUrl &);

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
    void setNoUploadUrl(const KUrl &);

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
    void setWebAccess(const KUrl &);

    /**
     * Retrieves the web frontend URL.
     *
     * @return web frontend URL
     */
    KUrl webAccess() const;

    /**
     * Sets the URL of the DXS, if offered by the provider.
     */
    void setWebService(const KUrl &);

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
    void setIcon(const KUrl &);

    /**
     * Retrieves the icon URL for this provider.
     *
     * @return icon URL
     */
    KUrl icon() const;

private:
    KTranslatable mName;
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
