// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998,1999,2000 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
		  1999,2000 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef KFILEREADER_H
#define KFILEREADER_H

#include <qstring.h>

#include <kdirlister.h>
#include <kurl.h>

class KFileReader : public KDirLister
{
    Q_OBJECT

public:
    /**
     * Sets the current working directory as URL, but does not start loading
     * it.
     */
    KFileReader();

    /**
     * Only sets the URL, does not start loading it.
     */
    KFileReader(const KURL& url, const QString& nameFilter= QString::null);
    ~KFileReader();

    /**
     * Indicates if the path is empty
     */
    bool isRoot() const { return url().path() == QChar('/'); }

    /**
     * Indicates if the path is readable. That means, if there are
     * entries to expect
     */
    bool isReadable() const;

protected:
  /**
   * called from KDirLister to create a KFileViewItem
   */
  virtual KFileItem * createFileItem( const KIO::UDSEntry&, const KURL&, bool );

private:
    void init();

    KFileReader(const KFileReader&);
    KFileReader& operator=(const KFileReader&);

private:
    class KFileReaderPrivate;
    KFileReaderPrivate *d;

};

#endif // KFILEREADER_H
