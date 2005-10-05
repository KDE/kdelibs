/*
    This file is part of libkresources.

    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <klocale.h>
#include <klineedit.h>
#include <kmessagebox.h>

#include <q3groupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include "factory.h"
#include "configdialog.h"

using namespace KRES;

ConfigDialog::ConfigDialog( QWidget *parent, const QString& resourceFamily,
    /*const QString& type,*/ Resource* resource, /*KConfig *config, */const char *name )
  : KDialogBase( parent, name, true, i18n( "Resource Configuration" ),
    Ok|Cancel, Ok, false )/*, mConfig( config )*/, mResource( resource )
{
  Factory *factory = Factory::self( resourceFamily );

  QFrame *main = makeMainWidget();

  QVBoxLayout *mainLayout = new QVBoxLayout( main, 0, spacingHint() );

  Q3GroupBox *generalGroupBox = new Q3GroupBox( 2, Qt::Horizontal, main );
  generalGroupBox->layout()->setSpacing( spacingHint() );
  generalGroupBox->setTitle( i18n( "General Settings" ) );

  new QLabel( i18n( "Name:" ), generalGroupBox );

  mName = new KLineEdit( generalGroupBox );

  mReadOnly = new QCheckBox( i18n( "Read-only" ), generalGroupBox );

  mName->setText( mResource->resourceName() );
  mReadOnly->setChecked( mResource->readOnly() );

  mainLayout->addWidget( generalGroupBox );

  Q3GroupBox *resourceGroupBox = new Q3GroupBox( 2, Qt::Horizontal,  main );
  resourceGroupBox->layout()->setSpacing( spacingHint() );
  resourceGroupBox->setTitle( i18n( "%1 Resource Settings" )
                              .arg( factory->typeName( resource->type() ) ) );
  mainLayout->addWidget( resourceGroupBox );

  mainLayout->addStretch();

  mConfigWidget = factory->configWidget( resource->type(), resourceGroupBox );
  if ( mConfigWidget ) {
    mConfigWidget->setInEditMode( false );
    mConfigWidget->loadSettings( mResource );
    mConfigWidget->show();
    connect( mConfigWidget, SIGNAL( setReadOnly( bool ) ),
        SLOT( setReadOnly( bool ) ) );
  }

  connect( mName, SIGNAL( textChanged(const QString &)),
      SLOT( slotNameChanged(const QString &)));

  slotNameChanged( mName->text() );
  setMinimumSize( sizeHint() );
}

void ConfigDialog::setInEditMode( bool value )
{
  if ( mConfigWidget )
    mConfigWidget->setInEditMode( value );
}

void ConfigDialog::slotNameChanged( const QString &text)
{
  enableButtonOK( !text.isEmpty() );
}

void ConfigDialog::setReadOnly( bool value )
{
  mReadOnly->setChecked( value );
}

void ConfigDialog::accept()
{
  if ( mName->text().isEmpty() ) {
    KMessageBox::sorry( this, i18n( "Please enter a resource name." ) );
    return;
  }

  mResource->setResourceName( mName->text() );
  mResource->setReadOnly( mReadOnly->isChecked() );

  if ( mConfigWidget ) {
    // First save generic information
    // Also save setting of specific resource type
    mConfigWidget->saveSettings( mResource );
  }

  KDialog::accept();
}

#include "configdialog.moc"
