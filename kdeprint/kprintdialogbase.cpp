#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file '../../kdeprint/kprintdialogbase.ui'
**
** Created: Thu Mar 8 00:28:58 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "../../kdeprint/kprintdialogbase.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a KPrintDialogBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
KPrintDialogBase::KPrintDialogBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "KPrintDialogBase" );
    resize( 483, 433 ); 
    setCaption( i18n( "Print" ) );
    KPrintDialogBaseLayout = new QVBoxLayout( this ); 
    KPrintDialogBaseLayout->setSpacing( 6 );
    KPrintDialogBaseLayout->setMargin( 11 );

    printerbox_ = new QGroupBox( this, "printerbox_" );
    printerbox_->setTitle( i18n( "Printer" ) );
    printerbox_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)5, printerbox_->sizePolicy().hasHeightForWidth() ) );
    printerbox_->setColumnLayout(0, Qt::Vertical );
    printerbox_->layout()->setSpacing( 0 );
    printerbox_->layout()->setMargin( 0 );
    printerbox_Layout = new QGridLayout( printerbox_->layout() );
    printerbox_Layout->setAlignment( Qt::AlignTop );
    printerbox_Layout->setSpacing( 6 );
    printerbox_Layout->setMargin( 11 );

    Layout20 = new QGridLayout; 
    Layout20->setSpacing( 6 );
    Layout20->setMargin( 0 );

    type_ = new QLabel( printerbox_, "type_" );
    type_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, type_->sizePolicy().hasHeightForWidth() ) );

    Layout20->addWidget( type_, 2, 1 );

    namelabel_ = new QLabel( printerbox_, "namelabel_" );
    namelabel_->setText( i18n( "Name:" ) );

    Layout20->addWidget( namelabel_, 0, 0 );

    location_ = new QLabel( printerbox_, "location_" );
    location_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, location_->sizePolicy().hasHeightForWidth() ) );

    Layout20->addWidget( location_, 3, 1 );

    TextLabel3 = new QLabel( printerbox_, "TextLabel3" );
    TextLabel3->setText( i18n( "State:" ) );

    Layout20->addWidget( TextLabel3, 1, 0 );

    TextLabel6 = new QLabel( printerbox_, "TextLabel6" );
    TextLabel6->setText( i18n( "Comment:" ) );

    Layout20->addWidget( TextLabel6, 4, 0 );

    TextLabel5 = new QLabel( printerbox_, "TextLabel5" );
    TextLabel5->setText( i18n( "Location:" ) );

    Layout20->addWidget( TextLabel5, 3, 0 );

    comment_ = new QLabel( printerbox_, "comment_" );
    comment_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, comment_->sizePolicy().hasHeightForWidth() ) );

    Layout20->addWidget( comment_, 4, 1 );

    state_ = new QLabel( printerbox_, "state_" );
    state_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, state_->sizePolicy().hasHeightForWidth() ) );

    Layout20->addWidget( state_, 1, 1 );

    TextLabel4 = new QLabel( printerbox_, "TextLabel4" );
    TextLabel4->setText( i18n( "Type:" ) );

    Layout20->addWidget( TextLabel4, 2, 0 );

    dest_ = new QComboBox( FALSE, printerbox_, "dest_" );
    dest_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, dest_->sizePolicy().hasHeightForWidth() ) );
    dest_->setMinimumSize( QSize( 0, 25 ) );

    Layout20->addWidget( dest_, 0, 1 );

    printerbox_Layout->addLayout( Layout20, 0, 0 );

    Layout16 = new QHBoxLayout; 
    Layout16->setSpacing( 6 );
    Layout16->setMargin( 0 );

    outputtofile_ = new QCheckBox( printerbox_, "outputtofile_" );
    outputtofile_->setText( i18n( "Print to file:" ) );
    Layout16->addWidget( outputtofile_ );

    outputfilename_ = new QLineEdit( printerbox_, "outputfilename_" );
    outputfilename_->setEnabled( FALSE );
    Layout16->addWidget( outputfilename_ );

    printerbox_Layout->addLayout( Layout16, 1, 0 );

    browsebutton_ = new QPushButton( printerbox_, "browsebutton_" );
    browsebutton_->setText( i18n( "Browse..." ) );
    browsebutton_->setEnabled( FALSE );

    printerbox_Layout->addWidget( browsebutton_, 1, 1 );

    Layout5 = new QVBoxLayout; 
    Layout5->setSpacing( 6 );
    Layout5->setMargin( 0 );

    propbutton_ = new QPushButton( printerbox_, "propbutton_" );
    propbutton_->setText( i18n( "Properties..." ) );
    propbutton_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, propbutton_->sizePolicy().hasHeightForWidth() ) );
    QToolTip::add(  propbutton_, i18n( "Configure selected printer" ) );
    Layout5->addWidget( propbutton_ );

    defaultbutton_ = new QPushButton( printerbox_, "defaultbutton_" );
    defaultbutton_->setText( i18n( "Set as default" ) );
    defaultbutton_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, defaultbutton_->sizePolicy().hasHeightForWidth() ) );
    QToolTip::add(  defaultbutton_, i18n( "Define selected printer as default" ) );
    Layout5->addWidget( defaultbutton_ );

    preview_ = new QCheckBox( printerbox_, "preview_" );
    preview_->setText( i18n( "Preview" ) );
    QToolTip::add(  preview_, i18n( "Show preview before printing" ) );
    Layout5->addWidget( preview_ );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout5->addItem( spacer );

    printerbox_Layout->addLayout( Layout5, 0, 1 );
    KPrintDialogBaseLayout->addWidget( printerbox_ );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 16, QSizePolicy::Minimum, QSizePolicy::Fixed );
    KPrintDialogBaseLayout->addItem( spacer_2 );

    Layout13 = new QHBoxLayout; 
    Layout13->setSpacing( 6 );
    Layout13->setMargin( 0 );

    optionsbutton_ = new QPushButton( this, "optionsbutton_" );
    optionsbutton_->setText( i18n( "Options..." ) );
    optionsbutton_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, optionsbutton_->sizePolicy().hasHeightForWidth() ) );
    Layout13->addWidget( optionsbutton_ );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout13->addItem( spacer_3 );

    okbutton_ = new QPushButton( this, "okbutton_" );
    okbutton_->setText( i18n( "OK" ) );
    okbutton_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, okbutton_->sizePolicy().hasHeightForWidth() ) );
    okbutton_->setDefault( TRUE );
    Layout13->addWidget( okbutton_ );

    cancelbutton_ = new QPushButton( this, "cancelbutton_" );
    cancelbutton_->setText( i18n( "Cancel" ) );
    cancelbutton_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, cancelbutton_->sizePolicy().hasHeightForWidth() ) );
    Layout13->addWidget( cancelbutton_ );
    KPrintDialogBaseLayout->addLayout( Layout13 );

    // signals and slots connections
    connect( cancelbutton_, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( dest_, SIGNAL( activated(int) ), this, SLOT( printerSelected(int) ) );
    connect( okbutton_, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( propbutton_, SIGNAL( clicked() ), this, SLOT( propertyClicked() ) );
    connect( defaultbutton_, SIGNAL( clicked() ), this, SLOT( setDefaultClicked() ) );
    connect( optionsbutton_, SIGNAL( clicked() ), this, SLOT( optionsClicked() ) );
    connect( outputtofile_, SIGNAL( toggled(bool) ), outputfilename_, SLOT( setEnabled(bool) ) );
    connect( outputtofile_, SIGNAL( toggled(bool) ), browsebutton_, SLOT( setEnabled(bool) ) );
    connect( outputtofile_, SIGNAL( toggled(bool) ), dest_, SLOT( setDisabled(bool) ) );
    connect( outputtofile_, SIGNAL( toggled(bool) ), defaultbutton_, SLOT( setDisabled(bool) ) );
    connect( outputtofile_, SIGNAL( toggled(bool) ), TextLabel6, SLOT( setDisabled(bool) ) );
    connect( outputtofile_, SIGNAL( toggled(bool) ), TextLabel5, SLOT( setDisabled(bool) ) );
    connect( outputtofile_, SIGNAL( toggled(bool) ), TextLabel4, SLOT( setDisabled(bool) ) );
    connect( outputtofile_, SIGNAL( toggled(bool) ), TextLabel3, SLOT( setDisabled(bool) ) );
    connect( outputtofile_, SIGNAL( toggled(bool) ), comment_, SLOT( setDisabled(bool) ) );
    connect( outputtofile_, SIGNAL( toggled(bool) ), location_, SLOT( setDisabled(bool) ) );
    connect( outputtofile_, SIGNAL( toggled(bool) ), state_, SLOT( setDisabled(bool) ) );
    connect( outputtofile_, SIGNAL( toggled(bool) ), type_, SLOT( setDisabled(bool) ) );
    connect( outputtofile_, SIGNAL( toggled(bool) ), namelabel_, SLOT( setDisabled(bool) ) );
    connect( browsebutton_, SIGNAL( clicked() ), this, SLOT( outputFileBrowseClicked() ) );
    connect( outputtofile_, SIGNAL( toggled(bool) ), preview_, SLOT( setDisabled(bool) ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
KPrintDialogBase::~KPrintDialogBase()
{
    // no need to delete child widgets, Qt does it all for us
}

void KPrintDialogBase::collateClicked()
{
    qWarning( "KPrintDialogBase::collateClicked(): Not implemented yet!" );
}

void KPrintDialogBase::optionsClicked()
{
    qWarning( "KPrintDialogBase::optionsClicked(): Not implemented yet!" );
}

void KPrintDialogBase::outputFileBrowseClicked()
{
    qWarning( "KPrintDialogBase::outputFileBrowseClicked(): Not implemented yet!" );
}

void KPrintDialogBase::printerSelected(int)
{
    qWarning( "KPrintDialogBase::printerSelected(int): Not implemented yet!" );
}

void KPrintDialogBase::propertyClicked()
{
    qWarning( "KPrintDialogBase::propertyClicked(): Not implemented yet!" );
}

void KPrintDialogBase::rangeEntered()
{
    qWarning( "KPrintDialogBase::rangeEntered(): Not implemented yet!" );
}

void KPrintDialogBase::setDefaultClicked()
{
    qWarning( "KPrintDialogBase::setDefaultClicked(): Not implemented yet!" );
}

