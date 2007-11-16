/*

Requires the Qt widget libraries, available at no cost at
http://www.troll.no

Copyright (C) 1996 Bernd Johannes Wuebben  <wuebben@kde.org>
Copyright (c) 1999 Preston Brown <pbrown@kde.org>
Copyright (c) 1999 Mario Weilguni <mweilguni@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include "kfontchooser.h"


#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtCore/QFile>
#include <QtGui/QFont>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QScrollBar>
#include <QtCore/QMutableStringListIterator>
#include <QtGui/QFontDatabase>
#include <QList>
#include <QtGui/QGroupBox>
#include <kcharsets.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klistwidget.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <knuminput.h>
#include <kconfiggroup.h>


static int minimumListWidth( const QListWidget *list )
{
    int w=0;
    for( int i=0; i<list->count(); i++ )
    {
        int itemWidth = list->visualItemRect(list->item(i)).width();
        w = qMax(w,itemWidth);
    }
    if( w == 0 ) { w = 40; }
    w += list->frameWidth() * 2;
    w += list->verticalScrollBar()->sizeHint().width();
    return w;
}

static int minimumListHeight( const QListWidget *list, int numVisibleEntry )
{
    int w = list->count() > 0 ? list->visualItemRect(list->item(0)).height() :
            list->fontMetrics().lineSpacing();

    if( w < 0 ) { w = 10; }
    if( numVisibleEntry <= 0 ) { numVisibleEntry = 4; }
    return ( w * numVisibleEntry + 2 * list->frameWidth() );
}

class KFontChooser::Private
{
public:
    Private( KFontChooser* qq )
        : q( qq )
    {
        m_palette.setColor(QPalette::Active, QPalette::Text, Qt::black);
        m_palette.setColor(QPalette::Active, QPalette::Base, Qt::white);
        signalsAllowed = true;
    }

    // pointer to an optinally supplied list of fonts to
    // inserted into the fontdialog font-family combo-box
//    QStringList  fontList;

    void fillFamilyListBox(bool onlyFixedFonts = false);
    void fillSizeList();
    void setupDisplay();

    void _k_toggled_checkbox();
    void _k_family_chosen_slot(const QString&);
    void _k_size_chosen_slot(const QString&);
    void _k_style_chosen_slot(const QString&);
    void _k_displaySample(const QFont &font);
    void _k_showXLFDArea(bool);
    void _k_size_value_slot(int);

    KFontChooser *q;

    QPalette m_palette;
    bool signalsAllowed:1;

    bool usingFixed:1;

    KIntNumInput *sizeOfFont;

    QLineEdit    *sampleEdit;
    QLineEdit    *xlfdEdit;

    QLabel       *familyLabel;
    QLabel       *styleLabel;
    QCheckBox    *familyCheckbox;
    QCheckBox    *styleCheckbox;
    QCheckBox    *sizeCheckbox;
    QLabel       *sizeLabel;
    KListWidget     *familyListBox;
    KListWidget     *styleListBox;
    KListWidget     *sizeListBox;
    QCheckBox    *sizeIsRelativeCheckBox;

    QFont        selFont;

    QString      selectedStyle;
    int          selectedSize;
    QMap<QString, QString> currentStyles;

};


KFontChooser::KFontChooser( QWidget *parent,
                            const DisplayFlags& flags,
                            const QStringList &fontList,
                            int visibleListSize,
                            Qt::CheckState *sizeIsRelativeState )
    : QWidget(parent),
      d( new KFontChooser::Private( this ) )
{
    d->usingFixed = flags & FixedFontsOnly;
    setWhatsThis(i18nc("@info:whatsthis", "Here you can choose the font to be used." ));

    QVBoxLayout *topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( KDialog::spacingHint() );
    int checkBoxGap = KDialog::spacingHint() / 2;

    QWidget *page;
    QGridLayout *gridLayout;
    int row = 0;
    if( flags & DisplayFrame )
    {
        page = new QGroupBox( i18n("Requested Font"), this );
        topLayout->addWidget(page);
        gridLayout = new QGridLayout( page );
        gridLayout->setMargin( KDialog::marginHint() );
        gridLayout->setSpacing( KDialog::spacingHint() );
        gridLayout->addItem( new QSpacerItem(0,fontMetrics().lineSpacing()), 0, 0 );
        row = 1;
    }
    else
    {
        page = new QWidget( this );
        topLayout->addWidget(page);
        gridLayout = new QGridLayout( page );
        gridLayout->setMargin( 0 );
        gridLayout->setSpacing( KDialog::spacingHint() );
    }

    //
    // first, create the labels across the top
    //
    QHBoxLayout *familyLayout = new QHBoxLayout();
    familyLayout->addSpacing( checkBoxGap );
    if ( flags & ShowDifferences ) {
        d->familyCheckbox = new QCheckBox(i18nc("@option:check","Font"), page);
        connect(d->familyCheckbox, SIGNAL(toggled(bool)),
                this, SLOT(_k_toggled_checkbox()));
        familyLayout->addWidget(d->familyCheckbox, 0, Qt::AlignLeft);
        d->familyCheckbox->setWhatsThis(i18nc("@info:whatsthis","Enable this checkbox to change the font family settings."));
        d->familyCheckbox->setToolTip(i18nc("@info:tooltip","Change font family?") );
        d->familyLabel = 0;
    } else {
        d->familyCheckbox = 0;
        d->familyLabel = new QLabel( i18nc("@label","Font:"), page );
        d->familyLabel->setObjectName( "familyLabel" );
        familyLayout->addWidget(d->familyLabel, 1, Qt::AlignLeft);
    }
    gridLayout->addLayout(familyLayout, row, 0 );

    QHBoxLayout *styleLayout = new QHBoxLayout();
    if ( flags & ShowDifferences ) {
        d->styleCheckbox = new QCheckBox(i18nc("@option:check","Font style"), page);
        connect(d->styleCheckbox, SIGNAL(toggled(bool)),
                this, SLOT(_k_toggled_checkbox()));
        styleLayout->addWidget(d->styleCheckbox, 0, Qt::AlignLeft);
        d->styleCheckbox->setWhatsThis(i18nc("@info:whatsthis","Enable this checkbox to change the font style settings."));
        d->styleCheckbox->setToolTip(i18nc("@info:tooltip","Change font style?"));
        d->styleLabel = 0;
    } else {
        d->styleCheckbox = 0;
        d->styleLabel = new QLabel(i18n("Font style:"), page );
        d->styleLabel->setObjectName( "styleLabel" );
        styleLayout->addWidget(d->styleLabel, 1, Qt::AlignLeft);
    }
    styleLayout->addSpacing( checkBoxGap );
    gridLayout->addLayout(styleLayout, row, 1 );

    QHBoxLayout *sizeLayout = new QHBoxLayout();
    if ( flags & ShowDifferences ) {
        d->sizeCheckbox = new QCheckBox(i18nc("@option:check","Size"),page);
        connect(d->sizeCheckbox, SIGNAL(toggled(bool)),
                this, SLOT(_k_toggled_checkbox()));
        sizeLayout->addWidget(d->sizeCheckbox, 0, Qt::AlignLeft);
        d->sizeCheckbox->setWhatsThis(i18nc("@info:whatsthis","Enable this checkbox to change the font size settings."));
        d->sizeCheckbox->setToolTip(i18nc("@info:tooltip","Change font size?"));
        d->sizeLabel = 0;
    } else {
        d->sizeCheckbox = 0;
        d->sizeLabel = new QLabel(i18n("Size:"), page );
        d->sizeLabel->setObjectName( "sizeLabel" );
        sizeLayout->addWidget(d->sizeLabel, 1, Qt::AlignLeft);
    }
    sizeLayout->addSpacing( checkBoxGap );
    sizeLayout->addSpacing( checkBoxGap ); // prevent label from eating border
    gridLayout->addLayout(sizeLayout, row, 2 );

    row ++;

    //
    // now create the actual boxes that hold the info
    //
    d->familyListBox = new KListWidget( page );
    d->familyListBox->setObjectName("familyListBox");
    d->familyListBox->setEnabled( flags ^ ShowDifferences );
    gridLayout->addWidget( d->familyListBox, row, 0 );
    QString fontFamilyWhatsThisText (
        i18nc("@info:whatsthis","Here you can choose the font family to be used." ));
    d->familyListBox->setWhatsThis(fontFamilyWhatsThisText );

    if ( flags & ShowDifferences ) {
        d->familyCheckbox->setWhatsThis(fontFamilyWhatsThisText );
    } else {
        d->familyLabel->setWhatsThis(fontFamilyWhatsThisText );
    }

    connect(d->familyListBox, SIGNAL(currentTextChanged(const QString &)),
            this, SLOT(_k_family_chosen_slot(const QString &)));
    if ( !fontList.isEmpty() ) {
        d->familyListBox->addItems(fontList);
    }
    else
    {
        d->fillFamilyListBox( flags & FixedFontsOnly );
    }

    d->familyListBox->setMinimumWidth( minimumListWidth( d->familyListBox ) );
    d->familyListBox->setMinimumHeight(
        minimumListHeight( d->familyListBox, visibleListSize  ) );

    d->styleListBox = new KListWidget( page );
    d->styleListBox->setObjectName("styleListBox");
    d->styleListBox->setEnabled( flags ^ ShowDifferences );
    gridLayout->addWidget(d->styleListBox, row, 1);
    d->styleListBox->setWhatsThis(i18nc("@info:whatsthis","Here you can choose the font style to be used." ));
    if ( flags & ShowDifferences ) {
        ((QWidget *)d->styleCheckbox)->setWhatsThis(fontFamilyWhatsThisText );
    } else {
        ((QWidget *)d->styleLabel)->setWhatsThis( fontFamilyWhatsThisText );
    }
    d->styleListBox->addItem(i18nc("@item font","Regular"));
    d->styleListBox->addItem(i18nc("@item font","Italic"));
    d->styleListBox->addItem(i18nc("@item font","Bold"));
    d->styleListBox->addItem(i18nc("@item font","Bold Italic"));
    d->styleListBox->setMinimumWidth( minimumListWidth( d->styleListBox ) );
    d->styleListBox->setMinimumHeight(
        minimumListHeight( d->styleListBox, visibleListSize  ) );

    connect(d->styleListBox, SIGNAL(currentTextChanged(const QString &)),
            this, SLOT(_k_style_chosen_slot(const QString &)));


    d->sizeListBox = new KListWidget( page );
    d->sizeListBox->setObjectName("sizeListBox");
    d->sizeOfFont = new KIntNumInput( page); // "sizeOfFont");
    d->sizeOfFont->setMinimum(4);
    d->sizeOfFont->setMaximum(128);

    d->sizeListBox->setEnabled( flags ^ ShowDifferences );
    d->sizeOfFont->setEnabled( flags ^ ShowDifferences );
    if( sizeIsRelativeState ) {
        QString sizeIsRelativeCBText =
            i18nc("@item font size","Relative");
        QString sizeIsRelativeCBToolTipText =
            i18n("Font size<br /><i>fixed</i> or <i>relative</i><br />to environment");
        QString sizeIsRelativeCBWhatsThisText =
            i18n("Here you can switch between fixed font size and font size "
                 "to be calculated dynamically and adjusted to changing "
                 "environment (e.g. widget dimensions, paper size)." );
        d->sizeIsRelativeCheckBox = new QCheckBox( sizeIsRelativeCBText,
                                                page );
        d->sizeIsRelativeCheckBox->setObjectName( "sizeIsRelativeCheckBox" );
        d->sizeIsRelativeCheckBox->setTristate( flags & ShowDifferences );
        QGridLayout *sizeLayout2 = new QGridLayout();
        sizeLayout2->setSpacing( KDialog::spacingHint()/2 );
        sizeLayout2->setObjectName( "sizeLayout2" );
        gridLayout->addLayout(sizeLayout2, row, 2);
        sizeLayout2->setColumnStretch( 1, 1 ); // to prevent text from eating the right border
        sizeLayout2->addWidget( d->sizeOfFont, 0, 0, 1, 2);
        sizeLayout2->addWidget(d->sizeListBox, 1,0, 1,2);
        sizeLayout2->addWidget(d->sizeIsRelativeCheckBox, 2, 0, Qt::AlignLeft);
        d->sizeIsRelativeCheckBox->setWhatsThis(sizeIsRelativeCBWhatsThisText );
        d->sizeIsRelativeCheckBox->setToolTip( sizeIsRelativeCBToolTipText );
    }
    else {
        d->sizeIsRelativeCheckBox = 0L;
        QGridLayout *sizeLayout2 = new QGridLayout();
        sizeLayout2->setSpacing( KDialog::spacingHint()/2 );
        sizeLayout2->setObjectName( "sizeLayout2" );
        gridLayout->addLayout(sizeLayout2, row, 2);
        sizeLayout2->addWidget( d->sizeOfFont, 0, 0);
        sizeLayout2->addWidget(d->sizeListBox, 1,0);
    }
    QString fontSizeWhatsThisText =
        i18n("Here you can choose the font size to be used." );
    d->sizeListBox->setWhatsThis(fontSizeWhatsThisText );

    if ( flags & ShowDifferences ) {
        ((QWidget *)d->sizeCheckbox)->setWhatsThis(fontSizeWhatsThisText );
    } else {
        ((QWidget *)d->sizeLabel)->setWhatsThis( fontSizeWhatsThisText );
    }

    d->fillSizeList();
    d->sizeListBox->setMinimumWidth( minimumListWidth(d->sizeListBox) +
                                  d->sizeListBox->fontMetrics().maxWidth() );
    d->sizeListBox->setMinimumHeight(
        minimumListHeight( d->sizeListBox, visibleListSize  ) );

    connect( d->sizeOfFont, SIGNAL( valueChanged(int) ),
             this, SLOT(_k_size_value_slot(int)));

    connect( d->sizeListBox, SIGNAL(currentTextChanged(const QString&)),
             this, SLOT(_k_size_chosen_slot(const QString&)) );

    // default to 10pt font size
    QList<QListWidgetItem*> list = d->sizeListBox->findItems(QString::number(10),Qt::MatchContains);
    if ( list.count() > 0 )
        d->sizeListBox->setCurrentItem(list.first());

    //d->sizeListBox->setSelected(d->sizeListBox->findItem(QString::number(10)), true); // default to 10pt.

    row ++;

    row ++;
    d->sampleEdit = new QLineEdit( page );
    d->sampleEdit->setObjectName( "sampleEdit");
    QFont tmpFont( KGlobalSettings::generalFont().family(), 64, QFont::Black );
    d->sampleEdit->setFont(tmpFont);
    //i18n: This is a classical test phrase. (It contains all letters from A to Z.)
    d->sampleEdit->setText(i18n("The Quick Brown Fox Jumps Over The Lazy Dog"));
    d->sampleEdit->setMinimumHeight( d->sampleEdit->fontMetrics().lineSpacing() );
    d->sampleEdit->setAlignment(Qt::AlignCenter);
    gridLayout->addWidget(d->sampleEdit, 4, 0, 1, 3);
    QString sampleEditWhatsThisText =
        i18n("This sample text illustrates the current settings. "
             "You may edit it to test special characters." );
    d->sampleEdit->setWhatsThis(sampleEditWhatsThisText );
    connect(this, SIGNAL(fontSelected(const QFont &)),
            this, SLOT(_k_displaySample(const QFont &)));

    QVBoxLayout *vbox;
    if( flags & DisplayFrame )
    {
        page = new QGroupBox( i18n("Actual Font"), this );
        topLayout->addWidget(page);
        vbox = new QVBoxLayout( page );
        vbox->setSpacing( KDialog::spacingHint() );
        vbox->addSpacing( fontMetrics().lineSpacing() );
    }
    else
    {
        page = new QWidget( this );
        topLayout->addWidget(page);
        vbox = new QVBoxLayout( page );
        vbox->setMargin( 0 );
        vbox->setSpacing( KDialog::spacingHint() );
        QLabel *label = new QLabel( i18n("Actual Font"), page );
        vbox->addWidget( label );
    }

    d->xlfdEdit = new QLineEdit( page );
    d->xlfdEdit->setObjectName( "xlfdEdit" );
    vbox->addWidget( d->xlfdEdit );

    // lets initialize the display if possible
    setFont( KGlobalSettings::generalFont(), d->usingFixed );
    // check or uncheck or gray out the "relative" checkbox
    if( sizeIsRelativeState && d->sizeIsRelativeCheckBox )
        setSizeIsRelative( *sizeIsRelativeState );

    KConfigGroup cg(KGlobal::config(), QLatin1String("General"));
    d->_k_showXLFDArea(cg.readEntry(QLatin1String("fontSelectorShowXLFD"), false));
}

KFontChooser::~KFontChooser()
{
    delete d;
}

void KFontChooser::Private::fillSizeList() {
    if ( !sizeListBox ) {
        return; //assertion.
    }

    static const int c[] =
        {
            4,  5,  6,  7,
            8,  9,  10, 11,
            12, 13, 14, 15,
            16, 17, 18, 19,
            20, 22, 24, 26,
            28, 32, 48, 64,
            72, 80, 96, 128,
	     0
        };
    for(int i = 0; c[i]; ++i)
    {
        sizeListBox->addItem(QString::number(c[i]));
    }
}

void KFontChooser::setColor( const QColor & col )
{
    d->m_palette.setColor( QPalette::Active, QPalette::Text, col );
    QPalette pal = d->sampleEdit->palette();
    pal.setColor( QPalette::Active, QPalette::Text, col );
    d->sampleEdit->setPalette( pal );
}

QColor KFontChooser::color() const
{
    return d->m_palette.color( QPalette::Active, QPalette::Text );
}

void KFontChooser::setBackgroundColor( const QColor & col )
{
    d->m_palette.setColor( QPalette::Active, QPalette::Base, col );
    QPalette pal = d->sampleEdit->palette();
    pal.setColor( QPalette::Active, QPalette::Base, col );
    d->sampleEdit->setPalette( pal );
}

QColor KFontChooser::backgroundColor() const
{
    return d->m_palette.color( QPalette::Active, QPalette::Base );
}

void KFontChooser::setSizeIsRelative( Qt::CheckState relative )
{
    // check or uncheck or gray out the "relative" checkbox
    if( d->sizeIsRelativeCheckBox ) {
        if( Qt::PartiallyChecked == relative )
            d->sizeIsRelativeCheckBox->setCheckState(Qt::PartiallyChecked);
        else
            d->sizeIsRelativeCheckBox->setCheckState(  (Qt::Checked == relative )  ? Qt::Checked : Qt::Unchecked);
    }
}

Qt::CheckState KFontChooser::sizeIsRelative() const
{
    return d->sizeIsRelativeCheckBox
        ? d->sizeIsRelativeCheckBox->checkState()
        : Qt::PartiallyChecked;
}

QString KFontChooser::sampleText() const
{
    return d->sampleEdit->text();
}

void KFontChooser::setSampleText( const QString &text )
{
    d->sampleEdit->setText( text );
}

void KFontChooser::setSampleBoxVisible( bool visible )
{
    d->sampleEdit->setVisible( visible );
}

QSize KFontChooser::sizeHint( void ) const
{
    return minimumSizeHint();
}


void KFontChooser::enableColumn( int column, bool state )
{
    if( column & FamilyList )
    {
        d->familyListBox->setEnabled(state);
    }
    if( column & StyleList )
    {
        d->styleListBox->setEnabled(state);
    }
    if( column & SizeList )
    {
        d->sizeListBox->setEnabled(state);
    }
}


void KFontChooser::setFont( const QFont& aFont, bool onlyFixed )
{
    d->selFont = aFont;
    d->selectedSize=aFont.pointSize();
    if (d->selectedSize == -1)
        d->selectedSize = QFontInfo(aFont).pointSize();

    if( onlyFixed != d->usingFixed)
    {
        d->usingFixed = onlyFixed;
        d->fillFamilyListBox(d->usingFixed);
    }
    d->setupDisplay();
    d->_k_displaySample(d->selFont);
}


KFontChooser::FontDiffFlags KFontChooser::fontDiffFlags() const
{
    FontDiffFlags diffFlags = NoFontDiffFlags;

    if ( d->familyCheckbox && d->familyCheckbox->isChecked() ) {
        diffFlags |= FontDiffFamily;
    }

    if ( d->styleCheckbox && d->styleCheckbox->isChecked() ) {
        diffFlags |= FontDiffStyle;
    }

    if ( d->sizeCheckbox && d->sizeCheckbox->isChecked() ) {
        diffFlags |= FontDiffSize;
    }

    return diffFlags;
}

QFont KFontChooser::font() const
{
    return d->selFont;
}

void KFontChooser::Private::_k_toggled_checkbox()
{
    familyListBox->setEnabled( familyCheckbox->isChecked() );
    styleListBox->setEnabled( styleCheckbox->isChecked() );
    sizeListBox->setEnabled( sizeCheckbox->isChecked() );
    sizeOfFont->setEnabled( sizeCheckbox->isChecked() );
}

void KFontChooser::Private::_k_family_chosen_slot(const QString& family)
{
    QFontDatabase dbase;
    signalsAllowed = false;
    QStringList styles = QStringList(dbase.styles(family));
    styleListBox->clear();
    currentStyles.clear();
    for ( QStringList::Iterator it = styles.begin(); it != styles.end(); ++it ) {
        QString style = *it;
        int pos = style.indexOf("Plain");
        if(pos >=0) style = style.replace(pos,5,i18nc("@item font","Regular"));
        pos = style.indexOf("Normal");
        if(pos >=0) style = style.replace(pos,6,i18nc("@item font","Regular"));
        pos = style.indexOf("Oblique");
        if(pos >=0) style = style.replace(pos,7,i18nc("@item font","Italic"));

        QList<QListWidgetItem*> styleList = styleListBox->findItems(style,Qt::MatchContains);
        if ( styleList.isEmpty() ) {
            styleListBox->addItem(i18n(style.toUtf8()));
            currentStyles.insert(i18n(style.toUtf8()), *it);
        }
    }
    if ( styleListBox->count() == 0 ) {
        styleListBox->addItem(i18nc("@item font","Regular"));
        currentStyles.insert(i18nc("@item font","Regular"), "Normal");
    }

    QList<QListWidgetItem*> selectedStyleList = styleListBox->findItems(selectedStyle,Qt::MatchContains);
    if (!selectedStyleList.isEmpty()) {
        styleListBox->setCurrentItem(selectedStyleList.first());
    } else {
        styleListBox->setCurrentRow(0);
    }
    signalsAllowed = true;
    selFont.setFamily( family );
    emit q->fontSelected( selFont );
}

void KFontChooser::Private::_k_size_chosen_slot(const QString& size){
    if ( !signalsAllowed ) {
        return;
    }

    selectedSize = size.toInt();
    Q_ASSERT( selectedSize );
    sizeOfFont->setValue( selectedSize );
    selFont.setPointSize( selectedSize );
    emit q->fontSelected( selFont );
}

void KFontChooser::Private::_k_size_value_slot(int val) {
    selFont.setPointSize( val );
    emit q->fontSelected( selFont );
}

void KFontChooser::Private::_k_style_chosen_slot(const QString& style)
{
    if ( !signalsAllowed ) {
        return;
    }

    signalsAllowed = false;
    QString currentStyle;
    if (style.isEmpty()) {
        currentStyle = styleListBox->currentItem()->text();
    } else {
        currentStyle = style;
    }

    int diff=0; // the difference between the font size requested and what we can show.

    sizeListBox->clear();
    QFontDatabase dbase;
    if( dbase.isSmoothlyScalable( familyListBox->currentItem()->text(),
                                  currentStyles[currentStyle])) {
        // is vector font
        //>sampleEdit->setPaletteBackgroundPixmap( VectorPixmap ); // TODO
        fillSizeList();
    } else {                                // is bitmap font.
        //sampleEdit->setPaletteBackgroundPixmap( BitmapPixmap ); // TODO
        QList<int> sizes = dbase.smoothSizes( familyListBox->currentItem()->text(),
                                              currentStyles[currentStyle] );
        if(sizes.count() > 0) {
            QList<int>::const_iterator it;
            diff=1000;
            for ( it = sizes.constBegin(); it != sizes.constEnd(); ++it ) {
                if ( *it <= selectedSize || diff > *it - selectedSize ) {
                    diff = selectedSize - *it;
                }
                sizeListBox->addItem( QString::number( *it ) );
            }
        } else {
            // there are times Qt does not provide the list..
            fillSizeList();
        }
    }

    QList<QListWidgetItem*> selectedSizeList =
        sizeListBox->findItems( QString::number(selectedSize),
                                Qt::MatchContains );
    if ( !selectedSizeList.isEmpty() ) {
        sizeListBox->setCurrentItem(selectedSizeList.first());
    }
    //TODO - KDE4 : sizeListBox->scrollTo(sizeListBox->currentItem());

    //kDebug() << "Showing: " << d->familyListBox->currentText() << ", " << d->currentStyles[currentStyle] << ", " << selectedSize-diff;
    selFont = dbase.font(familyListBox->currentItem()->text(),
                         currentStyles[currentStyle],
                         selectedSize-diff);
    emit q->fontSelected(selFont);
    if (!style.isEmpty()) {
        selectedStyle = style;
    }
    signalsAllowed = true;
}

void KFontChooser::Private::_k_displaySample( const QFont& font )
{
    sampleEdit->setFont(font);
    sampleEdit->setCursorPosition(0);
    xlfdEdit->setText(font.rawName());
    xlfdEdit->setCursorPosition(0);

    //QFontInfo a = QFontInfo(font);
    //kDebug() << "font: " << a.family () << ", " << a.pointSize ();
    //kDebug() << "      (" << font.toString() << ")\n";
}

void KFontChooser::Private::setupDisplay()
{
    // Calling familyListBox->setCurrentItem() causes the value of selFont
    // to change, so we save the family, style and size beforehand.
    QString family = selFont.family().toLower();
    int style = ( selFont.bold() ? 2 : 0 ) + ( selFont.italic() ? 1 : 0 );
    int size = selFont.pointSize();
    if (size == -1)
        size = QFontInfo( selFont ).pointSize();
    QString sizeStr = QString::number( size );

    int numEntries, i;

    numEntries = familyListBox->count();
    for (i = 0; i < numEntries; i++) {
        if (family == familyListBox->item(i)->text().toLower()) {
            familyListBox->setCurrentRow(i);
            break;
        }
    }

    // 1st Fallback
    if ( (i == numEntries) )
    {
        if (family.contains('['))
        {
            family = family.left(family.indexOf('[')).trimmed();
            for (i = 0; i < numEntries; i++) {
                if (family == familyListBox->item(i)->text().toLower()) {
                    familyListBox->setCurrentRow(i);
                    break;
                }
            }
        }
    }

    // 2nd Fallback
    if ( (i == numEntries) )
    {
        QString fallback = family+" [";
        for (i = 0; i < numEntries; i++) {
            if (familyListBox->item(i)->text().toLower().startsWith(fallback)) {
                familyListBox->setCurrentRow(i);
                break;
            }
        }
    }

    // 3rd Fallback
    if ( (i == numEntries) )
    {
        for (i = 0; i < numEntries; i++) {
            if (familyListBox->item(i)->text().toLower().startsWith(family)) {
                familyListBox->setCurrentRow(i);
                break;
            }
        }
    }

    // Fall back in case nothing matched. Otherwise, diff doesn't work
    if ( i == numEntries ) {
        familyListBox->setCurrentRow( 0 );
    }

    styleListBox->setCurrentRow(style);

    numEntries = sizeListBox->count();
    for ( i = 0; i < numEntries; i++ ){
        if ( sizeStr == sizeListBox->item(i)->text() ) {
            sizeListBox->setCurrentRow( i );
            break;
        }
    }

    sizeOfFont->setValue(size);
}


void KFontChooser::getFontList( QStringList &list, uint fontListCriteria)
{
    QFontDatabase dbase;
    QStringList lstSys(dbase.families());

    // if we have criteria; then check fonts before adding
    if (fontListCriteria)
    {
        QStringList lstFonts;
        for (QStringList::Iterator it = lstSys.begin(); it != lstSys.end(); ++it)
        {
            if ((fontListCriteria & FixedWidthFonts) > 0 && !dbase.isFixedPitch(*it)) continue;
            if (((fontListCriteria & (SmoothScalableFonts | ScalableFonts)) == ScalableFonts) &&
                !dbase.isBitmapScalable(*it)) continue;
            if ((fontListCriteria & SmoothScalableFonts) > 0 && !dbase.isSmoothlyScalable(*it)) continue;
            lstFonts.append(*it);
        }

        if((fontListCriteria & FixedWidthFonts) > 0) {
            // Fallback.. if there are no fixed fonts found, it's probably a
            // bug in the font server or Qt.  In this case, just use 'fixed'
            if (lstFonts.count() == 0)
                lstFonts.append("fixed");
        }

        lstSys = lstFonts;
    }

    lstSys.sort();

    list = lstSys;
}

void KFontChooser::Private::fillFamilyListBox(bool onlyFixedFonts)
{
    QStringList fontList;
    getFontList(fontList, onlyFixedFonts?FixedWidthFonts:0);
    familyListBox->clear();
    familyListBox->addItems(fontList);
}

void KFontChooser::Private::_k_showXLFDArea(bool show)
{
    if( show )
    {
        xlfdEdit->parentWidget()->show();
    }
    else
    {
        xlfdEdit->parentWidget()->hide();
    }
}

#include "kfontchooser.moc"
