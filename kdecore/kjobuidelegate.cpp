/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kjobuidelegate.h"
#include "kjob.h"

#include <kdebug.h>

struct KJobUiDelegate::Private
{
    Private() : job( 0 ), autoErrorHandling( false ),
                autoWarningHandling( true ) { }
    KJob *job;
    bool autoErrorHandling;
    bool autoWarningHandling;
};

KJobUiDelegate::KJobUiDelegate()
    : QObject(), d( new Private() )
{
}

KJobUiDelegate::~KJobUiDelegate()
{
    delete d;
}

bool KJobUiDelegate::setJob( KJob *job )
{
    if ( d->job!=0 )
    {
        return false;
    }

    d->job = job;
    setParent( job );
    connectJob( job );

    return true;
}

KJob *KJobUiDelegate::job()
{
    return d->job;
}

void KJobUiDelegate::showErrorMessage()
{
}

void KJobUiDelegate::setAutoErrorHandlingEnabled( bool enable)
{
    d->autoErrorHandling = enable;
}

bool KJobUiDelegate::isAutoErrorHandlingEnabled() const
{
    return d->autoErrorHandling;
}

void KJobUiDelegate::setAutoWarningHandlingEnabled( bool enable )
{
    d->autoWarningHandling = enable;
}

bool KJobUiDelegate::isAutoWarningHandlingEnabled() const
{
    return d->autoWarningHandling;
}

void KJobUiDelegate::connectJob( KJob *job )
{
    connect( d->job, SIGNAL( finished( KJob*, int ) ),
             this, SLOT( slotFinished( KJob*, int ) ) );

    connect( job, SIGNAL( warning( KJob*, const QString& ) ),
             this, SLOT( slotWarning( KJob*, const QString& ) ) );
}

void KJobUiDelegate::slotFinished( KJob */*job*/, int /*id*/ )
{
    if ( d->job->error() && d->autoErrorHandling )
        showErrorMessage();
}

void KJobUiDelegate::slotWarning( KJob */*job*/, const QString &/*errorText*/ )
{

}

#include "kjobuidelegate.moc"
