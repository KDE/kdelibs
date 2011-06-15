/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef COLLECTIONJOB_H
#define COLLECTIONJOB_H

#include "ksecretsservicecollection.h"
#include "ksecretsservicecollection_p.h"

#include <kcompositejob.h>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

namespace KSecretsService {
    
    
/**
 * Collection handling related jobs all inherit from this class.
 * It's main purpose it to get 'lazy connect' feature where a collection
 * object is firstly instantiated but not yet connected to the back-end waiting
 * for it's first real use, indicated by some method call. All these methods
 * calls would return a CollectionJob inheritor
 * 
 * @note this class is for internal use only and should not be used by client applications
 */
class CollectionJob : public KCompositeJob {
    Q_OBJECT
public:
    explicit CollectionJob(Collection *collection, QObject* parent = 0);
    
protected:
    CollectionPrivate *cd;
};

class DeleteCollectionJob : public CollectionJob {
public:
    explicit DeleteCollectionJob( Collection* collection, QObject* parent =0 ) :
        CollectionJob( collection, parent) {
    }
    
    virtual void start() {
        // the job is already started by now, as we're handling a dbus call
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( deleteReply, this );
        connect( watcher, SIGNAL( finished(QDBusPendingCallWatcher*) ), this, SLOT( callFinished(QDBusPendingCallWatcher*) ) );
    }
    
    void callFinished( QDBusPendingCallWatcher* /* watcher */ ) {
        Q_ASSERT( deleteReply.isFinished() );
        if ( deleteReply.isError() ) {
            
        }
        else {
        }
    }
    
    QDBusPendingReply<QDBusObjectPath> deleteReply;
};

class FindCollectionJob : public CollectionJob {
    Q_OBJECT
    Q_DISABLE_COPY(FindCollectionJob)
public:
    FindCollectionJob( Collection *collection, 
                       const QString& collName,
                       Collection::FindCollectionOptions options,
                       QObject *parent =0 ) : 
            CollectionJob( collection, parent ),
            collectionName( collName ),
            findCollectionOptions( options ) {
    }
    virtual void start() {
        // meanwhile another findJob instance would have already connected our collection object
        if ( ! cd->isValid() ) {
            // TODO: implement by checking that the collection is already connected
        }
        else {
            setError( 0 );
            emitResult();
        }
    }
    
private:
    QString                                              collectionName;
    KSecretsService::Collection::FindCollectionOptions   findCollectionOptions;
};

class Collection::SearchItemsJob : public CollectionJob {
    Q_OBJECT
    Q_DISABLE_COPY(SearchItemsJob)
public:
    explicit SearchItemsJob( Collection* collection,  QObject *parent =0 );
    
    QList< SecretItem > &items() const;
    
private:
    class Private;
    Private *d;
};

class Collection::SearchSecretsJob : public CollectionJob {
    Q_OBJECT
    Q_DISABLE_COPY(SearchSecretsJob)
public:
    explicit SearchSecretsJob( Collection* collection, QObject* parent =0 );
    
    QList< Secret >  secrets() const;
    
private:
    class Private;
    Private *d;
};

class Collection::CreateItemJob : public CollectionJob {
    Q_OBJECT
    Q_DISABLE_COPY(CreateItemJob)
public:
    explicit CreateItemJob( Collection* collection,  QObject *parent =0 );
    
    SecretItem item() const;
    
private:
    class Private;
    Private *d;
};

class Collection::ReadItemsJob : public CollectionJob {
    Q_OBJECT
    Q_DISABLE_COPY(ReadItemsJob)
public:
    explicit ReadItemsJob( Collection* collection,  QObject *parent =0 );
    
    QList< SecretItem > items() const;
    
private:
    class Private;
    Private *d;
};

class Collection::SearchItemsJob::Private {
};

class Collection::SearchSecretsJob::Private {
};

class Collection::ReadItemsJob::Private {
};

class Collection::CreateItemJob::Private {
};

}

#endif // COLLECTIONJOB_H
