// -*- c++ -*-
// vim: ts=4 sw=4 et
/*  This file is part of the KDE libraries
    Copyright (C) 2001 Rolf Magnus <ramagnus@kde.org>
    parts of this taken from previewjob.h

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation version 2.0.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    $Id$
*/

#ifndef __kio_metainfojob_h__
#define __kio_metainfojob_h__

#include <kio/job.h>

namespace KIO {
    class MetaInfoJob : public KIO::Job
    {
        Q_OBJECT
    public:
        /**
         * Constructor
         *  @param items   A list of KFileItems to get the metainfo for
         *  @param deleteItems If true, the finished KFileItems are deleted 
         */
        MetaInfoJob(const KFileItemList &items, bool deleteItems = false);
        virtual ~MetaInfoJob();

        /**
         * Removes an item from metainfo extraction.
         *
         * @param item the item that should be removed from the queue
         */
        void removeItem( const KFileItem *item );

        /**
         * @return a list of all available metainfo plugins. The list
         * contains the basenames of the plugins' .desktop files (no path,
         * no .desktop).
         */
        static QStringList availablePlugins();

        /**
         * @return a list of all supported MIME types. The list can
         * contain entries like text/ * (without the space).
         */
        static QStringList supportedMimeTypes();

    signals:
        /**
         * Emitted when the meta info for @p item has been successfully
         * retrieved.
         */
        void gotMetaInfo( const KFileItem *item );
        /**
         * Emitted when metainfo for @p item could not be extracted,
         * either because a plugin for its MIME type does not
         * exist, or because something went wrong.
         */
        void failed( const KFileItem *item );

    protected:
        void getMetaInfo();

    protected slots:
        virtual void slotResult( KIO::Job *job );

    private slots:
        void start();
        void slotMetaInfo(KIO::Job *, const QByteArray &);

    private:
        void determineNextFile();
//        void saveMetaInfo(const QByteArray info);

    private:
        struct MetaInfoJobPrivate *d;
    };

    /**
     * Retrieves meta information for the given items.
     *
     * @param items files to get metainfo for
     * @param save if the data should be cached for later use
     * @param enabledPlugins if non-zero, this points to a list containing
     * the names of the plugins that may be used.
     * see @ref availablePlugins.
     */
    MetaInfoJob* fileMetaInfo(const KFileItemList& items);

    /**
     * Same as above, but takes a URL list instead of KFileItemList
     */
    MetaInfoJob* fileMetaInfo(const KURL::List& items);
};

#endif
