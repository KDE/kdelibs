// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __kio_chmodjob_h__
#define __kio_chmodjob_h__

#include <kurl.h>
#include <qstring.h>

#include <kio/global.h>
#include <kfileitem.h>

/*

 NOTE : This job is currently in ksycoca because kfileitem needs to know
 about mimetypes. For KDE 3.0, make a base class for KFileItem, that holds
 all the information except the mimetype, so that jobs can use it.

*/

namespace KIO {

    struct ChmodInfo;

    /**
     * This job changes permissions on a list of files or directories,
     * optionally in a recursive manner.
     * @see KIO::chmod()
     */
    class ChmodJob : public KIO::Job
    {
        Q_OBJECT
    public:
	/**
	 * Create new ChmodJobs using the @ref KIO::chmod() function.
	 */
        ChmodJob( const KFileItemList & lstItems,  int permissions, int mask,
                  int newOwner, int newGroup,
                  bool recursive, bool showProgressInfo );

    protected:
        void chmodNextFile();

    protected slots:

        virtual void slotResult( KIO::Job *job );
        void slotEntries( KIO::Job * , const KIO::UDSEntryList & );
        void processList();

    private:
        enum { STATE_LISTING, STATE_CHMODING } state;
        int m_permissions;
        int m_mask;
        int m_newOwner;
        int m_newGroup;
        bool m_recursive;
        KFileItemList m_lstItems;
        QValueList<ChmodInfo> m_infos;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	class ChmodJobPrivate* d;
    };


    /**
     * Creates a job that changes permissions/ownership on several files or directories,
     * optionally recursively.
     * This version of chmod uses a KFileItemList so that it directly knows
     * what to do with the items. TODO: a version that takes a KURL::List,
     * and a general job that stats each url and returns a KFileItemList.
     *
     * Note that change of ownership is only supported for local files.
     *
     * Inside directories, the "x" bits will only be changed for files that had
     * at least one "x" bit before, and for directories.
     * This emulates the behaviour of chmod +X.
     *
     * @param lstItems The file items representing several files or directories.
     * @param permissions the permissions we want to set
     * @param mask the bits we are allowed to change.
     * For instance, if mask is 0077, we don't change
     * the "user" bits, only "group" and "others".
     * @param newOwner If non-empty, the new owner for the files
     * @param newGroup If non-empty, the new group for the files
     * @param recursive whether to open directories recursively
     * @param showProgressInfo true to show progess information
     * @return The job handling the operation.
     */
    ChmodJob * chmod( const KFileItemList& lstItems, int permissions, int mask,
                      QString newOwner, QString newGroup,
                      bool recursive, bool showProgressInfo = true );

};

#endif
