/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2002 Michael Goffioul <goffioul@imec.be>
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

#include "kpposterpage.h"
#include "posterpreview.h"
#include "kprinter.h"
#include "kmfactory.h"
#include "util.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qlineedit.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <knuminput.h>
#include <kiconloader.h>

KPPosterPage::KPPosterPage( QWidget *parent, const char *name )
	: KPrintDialogPage( parent, name )
{
	setTitle( i18n( "Poster" ) );

	m_postercheck = new QCheckBox( i18n( "&Print poster" ), this );
	QWidget *dummy = new QWidget( this );
	m_preview = new PosterPreview( dummy );
	m_postersize = new QComboBox( dummy );
	m_printsize = new QComboBox( dummy );
	m_lockbtn = new KPushButton( dummy );
	m_mediasize = new QLabel( dummy );
	m_mediasize->setFrameStyle( QFrame::Panel|QFrame::Sunken );
	QLabel *posterlab = new QLabel( i18n( "Poste&r size:" ), dummy );
	QLabel *medialab = new QLabel( i18n( "Media size:" ), dummy );
	QLabel *printlab = new QLabel( i18n( "Pri&nt size:" ), dummy );
	posterlab->setBuddy( m_postersize );
	printlab->setBuddy( m_printsize );
	m_cutmargin = new KIntNumInput( 5, dummy );
	// xgettext:no-c-format
	m_cutmargin->setLabel( i18n( "C&ut margin (% of media):" ) );
	m_cutmargin->setRange( 0, 100, 10, true );
	m_selection = new QLineEdit( dummy );
	QLabel *selectionlab = new QLabel( i18n( "&Tile pages (to be printed):" ), dummy );
	selectionlab->setBuddy( m_selection );
	m_lockbtn->setToggleButton( true );
	m_lockbtn->setPixmap( SmallIcon( "encrypted" ) );
	m_lockbtn->setOn( true );
	m_lockbtn->setFixedSize( m_lockbtn->sizeHint() );
	QToolTip::add( m_lockbtn, i18n( "Link/unlink poster and print size" ) );

	for ( int i=0; i<KPrinter::NPageSize-1; i++ )
	{
		m_postersize->insertItem( page_sizes[ i ].text );
		m_printsize->insertItem( page_sizes[ i ].text );
	}
	m_postersize->setCurrentItem( findIndex( KPrinter::A3 ) );
	slotPosterSizeChanged( m_postersize->currentItem() );

	connect( m_postercheck, SIGNAL( toggled( bool ) ), dummy, SLOT( setEnabled( bool ) ) );
	dummy->setEnabled( false );
	connect( m_postersize, SIGNAL( activated( int ) ), SLOT( slotPosterSizeChanged( int ) ) );
	connect( m_cutmargin, SIGNAL( valueChanged( int ) ), SLOT( slotMarginChanged( int ) ) );
	connect( m_lockbtn, SIGNAL( toggled( bool ) ), m_printsize, SLOT( setDisabled( bool ) ) );
	m_printsize->setEnabled( false );
	connect( m_lockbtn, SIGNAL( toggled( bool ) ), SLOT( slotLockToggled( bool ) ) );
	connect( m_selection, SIGNAL( textChanged( const QString& ) ), m_preview, SLOT( setSelectedPages( const QString& ) ) );
	connect( m_preview, SIGNAL( selectionChanged( const QString& ) ), m_selection, SLOT( setText( const QString& ) ) );

	if ( KMFactory::self()->settings()->application != KPrinter::Dialog )
	{
		m_printsize->hide();
		m_lockbtn->hide();
		printlab->hide();
	}

	QVBoxLayout *l0 = new QVBoxLayout( this, 0, 10 );
	l0->addWidget( m_postercheck );
	l0->addWidget( dummy, 1 );
	QGridLayout *l1 = new QGridLayout( dummy, 8, 3, 0, 5 );
	l1->addWidget( posterlab, 0, 0 );
	l1->addWidget( m_postersize, 0, 1 );
	l1->addWidget( printlab, 1, 0 );
	l1->addWidget( m_printsize, 1, 1 );
	l1->addWidget( medialab, 2, 0 );
	l1->addWidget( m_mediasize, 2, 1 );
	l1->addMultiCellWidget( m_preview, 4, 4, 0, 2 );
	l1->addMultiCellWidget( m_cutmargin, 6, 6, 0, 2 );
	l1->addMultiCellWidget( m_lockbtn, 0, 1, 2, 2 );
	QHBoxLayout *l2 = new QHBoxLayout( 0, 0, 5 );
	l1->addMultiCellLayout( l2, 7, 7, 0, 2 );
	l2->addWidget( selectionlab );
	l2->addWidget( m_selection );
	l1->setColStretch( 1, 1 );
	l1->setRowStretch( 4, 1 );
	l1->addRowSpacing( 3, 10 );
	l1->addRowSpacing( 5, 10 );
}

KPPosterPage::~KPPosterPage()
{
}

void KPPosterPage::setOptions( const QMap<QString,QString>& opts )
{
	QString ps = opts[ "PageSize" ];
	if ( ps.isEmpty() && !opts[ "kde-pagesize" ].isEmpty() )
	{
		ps = pageSizeToPageName( ( KPrinter::PageSize )opts[ "kde-pagesize" ].toInt() );
		if ( ps.isEmpty() )
			ps = opts[ "_kde-poster-media" ];
	}
	if ( ps.isEmpty() )
		m_mediasize->setText( i18n( "Unknown" ) );
	else
		m_mediasize->setText( ps );
	m_preview->setMediaSize( ps );

	if ( opts[ "_kde-filters" ].find( "poster" ) != -1 )
	{
		m_postercheck->setChecked( true );
		ps = opts[ "_kde-poster-size" ];
		QString prtsize = opts[ "kde-printsize" ];
		if ( !ps.isEmpty() )
		{
			m_postersize->setCurrentItem( findIndex( pageNameToPageSize( ps ) ) );
			m_lockbtn->setOn( !prtsize.isEmpty() && 
					page_sizes[ m_postersize->currentItem() ].ID == prtsize.toInt() );
			if ( !m_lockbtn->isOn() )
				m_printsize->setCurrentItem( findIndex( prtsize.toInt() ) );
			slotPosterSizeChanged( m_postersize->currentItem() );
		}
		if ( !opts[ "_kde-poster-cut" ].isEmpty() )
			m_cutmargin->setValue( opts[ "_kde-poster-cut" ].toInt() );
		m_selection->setText( opts[ "_kde-poster-select" ] );
	}
	else
		m_postercheck->setChecked( false );
}

void KPPosterPage::getOptions( QMap<QString,QString>& opts, bool )
{
	QStringList o = QStringList::split( ",", opts[ "_kde-filters" ], false );
	if ( !m_postercheck->isChecked() )
	{
		o.remove( "poster" );
		opts[ "_kde-filters" ] = o.join( "," );
		opts.remove( "_kde-poster-media" );
		opts.remove( "_kde-poster-size" );
		opts.remove( "_kde-poster-cut" );
		opts.remove( "kde-printsize" );
		opts.remove( "_kde-poster-select" );
	}
	else
	{
		if ( !o.contains( "poster" ) )
			o.append( "poster" );
		opts[ "_kde-filters" ] = o.join( "," );
		opts[ "_kde-poster-media" ] = m_mediasize->text();
		opts[ "_kde-poster-size" ] = pageSizeToPageName( ( KPrinter::PageSize )page_sizes[ m_postersize->currentItem() ].ID );
		opts[ "kde-printsize" ] = QString::number( page_sizes[ m_printsize->currentItem() ].ID );
		opts[ "_kde-poster-cut" ] = QString::number( m_cutmargin->value() );
		opts[ "_kde-poster-select" ] = m_selection->text().stripWhiteSpace();
	}
}

bool KPPosterPage::isValid()
{
	return true;
}

void KPPosterPage::slotPosterSizeChanged( int value )
{
	int ID = page_sizes[ m_postersize->currentItem() ].ID;
	m_preview->setPosterSize( ID );
	if ( m_lockbtn->isOn() )
		m_printsize->setCurrentItem( value );
}

void KPPosterPage::slotMarginChanged( int value )
{
	m_preview->setCutMargin( value );
}

void KPPosterPage::slotLockToggled( bool on )
{
	m_lockbtn->setPixmap( SmallIcon( on ? "encrypted" : "decrypted" ) );
	if ( on )
		m_printsize->setCurrentItem( m_postersize->currentItem() );
}

#include "kpposterpage.moc"
