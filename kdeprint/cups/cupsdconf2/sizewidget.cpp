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

#include "sizewidget.h"

#include <qcombobox.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qregexp.h>
#include <klocale.h>

SizeWidget::SizeWidget( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
	m_size = new QSpinBox( 0, 9999, 1, this );
	m_unit = new QComboBox( this );

	m_unit->insertItem( i18n( "KB" ) );
	m_unit->insertItem( i18n( "MB" ) );
	m_unit->insertItem( i18n( "GB" ) );
	m_unit->insertItem( i18n( "Tiles" ) );
	m_unit->setCurrentItem( 1 );
	m_size->setSpecialValueText( i18n( "Unlimited" ) );

	QHBoxLayout *l0 = new QHBoxLayout( this, 0, 5 );
	l0->addWidget( m_size, 1 );
	l0->addWidget( m_unit, 0 );
}

void SizeWidget::setSizeString( const QString& sz )
{
	int p = sz.find( QRegExp( "\\D" ) );
	m_size->setValue( sz.left( p ).toInt() );
	switch( sz[ p ].latin1() )
	{
		case 'k': p = 0; break;
		default:
		case 'm': p = 1; break;
		case 'g': p = 2; break;
		case 't': p = 3; break;
	}
	m_unit->setCurrentItem( p );
}

QString SizeWidget::sizeString() const
{
	QString result = QString::number( m_size->value() );
	switch ( m_unit->currentItem() )
	{
		case 0: result.append( "k" ); break;
		case 1: result.append( "m" ); break;
		case 2: result.append( "g" ); break;
		case 3: result.append( "t" ); break;
	}
	return result;
}

void SizeWidget::setValue( int value )
{
	m_size->setValue( value );
	m_unit->setCurrentItem( 1 );
}

int SizeWidget::value() const
{
	return m_size->value();
}
