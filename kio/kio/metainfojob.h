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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KIO_METAINFOJOB_H
#define KIO_METAINFOJOB_H

#include <kio/job.h>
#include <kfileitem.h>

namespace KIO {
    class MetaInfoJobPrivate;
    /**
     * MetaInfoJob is a KIO Job to retrieve meta information from files.
     *
     * @short KIO Job to retrieve meta information from files.
     */
    class KIO_EXPORT MetaInfoJob : public KIO::Job
    {
        Q_OBJECT
    public:
        /**
         * Creates a new MetaInfoJob.
         *  @param items   A list of KFileItems to get the metainfo for
         *  @param w       Flags which serve as a preset which can be customized
         *                 with other parameters.
         *  @param iocost  The allowed cost in terms of io to retrieve the
         *                 metainfo. The approximate maximum number of bytes to
         *                 be read is 10^iocost. Negative values mean that
         *                 there is no limit on the cost. 0 means that no fields
         *                 other than the required fields will be retrieved.
         *                 The default value of 3 means about 1024 bytes per
         *                 file may be read. This is merely a suggestion and not
         *                 a hard limit.
         *  @param cpucost The allowed cost in terms of cpu to determine the
         *                 information in the fields. The number mean the amount
         *                 of instructions allowed is 10^cpucost and is a suggestion only.
         *                 The default value of 6 means that about a million
         *                 instructions (10^6) are allowed. This is useful for
         *                 expensive fields like md5 or thumbnails.
         *  @param requiredfields The names of fields or groups of fields that should
         *                  be retrieved regardless of cost.
         *  @param requestedfields The names of fields or groups of fields that should
         *                  be retrieved first.
         */
        explicit MetaInfoJob(const KFileItemList& items, KFileMetaInfo::WhatFlags w = KFileMetaInfo::Everything,
            int iocost = 3, int cpucost = 6, const QStringList& requiredfields = QStringList(),
            const QStringList& requestedfields = QStringList());
        virtual ~MetaInfoJob();

        /**
         * Removes an item from metainfo extraction.
         *
         * @param item the item that should be removed from the queue
         */
        void removeItem( const KFileItem& item );

    Q_SIGNALS:
        /**
         * Emitted when the meta info for @p item has been successfully
         * retrieved.
	 * @param item the KFileItem describing the fetched item
         */
        void gotMetaInfo( const KFileItem& item );
        /**
         * Emitted when metainfo for @p item could not be extracted,
         * either because a plugin for its MIME type does not
         * exist, or because something went wrong.
	 * @param item the KFileItem of the file that failed
         */
        void failed( const KFileItem& item );

    protected:
        void getMetaInfo();

    protected Q_SLOTS:
        virtual void slotResult( KJob *job );

    private Q_SLOTS:
        void start();
        void slotMetaInfo(KIO::Job *, const QByteArray &);

    private:
        void determineNextFile();
//        void saveMetaInfo(const QByteArray info);

    private:
        Q_DECLARE_PRIVATE(MetaInfoJob)
    };

    /**
     * Retrieves meta information for the given items.
     *
     * @param items files to get metainfo for
     * @return the MetaInfoJob to retrieve the items
     */
    KIO_EXPORT MetaInfoJob* fileMetaInfo(const KFileItemList& items);

    /**
     * Retrieves meta information for the given items.
     *
     * @param items files to get metainfo for
     * @return the MetaInfoJob to retrieve the items
     */
    KIO_EXPORT MetaInfoJob* fileMetaInfo(const KUrl::List& items);
}

#endif
