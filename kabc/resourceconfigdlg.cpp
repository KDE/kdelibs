/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    $Id$
*/

#include <klocale.h>

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include "resourcefactory.h"
#include "resourceconfigdlg.h"

ResourceConfigDlg::ResourceConfigDlg( QWidget *parent, const QString& type,
	KConfig *config, const char *name )
    : QDialog( parent, name, true ), mConfig( config )
{
    KABC::ResourceFactory *factory = KABC::ResourceFactory::self();
    mConfigWidget = factory->configWidget( type, this );
    if ( mConfigWidget && mConfig )
	mConfigWidget->loadSettings( mConfig );

    setCaption( i18n( "Resource Configuration" ) );

    QVBoxLayout *mainLayout = new QVBoxLayout( this );
    
    QGroupBox *groupBox = new QGroupBox( 2, Qt::Horizontal,  this );
    groupBox->setTitle( i18n( "General Settings" ) );

    new QLabel( i18n( "Name:" ), groupBox );

    resourceName = new KLineEdit( groupBox );

    resourceIsReadOnly = new QCheckBox( i18n( "Read-only" ), groupBox );

    resourceIsFast = new QCheckBox( i18n( "Fast resource" ), groupBox );

    mainLayout->addWidget( groupBox );

    if ( mConfigWidget ) {
	mainLayout->addSpacing( 10 );
	mainLayout->addWidget( mConfigWidget );
	mConfigWidget->show();
    }
    mainLayout->addSpacing( 10 );

    buttonBox = new KButtonBox( this );

    buttonBox->addStretch();    
    buttonBox->addButton( i18n( "&Apply" ), this, SLOT( accept() ) );
    buttonBox->addButton( i18n( "&Cancel" ), this, SLOT( reject() ) );
    buttonBox->layout();

    mainLayout->addWidget( buttonBox );
}

int ResourceConfigDlg::exec()
{
    return QDialog::exec();
}

void ResourceConfigDlg::accept()
{
    if ( mConfigWidget && mConfig )
	mConfigWidget->saveSettings( mConfig );

    QDialog::accept();
}

#include "resourceconfigdlg.moc"
