#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file '../../../kdeprint/cups/kcupshpgl2pagebase.ui'
**
** Created: Thu Mar 8 00:35:20 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include <kdeprint/cups/kcupshpgl2pagebase.h>

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include "kprintdialogpage.h"
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a KCupsHPGL2PageBase which is a child of 'parent', with the 
 *  name 'name'.' 
 */
KCupsHPGL2PageBase::KCupsHPGL2PageBase( QWidget* parent,  const char* name )
    : KPrintDialogPage( parent, name )
{
    if ( !name )
	setName( "KCupsHPGL2PageBase" );
    resize( 347, 394 ); 
    setCaption( i18n( "KPrintDialogPageForm" ) );
    KCupsHPGL2PageBaseLayout = new QVBoxLayout( this ); 
    KCupsHPGL2PageBaseLayout->setSpacing( 6 );
    KCupsHPGL2PageBaseLayout->setMargin( 11 );

    hpglbox_ = new QGroupBox( this, "hpglbox_" );
    hpglbox_->setTitle( i18n( "HP-GL/2 options" ) );
    hpglbox_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, hpglbox_->sizePolicy().hasHeightForWidth() ) );
    hpglbox_->setColumnLayout(0, Qt::Vertical );
    hpglbox_->layout()->setSpacing( 0 );
    hpglbox_->layout()->setMargin( 0 );
    hpglbox_Layout = new QVBoxLayout( hpglbox_->layout() );
    hpglbox_Layout->setAlignment( Qt::AlignTop );
    hpglbox_Layout->setSpacing( 6 );
    hpglbox_Layout->setMargin( 20 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    hpglbox_Layout->addItem( spacer );

    black_ = new QCheckBox( hpglbox_, "black_" );
    black_->setText( i18n( "Use only black pen" ) );
    hpglbox_Layout->addWidget( black_ );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    hpglbox_Layout->addItem( spacer_2 );

    fit_ = new QCheckBox( hpglbox_, "fit_" );
    fit_->setText( i18n( "Fit plot to page" ) );
    hpglbox_Layout->addWidget( fit_ );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    hpglbox_Layout->addItem( spacer_3 );

    Layout5 = new QHBoxLayout; 
    Layout5->setSpacing( 6 );
    Layout5->setMargin( 0 );

    penwidthlabel_ = new QLabel( hpglbox_, "penwidthlabel_" );
    penwidthlabel_->setText( i18n( "Pen width:" ) );
    penwidthlabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, penwidthlabel_->sizePolicy().hasHeightForWidth() ) );
    Layout5->addWidget( penwidthlabel_ );

    penwidth_ = new QSpinBox( hpglbox_, "penwidth_" );
    penwidth_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, penwidth_->sizePolicy().hasHeightForWidth() ) );
    penwidth_->setMinimumSize( QSize( 100, 0 ) );
    penwidth_->setValue( 99 );
    penwidth_->setMinValue( 1 );
    penwidth_->setMaxValue( 10000 );
    penwidth_->setLineStep( 25 );
    Layout5->addWidget( penwidth_ );

    unitlabel_ = new QLabel( hpglbox_, "unitlabel_" );
    unitlabel_->setText( i18n( "[um]" ) );
    unitlabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, unitlabel_->sizePolicy().hasHeightForWidth() ) );
    Layout5->addWidget( unitlabel_ );
    hpglbox_Layout->addLayout( Layout5 );
    QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    hpglbox_Layout->addItem( spacer_4 );
    KCupsHPGL2PageBaseLayout->addWidget( hpglbox_ );
    QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    KCupsHPGL2PageBaseLayout->addItem( spacer_5 );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
KCupsHPGL2PageBase::~KCupsHPGL2PageBase()
{
    // no need to delete child widgets, Qt does it all for us
}

