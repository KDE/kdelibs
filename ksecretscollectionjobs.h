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

#ifndef KSECRETSCOLLECTIONJOBS_H
#define KSECRETSCOLLECTIONJOBS_H

#include "ksecretsservicecollection.h"
#include "ksecretsserviceitem.h"

#include <kcompositejob.h>
#include <qsharedpointer.h>

class DeleteCollectionJobPrivate;
class FindCollectionJobPrivate;
class CollectionJobPrivate;
class RenameCollectionJobPrivate;
class SearchItemsJobPrivate;
class CreateItemJobPrivate;

namespace KSecretsService {
    
class Collection;
typedef QMap< QString, QString > QStringStringMap;

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
    explicit CollectionJob( Collection *collection, QObject* parent = 0 );
    
    enum CollectionError {
        UndefinedError =-1,             /// this error should never be encountered
        NoError =0,
        InternalError,
        OperationCancelledByTheUser,    /// the user choose to cancel ther operation during a message prompt
        CollectionNotFound,
        CreateError,
        DeleteError,
        RenameError
    };

    /**
     * Get a pointer to the collection wich started this job
     */
    Collection *collection() const;
    
    /**
     * This override is intended to make it public and as such let subjob additions from other classes in this client implementation
     */
    virtual bool addSubjob( KJob* ); // override
    
protected:
    void finishedWithError( CollectionError err, const QString &errTxt );
    void finishedOk();
    virtual void startFindCollection();
    virtual void slotResult( KJob* job ); /// override  of the KCompositeJob::slotResult 
    virtual void onFindCollectionFinished();

protected:
    friend class ::FindCollectionJobPrivate;
    friend class ::DeleteCollectionJobPrivate;
    CollectionJobPrivate *d;
};

class DeleteCollectionJob : public CollectionJob {
    Q_OBJECT
    Q_DISABLE_COPY(DeleteCollectionJob)
public:
    explicit DeleteCollectionJob( Collection* collection, QObject* parent =0 );
    
    virtual void start();

protected Q_SLOTS:
    virtual void onFindCollectionFinished();
    void deleteIsDone( CollectionJob::CollectionError error, const QString &errorString );
    
private:
    QSharedPointer< DeleteCollectionJobPrivate > d;
};

class FindCollectionJob : public CollectionJob {
    Q_OBJECT
    Q_DISABLE_COPY(FindCollectionJob)
public:
    explicit FindCollectionJob( Collection *collection, QObject *parent =0 );
    
    virtual void start();
private:
    friend class ::FindCollectionJobPrivate;
    QSharedPointer< FindCollectionJobPrivate > d;
};

class RenameCollectionJob : public CollectionJob {
    Q_OBJECT
    Q_DISABLE_COPY(RenameCollectionJob)
public:
    RenameCollectionJob( Collection *coll, const QString& newName, QObject *parent =0 );
    
    virtual void start();
    
protected Q_SLOTS:
    virtual void onFindCollectionFinished();
    void renameIsDone( CollectionJob::CollectionError, const QString& );
    
private:
    friend class ::RenameCollectionJobPrivate;
    QSharedPointer< RenameCollectionJobPrivate > d;
};

class SearchItemsJob : public CollectionJob {
    Q_OBJECT
    Q_DISABLE_COPY(SearchItemsJob)
public:
    explicit SearchItemsJob( Collection* collection, const QStringStringMap &attributes, QObject *parent =0 );
    
    QList< SecretItem > &items() const;
    virtual void start();

private:
    friend class ::SearchItemsJobPrivate;
    QSharedPointer<SearchItemsJobPrivate> d;
};

class SearchSecretsJob : public CollectionJob {
    Q_OBJECT
    Q_DISABLE_COPY(SearchSecretsJob)
public:
    explicit SearchSecretsJob( Collection* collection, QObject* parent =0 );
    
    QList< Secret >  secrets() const;
    
private:
    class Private;
    Private *d;
};

class CreateItemJob : public CollectionJob {
    Q_OBJECT
    Q_DISABLE_COPY(CreateItemJob)
public:
    explicit CreateItemJob( Collection* collection,  const QMap< QString, QString >& attributes, const Secret& secret, bool replace );
    
    virtual void start();
    SecretItem item() const;
    
protected Q_SLOTS:
    virtual void onFindCollectionFinished();
    void createIsDone( CollectionJob::CollectionError, const QString& );
    
private:
    friend class ::CreateItemJobPrivate;
    QSharedPointer< CreateItemJobPrivate > d;
};

class ReadItemsJob : public CollectionJob {
    Q_OBJECT
    Q_DISABLE_COPY(ReadItemsJob)
public:
    explicit ReadItemsJob( Collection* collection,  QObject *parent =0 );
    
    QList< SecretItem > items() const;
    
private:
    class Private;
    Private *d;
};

class SearchSecretsJob::Private {
};

class ReadItemsJob::Private {
};

}

#endif // KSECRETSCOLLECTIONJOBS_H
