/*
    $Id$

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <qcombobox.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qfont.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qscrollbar.h>
#include <qstringlist.h>
#include <qfontdatabase.h>
#include <qwhatsthis.h>
#include <qtooltip.h>

#include <kapplication.h>
#include <kcharsets.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <qlineedit.h>
#include <klistbox.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

#include "kfontdialog.moc"

static int minimumListWidth( const QListBox *list )
{
  int w=0;
  for( uint i=0; i<list->count(); i++ )
  {
    int itemWidth = list->item(i)->width(list);
    w = QMAX(w,itemWidth);
  }
  if( w == 0 ) { w = 40; }
  w += list->frameWidth() * 2;
  w += list->verticalScrollBar()->sizeHint().width();
  return( w );
}

static int minimumListHeight( const QListBox *list, int numVisibleEntry )
{
  int w = list->count() > 0 ? list->item(0)->height(list) :
    list->fontMetrics().lineSpacing();

  if( w < 0 ) { w = 10; }
  if( numVisibleEntry <= 0 ) { numVisibleEntry = 4; }
  return( w * numVisibleEntry + 2 * list->frameWidth() );
}

class KFontChooser::KFontChooserPrivate
{
public:
    KFontChooserPrivate() { m_color = Qt::black; }
    QLabel *charsetLabel;
    QColor m_color;
};

KFontChooser::KFontChooser(QWidget *parent, const char *name,
			   bool onlyFixed, const QStringList &fontList,
			   bool makeFrame, int visibleListSize, bool diff,
                           QButton::ToggleState *sizeIsRelativeState )
  : QWidget(parent, name), usingFixed(onlyFixed)
{
  QString mainWhatsThisText =
    i18n( "Here you can choose the font to be used." );
  QWhatsThis::add( this, mainWhatsThisText );

  d = new KFontChooserPrivate;
  QVBoxLayout *topLayout = new QVBoxLayout( this, 0, KDialog::spacingHint() );
  int checkBoxGap = KDialog::spacingHint() / 2;

  QWidget *page;
  QGridLayout *gridLayout;
  int row = 0;
  if( makeFrame == true )
  {
    page = new QGroupBox( i18n("Requested Font"), this );
    topLayout->addWidget(page);
    gridLayout = new QGridLayout( page, 5, 3, 0, KDialog::spacingHint() );
    gridLayout->addRowSpacing( 0, fontMetrics().lineSpacing() );
    row = 1;
  }
  else
  {
    page = new QWidget( this );
    topLayout->addWidget(page);
    gridLayout = new QGridLayout( page, 4, 3, 0, KDialog::spacingHint() );
  }

  //
  // first, create the labels across the top
  //
  QHBoxLayout *familyLayout = new QHBoxLayout();
  familyLayout->addSpacing( checkBoxGap );
  if (diff) {
    familyCheckbox = new QCheckBox(i18n("Font"), page);
    connect(familyCheckbox, SIGNAL(toggled(bool)), SLOT(toggled_checkbox()));
    familyLayout->addWidget(familyCheckbox, 0, Qt::AlignLeft);
    QString familyCBToolTipText =
      i18n("Change font family?");
    QString familyCBWhatsThisText =
      i18n("Enable this checkbox to change the font family settings.");
    QWhatsThis::add( familyCheckbox, familyCBWhatsThisText );
    QToolTip::add(   familyCheckbox, familyCBToolTipText );
  } else {
    familyLabel = new QLabel( i18n("Font"), page, "familyLabel" );
    familyLayout->addWidget(familyLabel, 1, Qt::AlignLeft);
  }
  gridLayout->addLayout(familyLayout, row, 0 );

  QHBoxLayout *styleLayout = new QHBoxLayout();
  if (diff) {
     styleCheckbox = new QCheckBox(i18n("Font style"), page);
     connect(styleCheckbox, SIGNAL(toggled(bool)), SLOT(toggled_checkbox()));
     styleLayout->addWidget(styleCheckbox, 0, Qt::AlignLeft);
    QString styleCBToolTipText =
      i18n("Change font style?");
    QString styleCBWhatsThisText =
      i18n("Enable this checkbox to change the font style settings.");
    QWhatsThis::add( styleCheckbox, styleCBWhatsThisText );
    QToolTip::add(   styleCheckbox, styleCBToolTipText );
  } else {
    styleLabel = new QLabel( i18n("Font style"), page, "styleLabel");
    styleLayout->addWidget(styleLabel, 1, Qt::AlignLeft);
  }
  styleLayout->addSpacing( checkBoxGap );
  gridLayout->addLayout(styleLayout, row, 1 );

  QHBoxLayout *sizeLayout = new QHBoxLayout();
  if (diff) {
    sizeCheckbox = new QCheckBox(i18n("Size"),page);
    connect(sizeCheckbox, SIGNAL(toggled(bool)), SLOT(toggled_checkbox()));
    sizeLayout->addWidget(sizeCheckbox, 0, Qt::AlignLeft);
    QString sizeCBToolTipText =
      i18n("Change font size?");
    QString sizeCBWhatsThisText =
      i18n("Enable this checkbox to change the font size settings.");
    QWhatsThis::add( sizeCheckbox, sizeCBWhatsThisText );
    QToolTip::add(   sizeCheckbox, sizeCBToolTipText );
  } else {
    sizeLabel = new QLabel( i18n("Size"), page, "sizeLabel");
    sizeLayout->addWidget(sizeLabel, 1, Qt::AlignLeft);
  }
  sizeLayout->addSpacing( checkBoxGap );
  sizeLayout->addSpacing( checkBoxGap ); // prevent label from eating border
  gridLayout->addLayout(sizeLayout, row, 2 );

  row ++;

  //
  // now create the actual boxes that hold the info
  //
  familyListBox = new KListBox( page, "familyListBox");
  familyListBox->setEnabled( !diff );
  gridLayout->addWidget( familyListBox, row, 0 );
  QString fontFamilyWhatsThisText =
    i18n("Here you can choose the font family to be used." );
  QWhatsThis::add( familyListBox, fontFamilyWhatsThisText );
  QWhatsThis::add(diff?(QWidget *) familyCheckbox:(QWidget *) familyLabel, fontFamilyWhatsThisText );
  connect(familyListBox, SIGNAL(highlighted(const QString &)),
	  SLOT(family_chosen_slot(const QString &)));
  if(fontList.count() != 0)
  {
    familyListBox->insertStringList(fontList);
  }
  else
  {
    fillFamilyListBox(onlyFixed);
  }

  familyListBox->setMinimumWidth( minimumListWidth( familyListBox ) );
  familyListBox->setMinimumHeight(
    minimumListHeight( familyListBox, visibleListSize  ) );

  styleListBox = new KListBox( page, "styleListBox");
  styleListBox->setEnabled( !diff );
  gridLayout->addWidget(styleListBox, row, 1);
  QString fontStyleWhatsThisText =
    i18n("Here you can choose the font style to be used." );
  QWhatsThis::add( styleListBox, fontStyleWhatsThisText );
  QWhatsThis::add(diff?(QWidget *)styleCheckbox:(QWidget *)styleLabel, fontFamilyWhatsThisText );
  styleListBox->insertItem(i18n("Regular"));
  styleListBox->insertItem(i18n("Italic"));
  styleListBox->insertItem(i18n("Bold"));
  styleListBox->insertItem(i18n("Bold Italic"));
  styleListBox->setMinimumWidth( minimumListWidth( styleListBox ) );
  styleListBox->setMinimumHeight(
    minimumListHeight( styleListBox, visibleListSize  ) );

  connect(styleListBox, SIGNAL(highlighted(const QString &)),
	  SLOT(style_chosen_slot(const QString &)));


  sizeListBox = new KListBox( page, "sizeListBox");
  sizeListBox->setEnabled( !diff );
  if( sizeIsRelativeState ) {
    QString sizeIsRelativeCBText =
      i18n("relative");
    QString sizeIsRelativeCBToolTipText =
      i18n("Font size<br><i>fixed</i> or <i>relative</i><br>to environment");
    QString sizeIsRelativeCBWhatsThisText =
      i18n("Here you can switch between fixed font size and font size "
           "to be calculated dynamically and adjusted to changing "
           "environment (e.g. widget dimensions, paper size)." );
    sizeIsRelativeCheckBox = new QCheckBox( sizeIsRelativeCBText,
                                            page,
                                           "sizeIsRelativeCheckBox" );
    sizeIsRelativeCheckBox->setTristate( diff );
    QGridLayout *sizeLayout2 = new QGridLayout( 2,2, KDialog::spacingHint()/2, "sizeLayout2" );
    gridLayout->addLayout(sizeLayout2, row, 2);
    sizeLayout2->setColStretch( 1, 1 ); // to prevent text from eating the right border
    sizeLayout2->addMultiCellWidget(sizeListBox, 0,0, 0,1);
    sizeLayout2->addWidget(sizeIsRelativeCheckBox, 1, 0, Qt::AlignLeft);
    QWhatsThis::add( sizeIsRelativeCheckBox, sizeIsRelativeCBWhatsThisText );
    QToolTip::add(   sizeIsRelativeCheckBox, sizeIsRelativeCBToolTipText );
  }
  else {
    sizeIsRelativeCheckBox = 0L;
    gridLayout->addWidget(sizeListBox, row, 2);
  }
  QString fontSizeWhatsThisText =
    i18n("Here you can choose the font size to be used." );
  QWhatsThis::add( sizeListBox, fontSizeWhatsThisText );
  QWhatsThis::add( diff?(QWidget *)sizeCheckbox:(QWidget *)sizeLabel, fontSizeWhatsThisText );

  fillSizeList();
  sizeListBox->setMinimumWidth( minimumListWidth(sizeListBox) +
    sizeListBox->fontMetrics().maxWidth() );
  sizeListBox->setMinimumHeight(
    minimumListHeight( sizeListBox, visibleListSize  ) );

  connect( sizeListBox, SIGNAL(highlighted(const QString&)),
	   SLOT(size_chosen_slot(const QString&)) );
  sizeListBox->setSelected(sizeListBox->findItem(QString::number(10)), true); // default to 10pt.

  row ++;
#if QT_VERSION < 300
  d->charsetLabel = new QLabel( page, "charsetLabel");
  d->charsetLabel->setText(i18n("Character set:"));
  gridLayout->addWidget(d->charsetLabel, 3, 0, AlignRight);
  charsetsCombo = new QComboBox(true, page, "charsetsCombo");
  gridLayout->addMultiCellWidget(charsetsCombo, 3, 3, 1, 2);
  QString charsetWhatsThisText =
    i18n("Here you can choose the charset to be used." );
  QWhatsThis::add( charsetCombo,    charsetWhatsThisText );
  QWhatsThis::add( d->charsetLabel, charsetWhatsThisText );
  charsetsCombo->setInsertionPolicy(QComboBox::NoInsertion);
  connect(charsetsCombo, SIGNAL(activated(const QString&)),
	  SLOT(charset_chosen_slot(const QString&)));
#else
  d->charsetLabel = 0;
  charsetsCombo = 0;
#endif

  row ++;
  sampleEdit = new QLineEdit( page, "sampleEdit");
  QFont tmpFont( KGlobalSettings::generalFont().family(), 64, QFont::Black );
  sampleEdit->setFont(tmpFont);
  sampleEdit->setText(i18n("The Quick Brown Fox Jumps Over The Lazy Dog"));
  sampleEdit->setMinimumHeight( sampleEdit->fontMetrics().lineSpacing() );
  sampleEdit->setAlignment(Qt::AlignCenter);
  gridLayout->addMultiCellWidget(sampleEdit, 4, 4, 0, 2);
  QString sampleEditWhatsThisText =
    i18n("This sample text illustrates the current settings. "
         "You may edit it to test special characters." );
  QWhatsThis::add( sampleEdit, sampleEditWhatsThisText );
  connect(this, SIGNAL(fontSelected(const QFont &)),
	  SLOT(displaySample(const QFont &)));

  QVBoxLayout *vbox;
  if( makeFrame == true )
  {
    page = new QGroupBox( i18n("Actual Font"), this );
    topLayout->addWidget(page);
    vbox = new QVBoxLayout( page, KDialog::spacingHint() );
    vbox->addSpacing( fontMetrics().lineSpacing() );
  }
  else
  {
    page = new QWidget( this );
    topLayout->addWidget(page);
    vbox = new QVBoxLayout( page, 0, KDialog::spacingHint() );
    QLabel *label = new QLabel( i18n("Actual Font"), page );
    vbox->addWidget( label );
  }

  xlfdEdit = new QLineEdit( page, "xlfdEdit" );
  vbox->addWidget( xlfdEdit );

  // lets initialize the display if possible
  setFont( KGlobalSettings::generalFont(), usingFixed );
  // Create displayable charsets list
  fillCharsetsCombo();
  // check or uncheck or gray out the "relative" checkbox
  if( sizeIsRelativeState && sizeIsRelativeCheckBox )
    setSizeIsRelative( *sizeIsRelativeState ); 

  KConfig *config = KGlobal::config();
  KConfigGroupSaver saver(config, QString::fromLatin1("General"));
  showXLFDArea(config->readBoolEntry(QString::fromLatin1("fontSelectorShowXLFD"), false));
}

KFontChooser::~KFontChooser()
{
  delete d;
}

void KFontChooser::fillSizeList() {
  if(! sizeListBox) return; //assertion.

  static const int c[] =
  {
    4,  5,  6,  7,
    8,  9,  10, 11,
    12, 13, 14, 15,
    16, 17, 18, 19,
    20, 22, 24, 26,
    28, 32, 48, 64,
    0
  };
  for(int i = 0; c[i] != 0; i++)
  {
    sizeListBox->insertItem(QString::number(c[i]));
  }
}

void KFontChooser::setColor( const QColor & col )
{
  d->m_color = col;
  QPalette pal = sampleEdit->palette();
  pal.setColor( QPalette::Active, QColorGroup::Text, col );
  sampleEdit->setPalette( pal );
}

QColor KFontChooser::color() const
{
  return d->m_color;
}

void KFontChooser::setSizeIsRelative( QButton::ToggleState relative )
{
  // check or uncheck or gray out the "relative" checkbox
  if( sizeIsRelativeCheckBox ) {
    if( QButton::NoChange == relative )
      sizeIsRelativeCheckBox->setNoChange();
    else
      sizeIsRelativeCheckBox->setChecked(  QButton::On == relative );
  }
}

QButton::ToggleState KFontChooser::sizeIsRelative() const
{
  return sizeIsRelativeCheckBox
       ? sizeIsRelativeCheckBox->state()
       : QButton::NoChange;
}

QSize KFontChooser::sizeHint( void ) const
{
  return( minimumSizeHint() );
}


void KFontChooser::enableColumn( int column, bool state )
{
  if( column & FamilyList )
  {
    familyListBox->setEnabled(state);
  }
  if( column & StyleList )
  {
    styleListBox->setEnabled(state);
  }
  if( column & SizeList )
  {
    sizeListBox->setEnabled(state);
  }
  if( column & CharsetList && d->charsetLabel && charsetsCombo )
  {
    d->charsetLabel->setEnabled(state);
    charsetsCombo->setEnabled(state);
  }
}


void KFontChooser::setFont( const QFont& aFont, bool onlyFixed )
{
  selFont = aFont;
  if( onlyFixed != usingFixed)
  {
    usingFixed = onlyFixed;
    fillFamilyListBox(usingFixed);
  }
  setupDisplay();
  displaySample(selFont);
}

#if QT_VERSION < 300
void KFontChooser::setCharset( const QString & charset )
{
    for ( int i = 0; i < charsetsCombo->count(); i++ ) {
        if ( charsetsCombo->text( i ) == charset ) {
            charsetsCombo->setCurrentItem( i );
            return;
        }
    }
}
#endif

void KFontChooser::charset_chosen_slot(const QString& chset)
{
#if QT_VERSION < 300
  KCharsets *charsets = KGlobal::charsets();
  if (chset == i18n("default")) {
    selFont.setCharSet(charsets->nameToID(KGlobal::locale()->charset()));
  } else {
    kdDebug() << "KFontChooser::charset_chosen_slot chset=" << chset << endl;
    selFont.setCharSet(charsets->nameToID(chset));
  }

  emit fontSelected(selFont);
#else
  Q_UNUSED( chset );
#endif
}

#if QT_VERSION < 300
QString KFontChooser::charset() const
{
  return charsetsCombo->currentText();
}

#endif

int KFontChooser::fontDiffFlags() {
   int diffFlags = 0;
   if (familyCheckbox && styleCheckbox && sizeCheckbox) {
      diffFlags = familyCheckbox->isChecked() ? FontDiffFamily : 0
                |  styleCheckbox->isChecked() ? FontDiffStyle  : 0
                |   sizeCheckbox->isChecked() ? FontDiffSize   : 0;
   }
   return diffFlags;
}

void KFontChooser::fillCharsetsCombo()
{
#if QT_VERSION < 300
  int i;
  KCharsets *charsets=KGlobal::charsets();

  charsetsCombo->clear();
  QStringList sets=charsets->availableCharsetNames(selFont.family());
  charsetsCombo->insertItem( i18n("default") );
  for ( QStringList::Iterator it = sets.begin(); it != sets.end(); ++it )
      charsetsCombo->insertItem( *it );
  // This doesn't make any sense, according to everyone I asked (DF)
  // charsetsCombo->insertItem( i18n("any") );

  QString charset=charsets->xCharsetName(selFont.charSet());
  for(i = 0; i < charsetsCombo->count(); i++){
    if (charset == charsetsCombo->text(i)){
      charsetsCombo->setCurrentItem(i);
      break;
    }
  }
#endif
}

void KFontChooser::toggled_checkbox()
{
  familyListBox->setEnabled( familyCheckbox->isChecked() );
  styleListBox->setEnabled( styleCheckbox->isChecked() );
  sizeListBox->setEnabled( sizeCheckbox->isChecked() );
}

void KFontChooser::family_chosen_slot(const QString& family)
{
    fillCharsetsCombo();

    QFontDatabase dbase;
    QStringList styles = QStringList(dbase.styles(family));
    styleListBox->clear();
    currentStyles.clear();
    for ( QStringList::Iterator it = styles.begin(); it != styles.end(); ++it ) {
        QString style = *it;
        int pos = style.find("Plain");
        if(pos >=0) style = style.replace(pos,5,i18n("Regular"));
        pos = style.find("Normal");
        if(pos >=0) style = style.replace(pos,6,i18n("Regular"));
        pos = style.find("Oblique");
        if(pos >=0) style = style.replace(pos,7,i18n("Italic"));
        if(styleListBox->findItem(style) ==0) {
            styleListBox->insertItem(style);
            currentStyles.insert(style, *it);
        }
    }

    QString origSelectedStyle = selectedStyle; // don't let the next line overwrite our cache via signals/slots..
    styleListBox->setSelected(styleListBox->findItem(selectedStyle), true);

    QString style = selectedStyle;
    if(styleListBox->currentItem() < 0) { // fallback if the style does not exist for this font.
        style = styleListBox->text(0);
    }

    sizeListBox->clear();
    if(dbase.isSmoothlyScalable(family)) {  // is vector font
        // TODO set an appropriate "Vector" background to the preview area.
        fillSizeList();
    } else {                                // is bitmap font.
        // TODO set an appropriate "Bitmap" background to the preview area.
        QValueList<int> sizes = dbase.smoothSizes(family, style);
        if(sizes.count() > 0) {
            QValueList<int>::iterator it;
            for ( it = sizes.begin(); it != sizes.end(); ++it ) {
                sizeListBox->insertItem(QString::number(*it));
            }
        } else { // there are times QT does not provide the list..
            fillSizeList(); 
        }
    }
    int origSelectedSize = selectedSize; // same as selectedStyle above.
    // The next command will emit the fontSelected signal for a second time, no idea how to avoid it..
    sizeListBox->setSelected(sizeListBox->findItem(QString::number(selectedSize)), true);

    if(styleListBox->currentItem() < 0 ||  sizeListBox->currentItem() < 0) { 
        // only do this if the style was not present; because the slot of either list would have done this 
        // allready
        //kdDebug() << "Showing: " << family << ", " << currentStyles[style] << ", " << origSelectedSize << endl;
        selFont= dbase.font(family, currentStyles[style], origSelectedSize);
        emit fontSelected(selFont);
        selectedStyle = origSelectedStyle;
    }
    selectedSize=origSelectedSize;
}

void KFontChooser::size_chosen_slot(const QString& size){

  selectedSize=size.toInt();

  selFont.setPointSize(selectedSize);
  emit fontSelected(selFont);
}

void KFontChooser::style_chosen_slot(const QString& style)
{
  selectedStyle= style;

  QFontDatabase dbase;
  //kdDebug() << "Showing: " << familyListBox->currentText() << ", " << currentStyles[style] << ", " << selectedSize << endl;
  selFont = dbase.font(familyListBox->currentText(), currentStyles[style], selectedSize);
  emit fontSelected(selFont);
}

void KFontChooser::displaySample(const QFont& font)
{
  sampleEdit->setFont(font);
  sampleEdit->setCursorPosition(0);
  xlfdEdit->setText(font.rawName());
  xlfdEdit->setCursorPosition(0);
}

void KFontChooser::setupDisplay()
{
  QString aString;
  int numEntries, i=0;

  numEntries =  familyListBox->count();
  aString = selFont.family();

  for (i = 0; i < numEntries; i++) {
    if (aString.lower() == (familyListBox->text(i).lower())) {
      familyListBox->setCurrentItem(i);
      break;
    }
  }


  numEntries =  sizeListBox->count();
  aString.setNum(selFont.pointSize());

  for (i = 0; i < numEntries; i++){
    if (aString == sizeListBox->text(i)) {
      sizeListBox->setCurrentItem(i);
      break;
    }
  }

  i = (selFont.bold() ? 2 : 0);
  i += (selFont.italic() ? 1 : 0);

  styleListBox->setCurrentItem(i);

  // Re-create displayable charsets list
  fillCharsetsCombo();
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
        if ((fontListCriteria & (SmoothScalableFonts | ScalableFonts) == ScalableFonts) && 
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

void KFontChooser::addFont( QStringList &list, const char *xfont )
{
  const char *ptr = strchr( xfont, '-' );
  if ( !ptr )
    return;

  ptr = strchr( ptr + 1, '-' );
  if ( !ptr )
    return;

  QString font = QString::fromLatin1(ptr + 1);

  int pos;
  if ( ( pos = font.find( '-' ) ) > 0 ) {
    font.truncate( pos );

    if ( font.find( QString::fromLatin1("open look"), 0, false ) >= 0 )
      return;

    QStringList::Iterator it = list.begin();

    for ( ; it != list.end(); ++it )
      if ( *it == font )
	return;
    list.append( font );
  }
}

void KFontChooser::fillFamilyListBox(bool onlyFixedFonts)
{
  QStringList fontList;
  getFontList(fontList, onlyFixedFonts?FixedWidthFonts:0);
  familyListBox->clear();
  familyListBox->insertStringList(fontList);
}

void KFontChooser::showXLFDArea(bool show)
{
  if( show == true )
  {
    xlfdEdit->parentWidget()->show();
  }
  else
  {
    xlfdEdit->parentWidget()->hide();
  }
}

///////////////////////////////////////////////////////////////////////////////

KFontDialog::KFontDialog( QWidget *parent, const char* name,
			  bool onlyFixed, bool modal,
			  const QStringList &fontList, bool makeFrame, bool diff,
                          QButton::ToggleState *sizeIsRelativeState )
  : KDialogBase( parent, name, modal, i18n("Select Font"), Ok|Cancel, Ok )
{
  chooser = new KFontChooser( this, "fontChooser",
                              onlyFixed, fontList, makeFrame, 8,
                              diff, sizeIsRelativeState );
  setMainWidget(chooser);
}


int KFontDialog::getFontDiff( QFont &theFont, int &diffFlags, bool onlyFixed,
                             QWidget *parent, bool makeFrame,
                             QButton::ToggleState *sizeIsRelativeState )
{
  KFontDialog dlg( parent, "Font Selector", onlyFixed, true, QStringList(),
		   makeFrame, true, sizeIsRelativeState );
  dlg.setFont( theFont, onlyFixed );

  int result = dlg.exec();
  if( result == Accepted )
  {
    theFont = dlg.chooser->font();
    diffFlags = dlg.chooser->fontDiffFlags();
    if( sizeIsRelativeState )
      *sizeIsRelativeState = dlg.chooser->sizeIsRelative();
  }
  return( result );
}

int KFontDialog::getFont( QFont &theFont, bool onlyFixed,
                          QWidget *parent, bool makeFrame,
                          QButton::ToggleState *sizeIsRelativeState )
{
  KFontDialog dlg( parent, "Font Selector", onlyFixed, true, QStringList(),
		   makeFrame, false, sizeIsRelativeState );
  dlg.setFont( theFont, onlyFixed );

  int result = dlg.exec();
  if( result == Accepted )
  {
    theFont = dlg.chooser->font();
    if( sizeIsRelativeState )
      *sizeIsRelativeState = dlg.chooser->sizeIsRelative();
  }
  return( result );
}


int KFontDialog::getFontAndText( QFont &theFont, QString &theString,
				 bool onlyFixed, QWidget *parent,
				 bool makeFrame,
                                 QButton::ToggleState *sizeIsRelativeState )
{
  KFontDialog dlg( parent, "Font and Text Selector", onlyFixed, true,
		   QStringList(), makeFrame, false, sizeIsRelativeState );
  dlg.setFont( theFont, onlyFixed );

  int result = dlg.exec();
  if( result == Accepted )
  {
    theFont   = dlg.chooser->font();
    theString = dlg.chooser->sampleText();
    if( sizeIsRelativeState )
      *sizeIsRelativeState = dlg.chooser->sizeIsRelative();
  }
  return( result );
}


/*
****************************************************************************
*
* $Log$
* Revision 1.81  2001/12/21 20:08:35  zander
* For more control over which fonts are shown in the font dialog:
* -  static void getFontList( QStringList &list, bool fixed );
* +  static void getFontList( QStringList &list, uint fontListCriteria);
*
* The fonts styles are no longer expected to be the set of 4 normal types:
* generic/bold/italic/bold-italic.
* Each font is queried which styles are present and those are shown. This
* means the user will get what the dialog shows; not a surprise what QT will
* select.
*
* If present; a list of fontsizes will be used from the font server; otherwise
* the default set will be presented.
*
* Revision 1.80  2001/12/19 10:33:32  zander
* fixed warnings
*
* Revision 1.79  2001/12/18 22:07:01  zander
* Slight change of layout
*
* Revision 1.78  2001/12/18 14:08:37  khz
* minor bugfix: "relative" checkbox is a tristate box _only_ if dialog is in "diff" mode, note however that there is no difference from a programmers point of view: you still will specify a *QButton::ToggleState as parameter, no matter whether you are seeing a tristate box or not (the alternative would be having all respective functions duplicate in the KFontDialog class - something I am not sure we really want to have)
*
* Revision 1.77  2001/12/16 01:10:54  khz
* Changed sizeIsRelative parameter from *bool to *QButton::ToggleState (and converted the "relative" checkbox into a tri-state-checkbox accordingly) to ease interoperation with the new "diff" feature of KFontDialog.
*
* Revision 1.76  2001/12/14 23:48:23  khz
* added initialization of  sizeIsRelativeCheckBox
*
* Revision 1.75  2001/12/14 19:09:30  khz
* adjusted CVS comment: corrected Alexander\'s misspelled alias
*
* Revision 1.74  2001/12/14 19:02:50  khz
* (a) changed "lypanov"s patch (to prevent from breaking compatibility)
* (b) added sizeIsRelative checkbox under size listbox to enable
*     specification of relative font sizes (being adjusted automatically when
*     environment dimension change, e.g. size of the parent widget)
*
* Revision 1.73  2001/12/14 13:59:38  binner
* Use QFontDataBase::isFixedPitch() introduced in Qt 2.3.
*
* Revision 1.72  2001/12/14 10:29:56  lypanov
* Addition of getFontDiff to KFontDialog.
*
* Revision 1.71  2001/10/10 17:40:39  mueller
* CVS_SILENT: fixincludes
*
* Revision 1.70  2001/10/06 11:40:22  adrian
* According to bug #22745 Bold _and_ Italic fonts did not get a right preview.
* This depends on the language which is used. (I think these i18n() statements
* are not very clean).
*
* Revision 1.69  2001/09/22 19:53:57  mueller
* fixes for QT_NO_COMPAT
*
* Revision 1.68  2001/08/29 15:25:38  bero
* _WS_X11_ -> Q_WS_X11
* _WS_QWS_ -> Q_WS_QWS
*
* Revision 1.67  2001/08/26 20:14:19  lunakl
* Ok, watch closely :
* const is your friend !
*
* static int blah[] = { 1, 2, .... }
* should be
* static const int blah[] = { 1, 2, ... }
*
* static const char* txt[] = { "blah", "foo", ... }
* should be
* static const char* const txt[] = { "blah", "foo", ... }
*
* And just in case you wonder about those const_cast< const char** >, that's
* because QPixmap( const char** xpm ) and QImage( const char** xpm ) got it
* wrong too. Everybody guessing correctly where the const is missing wins
* a free cvs update.
*
* Revision 1.66  2001/08/26 10:03:47  bero
* Allow to build with Qt/Embedded
*
* Revision 1.65  2001/08/18 15:02:42  hausmann
* - disable all charset related functionality for qt3
*
* Revision 1.64  2001/07/19 10:20:35  faure
* Don't try to be clever when choosing a charset, it only hurts.
* -    charsets->setQFont(selFont, chset);
* +    selFont.setCharSet(charsets->nameToID(chset));
* For instance when choosing the Unicode charset, QFontInfo was saying "already
* using this charset" (when AA is enabled), so setQFont() would do _nothing_.
* Approved by Lukas.
*
* Revision 1.63  2001/07/01 20:10:30  faure
* Fixed BC breakage (Michael H's commit added a member var to the KFontChooser
* class). Just got a crash when using KWord compiled against 2.1 running
* against 2.2-cvs. Don't forget KFontChooser is part of the public API too :)
*
* Revision 1.62  2001/05/22 13:05:26  faure
* There was no objection, so I'm committing the patch for changing the color
* of the text in the preview.
*
* Revision 1.61  2001/04/11 14:50:21  haeckel
* Add the ability to disable the charset combo.
*
* Revision 1.60  2001/03/10 15:38:54  faure
* Don't call kdeFonts(), return all fonts.
*
* Revision 1.59  2001/01/31 19:15:20  porten
* commented out unused variables. a bit ugly but a hint to potential maintainers
*
* Revision 1.58  2001/01/15 14:27:24  knoll
* arrggll... and I was wondering why people complained about not
* seeing any thai fonts...
*
* Revision 1.57  2000/12/11 23:50:47  granroth
* Pretty major change in the way that fonts are found.  Previously, all
* fonts using KFontDialog or KFontChooser were found using XListFonts.
* Unfortunately, this breaks whenever such things like the RENDER
* extension are used or KDE runs in (theoretically!) Qt/Embedded or even
* Windows.
*
* The Right Way(tm) to do this is to just use Qt for all font
* information.  Since there is the QFontDatabase class that is *similar*
* in functionality to XListFonts, it's clearly the way to go.
*
* For 99.9% of the cases, this does work great -- it's fast and accurate
* and works with Xft.  The case where it fails is with fixed width
* fonts -- QFontDatabase doesn't contain that knowledge.  So I had to
* work around this with a huge hack involving iterating through all
* fonts and instantiating a QFontInfo on each.  If you would guess that
* this is *very* slow, you would be right.  On the one plus side, I only
* found one place in all of KDE that actually uses this feature
* (khexedit).
*
* I left the XListFont stuff in for backwards compatability (for now).
* Since nothing in KDE uses it, we'll have to get rid of it somehow.
*
* Revision 1.56  2000/09/25 13:45:17  faure
* * Added extractors for the charset that is set in the combobox.
* The font holds the charset, but this means 'default' was useless.
* Now the application can know when "default" was chosen.
* * Removed "any" entry in the charset combo.
* According to Denis, Lukas and Lars, it doesn't make any sense.
*
* Revision 1.55  2000/09/22 12:37:26  faure
* Fixed another charset bug: KFontDialog didn't find the charset in
* the list.
* -  QString charset=charsets->name(selFont);
* +  QString charset=charsets->xCharsetName(selFont.charSet());
*
* Revision 1.54  2000/06/16 11:40:38  coolo
* removing the #ifdef HAVE_CONFIG_H stuff. This isn't logical anymore now we
* define even prototypes within config.h. someone I know tried kdevelop to setup
* a little C++ project and the first line of code was #ifdef HAVE_CONFIG_H and
* asked me what this does. And the only valid answer one can have is "it's historical"
*
* Revision 1.53  2000/06/03 21:00:11  bieker
* DOn't include headers we don't use.
*
* Revision 1.52  2000/06/03 01:18:17  gehrmab
* KGlobal <-> KGlobalSettings <-> KApplication cleanup
*
* Revision 1.51  2000/05/03 19:25:41  jsixt
* Use KConfigGroupSaver instead of a simple setGroup() to change
* the group in config files and to revert it back again automatically.
*
* Revision 1.50  2000/04/23 11:44:20  espen
*
* New (bin. compatible) feature to allow an application to
* disable columns in the KFontChooser. Useful if only the font
* size should be selectable etc. kmail will use this capability.
*
* Revision 1.49  2000/01/18 21:15:02  espen
* QListBox -> KListBox
*
* Revision 1.48  2000/01/17 19:07:58  bieker
* Made it more QT_NO_CAST_ASCII and QT_NO_ASCII_CAST safe (this is not 100 %
* yet).
*
* Revision 1.47  1999/10/31 11:39:55  reggie
* KLineEdit -> QLineEdit
*
* Revision 1.46  1999/10/23 16:16:26  kulow
* here comes KInstance - "KApplication light"
* It's a new KLibGlobal and KGlobal only keeps a pointer to a global
* instance. KApplication is derived from KInstance - making it possible
* to move things out of KApplication into KInstance to make Components
* easier.
* Needs some testings and I bet some tweaks here and there :)
*
* Revision 1.45  1999/10/09 00:08:29  kalle
* The dreaded library cleanup: getConfig() -> config() and friends (see separate mail)
*
* Revision 1.44  1999/09/30 21:30:56  espensa
* KFontCooser:
* 1. Reimplemented the sizeHint() to return the value of minimumSizeHint()
* 2. Made getFontList( QStringList &, const char * ) static so that we have
* a tool for creating fontnames outside the KFontCooser. Useful when the
* default lists made by KFontCooser is not usable.
*
* Revision 1.43  1999/09/29 21:58:02  espensa
* KFontDialog: Converted to use KDialogBase. Result is underlined
* buttons and correct caption. The static methods can now specify
* a parent widget which enables centered dialogs. The frame can be
* turned off in the constructor. I suggest it should be default off.
* Currently it is on.
*
* KFontChooser: Cleanup. Uses KDialog::spacingHint() in the layouts.
* It can now be displayed without frames (selected from constructor).
* This makes it more versatile in dialogs where the frames may not
* fit. The lists are now resized properly on creation. No horizontal
* scrollbars on startup! Preview field made higher.
*
*
* Revision 1.42  1999/07/27 04:12:08  pbrown
* layout improvements.
*
* Revision 1.41  1999/06/14 10:56:28  kulow
* some more warnings fixed
*
* Revision 1.40  1999/06/12 21:43:58  knoll
* kapp->xxxFont() -> KGlobal::xxxFont()
*
* Revision 1.39  1999/06/11 20:01:33  knoll
* changes to make it working with new kcharsets
*
* Revision 1.38  1999/06/07 18:21:28  pbrown
* QLineEdit --> KLineEdit, header cleanup.
*
* Revision 1.37  1999/05/29 09:44:26  mario
* Mario: some small fixes, and optimized bloated code
*
*
****************************************************************************
*/
