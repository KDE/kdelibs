#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file '../../../kdeprint/cups/kcupsoptiondlgbase.ui'
**
** Created: Thu Mar 8 00:35:20 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include <kdeprint/cups/kcupsoptiondlgbase.h>

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a KCupsOptionDlgBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
KCupsOptionDlgBase::KCupsOptionDlgBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "KCupsOptionDlgBase" );
    resize( 330, 248 ); 
    setCaption( i18n( "CUPS settings" ) );
    KCupsOptionDlgBaseLayout = new QVBoxLayout( this ); 
    KCupsOptionDlgBaseLayout->setSpacing( 6 );
    KCupsOptionDlgBaseLayout->setMargin( 11 );

    serverbox_ = new QGroupBox( this, "serverbox_" );
    serverbox_->setTitle( i18n( "Server informations" ) );
    serverbox_->setColumnLayout(0, Qt::Vertical );
    serverbox_->layout()->setSpacing( 0 );
    serverbox_->layout()->setMargin( 0 );
    serverbox_Layout = new QGridLayout( serverbox_->layout() );
    serverbox_Layout->setAlignment( Qt::AlignTop );
    serverbox_Layout->setSpacing( 6 );
    serverbox_Layout->setMargin( 11 );

    serverlabel_ = new QLabel( serverbox_, "serverlabel_" );
    serverlabel_->setText( i18n( "Server:" ) );
    serverlabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)3, serverlabel_->sizePolicy().hasHeightForWidth() ) );

    serverbox_Layout->addWidget( serverlabel_, 0, 0 );

    portlabel_ = new QLabel( serverbox_, "portlabel_" );
    portlabel_->setText( i18n( "Port:" ) );
    portlabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)3, portlabel_->sizePolicy().hasHeightForWidth() ) );

    serverbox_Layout->addWidget( portlabel_, 1, 0 );

    server_ = new QLineEdit( serverbox_, "server_" );
    server_->setEnabled( TRUE );

    serverbox_Layout->addWidget( server_, 0, 1 );

    port_ = new QLineEdit( serverbox_, "port_" );

    serverbox_Layout->addWidget( port_, 1, 1 );
    KCupsOptionDlgBaseLayout->addWidget( serverbox_ );

    userbox_ = new QGroupBox( this, "userbox_" );
    userbox_->setTitle( i18n( "User informations" ) );
    userbox_->setColumnLayout(0, Qt::Vertical );
    userbox_->layout()->setSpacing( 0 );
    userbox_->layout()->setMargin( 0 );
    userbox_Layout = new QGridLayout( userbox_->layout() );
    userbox_Layout->setAlignment( Qt::AlignTop );
    userbox_Layout->setSpacing( 6 );
    userbox_Layout->setMargin( 11 );

    userlabel_ = new QLabel( userbox_, "userlabel_" );
    userlabel_->setText( i18n( "User:" ) );
    userlabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)3, userlabel_->sizePolicy().hasHeightForWidth() ) );

    userbox_Layout->addWidget( userlabel_, 0, 0 );

    passwordlabel_ = new QLabel( userbox_, "passwordlabel_" );
    passwordlabel_->setText( i18n( "Password:" ) );
    passwordlabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)3, passwordlabel_->sizePolicy().hasHeightForWidth() ) );

    userbox_Layout->addWidget( passwordlabel_, 1, 0 );

    login_ = new QLineEdit( userbox_, "login_" );

    userbox_Layout->addWidget( login_, 0, 1 );

    password_ = new QLineEdit( userbox_, "password_" );
    password_->setEchoMode( QLineEdit::Password );

    userbox_Layout->addWidget( password_, 1, 1 );

    anonymous_ = new QCheckBox( userbox_, "anonymous_" );
    anonymous_->setText( i18n( "Anonymous access" ) );

    userbox_Layout->addMultiCellWidget( anonymous_, 2, 2, 0, 1 );
    KCupsOptionDlgBaseLayout->addWidget( userbox_ );

    Layout1 = new QHBoxLayout; 
    Layout1->setSpacing( 6 );
    Layout1->setMargin( 0 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer );

    okbutton_ = new QPushButton( this, "okbutton_" );
    okbutton_->setText( i18n( "OK" ) );
    okbutton_->setDefault( TRUE );
    Layout1->addWidget( okbutton_ );

    cancelbutton_ = new QPushButton( this, "cancelbutton_" );
    cancelbutton_->setText( i18n( "Cancel" ) );
    Layout1->addWidget( cancelbutton_ );
    KCupsOptionDlgBaseLayout->addLayout( Layout1 );

    // signals and slots connections
    connect( okbutton_, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( cancelbutton_, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( anonymous_, SIGNAL( toggled(bool) ), login_, SLOT( setDisabled(bool) ) );
    connect( anonymous_, SIGNAL( toggled(bool) ), password_, SLOT( setDisabled(bool) ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
KCupsOptionDlgBase::~KCupsOptionDlgBase()
{
    // no need to delete child widgets, Qt does it all for us
}

