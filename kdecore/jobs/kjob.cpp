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

#include "kjobuidelegate.h"

#include <kglobal.h>
#include <QEventLoop>
#include <QMap>
#include <QTimer>
#include <QMetaType>

class KJob::Private
{
public:
    Private(KJob *job) : q(job), uiDelegate(0), error(KJob::NoError),
                         progressUnit(KJob::Bytes), percentage(0),
                         suspended(false), capabilities(KJob::NoCapabilities),
                         speedTimer(0)
    {
        if (!_k_kjobUnitEnumRegistered) {
            _k_kjobUnitEnumRegistered = qRegisterMetaType<KJob::Unit>("KJob::Unit");
        }
    }

    KJob *const q;

    KJobUiDelegate *uiDelegate;
    int error;
    QString errorText;
    KJob::Unit progressUnit;
    QMap<KJob::Unit, qulonglong> processedAmount;
    QMap<KJob::Unit, qulonglong> totalAmount;
    unsigned long percentage;
    bool suspended;
    KJob::Capabilities capabilities;
    QTimer *speedTimer;

    void _k_speedTimeout();

    static bool _k_kjobUnitEnumRegistered;
};

bool KJob::Private::_k_kjobUnitEnumRegistered = false;

KJob::KJob(QObject *parent)
    : QObject(parent), d(new Private(this))
{
    // Don't exit while this job is running
    KGlobal::ref();
}

KJob::~KJob()
{
    delete d->speedTimer;
    delete d->uiDelegate;
    delete d;

    KGlobal::deref();
}

void KJob::setUiDelegate( KJobUiDelegate *delegate )
{
    if ( delegate == 0 || delegate->setJob( this ) )
    {
        delete d->uiDelegate;
        d->uiDelegate = delegate;

        if ( d->uiDelegate )
        {
            d->uiDelegate->connectJob( this );
        }
    }
}

KJobUiDelegate *KJob::uiDelegate() const
{
    return d->uiDelegate;
}

KJob::Capabilities KJob::capabilities() const
{
    return d->capabilities;
}

bool KJob::isSuspended() const
{
    return d->suspended;
}

bool KJob::kill( KillVerbosity verbosity )
{
    if ( doKill() )
    {
        setError( KilledJobError );

        if ( verbosity!=Quietly )
        {
            emitResult();
        }
        else
        {
            // If we are displaying a progress dialog, remove it first.
            emit finished(this);

            deleteLater();
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool KJob::suspend()
{
    if ( !d->suspended )
    {
        if ( doSuspend() )
        {
            d->suspended = true;
            emit suspended(this);

            return true;
        }
    }

    return false;
}

bool KJob::resume()
{
    if ( d->suspended )
    {
        if ( doResume() )
        {
            d->suspended = false;
            emit resumed(this);

            return true;
        }
    }

    return false;
}

bool KJob::doKill()
{
    return false;
}

bool KJob::doSuspend()
{
    return false;
}

bool KJob::doResume()
{
    return false;
}

void KJob::setCapabilities( KJob::Capabilities capabilities )
{
    d->capabilities = capabilities;
}

bool KJob::exec()
{
    QEventLoop loop( this );

    connect( this, SIGNAL( result( KJob* ) ),
             &loop, SLOT( quit() ) );
    start();
    loop.exec();

    return ( d->error == NoError );
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

qulonglong KJob::processedAmount(Unit unit) const
{
    return d->processedAmount[unit];
}

qulonglong KJob::totalAmount(Unit unit) const
{
    return d->totalAmount[unit];
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

void KJob::setProcessedAmount(Unit unit, qulonglong amount)
{
    bool should_emit = (d->processedAmount[unit] != amount);

    d->processedAmount[unit] = amount;

    if ( should_emit )
    {
        emit processedAmount(this, unit, amount);
        if (unit==d->progressUnit) {
            emit processedSize(this, amount);
            emitPercent(d->processedAmount[unit], d->totalAmount[unit]);
        }
    }
}

void KJob::setTotalAmount(Unit unit, qulonglong amount)
{
    bool should_emit = (d->totalAmount[unit] != amount);

    d->totalAmount[unit] = amount;

    if ( should_emit )
    {
        emit totalAmount(this, unit, amount);
        if (unit==d->progressUnit) {
            emit totalSize(this, amount);
            emitPercent(d->processedAmount[unit], d->totalAmount[unit]);
        }
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
    emit finished( this );

    emit result( this );

    deleteLater();
}

void KJob::emitPercent( qulonglong processedAmount, qulonglong totalAmount )
{
    // calculate percents
    unsigned long ipercentage = d->percentage;

    if (totalAmount == 0)
    {
        d->percentage = 100;
    }
    else
    {
        d->percentage = (unsigned long)(( (float)(processedAmount) / (float)(totalAmount) ) * 100.0);
    }

    if ( d->percentage != ipercentage || d->percentage == 100 /* for those buggy total sizes that grow */ )
    {
        emit percent( this, d->percentage );
    }
}

void KJob::emitSpeed(unsigned long value)
{
    if (!d->speedTimer) {
        d->speedTimer = new QTimer(this);
        connect(d->speedTimer, SIGNAL(timeout()), SLOT(_k_speedTimeout()));
    }

    emit speed(this, value);
    d->speedTimer->start(5000);   // 5 seconds interval should be enough
}

void KJob::Private::_k_speedTimeout()
{
    // send 0 and stop the timer
    // timer will be restarted only when we receive another speed event
    emit q->speed(q, 0);
    speedTimer->stop();
}

#include "kjob.moc"
