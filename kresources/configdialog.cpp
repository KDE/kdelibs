/*
    This file is part of libkresources.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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
*/

#include <klocale.h>
#include <kmessagebox.h>

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <qcheckbox.h>

#include <kbuttonbox.h>
#include <kdialog.h>
#include <klineedit.h>

#include "resourcefactory.h"
#include "resourceconfigdlg.h"

using namespace KRES;

ResourceConfigDlg::ResourceConfigDlg( QWidget *parent, const QString& resourceFamily,
    /*const QString& type,*/ Resource* resource, /*KConfig *config, */const char *name )
  : KDialog( parent, name, true )/*, mConfig( config )*/, mResource( resource )
{
  ResourceFactory *factory = ResourceFactory::self( resourceFamily );

  setCaption( i18n( "Resource Configuration" ) );
  resize( 250, 240 );

  QVBoxLayout *mainLayout = new QVBoxLayout( this, marginHint(), spacingHint() );

  QGroupBox *generalGroupBox = new QGroupBox( 2, Qt::Horizontal,  this );
  generalGroupBox->setTitle( i18n( "General Settings" ) );

  new QLabel( i18n( "Name:" ), generalGroupBox );

  mName = new KLineEdit( generalGroupBox );

  mReadOnly = new QCheckBox( i18n( "Read-only" ), generalGroupBox );

  mName->setText( mResource->resourceName() );
  mReadOnly->setChecked( mResource->readOnly() );

  mainLayout->addWidget( generalGroupBox );

  QGroupBox *resourceGroupBox = new QGroupBox( 2, Qt::Horizontal,  this );
  resourceGroupBox->setTitle( i18n( "Resource Settings" ) );

  mainLayout->addSpacing( 10 );
  mainLayout->addWidget( resourceGroupBox );
  mainLayout->addSpacing( 10 );

  mConfigWidget = factory->configWidget( resource->type(), resourceGroupBox );
  if ( mConfigWidget ) { // && mConfig ) {
    mConfigWidget->loadSettings( mResource );
    mConfigWidget->show();
    // connect( mConfigWidget, SIGNAL( setResourceName( const QString & ) ), SLOT( setResourceName( const QString & ) ) );
    connect( mConfigWidget, SIGNAL( setReadOnly( bool ) ), SLOT( setReadOnly( bool ) ) );
    // connect( mConfigWidget, SIGNAL( setFast( bool ) ), SLOT( setFast( bool ) ) );
  }


  KButtonBox *mButtonBox = new KButtonBox( this );

  mButtonBox->addStretch();
  mbuttonOk = mButtonBox->addButton( i18n( "&OK" ), this, SLOT( accept() ) );
  mbuttonOk->setFocus();
  mButtonBox->addButton( i18n( "&Cancel" ), this, SLOT( reject() ) );
  mButtonBox->layout();
  connect( mName, SIGNAL( textChanged ( const QString & )),this, SLOT( slotNameChanged( const QString &)));
  mainLayout->addWidget( mButtonBox );
  slotNameChanged( mName->text());

}

void ResourceConfigDlg::slotNameChanged( const QString &text)
{
    mbuttonOk->setEnabled( !text.isEmpty() );
}

int ResourceConfigDlg::exec()
{
  return QDialog::exec();
}

void ResourceConfigDlg::setReadOnly( bool value )
{
  mReadOnly->setChecked( value );
}

void ResourceConfigDlg::accept()
{
  if ( mName->text().isEmpty() ) {
    KMessageBox::sorry( this, i18n( "Please enter a resource name" ) );
    return;
  }

  mResource->setResourceName( mName->text() );
  mResource->setReadOnly( mReadOnly->isChecked() );

  if ( mConfigWidget /*&& mConfig*/ ) {
    // First save generic information
    // Also save setting of specific resource type
    mConfigWidget->saveSettings( mResource );
  }

  QDialog::accept();
}

#include "resourceconfigdlg.moc"
