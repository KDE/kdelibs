/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2008 John Layt <john@layt.net>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kcupsoptionswidget_p.h"

#include <fixx11h.h>
#include <QPrinter>
#include <QPrintEngine>
#include <QPrintDialog>
#include <QFile>
#include <QtNetwork/QTcpSocket>

#include <kdebug.h>

/** @internal */
KCupsOptionsWidget::KCupsOptionsWidget( QPrintDialog *parent ) : QWidget( parent )
{
    m_dialog = parent;

    // When user accepts the dialog, then set up the QPrinter with the CUPS options
    connect( m_dialog, SIGNAL(accepted()), this, SLOT(setupPrinter()) );
}

KCupsOptionsWidget::~KCupsOptionsWidget()
{
}

bool KCupsOptionsWidget::cupsAvailable()
{
#ifdef Q_WS_X11
    // Ideally we would have access to the private Qt method
    // QCUPSSupport::cupsAvailable() to do this as it is very complex routine.
    // However, if CUPS is available then QPrinter::numCopies() will always return 1
    // whereas if CUPS is not available it will return the real number of copies.
    // This behaviour is guaranteed never to change, so we can use it as a reliable substitute.
    QPrinter testPrinter;
    testPrinter.setNumCopies( 2 );
    return ( testPrinter.numCopies() == 1 );
#else
    return false;
#endif
}

void KCupsOptionsWidget::setupPrinter()
{
    if ( cupsAvailable() ) {
        QStringList cupsOptions = m_dialog->printer()->printEngine()->property(QPrintEngine::PrintEnginePropertyKey(0xfe00)).toStringList();

        setupCupsOptions( cupsOptions );

        m_dialog->printer()->printEngine()->setProperty(QPrintEngine::PrintEnginePropertyKey(0xfe00), QVariant(cupsOptions));
    }
}

void KCupsOptionsWidget::setupCupsOptions( QStringList &cupsOptions )
{
    Q_UNUSED(cupsOptions);
}

void KCupsOptionsWidget::setCupsOption( QStringList &cupsOptions, const QString option, const QString value )
{
    if ( cupsOptions.contains( option ) ) {
        cupsOptions.replace( cupsOptions.indexOf( option ) + 1, value );
    } else {
        cupsOptions.append( option );
        cupsOptions.append( value );
    }
}

