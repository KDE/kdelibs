/* This file is part of the KDE project
   Copyright (C) 2001 Carsten Pfeiffer <pfeiffer@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KHISTORYPROVIDER_H
#define KHISTORYPROVIDER_H

#include <qobject.h>

namespace KParts {

/**
 * Basic class to manage a history of "items". This class is only meant
 * for fast lookup, if an item is in the history or not.
 *
 * May be subclassed to implement a persistent history for example.
 * For usage with khtml, just create your subclassed object and call the
 * HistoryProvider constructor _before_ you do any khtml stuff. That way,
 * khtml, using the self()-method, will use your subclassed provider.
 */
class HistoryProvider : public QObject
{
    Q_OBJECT

public:
    static HistoryProvider * self();

    /**
     * Creates a KHistoryProvider with an optional parent and name
     */
    HistoryProvider( QObject *parent = 0L, const char *name = 0 );

    /**
     * Destroys the provider.
     */
    virtual ~HistoryProvider();

    /**
     * @returns true if @p item is present in the history.
     */
    virtual bool contains( const QString& item ) const;

    /**
     * Inserts @p item into the history.
     */
    virtual void insert( const QString& item );

    /**
     * Removes @p item from the history.
     */
    virtual void remove( const QString& item );

    /**
     * Clears the history.
     */
    virtual void clear();

signals:
    /**
     * Emitted after a new entry has been inserted.
     */
    void inserted( const QString& );

    /**
     * Emitted after an entry has been removed.
     */
    void removed( const QString& );
    
    /**
     * Emitted after the history has been cleared.
     */
    void cleared();
    
private:
    static HistoryProvider *s_self;

    class HistoryProviderPrivate;
    HistoryProviderPrivate *d;
};

};

#endif // KHISTORYPROVIDER_H
