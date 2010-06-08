#include "ratingpaintertestwidget.h"
#include "kratingwidget.h"

#include <QtGui/QGridLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

#include <kicon.h>
#include <kicondialog.h>
#include <knuminput.h>


RatingPainterTestWidget::RatingPainterTestWidget()
    : QWidget( 0 )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    m_ratingWidget = new KRatingWidget( this );
    m_ratingWidget->setFrameStyle( QFrame::StyledPanel|QFrame::Sunken );
    m_checkHalfSteps = new QCheckBox( "Enable half steps", this );
    m_checkEnalbed = new QCheckBox( "Rating disabled", this );
    m_spinMaxRating = new KIntSpinBox( this );
    m_spinRating = new KIntSpinBox( this );
    m_spinSpacing = new KIntSpinBox( this );

    m_comboAlignmentH = new QComboBox( this );
    m_comboAlignmentV = new QComboBox( this );

    m_buttonIcon = new QPushButton( "Change Icon...", this );

    m_comboAlignmentH->addItem( "Left" );
    m_comboAlignmentH->addItem( "Right" );
    m_comboAlignmentH->addItem( "Center" );
    m_comboAlignmentH->addItem( "Justify" );

    m_comboAlignmentV->addItem( "Top" );
    m_comboAlignmentV->addItem( "Bottom" );
    m_comboAlignmentV->addItem( "Center" );

    layout->addWidget( m_ratingWidget, 1 );
    layout->addWidget( new QLabel( "Alignment:" ) );
    layout->addWidget( m_comboAlignmentH );
    layout->addWidget( m_comboAlignmentV );
    layout->addWidget( m_checkHalfSteps );
    layout->addWidget( m_checkEnalbed );

    QHBoxLayout* ratingLayout = new QHBoxLayout;
    ratingLayout->addWidget( new QLabel( "Rating:", this ) );
    ratingLayout->addWidget( m_spinRating );
    layout->addLayout( ratingLayout );

    QHBoxLayout* maxRatingLayout = new QHBoxLayout;
    maxRatingLayout->addWidget( new QLabel( "Max Rating:", this ) );
    maxRatingLayout->addWidget( m_spinMaxRating );
    layout->addLayout( maxRatingLayout );

    QHBoxLayout* spacingLayout = new QHBoxLayout;
    spacingLayout->addWidget( new QLabel( "Spacing:", this ) );
    spacingLayout->addWidget( m_spinSpacing );
    layout->addLayout( spacingLayout );

    layout->addWidget( m_buttonIcon );

    connect( m_comboAlignmentH, SIGNAL( activated( int ) ),
             this, SLOT( slotAlignmentChanged() ) );
    connect( m_comboAlignmentV, SIGNAL( activated( int ) ),
             this, SLOT( slotAlignmentChanged() ) );
    connect( m_spinMaxRating, SIGNAL(valueChanged(int)),
             m_ratingWidget, SLOT(setMaxRating(int)) );
    connect( m_spinRating, SIGNAL(valueChanged(int)),
             m_ratingWidget, SLOT(setRating(int)) );
    connect( m_spinSpacing, SIGNAL(valueChanged(int)),
             m_ratingWidget, SLOT(setSpacing(int)) );
    connect( m_checkHalfSteps, SIGNAL(toggled(bool)),
             m_ratingWidget, SLOT(setHalfStepsEnabled(bool)) );
    connect( m_checkEnalbed, SIGNAL(toggled(bool)),
             m_ratingWidget, SLOT(setDisabled(bool)) );
    connect( m_ratingWidget, SIGNAL(ratingChanged(int)),
             m_spinRating, SLOT(setValue(int)) );
    connect( m_buttonIcon, SIGNAL(clicked()),
             this, SLOT(slotChangeIcon()) );

    m_comboAlignmentH->setCurrentIndex( 2 );
    m_comboAlignmentV->setCurrentIndex( 2 );
    m_spinMaxRating->setValue( m_ratingWidget->maxRating() );
    m_spinRating->setValue( m_ratingWidget->rating() );
    m_spinSpacing->setValue( m_ratingWidget->spacing() );
    m_checkHalfSteps->setChecked( m_ratingWidget->halfStepsEnabled() );
}


RatingPainterTestWidget::~RatingPainterTestWidget()
{
}


void RatingPainterTestWidget::slotAlignmentChanged()
{
    Qt::Alignment align = Qt::AlignLeft;
    if ( m_comboAlignmentH->currentText() == "Left" ) {
        align = Qt::AlignLeft;
    }
    else if ( m_comboAlignmentH->currentText() == "Right" ) {
        align = Qt::AlignRight;
    }
    else if ( m_comboAlignmentH->currentText() == "Center" ) {
        align = Qt::AlignHCenter;
    }
    else if ( m_comboAlignmentH->currentText() == "Justify" ) {
        align = Qt::AlignJustify;
    }

    if ( m_comboAlignmentV->currentText() == "Top" ) {
        align |= Qt::AlignTop;
    }
    else if ( m_comboAlignmentV->currentText() == "Bottom" ) {
        align |= Qt::AlignBottom;
    }
    else if ( m_comboAlignmentV->currentText() == "Center" ) {
        align |= Qt::AlignVCenter;
    }

    m_ratingWidget->setAlignment( align );
}


void RatingPainterTestWidget::slotChangeIcon()
{
    QString icon = KIconDialog::getIcon();
    if ( !icon.isEmpty() ) {
        m_ratingWidget->setIcon( KIcon( icon ) );
        m_buttonIcon->setIcon( KIcon( icon ) );
    }
}

#include "ratingpaintertestwidget.moc"
