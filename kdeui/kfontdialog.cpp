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
#include <qfile.h>
#include <qfont.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qscrollbar.h>
#include <qstringlist.h>
#include <qfontdatabase.h>

#include <kapp.h>
#include <kcharsets.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <qlineedit.h>
#include <klistbox.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kdebug.h>

#include <X11/Xlib.h>

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
			   bool makeFrame, int visibleListSize )
  : QWidget(parent, name), usingFixed(onlyFixed)
{
  d = new KFontChooserPrivate;
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
  familyLabel = new QLabel( i18n("Font"), page, "familyLabel" );
  gridLayout->addWidget(familyLabel, row, 0, AlignLeft );
  styleLabel = new QLabel( i18n("Font style"), page, "styleLabel");
  gridLayout->addWidget(styleLabel, row, 1, AlignLeft);
  sizeLabel = new QLabel( i18n("Size"), page, "sizeLabel");
  gridLayout->addWidget(sizeLabel, row, 2, AlignLeft);

  row ++;

  //
  // now create the actual boxes that hold the info
  //
  familyListBox = new KListBox( page, "familyListBox");
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

  styleListBox = new KListBox( page, "styleListBox");
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


  sizeListBox = new KListBox( page, "sizeListBox");
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
    sizeListBox->insertItem(QString::fromLatin1(c[i]));
  }
  sizeListBox->setMinimumWidth( minimumListWidth(sizeListBox) +
    sizeListBox->fontMetrics().maxWidth() );
  sizeListBox->setMinimumHeight(
    minimumListHeight( sizeListBox, visibleListSize  ) );


  connect( sizeListBox, SIGNAL(highlighted(const QString&)),
	   SLOT(size_chosen_slot(const QString&)) );

  row ++;
  d->charsetLabel = new QLabel( page, "charsetLabel");
  d->charsetLabel->setText(i18n("Character set:"));
  gridLayout->addWidget(d->charsetLabel, 3, 0, AlignRight);
  charsetsCombo = new QComboBox(true, page, "charsetsCombo");
  gridLayout->addMultiCellWidget(charsetsCombo, 3, 3, 1, 2);
  charsetsCombo->setInsertionPolicy(QComboBox::NoInsertion);
  connect(charsetsCombo, SIGNAL(activated(const QString&)),
	  SLOT(charset_chosen_slot(const QString&)));

  row ++;
  sampleEdit = new QLineEdit( page, "sampleEdit");
  QFont tmpFont( KGlobalSettings::generalFont().family(), 64, QFont::Black );
  sampleEdit->setFont(tmpFont);
  sampleEdit->setText(i18n("The Quick Brown Fox Jumps Over The Lazy Dog"));
  sampleEdit->setMinimumHeight( sampleEdit->fontMetrics().lineSpacing() );
  sampleEdit->setAlignment(Qt::AlignCenter);
  gridLayout->addMultiCellWidget(sampleEdit, 4, 4, 0, 2);
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

  KConfig *config = KGlobal::config();
  KConfigGroupSaver saver(config, QString::fromLatin1("General"));
  showXLFDArea(config->readBoolEntry(QString::fromLatin1("fontSelectorShowXLFD"), false));
}

KFontChooser::~KFontChooser()
{
  delete d;
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

QSize KFontChooser::sizeHint( void ) const
{
  return( minimumSizeHint() );
}


void KFontChooser::enableColumn( int column, bool state )
{
  if( column & FamilyList )
  {
    familyLabel->setEnabled(state);
    familyListBox->setEnabled(state);
  }
  if( column & StyleList )
  {
    styleLabel->setEnabled(state);
    styleListBox->setEnabled(state);
  }
  if( column & SizeList )
  {
    sizeLabel->setEnabled(state);
    sizeListBox->setEnabled(state);
  }
  if( column & CharsetList )
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

void KFontChooser::setCharset( const QString & charset )
{
    for ( int i = 0; i < charsetsCombo->count(); i++ ) {
        if ( charsetsCombo->text( i ) == charset ) {
            charsetsCombo->setCurrentItem( i );
            return;
        }
    }
}

void KFontChooser::charset_chosen_slot(const QString& chset)
{
  KCharsets *charsets = KGlobal::charsets();
  if (chset == i18n("default")) {
    selFont.setCharSet(charsets->nameToID(KGlobal::locale()->charset()));
  } else {
    kdDebug() << "KFontChooser::charset_chosen_slot chset=" << chset << endl;
    selFont.setCharSet(charsets->nameToID(chset));
  }

  emit fontSelected(selFont);
}

QString KFontChooser::charset() const
{
  return charsetsCombo->currentText();
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
  // This doesn't make any sense, according to everyone I asked (DF)
  // charsetsCombo->insertItem( i18n("any") );

  QString charset=charsets->xCharsetName(selFont.charSet());
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

  if ( style_string.contains(i18n("Italic"))||
       style_string.contains(i18n("Bold Italic")) )
    selFont.setItalic(true);
  else
    selFont.setItalic(false);

  if ( style_string.contains(i18n("Bold")) ||
       style_string.contains(i18n("Bold Italic")) )
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
  //  bool found;

  numEntries =  familyListBox->count();
  aString = selFont.family();
  //  found = false;

  for (i = 0; i < numEntries; i++) {
    if (aString.lower() == (familyListBox->text(i).lower())) {
      familyListBox->setCurrentItem(i);
      //      found = true;
      break;
    }
  }


  numEntries =  sizeListBox->count();
  aString.setNum(selFont.pointSize());
  //  found = false;

  for (i = 0; i < numEntries; i++){
    if (aString == sizeListBox->text(i)) {
      sizeListBox->setCurrentItem(i);
      //      found = true;
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
  QFontDatabase dbase;
  QStringList lstSys(dbase.families( false ));

  // Since QFontDatabase doesn't have any easy way of returning just
  // the fixed width fonts, we'll do it in a very hacky way
  if (fixed)
  {
    QStringList lstFixed;
    for (QStringList::Iterator it = lstSys.begin(); it != lstSys.end(); ++it)
    {
        // To get the fixed with info (known as fixed pitch in Qt), we
        // need to get a QFont or QFontInfo object.  To do this, we
        // need a family name, style, and point size.
        QStringList styles(dbase.styles(*it));
        QStringList::Iterator astyle = styles.begin();

        QFontInfo info(dbase.font(*it, *astyle, 10));
        if (info.fixedPitch())
          lstFixed.append(*it);
    }

    // Fallback.. if there are no fixed fonts found, it's probably a
    // bug in the font server or Qt.  In this case, just use 'fixed'
    if (lstFixed.count() == 0)
      lstFixed.append("fixed");

    lstSys = lstFixed;
  }

  lstSys.sort();

  list = lstSys;
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
