/* This file is part of the KDE libraries

   Copyright (c) 2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
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

#ifndef KFILEFILTER_H
#define KFILEFILTER_H

#include <qptrlist.h>
#include <qstringlist.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

class QRegExp;
class KFileItem;

class KFileFilter
{
public:
    virtual bool passesFilter( const KFileItem *item ) const = 0;
protected:
    virtual void virtual_hook( int id, void* data );
};


class KSimpleFileFilter : public KFileFilter
{
public:
    KSimpleFileFilter();
    virtual ~KSimpleFileFilter();

    virtual void setFilterDotFiles( bool filter );
    bool filterDotFiles() const { return m_filterDotFiles; }

    /**
     * "." and "..", default is true.
     */
    virtual void setFilterSpecials( bool filter );
    bool filterSpecials() const { return m_filterSpecials; }

    // ### KDE4 make virtual and bool caseSensitive = false
    void setNameFilters( const QString& nameFilters, bool caseSensitive,
                         const QChar& separator = ' ' );
    virtual void setNameFilters( const QString& nameFilters );
    QString nameFilters() const;

    virtual void setMimeFilters( const QStringList& mimeFilters );
    QStringList mimeFilters() const { return m_mimeFilters; }

    virtual void setModeFilter( mode_t mode );
    mode_t modeFilter() const { return m_modeFilter; }

    virtual bool passesFilter( const KFileItem *item ) const;

protected:
    QPtrList<QRegExp>   m_nameFilters;

private:
    QStringList         m_mimeFilters;
    bool                m_filterDotFiles :1;
    bool                m_filterSpecials :1;
    mode_t              m_modeFilter;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KSimpleFileFilterPrivate* d;
};

#endif // KFILEFILTER_H
