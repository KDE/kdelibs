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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <qfile.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qscrollbar.h>
#include <qfont.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <kglobal.h>
#include <kdialog.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kbuttonbox.h>
#include <qlineedit.h>
#include <kstddirs.h>
#include <kapp.h>
#include <kconfig.h>

#include <X11/Xlib.h>

#include "kfontdialog.moc"

#define MINSIZE(x) x->setMinimumSize(x->sizeHint());

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



KFontChooser::KFontChooser(QWidget *parent, const char *name,
			   bool onlyFixed, const QStringList &fontList,
			   bool makeFrame, int visibleListSize )
  : QWidget(parent, name), usingFixed(onlyFixed)
{
  QVBoxLayout *topLayout = new QVBoxLayout( this, 0, KDialog::spacingHint() );

  QWidget *page;
  QGridLayout *gridLayout;
  int row = 0;
  if( makeFrame == true )
  {
    page = new QGroupBox( i18n("Requested Font"), this );
    topLayout->addWidget(page);
    gridLayout = new QGridLayout( page, 5, 3, KDialog::spacingHint() );
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
  QLabel *familyLabel = new QLabel( i18n("Font"), page, "familyLabel" );
  gridLayout->addWidget(familyLabel, row, 0, AlignLeft );
  QLabel *styleLabel = new QLabel( i18n("Font style"), page, "styleLabel");
  gridLayout->addWidget(styleLabel, row, 1, AlignLeft);
  QLabel *sizeLabel = new QLabel( i18n("Size"), page, "sizeLabel");
  gridLayout->addWidget(sizeLabel, row, 2, AlignLeft);

  row ++;

  //
  // now create the actual boxes that hold the info
  //
  familyListBox = new QListBox( page, "familyListBox");
  gridLayout->addWidget( familyListBox, row, 0 );
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

  styleListBox = new QListBox( page, "styleListBox");
  gridLayout->addWidget(styleListBox, row, 1);
  styleListBox->insertItem(i18n("Regular"));
  styleListBox->insertItem(i18n("Italic"));
  styleListBox->insertItem(i18n("Bold"));
  styleListBox->insertItem(i18n("Bold Italic"));
  styleListBox->setMinimumWidth( minimumListWidth( styleListBox ) );
  styleListBox->setMinimumHeight(
    minimumListHeight( styleListBox, visibleListSize  ) );

  connect(styleListBox, SIGNAL(highlighted(const QString &)),
	  SLOT(style_chosen_slot(const QString &)));


  sizeListBox = new QListBox( page, "sizeListBox");
  gridLayout->addWidget(sizeListBox, row, 2);

  const char *c[] =
  {
    "4",  "5",  "6",  "7",
    "8",  "9",  "10", "11",
    "12", "13", "14", "15",
    "16", "17", "18", "19",
    "20", "22", "24", "26",
    "28", "32", "48", "64",
    0
  };
  for(int i = 0; c[i] != 0; i++)
  {
    sizeListBox->insertItem(c[i]);
  }
  sizeListBox->setMinimumWidth( minimumListWidth(sizeListBox) +
    sizeListBox->fontMetrics().maxWidth() );
  sizeListBox->setMinimumHeight(
    minimumListHeight( sizeListBox, visibleListSize  ) );


  connect( sizeListBox, SIGNAL(highlighted(const QString&)),
	   SLOT(size_chosen_slot(const QString&)) );

  row ++;
  QLabel *charsetLabel = new QLabel( page, "charsetLabel");
  charsetLabel->setText(i18n("Character set:"));
  gridLayout->addWidget(charsetLabel, 3, 0, AlignRight);
  charsetsCombo = new QComboBox(true, page, "charsetsCombo");
  gridLayout->addMultiCellWidget(charsetsCombo, 3, 3, 1, 2);
  charsetsCombo->setInsertionPolicy(QComboBox::NoInsertion);
  connect(charsetsCombo, SIGNAL(activated(const QString&)),
	  SLOT(charset_chosen_slot(const QString&)));

  row ++;
  sampleEdit = new QLineEdit( page, "sampleEdit");
  QFont tmpFont( KGlobal::generalFont().family(), 64, QFont::Black );
  sampleEdit->setFont(tmpFont);
  sampleEdit->setText("The Quick Brown Fox Jumps Over The Lazy Dog");
  sampleEdit->setMinimumHeight( sampleEdit->fontMetrics().lineSpacing() );
  sampleEdit->setAlignment(Qt::AlignCenter);
  gridLayout->addMultiCellWidget(sampleEdit, 4, 4, 0, 2);
  connect(this, SIGNAL(fontSelected(const QFont &)),
	  SLOT(displaySample(const QFont &)));

  gridLayout->activate();

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
  setFont( KGlobal::generalFont(), usingFixed );
  // Create displayable charsets list
  fillCharsetsCombo();

  vbox->activate();

  KConfig *config = KGlobal::config();
  config->setGroup("General");
  showXLFDArea(config->readBoolEntry("fontSelectorShowXLFD", false));

  topLayout->activate();
}

QSize KFontChooser::sizeHint( void ) const
{
  return( minimumSizeHint() );
}



void KFontChooser::charset_chosen_slot(const QString& chset)
{
  KCharsets *charsets = KGlobal::charsets();
  if (chset == "default") {
    charsets->setQFont(selFont, KGlobal::locale()->charset());
  } else {
    charsets->setQFont(selFont, chset);
  }

  emit fontSelected(selFont);
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


void KFontChooser::fillCharsetsCombo()
{
  int i;
  KCharsets *charsets=KGlobal::charsets();

  charsetsCombo->clear();
  QStringList sets=charsets->availableCharsetNames(selFont.family());
  charsetsCombo->insertItem( i18n("default") );
  for ( QStringList::Iterator it = sets.begin(); it != sets.end(); ++it )
      charsetsCombo->insertItem( *it );
  charsetsCombo->insertItem( i18n("any") );

  QString charset=charsets->name(selFont);
  for(i = 0; i < charsetsCombo->count(); i++){
    if (charset == charsetsCombo->text(i)){
      charsetsCombo->setCurrentItem(i);
      break;
    }
  }
}

void KFontChooser::family_chosen_slot(const QString& family)
{
  selFont.setFamily(family);

  fillCharsetsCombo();

  emit fontSelected(selFont);
}

void KFontChooser::size_chosen_slot(const QString& size){

  QString size_string = size;

  selFont.setPointSize(size_string.toInt());
  emit fontSelected(selFont);
}

void KFontChooser::style_chosen_slot(const QString& style)
{
  QString style_string = style;

  if ( style_string.find(i18n("Italic")) != -1)
    selFont.setItalic(true);
  else
    selFont.setItalic(false);
  if ( style_string.find(i18n("Bold")) != -1)
    selFont.setBold(true);
  else
    selFont.setBold(false);
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
  bool found;

  numEntries =  familyListBox->count();
  aString = selFont.family();
  found = false;

  for (i = 0; i < numEntries; i++) {
    if (aString.lower() == (familyListBox->text(i).lower())) {
      familyListBox->setCurrentItem(i);
      found = true;
      break;
    }
  }


  numEntries =  sizeListBox->count();
  aString.setNum(selFont.pointSize());
  found = false;

  for (i = 0; i < numEntries; i++){
    if (aString == sizeListBox->text(i)) {
      sizeListBox->setCurrentItem(i);
      found = true;
      break;
    }
  }

  i = (selFont.bold() ? 2 : 0);
  i += (selFont.italic() ? 1 : 0);

  styleListBox->setCurrentItem(i);

  // Re-create displayable charsets list
  fillCharsetsCombo();
}


void KFontChooser::getFontList( QStringList &list, bool fixed )
{
  //
  // Use KDE fonts if there is a KDE font list and check that the fonts
  // exist on the server where the desktop is running.
  //
  QStringList lstSys, lstKDE;

  if ( fixed )
  {
    getFontList( lstSys, "-*-*-*-*-*-*-*-*-*-*-m-*-*-*" );
    getFontList( lstSys, "-*-*-*-*-*-*-*-*-*-*-c-*-*-*" );
  }
  else
  {
    //getFontList( lstSys, "-*-*-*-*-*-*-*-*-*-*-p-*-*-*" );
    getFontList(lstSys, "*");
  }

  lstSys.sort();

  if ( !kapp->kdeFonts( lstKDE ) )
  {
    list = lstSys;
    return;
  }

  for (int i = 0; i < (int) lstKDE.count(); i++) {
    if (lstSys.contains(lstKDE[i]))
      list.append(lstKDE[i]);
  }
  list.sort();
}



void KFontChooser::getFontList( QStringList &list, const char *pattern )
{
  int num;
  char **xFonts = XListFonts( qt_xdisplay(), pattern, 2000, &num );

  for( int i = 0; i < num; i++ )
  {
    addFont( list, xFonts[i] );
  }
  XFreeFontNames( xFonts );
}



void KFontChooser::addFont( QStringList &list, const char *xfont )
{
  const char *ptr = strchr( xfont, '-' );
  if ( !ptr )
    return;

  ptr = strchr( ptr + 1, '-' );
  if ( !ptr )
    return;

  QString font = ptr + 1;

  int pos;
  if ( ( pos = font.find( '-' ) ) > 0 ) {
    font.truncate( pos );

    if ( font.find( "open look", 0, false ) >= 0 )
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
  getFontList(fontList, onlyFixedFonts);
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
			  const QStringList &fontList, bool makeFrame )
  : KDialogBase( parent, name, modal, i18n("Select Font"), Ok|Cancel, Ok )
{
  chooser = new KFontChooser(this,"fontChooser",onlyFixed,fontList,makeFrame);
  setMainWidget(chooser);
}


int KFontDialog::getFont( QFont &theFont, bool onlyFixed, QWidget *parent,
			  bool makeFrame )
{
  KFontDialog dlg( parent, "Font Selector", onlyFixed, true, QStringList(),
		   makeFrame );
  dlg.setFont( theFont, onlyFixed );

  int result = dlg.exec();
  if( result == Accepted )
  {
    theFont = dlg.chooser->font();
  }
  return( result );
}


int KFontDialog::getFontAndText( QFont &theFont, QString &theString,
				 bool onlyFixed, QWidget *parent,
				 bool makeFrame )
{
  KFontDialog dlg( parent, "Font and Text Selector", onlyFixed, true,
		   QStringList(), makeFrame );
  dlg.setFont( theFont, onlyFixed );

  int result = dlg.exec();
  if( result == Accepted )
  {
    theFont   = dlg.chooser->font();
    theString = dlg.chooser->sampleText();
  }
  return( result );
}


/*
****************************************************************************
*
* $Log$
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
