#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file '../../../kdeprint/cups/kcupstextpagebase.ui'
**
** Created: Thu Mar 8 00:35:19 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "../../../kdeprint/cups/kcupstextpagebase.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include "columnpreview.h"
#include "kprintdialogpage.h"
#include "marginpreview.h"
#include "qclineedit.h"
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a KCupsTextPageBase which is a child of 'parent', with the 
 *  name 'name'.' 
 */
KCupsTextPageBase::KCupsTextPageBase( QWidget* parent,  const char* name )
    : KPrintDialogPage( parent, name )
{
    if ( !name )
	setName( "KCupsTextPageBase" );
    resize( 345, 421 ); 
    setCaption( i18n( "KPrintDialogPageForm" ) );
    KCupsTextPageBaseLayout = new QVBoxLayout( this ); 
    KCupsTextPageBaseLayout->setSpacing( 6 );
    KCupsTextPageBaseLayout->setMargin( 11 );

    Layout7 = new QHBoxLayout; 
    Layout7->setSpacing( 6 );
    Layout7->setMargin( 0 );

    formatbox_ = new QGroupBox( this, "formatbox_" );
    formatbox_->setTitle( i18n( "Text format" ) );
    formatbox_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)1, formatbox_->sizePolicy().hasHeightForWidth() ) );
    formatbox_->setColumnLayout(0, Qt::Vertical );
    formatbox_->layout()->setSpacing( 0 );
    formatbox_->layout()->setMargin( 0 );
    formatbox_Layout = new QVBoxLayout( formatbox_->layout() );
    formatbox_Layout->setAlignment( Qt::AlignTop );
    formatbox_Layout->setSpacing( 6 );
    formatbox_Layout->setMargin( 11 );

    Layout4 = new QGridLayout; 
    Layout4->setSpacing( 6 );
    Layout4->setMargin( 0 );

    lpi_ = new QSpinBox( formatbox_, "lpi_" );
    lpi_->setMinValue( 1 );
    lpi_->setValue( 6 );

    Layout4->addWidget( lpi_, 1, 1 );

    lpilabel_ = new QLabel( formatbox_, "lpilabel_" );
    lpilabel_->setText( i18n( "Lines per inch:" ) );
    lpilabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, lpilabel_->sizePolicy().hasHeightForWidth() ) );

    Layout4->addWidget( lpilabel_, 1, 0 );

    cpi_ = new QSpinBox( formatbox_, "cpi_" );
    cpi_->setMinValue( 1 );
    cpi_->setValue( 10 );

    Layout4->addWidget( cpi_, 0, 1 );

    cpilabel_ = new QLabel( formatbox_, "cpilabel_" );
    cpilabel_->setText( i18n( "Chars per inch:" ) );
    cpilabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, cpilabel_->sizePolicy().hasHeightForWidth() ) );

    Layout4->addWidget( cpilabel_, 0, 0 );
    formatbox_Layout->addLayout( Layout4 );

    separator_ = new QFrame( formatbox_, "separator_" );
    separator_->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    separator_->setMinimumSize( QSize( 0, 7 ) );
    formatbox_Layout->addWidget( separator_ );

    Layout8 = new QHBoxLayout; 
    Layout8->setSpacing( 6 );
    Layout8->setMargin( 0 );

    Layout7_2 = new QVBoxLayout; 
    Layout7_2->setSpacing( 0 );
    Layout7_2->setMargin( 0 );

    columnlabel_ = new QLabel( formatbox_, "columnlabel_" );
    columnlabel_->setText( i18n( "Columns:" ) );
    columnlabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, columnlabel_->sizePolicy().hasHeightForWidth() ) );
    Layout7_2->addWidget( columnlabel_ );

    column_ = new QSpinBox( formatbox_, "column_" );
    column_->setValue( 1 );
    column_->setMinValue( 1 );
    column_->setMaxValue( 10 );
    column_->setMinimumSize( QSize( 70, 0 ) );
    Layout7_2->addWidget( column_ );
    Layout8->addLayout( Layout7_2 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout8->addItem( spacer );

    columnpreview_ = new ColumnPreview( formatbox_, "columnpreview_" );
    columnpreview_->setMinimumSize( QSize( 60, 60 ) );
    columnpreview_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, columnpreview_->sizePolicy().hasHeightForWidth() ) );
    Layout8->addWidget( columnpreview_ );
    formatbox_Layout->addLayout( Layout8 );
    Layout7->addWidget( formatbox_ );

    prettybox_ = new QButtonGroup( this, "prettybox_" );
    prettybox_->setTitle( i18n( "Syntax coloring (C/C++)" ) );
    prettybox_->setExclusive( TRUE );
    prettybox_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)1, prettybox_->sizePolicy().hasHeightForWidth() ) );
    prettybox_->setColumnLayout(0, Qt::Vertical );
    prettybox_->layout()->setSpacing( 0 );
    prettybox_->layout()->setMargin( 0 );
    prettybox_Layout = new QHBoxLayout( prettybox_->layout() );
    prettybox_Layout->setAlignment( Qt::AlignTop );
    prettybox_Layout->setSpacing( 6 );
    prettybox_Layout->setMargin( 11 );

    Layout6 = new QVBoxLayout; 
    Layout6->setSpacing( 6 );
    Layout6->setMargin( 10 );

    prettyon_ = new QRadioButton( prettybox_, "prettyon_" );
    prettyon_->setText( i18n( "On" ) );
    prettyon_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, prettyon_->sizePolicy().hasHeightForWidth() ) );
    prettybox_->insert( prettyon_, -1 );
    Layout6->addWidget( prettyon_ );

    prettyoff_ = new QRadioButton( prettybox_, "prettyoff_" );
    prettyoff_->setText( i18n( "Off" ) );
    prettyoff_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, prettyoff_->sizePolicy().hasHeightForWidth() ) );
    prettybox_->insert( prettyoff_, -1 );
    Layout6->addWidget( prettyoff_ );
    prettybox_Layout->addLayout( Layout6 );

    prettypix_ = new QLabel( prettybox_, "prettypix_" );
    prettypix_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, prettypix_->sizePolicy().hasHeightForWidth() ) );
    prettypix_->setAlignment( int( QLabel::AlignCenter ) );
    prettybox_Layout->addWidget( prettypix_ );
    Layout7->addWidget( prettybox_ );
    KCupsTextPageBaseLayout->addLayout( Layout7 );

    marginbox_ = new QGroupBox( this, "marginbox_" );
    marginbox_->setTitle( i18n( "Page margins" ) );
    marginbox_->setMinimumSize( QSize( 0, 0 ) );
    marginbox_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)7, marginbox_->sizePolicy().hasHeightForWidth() ) );
    marginbox_->setColumnLayout(0, Qt::Vertical );
    marginbox_->layout()->setSpacing( 0 );
    marginbox_->layout()->setMargin( 0 );
    marginbox_Layout = new QHBoxLayout( marginbox_->layout() );
    marginbox_Layout->setAlignment( Qt::AlignTop );
    marginbox_Layout->setSpacing( 6 );
    marginbox_Layout->setMargin( 11 );

    Layout7_3 = new QGridLayout; 
    Layout7_3->setSpacing( 6 );
    Layout7_3->setMargin( 0 );

    unitslabel_ = new QLabel( marginbox_, "unitslabel_" );
    unitslabel_->setText( i18n( "Units:" ) );
    unitslabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, unitslabel_->sizePolicy().hasHeightForWidth() ) );

    Layout7_3->addWidget( unitslabel_, 6, 0 );

    left_ = new QCLineEdit( marginbox_, "left_" );

    Layout7_3->addWidget( left_, 3, 1 );

    bottomlabel_ = new QLabel( marginbox_, "bottomlabel_" );
    bottomlabel_->setText( i18n( "Bottom:" ) );
    bottomlabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, bottomlabel_->sizePolicy().hasHeightForWidth() ) );

    Layout7_3->addWidget( bottomlabel_, 2, 0 );

    bottom_ = new QCLineEdit( marginbox_, "bottom_" );

    Layout7_3->addWidget( bottom_, 2, 1 );

    margincustom_ = new QCheckBox( marginbox_, "margincustom_" );
    margincustom_->setText( i18n( "Use custom margins" ) );

    Layout7_3->addMultiCellWidget( margincustom_, 0, 0, 0, 1 );

    leftlabel_ = new QLabel( marginbox_, "leftlabel_" );
    leftlabel_->setText( i18n( "Left:" ) );
    leftlabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, leftlabel_->sizePolicy().hasHeightForWidth() ) );

    Layout7_3->addWidget( leftlabel_, 3, 0 );

    units_ = new QComboBox( FALSE, marginbox_, "units_" );
    units_->insertItem( i18n( "cm" ) );
    units_->insertItem( i18n( "in" ) );
    units_->insertItem( i18n( "pixels" ) );
    units_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, units_->sizePolicy().hasHeightForWidth() ) );

    Layout7_3->addWidget( units_, 6, 1 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 16, QSizePolicy::Minimum, QSizePolicy::Fixed );
    Layout7_3->addItem( spacer_2, 5, 0 );

    rightlabel_ = new QLabel( marginbox_, "rightlabel_" );
    rightlabel_->setText( i18n( "Right:" ) );
    rightlabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, rightlabel_->sizePolicy().hasHeightForWidth() ) );

    Layout7_3->addWidget( rightlabel_, 4, 0 );

    right_ = new QCLineEdit( marginbox_, "right_" );

    Layout7_3->addWidget( right_, 4, 1 );

    top_ = new QCLineEdit( marginbox_, "top_" );

    Layout7_3->addWidget( top_, 1, 1 );

    toplabel_ = new QLabel( marginbox_, "toplabel_" );
    toplabel_->setText( i18n( "Top:" ) );
    toplabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, toplabel_->sizePolicy().hasHeightForWidth() ) );

    Layout7_3->addWidget( toplabel_, 1, 0 );
    marginbox_Layout->addLayout( Layout7_3 );

    Layout6_2 = new QGridLayout; 
    Layout6_2->setSpacing( 6 );
    Layout6_2->setMargin( 0 );

    marginpreview_ = new MarginPreview( marginbox_, "marginpreview_" );
    marginpreview_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, marginpreview_->sizePolicy().hasHeightForWidth() ) );
    marginpreview_->setMinimumSize( QSize( 150, 170 ) );

    Layout6_2->addWidget( marginpreview_, 1, 1 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout6_2->addItem( spacer_3, 0, 1 );
    QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout6_2->addItem( spacer_4, 2, 1 );
    QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout6_2->addItem( spacer_5, 1, 0 );
    marginbox_Layout->addLayout( Layout6_2 );
    KCupsTextPageBaseLayout->addWidget( marginbox_ );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
KCupsTextPageBase::~KCupsTextPageBase()
{
    // no need to delete child widgets, Qt does it all for us
}

