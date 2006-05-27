/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#include "effectwidget.h"
#include "effectwidget_p.h"

#include <QtAlgorithms>
#include <QList>

#include "../effect.h"
#include "../effectparameter.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>

namespace Phonon
{

EffectWidget::EffectWidget( Effect* effect, QWidget* parent )
	: QWidget( parent )
	, d_ptr( new EffectWidgetPrivate )
{
	init( effect );
}

EffectWidget::~EffectWidget()
{
	delete d_ptr;
	d_ptr = 0;
}

EffectWidget::EffectWidget( EffectWidgetPrivate& dd, Effect* effect, QWidget* parent )
	: QWidget( parent )
	, d_ptr( &dd )
{
	init( effect );
}

void EffectWidget::init( Effect* effect )
{
	Q_D( EffectWidget );
	d->q_ptr = this;
	d->effect = effect;
	//TODO: look up whether there is a specialized widget for this effect. This
	//could be a DSO or a Designer ui file found via KTrader.
	//
	//if no specialized widget is available:
	autogenerateUi();
}

void EffectWidget::autogenerateUi()
{
	Q_D( EffectWidget );
	QVBoxLayout* mainLayout = new QVBoxLayout( this );
	QList<EffectParameter> plist = d->effect->parameterList();
	qSort( plist );
	foreach( EffectParameter para, plist )
	{
		QHBoxLayout* pLayout = new QHBoxLayout;
		mainLayout->addLayout( pLayout );

		QLabel* label = new QLabel( this );
		pLayout->addWidget( label );
		label->setText( para.name() );
		label->setToolTip( para.description() );

		QWidget* control;
		if( para.isToggleControl() )
		{
			QCheckBox* cb = new QCheckBox( this );
			control = cb;
			cb->setChecked( para.value().toBool() );
			connect( cb, SIGNAL( toggled( bool ) ), SLOT( setToggleParameter( bool ) ) );
		}
		else if( para.minimumValue().isValid() && para.maximumValue().isValid() )
		{
			if( para.isIntegerControl() )
			{
				QSpinBox* sb = new QSpinBox( this );
				control = sb;
				sb->setRange( para.minimumValue().toInt(),
						para.maximumValue().toInt() );
				sb->setValue( para.value().toInt() );
				connect( sb, SIGNAL( valueChanged( int ) ),
						SLOT( setIntParameter( int ) ) );
			}
			else
			{
				QDoubleSpinBox* sb = new QDoubleSpinBox( this );
				control = sb;
				sb->setRange( para.minimumValue().toDouble(),
						para.maximumValue().toDouble() );
				sb->setValue( para.value().toDouble() );
				connect( sb, SIGNAL( valueChanged( double ) ),
						SLOT( setDoubleParameter( double ) ) );
			}
		}
		else
		{
			QDoubleSpinBox* sb = new QDoubleSpinBox( this );
			control = sb;
			sb->setDecimals( 7 );
			sb->setRange( -1e100, 1e100 );
			connect( sb, SIGNAL( valueChanged( double ) ),
					SLOT( setDoubleParameter( double ) ) );
		}
		control->setToolTip( para.description() );
		label->setBuddy( control );
		pLayout->addWidget( control );
		d->parameterForObject.insert( control, para );
	}
}

void EffectWidget::setToggleParameter( bool checked )
{
	Q_D( EffectWidget );
	EffectParameter p = d->parameterForObject[ sender() ];
	if( p.isValid() )
		p.setValue( checked );
}

void EffectWidget::setIntParameter( int value )
{
	Q_D( EffectWidget );
	EffectParameter p = d->parameterForObject[ sender() ];
	if( p.isValid() )
		p.setValue( value );
}

void EffectWidget::setDoubleParameter( double value )
{
	Q_D( EffectWidget );
	EffectParameter p = d->parameterForObject[ sender() ];
	if( p.isValid() )
		p.setValue( value );
}

} // namespace Phonon

#include "effectwidget.moc"

// vim: sw=4 ts=4 noet
