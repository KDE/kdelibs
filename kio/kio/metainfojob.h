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
    /**
     * MetaInfoJob is a KIO Job to retrieve meta information from files.
     * 
     * @short KIO Job to retrieve meta information from files.
     * @since 3.1
     */
    class MetaInfoJob : public KIO::Job
    {
        Q_OBJECT
    public:
        /**
         * Creates a new MetaInfoJob.
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
         * Returns a list of all available metainfo plugins. The list
         * contains the basenames of the plugins' .desktop files (no path,
         * no .desktop).
	 * @return the list of available meta info plugins
         */
        static QStringList availablePlugins();

        /**
         * Returns a list of all supported MIME types. The list can
         * contain entries like text/ * (without the space).
	 * @return the list of MIME types that are supported
         */
        static QStringList supportedMimeTypes();

    signals:
        /**
         * Emitted when the meta info for @p item has been successfully
         * retrieved.
	 * @param item the KFileItem describing the fetched item
         */
        void gotMetaInfo( const KFileItem *item );
        /**
         * Emitted when metainfo for @p item could not be extracted,
         * either because a plugin for its MIME type does not
         * exist, or because something went wrong.
	 * @param item the KFileItem of the file that failed
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
     * @return the MetaInfoJob to retrieve the items
     */
    MetaInfoJob* fileMetaInfo(const KFileItemList& items);

    /**
     * Retrieves meta information for the given items.
     *
     * @param items files to get metainfo for
     * @return the MetaInfoJob to retrieve the items
     */
    MetaInfoJob* fileMetaInfo(const KURL::List& items);
};

#endif
