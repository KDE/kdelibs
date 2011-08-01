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
class SearchSecretsJobPrivate;
class ReadItemsJobPrivate;
class ReadCollectionPropertyJobPrivate;
class WriteCollectionPropertyJobPrivate;

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

    // TODO: rename this to CollectionJobError
    enum CollectionError {
        UndefinedError =-1,             /// this error should never be encountered
        NoError =0,
        InternalError,
        OperationCancelledByTheUser,    /// the user choose to cancel ther operation during a message prompt
        CollectionNotFound,
        CreateError,
        DeleteError,
        RenameError,
        MissingParameterError
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
    /**
     * Request job abort on error
     * @param err is the error code
     * @param errTxt is the internationalized error message user might see
     */
    void finishedWithError( CollectionError err, const QString &errTxt );
    
    /**
     * Request job finish without error condition
     */
    void finishedOk();
    
    /**
     * Start the actual collection search. Current implementation would trigger DBus connection to the daemon
     */
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

protected:
    virtual void onFindCollectionFinished();
    
protected Q_SLOTS:
    void renameIsDone( CollectionJob::CollectionError, const QString& );
    
private:
    friend class ::RenameCollectionJobPrivate;
    QSharedPointer< RenameCollectionJobPrivate > d;
};

class SearchItemsJob : public CollectionJob {
public:
    explicit SearchItemsJob( Collection* collection, const QStringStringMap &attributes, QObject *parent =0 );
    
    QList< SecretItem > items() const;
    virtual void start();

protected:
    virtual void onFindCollectionFinished();

private:
    friend class ::SearchItemsJobPrivate;
    QSharedPointer<SearchItemsJobPrivate> d;
};

class SearchSecretsJob : public CollectionJob {
    Q_OBJECT
    Q_DISABLE_COPY(SearchSecretsJob)
public:
    explicit SearchSecretsJob( Collection* collection, const QStringStringMap &attributes, QObject* parent =0 );
    
    virtual void start();
    QList< Secret >  secrets() const;
    
protected Q_SLOTS:
    virtual void onFindCollectionFinished();
    void searchIsDone( CollectionJob::CollectionError, const QString& );
    
private:
    friend class ::SearchSecretsJobPrivate;
    QSharedPointer<SearchSecretsJobPrivate> d;
};

class CreateItemJob : public CollectionJob {
public:
    explicit CreateItemJob( Collection* collection, const QString& label, const QMap< QString, QString >& attributes, const Secret& secret, bool replace );
    
    virtual void start();
    SecretItem * item() const;
    
    virtual void onFindCollectionFinished();
    
private:
    friend class ::CreateItemJobPrivate;
    QSharedPointer< CreateItemJobPrivate > d;
};

class ReadItemsJob : public CollectionJob {
public:
    explicit ReadItemsJob( Collection* collection,  QObject *parent =0 );

    virtual void start();
    QList< SecretItem > items() const;
    
private:
    friend class ::ReadItemsJobPrivate;
    QSharedPointer< ReadItemsJobPrivate > d;
};

class ReadCollectionPropertyJob : public CollectionJob {
    explicit ReadCollectionPropertyJob( Collection *collection, const char *propName, QObject *parent =0 );
    explicit ReadCollectionPropertyJob( Collection *collection, void (Collection::*propReadMember)( ReadCollectionPropertyJob* ), QObject *parent =0 );
    friend class Collection; // only Collection class can instantiated us
public:
    
    virtual void start();
    virtual void onFindCollectionFinished();
    const QVariant& propertyValue() const;
    
private:
    friend class ::ReadCollectionPropertyJobPrivate;
    QSharedPointer< ReadCollectionPropertyJobPrivate > d;
    void (Collection::*propertyReadMember)( ReadCollectionPropertyJob* );
};

class WriteCollectionPropertyJob : public CollectionJob {
public:
    explicit WriteCollectionPropertyJob( Collection *collection, const char *propName, const QVariant& value, QObject *parent =0 );
    
    virtual void start();
    virtual void onFindCollectionFinished();
    
private:
    friend class ::WriteCollectionPropertyJobPrivate;
    QSharedPointer< WriteCollectionPropertyJobPrivate > d;
};

    
}

#endif // KSECRETSCOLLECTIONJOBS_H
