/*  This file is part of the KDE project
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "kjob.h"

#include <kapplication.h>

class KJob::Private
{
public:
    Private() : progressId( 0 ), error( 0 ),
                processedSize( 0 ), totalSize( 0 ), percentage( 0 ) {}

    int progressId;
    int error;
    QString errorText;
    qulonglong processedSize;
    qulonglong totalSize;
    unsigned long percentage;
};

KJob::KJob( QObject *parent )
    : QObject( parent ), d( new Private() )
{
    // Don't exit while this job is running
    if (kapp)
        kapp->ref();
}

KJob::~KJob()
{
    delete d;

    if (kapp)
        kapp->deref();
}

int KJob::error() const
{
    return d->error;
}

QString KJob::errorText() const
{
    return d->errorText;
}

QString KJob::errorString() const
{
    return d->errorText;
}

void KJob::setProgressId( int id )
{
    d->progressId = id;
}

int KJob::progressId() const
{
    return d->progressId;
}

qulonglong KJob::processedSize() const
{
    return d->processedSize;
}

qulonglong KJob::totalSize() const
{
    return d->totalSize;
}

unsigned long KJob::percent() const
{
    return d->percentage;
}

void KJob::setError( int errorCode )
{
    d->error = errorCode;
}

void KJob::setErrorText( const QString &errorText )
{
    d->errorText = errorText;
}

void KJob::setProcessedSize( qulonglong size )
{
    bool should_emit = ( d->processedSize != size );

    d->processedSize = size;

    if ( should_emit )
    {
        emit processedSize( this, size );
        emitPercent( d->processedSize, d->totalSize );
    }
}

void KJob::setTotalSize( qulonglong size )
{
    bool should_emit = ( d->totalSize != size );

    d->totalSize = size;

    if ( should_emit )
    {
        emit totalSize( this, size );
        emitPercent( d->processedSize, d->totalSize );
    }
}

void KJob::setPercent( unsigned long percentage )
{
    if ( d->percentage!=percentage )
    {
        d->percentage = percentage;
        emit percent( this, percentage );
    }
}

void KJob::emitResult()
{
    // If we are displaying a progress dialog, remove it first.
    if ( d->progressId )
    {
        emit finished( this, d->progressId );
    }

    emit result( this );

    deleteLater();
}

void KJob::emitPercent( qulonglong processedSize, qulonglong totalSize )
{
    // calculate percents
    unsigned long ipercentage = d->percentage;

    if ( totalSize == 0 )
    {
        d->percentage = 100;
    }
    else
    {
        d->percentage = (unsigned long)(( (float)(processedSize) / (float)(totalSize) ) * 100.0);
    }

    if ( d->percentage != ipercentage || d->percentage == 100 /* for those buggy total sizes that grow */ )
    {
        emit percent( this, d->percentage );
    }
}

#include "kjob.moc"
