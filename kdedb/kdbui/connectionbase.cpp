#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './connectionbase.ui'
**
** Created: Fri Mar 23 15:07:56 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "connectionbase.h"

#include <kcombobox.h>
#include <klineedit.h>
#include <klocale.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a ConnectionBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
ConnectionBase::ConnectionBase( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "ConnectionBase" );
    resize( 400, 179 ); 
    setCaption( i18n( "Form1" ) );
    ConnectionBaseLayout = new QGridLayout( this ); 
    ConnectionBaseLayout->setSpacing( 6 );
    ConnectionBaseLayout->setMargin( 11 );

    pluginCombo = new KComboBox( FALSE, this, "pluginCombo" );
    pluginCombo->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, pluginCombo->sizePolicy().hasHeightForWidth() ) );

    ConnectionBaseLayout->addMultiCellWidget( pluginCombo, 0, 0, 1, 2 );

    Layout5 = new QHBoxLayout; 
    Layout5->setSpacing( 6 );
    Layout5->setMargin( 0 );

    m_host = new KLineEdit( this, "m_host" );
    m_host->setText( i18n( "localhost" ) );
    Layout5->addWidget( m_host );

    TextLabel3 = new QLabel( this, "TextLabel3" );
    TextLabel3->setText( i18n( ":" ) );
    Layout5->addWidget( TextLabel3 );

    m_port = new KLineEdit( this, "m_port" );
    m_port->setMaximumSize( QSize( 50, 32767 ) );
    Layout5->addWidget( m_port );

    ConnectionBaseLayout->addMultiCellLayout( Layout5, 1, 1, 1, 2 );

    m_user = new KLineEdit( this, "m_user" );
    m_user->setMaximumSize( QSize( 32767, 32767 ) );

    ConnectionBaseLayout->addMultiCellWidget( m_user, 2, 2, 1, 2 );

    TextLabel4 = new QLabel( this, "TextLabel4" );
    TextLabel4->setText( i18n( "User:" ) );

    ConnectionBaseLayout->addWidget( TextLabel4, 2, 0 );

    TextLabel2 = new QLabel( this, "TextLabel2" );
    TextLabel2->setText( i18n( "Host:" ) );

    ConnectionBaseLayout->addWidget( TextLabel2, 1, 0 );

    Layout6 = new QHBoxLayout; 
    Layout6->setSpacing( 6 );
    Layout6->setMargin( 0 );

    m_password = new KLineEdit( this, "m_password" );
    m_password->setEchoMode( KLineEdit::Password );
    Layout6->addWidget( m_password );

    remember = new QCheckBox( this, "remember" );
    remember->setText( i18n( "Remember password" ) );
    Layout6->addWidget( remember );

    ConnectionBaseLayout->addMultiCellLayout( Layout6, 3, 3, 1, 2 );

    TextLabel1 = new QLabel( this, "TextLabel1" );
    TextLabel1->setText( i18n( "DBMS:" ) );

    ConnectionBaseLayout->addWidget( TextLabel1, 0, 0 );

    saveConnection = new QCheckBox( this, "saveConnection" );
    saveConnection->setText( i18n( "Save connection in connection list" ) );

    ConnectionBaseLayout->addMultiCellWidget( saveConnection, 5, 5, 0, 1 );

    createButton = new QPushButton( this, "createButton" );
    createButton->setText( i18n( "&Create connection" ) );

    ConnectionBaseLayout->addWidget( createButton, 5, 2 );

    TextLabel5 = new QLabel( this, "TextLabel5" );
    TextLabel5->setText( i18n( "Password:" ) );

    ConnectionBaseLayout->addWidget( TextLabel5, 3, 0 );

    Line2 = new QFrame( this, "Line2" );
    Line2->setFrameStyle( QFrame::HLine | QFrame::Sunken );

    ConnectionBaseLayout->addMultiCellWidget( Line2, 4, 4, 0, 2 );

    // signals and slots connections
    connect( createButton, SIGNAL( clicked() ), this, SLOT( slotCreateConnection() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
ConnectionBase::~ConnectionBase()
{
    // no need to delete child widgets, Qt does it all for us
}

void ConnectionBase::slotCreateConnection()
{
    qWarning( "ConnectionBase::slotCreateConnection(): Not implemented yet!" );
}

void ConnectionBase::slotDatabaseSelected()
{
    qWarning( "ConnectionBase::slotDatabaseSelected(): Not implemented yet!" );
}

#include "connectionbase.moc"
