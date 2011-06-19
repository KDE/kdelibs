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

#include <kcompositejob.h>
#include <qsharedpointer.h>

class DeleteCollectionJobPrivate;
class FindCollectionJobPrivate;

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
    Q_DISABLE_COPY(CollectionJob)
public:
    explicit CollectionJob( Collection *collection, QObject* parent = 0, bool shouldTriggerFind = true );
    
    enum CollectionError {
        UndefinedError =-1, /// this error should never be encountered
        NoError =0,
        InternalError,
        CollectionNotFound,
        CreateError,
        DeleteError
    };

    /**
     * Returns the CollectionError corresponding to the outcome of the job
     * @see KCompositJob::errorString()
     */
    CollectionError error() const { return _error; }
    
protected:
    virtual void startFindCollection();
    virtual void slotResult( KJob* job ); /// override  of the KCompositeJob::slotResult 
    virtual void onFindCollectionFinished();
    
protected:
    Collection      *collection;
    CollectionError _error;
};

class DeleteCollectionJob : public CollectionJob {
    Q_OBJECT
    Q_DISABLE_COPY(DeleteCollectionJob)
public:
    explicit DeleteCollectionJob( Collection* collection, QObject* parent =0 );
    
    virtual void start();

protected Q_SLOTS:
    virtual void onFindCollectionFinished();
    void deleteIsDone( CollectionError error, const QString &errorString );
    
private:
    QSharedPointer< DeleteCollectionJobPrivate > d;
};

class FindCollectionJob : public CollectionJob {
    Q_OBJECT
    Q_DISABLE_COPY(FindCollectionJob)
public:
    FindCollectionJob( Collection *collection, 
                       const QString& collName,
                       Collection::FindCollectionOptions options,
                       QObject *parent =0 );
    
    virtual void start();
    
private:
    QSharedPointer< FindCollectionJobPrivate > d;
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
