/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2002 Michael Goffioul <goffioul@imec.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "messagewindow.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <qhbox.h>
#include <kiconloader.h>
#include <kapplication.h>

MessageWindow::MessageWindow( const QString& txt, int delay, QWidget *parent, const char *name )
	: QWidget( parent, name, WStyle_Customize|WStyle_NoBorder|WShowModal|WType_Dialog|WDestructiveClose )
{
	QHBox *box = new QHBox( this );
	box->setFrameStyle( QFrame::Panel|QFrame::Raised );
	box->setLineWidth( 1 );
	box->setSpacing( 10 );
	box->setMargin( 5 );
	QLabel *pix = new QLabel( box );
	pix->setPixmap( DesktopIcon( "kdeprint_printer" ) );
	m_text = new QLabel( txt, box );

	QHBoxLayout *l0 = new QHBoxLayout( this, 0, 0 );
	l0->addWidget( box );

	if ( delay == 0 )
		slotTimer();
	else
		QTimer::singleShot( delay, this, SLOT( slotTimer() ) );
}

void MessageWindow::slotTimer()
{
	QSize psz = parentWidget()->size(), sz = sizeHint();
	move( parentWidget()->mapToGlobal( QPoint( (psz.width()-sz.width())/2, (psz.height()-sz.height())/2 ) ) );
	if ( !isVisible() )
	{
		show();
		kapp->processEvents();
	}
}

QString MessageWindow::text() const
{
	return m_text->text();
}

void MessageWindow::setText( const QString& txt )
{
	m_text->setText( txt );
}

#include "messagewindow.moc"
