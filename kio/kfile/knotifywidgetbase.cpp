#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './knotifywidgetbase.ui'
**
** Created: Fre Jul 12 00:07:19 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "knotifywidgetbase.h"

#include <qvariant.h>
#include <kcombobox.h>
#include <klistview.h>
#include <kurlrequester.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a KNotifyWidgetBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
KNotifyWidgetBase::KNotifyWidgetBase( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "KNotifyWidgetBase" );
    resize( 618, 599 ); 
    setCaption( tr2i18n( "Form2" ) );
    KNotifyWidgetBaseLayout = new QVBoxLayout( this, 11, 6, "KNotifyWidgetBaseLayout"); 

    m_listview = new KListView( this, "m_listview" );
    m_listview->addColumn( tr2i18n( "Events" ) );
    m_listview->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 10, 0, m_listview->sizePolicy().hasHeightForWidth() ) );
    KNotifyWidgetBaseLayout->addWidget( m_listview );

    m_actionsBox = new QGroupBox( this, "m_actionsBox" );
    m_actionsBox->setTitle( tr2i18n( "Actions" ) );
    m_actionsBox->setColumnLayout(0, Qt::Vertical );
    m_actionsBox->layout()->setSpacing( 6 );
    m_actionsBox->layout()->setMargin( 11 );
    m_actionsBoxLayout = new QVBoxLayout( m_actionsBox->layout() );
    m_actionsBoxLayout->setAlignment( Qt::AlignTop );

    Layout26 = new QGridLayout( 0, 1, 1, 0, 6, "Layout26"); 

    m_logfilePath = new KURLRequester( m_actionsBox, "m_logfilePath" );
    m_logfilePath->setEnabled( FALSE );
    m_logfilePath->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 1, 0, m_logfilePath->sizePolicy().hasHeightForWidth() ) );

    Layout26->addWidget( m_logfilePath, 1, 1 );

    m_soundPath = new KURLRequester( m_actionsBox, "m_soundPath" );
    m_soundPath->setEnabled( FALSE );
    m_soundPath->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 1, 0, m_soundPath->sizePolicy().hasHeightForWidth() ) );

    Layout26->addWidget( m_soundPath, 0, 1 );

    m_logToFile = new QCheckBox( m_actionsBox, "m_logToFile" );
    m_logToFile->setText( tr2i18n( "&Log to a file:" ) );

    Layout26->addWidget( m_logToFile, 1, 0 );

    m_executePath = new KURLRequester( m_actionsBox, "m_executePath" );
    m_executePath->setEnabled( FALSE );
    m_executePath->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 1, 0, m_executePath->sizePolicy().hasHeightForWidth() ) );

    Layout26->addWidget( m_executePath, 2, 1 );

    Layout25 = new QHBoxLayout( 0, 0, 6, "Layout25"); 

    m_playSound = new QCheckBox( m_actionsBox, "m_playSound" );
    m_playSound->setText( tr2i18n( "Play a &sound:" ) );
    Layout25->addWidget( m_playSound );

    m_playButton = new QPushButton( m_actionsBox, "m_playButton" );
    m_playButton->setText( QString::null );
    QToolTip::add( m_playButton, tr2i18n( "Test the Sound" ) );
    Layout25->addWidget( m_playButton );

    Layout26->addLayout( Layout25, 0, 0 );

    m_execute = new QCheckBox( m_actionsBox, "m_execute" );
    m_execute->setText( tr2i18n( "E&xecute a program:" ) );

    Layout26->addWidget( m_execute, 2, 0 );
    m_actionsBoxLayout->addLayout( Layout26 );

    Layout27 = new QHBoxLayout( 0, 0, 6, "Layout27"); 

    m_messageBox = new QCheckBox( m_actionsBox, "m_messageBox" );
    m_messageBox->setText( tr2i18n( "Show a &message in a pop-up window" ) );
    Layout27->addWidget( m_messageBox );
    QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout27->addItem( spacer );
    m_actionsBoxLayout->addLayout( Layout27 );

    Layout28 = new QHBoxLayout( 0, 0, 6, "Layout28"); 
    QSpacerItem* spacer_2 = new QSpacerItem( 30, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    Layout28->addItem( spacer_2 );

    m_passivePopup = new QCheckBox( m_actionsBox, "m_passivePopup" );
    m_passivePopup->setEnabled( FALSE );
    m_passivePopup->setText( tr2i18n( "&Use a passive window that does not interrupt other work" ) );
    Layout28->addWidget( m_passivePopup );
    QSpacerItem* spacer_3 = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout28->addItem( spacer_3 );
    m_actionsBoxLayout->addLayout( Layout28 );

    Layout29 = new QHBoxLayout( 0, 0, 6, "Layout29"); 

    m_stderr = new QCheckBox( m_actionsBox, "m_stderr" );
    m_stderr->setText( tr2i18n( "Print a message to standard &error output" ) );
    Layout29->addWidget( m_stderr );
    QSpacerItem* spacer_4 = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout29->addItem( spacer_4 );
    m_actionsBoxLayout->addLayout( Layout29 );

    m_extension = new QPushButton( m_actionsBox, "m_extension" );
    m_extension->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)0, 0, 0, m_extension->sizePolicy().hasHeightForWidth() ) );
    m_extension->setText( tr2i18n( "Less Options" ) );
    m_extension->setToggleButton( FALSE );
    m_actionsBoxLayout->addWidget( m_extension );
    KNotifyWidgetBaseLayout->addWidget( m_actionsBox );

    m_controlsBox = new QGroupBox( this, "m_controlsBox" );
    m_controlsBox->setTitle( tr2i18n( "Quick Controls" ) );
    m_controlsBox->setColumnLayout(0, Qt::Vertical );
    m_controlsBox->layout()->setSpacing( 6 );
    m_controlsBox->layout()->setMargin( 11 );
    m_controlsBoxLayout = new QVBoxLayout( m_controlsBox->layout() );
    m_controlsBoxLayout->setAlignment( Qt::AlignTop );

    m_affectAllApps = new QCheckBox( m_controlsBox, "m_affectAllApps" );
    m_affectAllApps->setText( tr2i18n( "Apply to all App&lications" ) );
    m_controlsBoxLayout->addWidget( m_affectAllApps );

    Layout12 = new QHBoxLayout( 0, 0, 6, "Layout12"); 
    QSpacerItem* spacer_5 = new QSpacerItem( 30, 20, QSizePolicy::Fixed, QSizePolicy::Minimum );
    Layout12->addItem( spacer_5 );

    Layout4 = new QGridLayout( 0, 1, 1, 0, 6, "Layout4"); 

    m_buttonDisable = new QPushButton( m_controlsBox, "m_buttonDisable" );
    m_buttonDisable->setText( tr2i18n( "Turn O&ff All:" ) );
    QToolTip::add( m_buttonDisable, tr2i18n( "Allows you to change the behavior for all events at once" ) );

    Layout4->addWidget( m_buttonDisable, 1, 0 );

    m_buttonEnable = new QPushButton( m_controlsBox, "m_buttonEnable" );
    m_buttonEnable->setText( tr2i18n( "Turn O&n All:" ) );
    QToolTip::add( m_buttonEnable, tr2i18n( "Allows you to change the behavior for all events at once" ) );

    Layout4->addWidget( m_buttonEnable, 0, 0 );

    m_comboEnable = new KComboBox( FALSE, m_controlsBox, "m_comboEnable" );

    Layout4->addWidget( m_comboEnable, 0, 1 );

    m_comboDisable = new KComboBox( FALSE, m_controlsBox, "m_comboDisable" );

    Layout4->addWidget( m_comboDisable, 1, 1 );
    Layout12->addLayout( Layout4 );
    QSpacerItem* spacer_6 = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout12->addItem( spacer_6 );
    m_controlsBoxLayout->addLayout( Layout12 );
    KNotifyWidgetBaseLayout->addWidget( m_controlsBox );

    // signals and slots connections
    connect( m_messageBox, SIGNAL( toggled(bool) ), m_passivePopup, SLOT( setEnabled(bool) ) );
    connect( m_playSound, SIGNAL( toggled(bool) ), m_soundPath, SLOT( setEnabled(bool) ) );
    connect( m_logToFile, SIGNAL( toggled(bool) ), m_logfilePath, SLOT( setEnabled(bool) ) );
    connect( m_execute, SIGNAL( toggled(bool) ), m_executePath, SLOT( setEnabled(bool) ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
KNotifyWidgetBase::~KNotifyWidgetBase()
{
    // no need to delete child widgets, Qt does it all for us
}

#include "knotifywidgetbase.moc"
