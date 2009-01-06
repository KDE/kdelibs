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

#include <kdebug.h>

/** @internal */
KCupsOptionsWidget::KCupsOptionsWidget( QPrintDialog *parent ) : QWidget( parent )
{
    m_dialog = parent;

    // When user accepts the dialog, then set up the QPrinter with the CUPS options
    connect( m_dialog, SIGNAL(accepted()), this, SLOT(setupPrinter()) );

    // When the user changes the selected Printer in the dialog, we need to check
    // if a CUPS printer is still selected.  Dunno if this will actually work :-)
    //Laurent:2008-07-05 disable it. This signal doesn't exist
    //connect( m_dialog, SIGNAL(currentIndexChanged(int)), this, SLOT(toggleEnabled()) );
}

KCupsOptionsWidget::~KCupsOptionsWidget()
{
}

bool KCupsOptionsWidget::cupsAvailable()
{
    // Ideally we would have access to the private Qt method
    // QCUPSSupport::cupsAvailable() to do this as it is very complex,
    // I have requested this for Qt4.5 through the KPrinterInfo class, but
    // for now we have to fake it ourselves.

    /* This should work but doesn't, can anyone figure this out?
    // In Qt4.4, if the printer is a CUPS printer, then the widget cupsPropertiesPage
    // is enabled, if it's an lpr printer or save to file is selected then it is
    // disabled.
    QWidget *cupsProperties = m_dialog->findChild<QWidget*>("cupsPropertiesPage");
    return ( cupsProperties && cupsProperties->isEnabled() );
    */

    // If the user has CUPS installed, assume the printer is CUPS, but this
    // may not be true as Qt may have fallen back to lpr if there are problems
    // with CUPS, or the user may have selected Print To File.
    if ( QFile::exists("/etc/cups/cupsd.conf") ) return true;
    if ( QFile::exists("/usr/etc/cups/cupsd.conf") ) return true;
    if ( QFile::exists("/usr/local/etc/cups/cupsd.conf") ) return true;
    if ( QFile::exists("/opt/etc/cups/cupsd.conf") ) return true;
    if ( QFile::exists("/opt/local/etc/cups/cupsd.conf") ) return true;
    return false;

    // This bit would solve the issue of CUPS having problems, but again
    // I can't get it to work :-)
    // copied from KDE3 KdeprintChecker::checkService()
    // Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
    // original license LGPL
    /*
    KLocalSocket sock;
    sock.connectToPath("/ipp");
    kDebug(OkularDebug) << "socket wait =" << sock.waitForConnected();
    kDebug(OkularDebug) << "socket error =" << sock.error();
    kDebug(OkularDebug) << "socket isOpen() =" << sock.isOpen();
    return sock.isOpen();
    */
}

void KCupsOptionsWidget::toggleEnabled()
{
    setEnabled( cupsAvailable() );
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

