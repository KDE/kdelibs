#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file '../../kdeprint/kprintcopiespagebase.ui'
**
** Created: Thu Mar 8 00:28:59 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "../../kdeprint/kprintcopiespagebase.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include "kprintdialogpage.h"
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a KPrintCopiesPageBase which is a child of 'parent', with the 
 *  name 'name'.' 
 */
KPrintCopiesPageBase::KPrintCopiesPageBase( QWidget* parent,  const char* name )
    : KPrintDialogPage( parent, name )
{
    if ( !name )
	setName( "KPrintCopiesPageBase" );
    resize( 495, 201 ); 
    setCaption( i18n( "KPrintCopiesPageBase" ) );
    KPrintCopiesPageBaseLayout = new QGridLayout( this ); 
    KPrintCopiesPageBaseLayout->setSpacing( 6 );
    KPrintCopiesPageBaseLayout->setMargin( 0 );

    copiesbox__2 = new QGroupBox( this, "copiesbox__2" );
    copiesbox__2->setFrameShadow( QGroupBox::Sunken );
    copiesbox__2->setFrameShape( QGroupBox::Box );
    copiesbox__2->setTitle( i18n( "Copies" ) );
    copiesbox__2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)7, copiesbox__2->sizePolicy().hasHeightForWidth() ) );
    copiesbox__2->setColumnLayout(0, Qt::Vertical );
    copiesbox__2->layout()->setSpacing( 0 );
    copiesbox__2->layout()->setMargin( 0 );
    copiesbox__2Layout = new QVBoxLayout( copiesbox__2->layout() );
    copiesbox__2Layout->setAlignment( Qt::AlignTop );
    copiesbox__2Layout->setSpacing( 6 );
    copiesbox__2Layout->setMargin( 11 );

    Layout5 = new QHBoxLayout; 
    Layout5->setSpacing( 6 );
    Layout5->setMargin( 0 );

    copieslabel_ = new QLabel( copiesbox__2, "copieslabel_" );
    copieslabel_->setText( i18n( "Number of copies:" ) );
    copieslabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, copieslabel_->sizePolicy().hasHeightForWidth() ) );
    Layout5->addWidget( copieslabel_ );

    copies_ = new QSpinBox( copiesbox__2, "copies_" );
    copies_->setMinValue( 1 );
    copies_->setValue( 1 );
    copies_->setMaxValue( 999 );
    copies_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, copies_->sizePolicy().hasHeightForWidth() ) );
    Layout5->addWidget( copies_ );
    copiesbox__2Layout->addLayout( Layout5 );

    Layout4 = new QHBoxLayout; 
    Layout4->setSpacing( 6 );
    Layout4->setMargin( 0 );

    copiespix_ = new QLabel( copiesbox__2, "copiespix_" );
    copiespix_->setScaledContents( FALSE );
    copiespix_->setMinimumSize( QSize( 100, 50 ) );
    Layout4->addWidget( copiespix_ );

    Layout2 = new QVBoxLayout; 
    Layout2->setSpacing( 10 );
    Layout2->setMargin( 10 );

    collate_ = new QCheckBox( copiesbox__2, "collate_" );
    collate_->setText( i18n( "Collate copies" ) );
    collate_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, collate_->sizePolicy().hasHeightForWidth() ) );
    Layout2->addWidget( collate_ );

    reverse_ = new QCheckBox( copiesbox__2, "reverse_" );
    reverse_->setText( i18n( "Reverse order" ) );
    reverse_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, reverse_->sizePolicy().hasHeightForWidth() ) );
    Layout2->addWidget( reverse_ );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout2->addItem( spacer );
    Layout4->addLayout( Layout2 );
    copiesbox__2Layout->addLayout( Layout4 );

    KPrintCopiesPageBaseLayout->addWidget( copiesbox__2, 0, 1 );

    pagebox__2 = new QButtonGroup( this, "pagebox__2" );
    pagebox__2->setTitle( i18n( "Page selection" ) );
    pagebox__2->setFrameShadow( QButtonGroup::Sunken );
    pagebox__2->setFrameShape( QButtonGroup::Box );
    pagebox__2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, pagebox__2->sizePolicy().hasHeightForWidth() ) );
    pagebox__2->setExclusive( TRUE );
    pagebox__2->setColumnLayout(0, Qt::Vertical );
    pagebox__2->layout()->setSpacing( 0 );
    pagebox__2->layout()->setMargin( 0 );
    pagebox__2Layout = new QVBoxLayout( pagebox__2->layout() );
    pagebox__2Layout->setAlignment( Qt::AlignTop );
    pagebox__2Layout->setSpacing( 6 );
    pagebox__2Layout->setMargin( 11 );

    Layout10 = new QVBoxLayout; 
    Layout10->setSpacing( 6 );
    Layout10->setMargin( 0 );

    all_ = new QRadioButton( pagebox__2, "all_" );
    all_->setText( i18n( "All pages" ) );
    all_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, all_->sizePolicy().hasHeightForWidth() ) );
    Layout10->addWidget( all_ );

    current_ = new QRadioButton( pagebox__2, "current_" );
    current_->setText( i18n( "Current page" ) );
    current_->setEnabled( FALSE );
    current_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, current_->sizePolicy().hasHeightForWidth() ) );
    Layout10->addWidget( current_ );

    Layout8 = new QHBoxLayout; 
    Layout8->setSpacing( 6 );
    Layout8->setMargin( 0 );

    range_ = new QRadioButton( pagebox__2, "range_" );
    range_->setText( i18n( "Pages:" ) );
    range_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, range_->sizePolicy().hasHeightForWidth() ) );
    Layout8->addWidget( range_ );

    rangeedit_ = new QLineEdit( pagebox__2, "rangeedit_" );
    rangeedit_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)1, rangeedit_->sizePolicy().hasHeightForWidth() ) );
    Layout8->addWidget( rangeedit_ );
    Layout10->addLayout( Layout8 );
    pagebox__2Layout->addLayout( Layout10 );

    rangeexpl_ = new QLabel( pagebox__2, "rangeexpl_" );
    rangeexpl_->setText( i18n( "<p>Enter page numbers and/or groups of pages to print separated by commas (1,2-5,10-12,17).</p>" ) );
    rangeexpl_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, rangeexpl_->sizePolicy().hasHeightForWidth() ) );
    pagebox__2Layout->addWidget( rangeexpl_ );

    KPrintCopiesPageBaseLayout->addWidget( pagebox__2, 0, 0 );

    Layout1 = new QHBoxLayout; 
    Layout1->setSpacing( 6 );
    Layout1->setMargin( 0 );

    pagelabel_ = new QLabel( this, "pagelabel_" );
    pagelabel_->setText( i18n( "Print:" ) );
    pagelabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, pagelabel_->sizePolicy().hasHeightForWidth() ) );
    Layout1->addWidget( pagelabel_ );

    pagecombo_ = new QComboBox( FALSE, this, "pagecombo_" );
    pagecombo_->insertItem( i18n( "All pages" ) );
    pagecombo_->insertItem( i18n( "Odd pages" ) );
    pagecombo_->insertItem( i18n( "Even pages" ) );
    pagecombo_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, pagecombo_->sizePolicy().hasHeightForWidth() ) );
    Layout1->addWidget( pagecombo_ );

    KPrintCopiesPageBaseLayout->addLayout( Layout1, 1, 1 );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
KPrintCopiesPageBase::~KPrintCopiesPageBase()
{
    // no need to delete child widgets, Qt does it all for us
}

