#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file '../../../kdeprint/cups/kcupsgeneralpagebase.ui'
**
** Created: Thu Mar 8 00:35:18 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include <kdeprint/cups/kcupsgeneralpagebase.h>

#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include "kprintdialogpage.h"
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/* 
 *  Constructs a KCupsGeneralPageBase which is a child of 'parent', with the 
 *  name 'name'.' 
 */
KCupsGeneralPageBase::KCupsGeneralPageBase( QWidget* parent,  const char* name )
    : KPrintDialogPage( parent, name )
{
    if ( !name )
	setName( "KCupsGeneralPageBase" );
    resize( 390, 404 ); 
    setCaption( i18n( "KCupsGeneralPageBase" ) );
    KCupsGeneralPageBaseLayout = new QVBoxLayout( this ); 
    KCupsGeneralPageBaseLayout->setSpacing( 10 );
    KCupsGeneralPageBaseLayout->setMargin( 11 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    KCupsGeneralPageBaseLayout->addItem( spacer );

    Layout12 = new QGridLayout; 
    Layout12->setSpacing( 3 );
    Layout12->setMargin( 0 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 6, QSizePolicy::Minimum, QSizePolicy::Fixed );
    Layout12->addItem( spacer_2, 3, 0 );

    source_ = new QComboBox( FALSE, this, "source_" );
    source_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)1, source_->sizePolicy().hasHeightForWidth() ) );
    source_->setMinimumSize( QSize( 0, 25 ) );

    Layout12->addWidget( source_, 4, 1 );

    sizelabel_ = new QLabel( this, "sizelabel_" );
    sizelabel_->setText( i18n( "Paper size:" ) );
    sizelabel_->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    sizelabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, sizelabel_->sizePolicy().hasHeightForWidth() ) );

    Layout12->addWidget( sizelabel_, 0, 0 );

    size_ = new QComboBox( FALSE, this, "size_" );
    size_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)1, size_->sizePolicy().hasHeightForWidth() ) );
    size_->setMinimumSize( QSize( 0, 25 ) );

    Layout12->addWidget( size_, 0, 1 );

    typelabel_ = new QLabel( this, "typelabel_" );
    typelabel_->setText( i18n( "Paper type:" ) );
    typelabel_->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    typelabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, typelabel_->sizePolicy().hasHeightForWidth() ) );

    Layout12->addWidget( typelabel_, 2, 0 );

    type_ = new QComboBox( FALSE, this, "type_" );
    type_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)1, type_->sizePolicy().hasHeightForWidth() ) );
    type_->setMinimumSize( QSize( 0, 25 ) );

    Layout12->addWidget( type_, 2, 1 );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 6, QSizePolicy::Minimum, QSizePolicy::Fixed );
    Layout12->addItem( spacer_3, 1, 0 );

    sourcelabel_ = new QLabel( this, "sourcelabel_" );
    sourcelabel_->setText( i18n( "Paper source:" ) );
    sourcelabel_->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    sourcelabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, sourcelabel_->sizePolicy().hasHeightForWidth() ) );

    Layout12->addWidget( sourcelabel_, 4, 0 );
    KCupsGeneralPageBaseLayout->addLayout( Layout12 );
    QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    KCupsGeneralPageBaseLayout->addItem( spacer_4 );

    Layout15 = new QGridLayout; 
    Layout15->setSpacing( 6 );
    Layout15->setMargin( 0 );

    nupbox_ = new QButtonGroup( this, "nupbox_" );
    nupbox_->setTitle( i18n( "Pages per sheet" ) );
    nupbox_->setExclusive( TRUE );
    nupbox_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, nupbox_->sizePolicy().hasHeightForWidth() ) );
    nupbox_->setMinimumSize( QSize( 180, 120 ) );
    nupbox_->setColumnLayout(0, Qt::Vertical );
    nupbox_->layout()->setSpacing( 0 );
    nupbox_->layout()->setMargin( 0 );
    nupbox_Layout = new QHBoxLayout( nupbox_->layout() );
    nupbox_Layout->setAlignment( Qt::AlignTop );
    nupbox_Layout->setSpacing( 6 );
    nupbox_Layout->setMargin( 11 );

    Layout14 = new QVBoxLayout; 
    Layout14->setSpacing( 6 );
    Layout14->setMargin( 0 );

    nup1_ = new QRadioButton( nupbox_, "nup1_" );
    nup1_->setText( i18n( "1" ) );
    nup1_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, nup1_->sizePolicy().hasHeightForWidth() ) );
    nupbox_->insert( nup1_, -1 );
    Layout14->addWidget( nup1_ );

    nup2_ = new QRadioButton( nupbox_, "nup2_" );
    nup2_->setText( i18n( "2" ) );
    nup2_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, nup2_->sizePolicy().hasHeightForWidth() ) );
    nupbox_->insert( nup2_, -1 );
    Layout14->addWidget( nup2_ );

    nup4_ = new QRadioButton( nupbox_, "nup4_" );
    nup4_->setText( i18n( "4" ) );
    nup4_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, nup4_->sizePolicy().hasHeightForWidth() ) );
    nupbox_->insert( nup4_, -1 );
    Layout14->addWidget( nup4_ );
    nupbox_Layout->addLayout( Layout14 );

    nuppix_ = new QLabel( nupbox_, "nuppix_" );
    nuppix_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, nuppix_->sizePolicy().hasHeightForWidth() ) );
    nuppix_->setAlignment( int( QLabel::AlignCenter ) );
    nupbox_Layout->addWidget( nuppix_ );

    Layout15->addWidget( nupbox_, 1, 1 );

    orientbox_ = new QButtonGroup( this, "orientbox_" );
    orientbox_->setTitle( i18n( "Orientation" ) );
    orientbox_->setExclusive( TRUE );
    orientbox_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, orientbox_->sizePolicy().hasHeightForWidth() ) );
    orientbox_->setMinimumSize( QSize( 180, 120 ) );
    orientbox_->setColumnLayout(0, Qt::Vertical );
    orientbox_->layout()->setSpacing( 0 );
    orientbox_->layout()->setMargin( 0 );
    orientbox_Layout = new QHBoxLayout( orientbox_->layout() );
    orientbox_Layout->setAlignment( Qt::AlignTop );
    orientbox_Layout->setSpacing( 6 );
    orientbox_Layout->setMargin( 11 );

    Layout12_2 = new QVBoxLayout; 
    Layout12_2->setSpacing( 6 );
    Layout12_2->setMargin( 0 );

    portrait_ = new QRadioButton( orientbox_, "portrait_" );
    portrait_->setText( i18n( "Portrait" ) );
    portrait_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, portrait_->sizePolicy().hasHeightForWidth() ) );
    orientbox_->insert( portrait_, -1 );
    Layout12_2->addWidget( portrait_ );

    landscape_ = new QRadioButton( orientbox_, "landscape_" );
    landscape_->setText( i18n( "Landscape" ) );
    landscape_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, landscape_->sizePolicy().hasHeightForWidth() ) );
    orientbox_->insert( landscape_, -1 );
    Layout12_2->addWidget( landscape_ );

    reverse_ = new QRadioButton( orientbox_, "reverse_" );
    reverse_->setText( i18n( "Reverse" ) );
    reverse_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, reverse_->sizePolicy().hasHeightForWidth() ) );
    orientbox_->insert( reverse_, -1 );
    Layout12_2->addWidget( reverse_ );

    reverseportrait_ = new QRadioButton( orientbox_, "reverseportrait_" );
    reverseportrait_->setText( i18n( "Reverse portrait" ) );
    reverseportrait_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, reverseportrait_->sizePolicy().hasHeightForWidth() ) );
    orientbox_->insert( reverseportrait_, -1 );
    Layout12_2->addWidget( reverseportrait_ );
    orientbox_Layout->addLayout( Layout12_2 );

    orientpix_ = new QLabel( orientbox_, "orientpix_" );
    orientpix_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, orientpix_->sizePolicy().hasHeightForWidth() ) );
    orientpix_->setAlignment( int( QLabel::AlignCenter ) );
    orientbox_Layout->addWidget( orientpix_ );

    Layout15->addWidget( orientbox_, 0, 0 );

    duplexbox_ = new QButtonGroup( this, "duplexbox_" );
    duplexbox_->setTitle( i18n( "Duplex printing" ) );
    duplexbox_->setExclusive( TRUE );
    duplexbox_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, duplexbox_->sizePolicy().hasHeightForWidth() ) );
    duplexbox_->setMinimumSize( QSize( 180, 120 ) );
    duplexbox_->setColumnLayout(0, Qt::Vertical );
    duplexbox_->layout()->setSpacing( 0 );
    duplexbox_->layout()->setMargin( 0 );
    duplexbox_Layout = new QHBoxLayout( duplexbox_->layout() );
    duplexbox_Layout->setAlignment( Qt::AlignTop );
    duplexbox_Layout->setSpacing( 6 );
    duplexbox_Layout->setMargin( 11 );

    Layout13 = new QVBoxLayout; 
    Layout13->setSpacing( 6 );
    Layout13->setMargin( 0 );

    normal_ = new QRadioButton( duplexbox_, "normal_" );
    normal_->setText( i18n( "None" ) );
    normal_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, normal_->sizePolicy().hasHeightForWidth() ) );
    duplexbox_->insert( normal_, -1 );
    Layout13->addWidget( normal_ );

    book_ = new QRadioButton( duplexbox_, "book_" );
    book_->setText( i18n( "Long edge" ) );
    book_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, book_->sizePolicy().hasHeightForWidth() ) );
    duplexbox_->insert( book_, -1 );
    Layout13->addWidget( book_ );

    tablet_ = new QRadioButton( duplexbox_, "tablet_" );
    tablet_->setText( i18n( "Short edge" ) );
    tablet_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, tablet_->sizePolicy().hasHeightForWidth() ) );
    duplexbox_->insert( tablet_, -1 );
    Layout13->addWidget( tablet_ );
    duplexbox_Layout->addLayout( Layout13 );

    duplexpix_ = new QLabel( duplexbox_, "duplexpix_" );
    duplexpix_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, duplexpix_->sizePolicy().hasHeightForWidth() ) );
    duplexpix_->setAlignment( int( QLabel::AlignCenter ) );
    duplexbox_Layout->addWidget( duplexpix_ );

    Layout15->addWidget( duplexbox_, 0, 1 );

    bannerbox_ = new QGroupBox( this, "bannerbox_" );
    bannerbox_->setTitle( i18n( "Banners" ) );
    bannerbox_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, bannerbox_->sizePolicy().hasHeightForWidth() ) );
    bannerbox_->setMinimumSize( QSize( 180, 120 ) );
    bannerbox_->setColumnLayout(0, Qt::Vertical );
    bannerbox_->layout()->setSpacing( 0 );
    bannerbox_->layout()->setMargin( 0 );
    bannerbox_Layout = new QVBoxLayout( bannerbox_->layout() );
    bannerbox_Layout->setAlignment( Qt::AlignTop );
    bannerbox_Layout->setSpacing( 6 );
    bannerbox_Layout->setMargin( 11 );
    QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    bannerbox_Layout->addItem( spacer_5 );

    Layout6 = new QHBoxLayout; 
    Layout6->setSpacing( 6 );
    Layout6->setMargin( 0 );

    startbannerlabel_ = new QLabel( bannerbox_, "startbannerlabel_" );
    startbannerlabel_->setText( i18n( "Start:" ) );
    startbannerlabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, startbannerlabel_->sizePolicy().hasHeightForWidth() ) );
    Layout6->addWidget( startbannerlabel_ );

    startbanner_ = new QComboBox( FALSE, bannerbox_, "startbanner_" );
    startbanner_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, startbanner_->sizePolicy().hasHeightForWidth() ) );
    startbanner_->setEnabled( TRUE );
    Layout6->addWidget( startbanner_ );
    bannerbox_Layout->addLayout( Layout6 );
    QSpacerItem* spacer_6 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    bannerbox_Layout->addItem( spacer_6 );

    Layout7 = new QHBoxLayout; 
    Layout7->setSpacing( 6 );
    Layout7->setMargin( 0 );

    stopbannerlabel_ = new QLabel( bannerbox_, "stopbannerlabel_" );
    stopbannerlabel_->setText( i18n( "End:" ) );
    stopbannerlabel_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, stopbannerlabel_->sizePolicy().hasHeightForWidth() ) );
    Layout7->addWidget( stopbannerlabel_ );

    stopbanner_ = new QComboBox( FALSE, bannerbox_, "stopbanner_" );
    stopbanner_->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, stopbanner_->sizePolicy().hasHeightForWidth() ) );
    stopbanner_->setEnabled( TRUE );
    Layout7->addWidget( stopbanner_ );
    bannerbox_Layout->addLayout( Layout7 );
    QSpacerItem* spacer_7 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    bannerbox_Layout->addItem( spacer_7 );

    Layout15->addWidget( bannerbox_, 1, 0 );
    KCupsGeneralPageBaseLayout->addLayout( Layout15 );

    // signals and slots connections
}

/*  
 *  Destroys the object and frees any allocated resources
 */
KCupsGeneralPageBase::~KCupsGeneralPageBase()
{
    // no need to delete child widgets, Qt does it all for us
}

void KCupsGeneralPageBase::orientationChanged(int)
{
    qWarning( "KCupsGeneralPageBase::orientationChanged(int): Not implemented yet!" );
}

void KCupsGeneralPageBase::nupChanged(int)
{
    qWarning( "KCupsGeneralPageBase::nupChanged(int): Not implemented yet!" );
}

void KCupsGeneralPageBase::duplexChanged(int)
{
    qWarning( "KCupsGeneralPageBase::duplexChanged(int): Not implemented yet!" );
}

