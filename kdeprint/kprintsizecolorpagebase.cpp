#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file '../../kdeprint/kprintsizecolorpagebase.ui'
**
** Created: Thu Mar 8 00:28:59 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "../../kdeprint/kprintsizecolorpagebase.h"

#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include "kprintdialogpage.h"
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a KPrintSizeColorPageBase which is a child of 'parent', with the 
 *  name 'name'.' 
 */
KPrintSizeColorPageBase::KPrintSizeColorPageBase( QWidget* parent,  const char* name )
    : KPrintDialogPage( parent, name )
{
    if ( !name )
	setName( "KPrintSizeColorPageBase" );
    resize( 391, 183 ); 
    setCaption( i18n( "KPrintSizeColorPageBase" ) );
    KPrintSizeColorPageBaseLayout = new QGridLayout( this ); 
    KPrintSizeColorPageBaseLayout->setSpacing( 6 );
    KPrintSizeColorPageBaseLayout->setMargin( 11 );

    orientbox_ = new QButtonGroup( this, "orientbox_" );
    orientbox_->setTitle( i18n( "Orientation" ) );
    orientbox_->setColumnLayout(0, Qt::Vertical );
    orientbox_->layout()->setSpacing( 0 );
    orientbox_->layout()->setMargin( 0 );
    orientbox_Layout = new QGridLayout( orientbox_->layout() );
    orientbox_Layout->setAlignment( Qt::AlignTop );
    orientbox_Layout->setSpacing( 6 );
    orientbox_Layout->setMargin( 15 );

    portrait_ = new QRadioButton( orientbox_, "portrait_" );
    portrait_->setText( i18n( "Portrait" ) );
    portrait_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)3, portrait_->sizePolicy().hasHeightForWidth() ) );

    orientbox_Layout->addWidget( portrait_, 0, 0 );

    landscape_ = new QRadioButton( orientbox_, "landscape_" );
    landscape_->setText( i18n( "Landscape" ) );
    landscape_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)3, landscape_->sizePolicy().hasHeightForWidth() ) );

    orientbox_Layout->addWidget( landscape_, 1, 0 );

    orientpix_ = new QLabel( orientbox_, "orientpix_" );
    orientpix_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, orientpix_->sizePolicy().hasHeightForWidth() ) );
    orientpix_->setMinimumSize( QSize( 60, 60 ) );
    orientpix_->setAlignment( int( QLabel::AlignCenter ) );

    orientbox_Layout->addMultiCellWidget( orientpix_, 0, 1, 1, 1 );

    KPrintSizeColorPageBaseLayout->addWidget( orientbox_, 2, 0 );

    size_ = new QComboBox( FALSE, this, "size_" );
    size_->setMinimumSize( QSize( 0, 25 ) );
    size_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, size_->sizePolicy().hasHeightForWidth() ) );

    KPrintSizeColorPageBaseLayout->addWidget( size_, 0, 1 );

    sizelabel_ = new QLabel( this, "sizelabel_" );
    sizelabel_->setText( i18n( "Paper size:" ) );
    sizelabel_->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    sizelabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, sizelabel_->sizePolicy().hasHeightForWidth() ) );

    KPrintSizeColorPageBaseLayout->addWidget( sizelabel_, 0, 0 );
    QSpacerItem* spacer = new QSpacerItem( 20, 16, QSizePolicy::Minimum, QSizePolicy::Fixed );
    KPrintSizeColorPageBaseLayout->addItem( spacer, 1, 0 );

    colorbox_ = new QButtonGroup( this, "colorbox_" );
    colorbox_->setTitle( i18n( "Color settings" ) );
    colorbox_->setColumnLayout(0, Qt::Vertical );
    colorbox_->layout()->setSpacing( 0 );
    colorbox_->layout()->setMargin( 0 );
    colorbox_Layout = new QGridLayout( colorbox_->layout() );
    colorbox_Layout->setAlignment( Qt::AlignTop );
    colorbox_Layout->setSpacing( 6 );
    colorbox_Layout->setMargin( 15 );

    colorpix_ = new QLabel( colorbox_, "colorpix_" );
    colorpix_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, colorpix_->sizePolicy().hasHeightForWidth() ) );
    colorpix_->setMinimumSize( QSize( 60, 60 ) );
    colorpix_->setAlignment( int( QLabel::AlignCenter ) );

    colorbox_Layout->addMultiCellWidget( colorpix_, 0, 1, 1, 1 );

    color_ = new QRadioButton( colorbox_, "color_" );
    color_->setText( i18n( "Color" ) );
    color_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)3, color_->sizePolicy().hasHeightForWidth() ) );

    colorbox_Layout->addWidget( color_, 0, 0 );

    grayscale_ = new QRadioButton( colorbox_, "grayscale_" );
    grayscale_->setText( i18n( "Grayscale" ) );
    grayscale_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)3, grayscale_->sizePolicy().hasHeightForWidth() ) );

    colorbox_Layout->addWidget( grayscale_, 1, 0 );

    KPrintSizeColorPageBaseLayout->addWidget( colorbox_, 2, 1 );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
KPrintSizeColorPageBase::~KPrintSizeColorPageBase()
{
    // no need to delete child widgets, Qt does it all for us
}

