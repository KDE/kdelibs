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
#include <qfont.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <kglobal.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kbuttonbox.h>
#include <klined.h>
#include <kstddirs.h>
#include <kapp.h>
#include <kconfig.h>

#include <X11/Xlib.h>

#include "kfontdialog.moc"

#define MINSIZE(x) x->setMinimumSize(x->sizeHint());


KFontChooser::KFontChooser(QWidget *parent, const char *name,
			   bool onlyFixed,
			   const QStringList &fontList)
  : QWidget(parent, name), usingFixed(onlyFixed)
{
  QVBoxLayout *layout = new QVBoxLayout(this, 10);

  QGroupBox *box1 = new QGroupBox(i18n("Requested Font"), this);
  layout->addWidget(box1);

  xlfdBox = new QGroupBox(i18n("Actual Font"), this);
  layout->addWidget(xlfdBox);

  QGridLayout *box1Layout = new QGridLayout(box1, 7, 5, 5);

  // some spacing at the top
  box1Layout->addRowSpacing(0, 15);

  // space on either side
  box1Layout->addColSpacing(0, 10);
  box1Layout->addColSpacing(6, 10);
  box1Layout->setColStretch(6, 1);

  // first, create the labels across the top
  QLabel *familyLabel = new QLabel(box1, "familyLabel");
  box1Layout->addWidget(familyLabel, 1, 1);
  familyLabel->setText( i18n("Font:") );
  MINSIZE(familyLabel);

  // space between listboxes
  box1Layout->addColSpacing(2, 5);

  QLabel *styleLabel = new QLabel(box1, "styleLabel");
  styleLabel->setText(i18n("Font style:"));
  MINSIZE(styleLabel);
  box1Layout->addWidget(styleLabel, 1, 3);

  // space between listboxes
  box1Layout->addColSpacing(4, 5);

  QLabel *sizeLabel = new QLabel(box1, "sizeLabel");
  box1Layout->addWidget(sizeLabel, 1, 5);
  sizeLabel->setText("Size:");
  MINSIZE(sizeLabel);

  // now create the actual boxes that hold the info
  familyListBox = new QListBox(box1, "familyListBox");
  box1Layout->addWidget(familyListBox, 2, 1);
  connect(familyListBox, SIGNAL(highlighted(const QString &)),
	  SLOT(family_chosen_slot(const QString &)));
  if (fontList.count() != 0) {
    familyListBox->insertStringList(fontList);
  } else
    fillFamilyListBox(onlyFixed);

  // right now, Qt seems to be having trouble with sizeHint() for listboxes,
  // so we will just set a sane height.  FIX ME
    //  familyListBox->setRowMode(8);
  //  MINSIZE(familyListBox);
  familyListBox->setFixedSize(180,140);

  styleListBox = new QListBox(box1, "styleListBox");
  box1Layout->addWidget(styleListBox, 2, 3);
  styleListBox->insertItem(i18n("Regular"));
  styleListBox->insertItem(i18n("Italic"));
  styleListBox->insertItem(i18n("Bold"));
  styleListBox->insertItem(i18n("Bold Italic"));

  connect(styleListBox, SIGNAL(highlighted(const QString &)),
	  SLOT(style_chosen_slot(const QString &)));
  //  styleListBox->setRowMode(8);
  //  MINSIZE(styleListBox);
  styleListBox->setFixedSize(100,140);

  sizeListBox = new QListBox(box1, "sizeListBox");
  box1Layout->addWidget(sizeListBox, 2, 5);

  char *c[] = {"4",  "5",  "6",  "7",
	       "8",  "9",  "10", "11",
	       "12", "13", "14", "15",
	       "16", "17", "18", "19",
	       "20", "22", "24", "26",
	       "28", "32", "48", "64",
	       0};
  for(int i = 0; c[i] != 0; i++)
    sizeListBox->insertItem(c[i]);

  connect( sizeListBox, SIGNAL(highlighted(const QString&)),
	  SLOT(size_chosen_slot(const QString&)) );
  //  sizeListBox->setRowMode(8);
  //  MINSIZE(sizeListBox);
  sizeListBox->setFixedSize(50,140);

  QLabel *charsetLabel = new QLabel(box1, "charsetLabel");
  box1Layout->addWidget(charsetLabel, 3, 1, Qt::AlignRight);
  charsetLabel->setText(i18n("Character set:"));
  MINSIZE(charsetLabel);

  charsetsCombo = new QComboBox(true, box1, "charsetsCombo");
  box1Layout->addMultiCellWidget(charsetsCombo, 3, 3, 2, 3);

  charsetsCombo->setInsertionPolicy(QComboBox::NoInsertion);
  connect(charsetsCombo, SIGNAL(activated(const QString&)),
	  SLOT(charset_chosen_slot(const QString&)));
  MINSIZE(charsetsCombo);

  sampleEdit = new KLineEdit(box1, "sampleEdit");
  sampleEdit->setAlignment(Qt::AlignCenter);
  box1Layout->addMultiCellWidget(sampleEdit, 4, 4, 1, 5);
  QFont tmpFont(selFont);
  tmpFont.setPointSize(24);
  sampleEdit->setFont(tmpFont);
  sampleEdit->setText("The Quick Brown Fox Jumps Over The Lazy Dog");

  connect(this, SIGNAL(fontSelected(const QFont &)),
	  SLOT(displaySample(const QFont &)));
  MINSIZE(sampleEdit);
  sampleEdit->setFont(selFont);

  box1Layout->addRowSpacing(5, 15);

  box1Layout->activate();

  QVBoxLayout *box2Layout = new QVBoxLayout(xlfdBox, 10);

  box2Layout->addSpacing(15);

  xlfdLabel = new QLabel(xlfdBox, "xlfdLabel");
  xlfdLabel->setFont(kapp->fixedFont());
  xlfdLabel->setAlignment(Qt::AlignCenter);
  box2Layout->addWidget(xlfdLabel);

  box2Layout->addSpacing(15);

  box2Layout->activate();

  // lets initialize the display if possible
  setFont(kapp->generalFont());

  // Create displayable charsets list
  fillCharsetsCombo();

  layout->activate();

  KConfig *config = KGlobal::config();
  config->setGroup("General");
  showXLFDArea(config->readBoolEntry("fontSelectorShowXLFD", false));
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

void KFontChooser::setFont( const QFont& aFont, bool onlyFixed)
{
  selFont = aFont;
  if (onlyFixed != usingFixed) {
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
  QStrList sets=charsets->displayable(selFont.family().ascii());
  charsetsCombo->insertItem( i18n("default") );
  for(QString set=sets.first();!set.isNull();set=sets.next())
    charsetsCombo->insertItem( set );
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

  xlfdLabel->setText(font.rawName());
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

void KFontChooser::getFontList( QStringList &list, const char *pattern )
{
  int num;
  char **xFonts = XListFonts( qt_xdisplay(), pattern, 2000, &num );

  for ( int i = 0; i < num; i++ ) {
    addFont( list, xFonts[i] );
  }

  XFreeFontNames( xFonts );
}


void KFontChooser::getFontList( QStringList &list, bool fixed )
{
  // Use KDE fonts if there is a KDE font list and check that the fonts
  // exist on the server where the desktop is running.
	
  QStringList lstSys, lstKDE;

  if ( fixed ) {
    getFontList( lstSys, "-*-*-*-*-*-*-*-*-*-*-m-*-*-*" );
    getFontList( lstSys, "-*-*-*-*-*-*-*-*-*-*-c-*-*-*" );
  } else
    //getFontList( lstSys, "-*-*-*-*-*-*-*-*-*-*-p-*-*-*" );
    getFontList(lstSys, "*");

  lstSys.sort();

  if ( !kapp->getKDEFonts( lstKDE ) ) {
    list = lstSys;
    return;
  }

  for (int i = 0; i < (int) lstKDE.count(); i++) {
    if (lstSys.contains(lstKDE[i]))
      list.append(lstKDE[i]);
  }
  list.sort();
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

  familyListBox->clear(); familyListBox->insertStringList(fontList);
}

void KFontChooser::showXLFDArea(bool show)
{
  if (show)
    xlfdBox->show();
  else
    xlfdBox->hide();
}

///////////////////////////////////////////////////////////////////////////////

KFontDialog::KFontDialog( QWidget *parent, const char* name,
			  bool onlyFixed, bool modal,
			  const QStringList &fontList)
  : QDialog( parent, name, modal )
{
  setCaption( i18n("Select Font") );

  QVBoxLayout *layout = new QVBoxLayout(this, 10);

  chooser = new KFontChooser(this, "fontChooser", onlyFixed, fontList);
  layout->addWidget(chooser);

  KButtonBox *bbox = new KButtonBox(this);
  layout->addWidget(bbox);

  bbox->addStretch(1);
  QPushButton *button = bbox->addButton(i18n("OK"));
  button->setDefault(true);
  connect( button, SIGNAL( clicked() ),
	  SLOT( accept() ) );
  button = bbox->addButton(i18n("Cancel"));
  connect( button, SIGNAL( clicked() ),
	  SLOT( reject() ) );
  bbox->layout();
  MINSIZE(bbox);

  layout->activate();

  // propogate signal from chooser
  connect(chooser, SIGNAL(fontSelected(const QFont &)),
	  SIGNAL(fontSelected(const QFont &)));
}


int KFontDialog::getFont( QFont &theFont, bool onlyFixed )
{
  KFontDialog dlg( 0L, "Font Selector", onlyFixed, true );
  dlg.setFont( theFont );
  int result = dlg.exec();

  if ( result == Accepted )
    theFont = dlg.chooser->font();

  return result;
}


int KFontDialog::getFontAndText( QFont &theFont, QString &theString,
				 bool onlyFixed )
{
  KFontDialog dlg( 0L, "Font and Text Selector", true, onlyFixed );
  dlg.setFont( theFont );
  int result = dlg.exec();

  if( result == Accepted ) {
    theFont = dlg.chooser->font();
    theString = dlg.chooser->sampleText();
  }

  return result;
}


/*
****************************************************************************
*
* $Log$
* Revision 1.37  1999/05/29 09:44:26  mario
* Mario: some small fixes, and optimized bloated code
* 
*
****************************************************************************
*/
