/*
    $Id$

    Requires the Qt widget libraries, available at no cost at 
    http://www.troll.no
       
    Copyright (C) 1996 Bernd Johannes Wuebben   
                       wuebben@math.cornell.edu

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
  
    $Log$
    Revision 1.26  1998/09/14 20:44:04  kulow
    I know, Ok is ok too, but OK is more OK some GUI guides say :)

    Revision 1.25  1998/09/01 20:21:53  kulow
    I renamed all old qt header files to the new versions. I think, this looks
    nicer (and gives the change in configure a sense :)

    Revision 1.24  1998/08/31 12:43:26  esken
    GPL -> LGPL

    Revision 1.23  1998/06/15 12:49:33  kulow
    applied patch to replace .kde with localkdedir()

    Revision 1.22  1998/06/10 18:23:37  schreter
    New klocale code had a little bug - it tried to find if there is "charsets"
    file in a locale dir instead of "charset" file, but then opened "charset"
    file. So I've fixed it. I've also fixed font dialog label sizes.

    Revision 1.21  1998/06/01 09:13:34  kalle
    Added static getFontAndText()

    Revision 1.20  1998/06/01 08:42:41  kalle
    KFontDialog:
    - you can now enter your own example string
    - new static method getXLFD() that converts a QFont() to a X Logical Font Description

    KIntegerLine:
    - new signal valueChanged( int )

    Revision 1.19  1998/05/27 20:38:48  schreter


    Fixed font dialog charset display problem - combo wouldn't load charsets
    for current font on invocation of the dialog.

    Revision 1.18  1998/04/12 08:56:13  jacek

    Small fix in setting charset combo

    Revision 1.17  1998/03/08 22:08:49  wuebben
    Bernd: adjusted the size of kfontdialog for localization

    Revision 1.16  1998/01/21 15:06:57  jacek
    Added real KCharsets support

    Revision 1.14  1997/11/20 22:36:48  kalle
    - Removed some more hardcoded colors
    - A patch from Bernd regarding KProgress

    Revision 1.13  1997/11/09 22:56:11  wuebben
    Bernd: colorscheme related changes

    Revision 1.12  1997/11/09 05:18:57  wuebben
    Bernd: fix

    Revision 1.11  1997/11/09 04:41:44  wuebben
    Bernd: fixed the broken kfontdialog ...

    Revision 1.10  1997/11/09 03:45:55  wuebben
    *** empty log message ***

    Revision 1.9  1997/10/21 20:45:00  kulow
    removed all NULLs and replaced it with 0L or "".
    There are some left in mediatool, but this is not C++

    Revision 1.8  1997/10/16 11:15:21  torben
    Kalle: Copyright headers
    kdoctoolbar removed

    Revision 1.7  1997/08/31 19:18:37  kdecvs
    Kalle:
    new usage of KLocale
    default string for KConfig::readEntry to const char*

    Revision 1.6  1997/08/24 20:40:40  kulow
    Coolo: translated the dialogs and others. I hope, noone minds.

    Revision 1.5  1997/07/24 21:06:04  kalle
    Kalle:
    KToolBar upgraded to newtoolbar 0.6
    KTreeList has rubberbanding now
    Patches for SGI

    Revision 1.4  1997/05/03 19:37:02  kulow
    Coolo: Again a little bug in acinclude
    included the moc file in kfontdialog

    Revision 1.3  1997/05/02 16:46:39  kalle
    Kalle: You may now override how KApplication reacts to external changes
    KButton uses the widget default palette
    new kfontdialog version 0,5
    new kpanner by Paul Kendall
    new: KIconLoader

    Revision 1.4  1997/04/29 02:44:24  wuebben
    *** empty log message ***

    Revision 1.3  1997/04/27 01:50:49  wuebben
    added ~/.kde/config/kdefonts support

    Revision 1.2  1997/04/20 14:59:44  wuebben
    fixed a minor bug which caused the last font in the font list to not
    be displayed

    Revision 1.1  1997/04/20 00:18:15  wuebben
    Initial revision

    Revision 1.1  1997/01/04 17:36:44  wuebben
    Initial revision


*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kfontdialog.moc"
#include "stdio.h"
#include "stdlib.h"
#include "qfile.h"
#include <qstrlist.h> 
#include <qfile.h>
#include <qtextstream.h> 
#include <qapplication.h>
#include <X11/Xlib.h>

#include <klocale.h>
#include <kcharsets.h>
#include <kapp.h>

#define MINSIZE(x) x->setMinimumSize(x->sizeHint());

KFontDialog::KFontDialog( QWidget *parent, const char *name, 
   bool modal, const QStrList* fontlist)  : QDialog( parent, name, modal )
{
  QPushButton *button;
  
  setCaption( klocale->translate("Select Font") );
  
  layout = new QVBoxLayout(this, 10);
  
  box1 = new QGroupBox(klocale->translate("Requested Font"), this);
  box2 = new QGroupBox(klocale->translate("Actual Font"), this);
  
  box1layout = new QGridLayout(box1, 5, 7, 7);
  box1layout->addColSpacing(0, 3);
  box1layout->addColSpacing(6, 3);
  box1layout->addRowSpacing(0, 13);
  box1layout->addRowSpacing(4, 3);
  box1layout->addColSpacing(3, 10);
  box1layout->setColStretch(2, 1);
  box1layout->setColStretch(5, 1);
  
  box2layout = new QGridLayout(box2, 7, 5, 7);
  box2layout->addColSpacing(0, 3);
  box2layout->addColSpacing(4, 3);
  box2layout->addRowSpacing(0, 13);
  box2layout->addRowSpacing(6, 3);
  box2layout->setColStretch(2, 1);
  box2layout->setColStretch(3, 2);
  
  family_label = new QLabel(box1,"family");
  family_label->setText( klocale->translate("Family:") );
  MINSIZE(family_label);
  box1layout->addWidget(family_label, 1, 1);
  
  actual_family_label = new QLabel(box2,"afamily");
  actual_family_label->setText(klocale->translate("Family:"));
  MINSIZE(actual_family_label);
  actual_family_label_data = new QLabel(box2,"afamilyd");
  box2layout->addWidget(actual_family_label, 2, 1);
  box2layout->addWidget(actual_family_label_data, 2, 2);
  
  charset_label = new QLabel(box1,"charset");
  charset_label->setText(klocale->translate("Charset:"));
  MINSIZE(charset_label);
  box1layout->addWidget(charset_label, 3, 1);
  
  actual_charset_label = new QLabel(box2,"acharset");
  actual_charset_label->setText(klocale->translate("Charset:"));
  MINSIZE(actual_charset_label);
  
  actual_charset_label_data = new QLabel(box2,"acharsetd");
  box2layout->addWidget(actual_charset_label, 1, 1);
  box2layout->addWidget(actual_charset_label_data, 1, 2);
  
  size_label = new QLabel(box1,"size");
  size_label->setText(klocale->translate("Size:"));
  MINSIZE(size_label);
  box1layout->addWidget(size_label, 1, 4);
  
  actual_size_label = new QLabel(box2,"asize");
  actual_size_label->setText(klocale->translate("Size:"));
  MINSIZE(actual_size_label);
  actual_size_label_data = new QLabel(box2,"asized");
  box2layout->addWidget(actual_size_label, 3, 1);
  box2layout->addWidget(actual_size_label_data, 3, 2);
  
  weight_label = new QLabel(box1,"weight");
  weight_label->setText(klocale->translate("Weight:"));
  MINSIZE(weight_label);
  box1layout->addWidget(weight_label, 2, 1);
  
  actual_weight_label = new QLabel(box2,"aweight");
  actual_weight_label->setText(klocale->translate("Weight:"));
  MINSIZE(actual_weight_label);
  actual_weight_label_data = new QLabel(box2,"aweightd");
  box2layout->addWidget(actual_weight_label, 4, 1);
  box2layout->addWidget(actual_weight_label_data, 4, 2);
  
  style_label = new QLabel(box1,"style");
  style_label->setText(klocale->translate("Style:"));
  MINSIZE(style_label);
  box1layout->addWidget(style_label, 2, 4);
  
  actual_style_label = new QLabel(box2,"astyle");
  actual_style_label->setText(klocale->translate("Style:"));
  MINSIZE(actual_style_label);
  actual_style_label_data = new QLabel(box2,"astyled");
  box2layout->addWidget(actual_style_label, 5, 1);
  box2layout->addWidget(actual_style_label_data, 5, 2);
  
  
  
  family_combo = new QComboBox(true, box1, "Family" );
  box1layout->addWidget(family_combo, 1, 2);
  family_combo->setInsertionPolicy(QComboBox::NoInsertion);
  
  connect( family_combo, SIGNAL(activated(const char *)),
	  SLOT(family_chosen_slot(const char *)) );
  
  if (fontlist != 0L){
    if(fontlist->count() !=0){
      family_combo->insertStrList(fontlist, -1);
      // this is a dirty fix due to a bug in Qt 1.2
      family_combo->setCurrentItem(1);
      family_combo->setCurrentItem(0);
    }
  }
  else{
    fill_family_combo();
  }
  MINSIZE(family_combo);
  
  size_combo = new QComboBox( true, box1, klocale->translate("Size") );
  box1layout->addWidget(size_combo, 1, 5);
  
  size_combo->insertItem( "4" );
  size_combo->insertItem( "5" );
  size_combo->insertItem( "6" );
  size_combo->insertItem( "7" );
  size_combo->insertItem( "8" );
  size_combo->insertItem( "9" );
  size_combo->insertItem( "10" );
  size_combo->insertItem( "11" );
  size_combo->insertItem( "12" );
  size_combo->insertItem( "13" );
  size_combo->insertItem( "14" );
  size_combo->insertItem( "15" );
  size_combo->insertItem( "16" );
  size_combo->insertItem( "17" );
  size_combo->insertItem( "18" );
  size_combo->insertItem( "19" );
  size_combo->insertItem( "20" );
  size_combo->insertItem( "22" );
  size_combo->insertItem( "24" );
  size_combo->insertItem( "26" );
  size_combo->insertItem( "28" );
  size_combo->insertItem( "32" );
  size_combo->insertItem( "48" );
  size_combo->insertItem( "64" );
  
  // we may want to allow the user to choose another size, since I
  // can really not presume to have listed all useful sizes.
  
  //  size_combo->setInsertionPolicy(QComboBox::NoInsertion);
  
  connect( size_combo, SIGNAL(activated(const char *)),
	  SLOT(size_chosen_slot(const char *)) );
  MINSIZE(size_combo);
  
  
  weight_combo = new QComboBox( TRUE, box1, klocale->translate("Weight") );
  box1layout->addWidget(weight_combo, 2, 2);
  
  weight_combo->insertItem( klocale->translate("normal") );
  weight_combo->insertItem( klocale->translate("bold") );
  
  weight_combo->setInsertionPolicy(QComboBox::NoInsertion);
  connect( weight_combo, SIGNAL(activated(const char *)),
	  SLOT(weight_chosen_slot(const char *)) );
  // QToolTip::add( weight_combo, "Select Font Weight" );
  
  MINSIZE(weight_combo);
  
  
  style_combo = new QComboBox( TRUE, box1, klocale->translate("Style") );
  box1layout->addWidget(style_combo, 2, 5);
  
  style_combo->insertItem( klocale->translate("roman") );
  style_combo->insertItem( klocale->translate("italic") );
  
  style_combo->setInsertionPolicy(QComboBox::NoInsertion);
  connect( style_combo, SIGNAL(activated(const char *)),
	  SLOT(style_chosen_slot(const char *)) );
  MINSIZE(style_combo);
  
  
  charset_combo = new QComboBox( TRUE, box1, klocale->translate("Charset") );
  box1layout->addWidget(charset_combo, 3, 2);
  
  charset_combo->setInsertionPolicy(QComboBox::NoInsertion);
  connect( charset_combo, SIGNAL(activated(const char *)),
	  SLOT(charset_chosen_slot(const char *)) );
  MINSIZE(charset_combo);
  
  
  example_edit = new QLineEdit(box2, "examples");
  
  box2layout->addMultiCellWidget(example_edit, 1, 5, 3, 3);
  
  example_edit->setFont(selFont);
  
  //  example_edit->setAlignment(AlignCenter);
  //  example_edit->setBackgroundColor(white);
  //  example_edit->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
  //  example_edit->setLineWidth( 1 );
  example_edit->setText(klocale->translate("Dolor Ipse"));
  //  example_edit->setAutoResize(true);
  
  connect(this,SIGNAL(fontSelected( const QFont&  )),
	  this,SLOT(display_example( const QFont&)));
  MINSIZE(example_edit);
  
  // lets initialize the display if possible
  if(family_combo->count() != 0){
    this->setFont(QFont(family_combo->text(0),12,QFont::Normal)); 
  }
  
  // Create displayable charsets list
  KCharsets *charsets=KApplication::getKApplication()->getCharsets();
  QStrList lst=charsets->displayable(selFont.family());
  for(const char * chset=lst.first();chset;chset=lst.next())
    charset_combo->insertItem( chset );
  charset_combo->insertItem( "any" );
  
  setColors();
  connect(KApplication::getKApplication(),SIGNAL(kdisplayPaletteChanged()),
	  this,SLOT(setColors()));
  
  
  box1layout->activate();
  box2layout->activate();
  
  layout->addWidget(box1, 1);
  layout->addWidget(box2, 1);
  
  KButtonBox *bbox = new KButtonBox(this);
  
  bbox->addStretch(1);
  button = bbox->addButton(klocale->translate("OK"));
  connect( button, SIGNAL( clicked() ), 
	  SLOT( accept() ) );
  button = bbox->addButton(klocale->translate("Cancel"));
  connect( button, SIGNAL( clicked() ), 
	  SLOT( reject() ) );
  bbox->layout();
  bbox->setMinimumSize(bbox->sizeHint());
  
  layout->addWidget(bbox, 0);
  layout->activate();
}


void KFontDialog::charset_chosen_slot(const char *chset){

  KCharset(chset).setQFont(selFont);
  emit fontSelected(selFont);
}

int KFontDialog::getFont( QFont &theFont )
{
	KFontDialog dlg( 0L, "Font Selector", TRUE );
	dlg.setFont( theFont );
	int result = dlg.exec();

	if ( result == Accepted )
		theFont = dlg.font();

	return result;
}


int KFontDialog::getFontAndText( QFont &theFont, QString &theString )
{
  KFontDialog dlg( 0L, "Font and Text Selector", TRUE );
  dlg.setFont( theFont );
  int result = dlg.exec();

  if( result == Accepted ) {
	theFont = dlg.font();
	theString = dlg.example_edit->text();
  }

  return result;
}

void KFontDialog::setFont( const QFont& aFont){

  selFont = aFont;
  setCombos();
  display_example(selFont);
}  


void KFontDialog::family_chosen_slot(const char* family){

  selFont.setFamily(family);
 
  // Re-create displayable charsets list
  KCharsets *charsets=KApplication::getKApplication()->getCharsets();
  QStrList lst=charsets->displayable(selFont.family());
  charset_combo->clear();
  for(const char * chset=lst.first();chset;chset=lst.next())
      charset_combo->insertItem( chset );
  charset_combo->insertItem( "any" );
  
  //display_example();
  emit fontSelected(selFont);
}

void KFontDialog::size_chosen_slot(const char* size){
  
  QString size_string = size;

  selFont.setPointSize(size_string.toInt());
  //display_example();
  emit fontSelected(selFont);
}

void KFontDialog::weight_chosen_slot(const char* weight){

  QString weight_string = weight;

  if ( weight_string == QString(klocale->translate("normal")))
    selFont.setBold(false);
  if ( weight_string == QString(klocale->translate("bold")))
       selFont.setBold(true);
  // display_example();
  emit fontSelected(selFont);
}

void KFontDialog::style_chosen_slot(const char* style){


  QString style_string = style;

  if ( style_string == QString(klocale->translate("roman")))
    selFont.setItalic(false);
  if ( style_string == QString(klocale->translate("italic")))
    selFont.setItalic(true);
  //  display_example();
  emit fontSelected(selFont);
}
       

void KFontDialog::display_example(const QFont& font){

  QString string;

  example_edit->setFont(font);

  QFontInfo info = example_edit->fontInfo();
  actual_family_label_data->setText(info.family());
  
  string.setNum(info.pointSize());
  actual_size_label_data->setText(string);

  if (info.bold())
    actual_weight_label_data->setText(klocale->translate("Bold"));
  else
    actual_weight_label_data->setText(klocale->translate("Normal"));
 
  if (info.italic())
    actual_style_label_data->setText(klocale->translate("italic"));
  else
    actual_style_label_data->setText(klocale->translate("roman"));
  
  KCharsets *charsets=KApplication::getKApplication()->getCharsets();
  const char * charset=charsets->name(selFont);
  actual_charset_label_data->setText(charset);
}

void KFontDialog::setCombos(){

 QString string;
 QComboBox* combo;
 int number_of_entries, i=0; 
 bool found;

 number_of_entries =  family_combo->count(); 
 string = selFont.family();
 combo = family_combo; 
 found = false;

 for (i = 0;i < number_of_entries ; i++){
   //   printf("%s with %s\n",string.data(), ((QString) combo->text(i)).data());
   if ( string.lower() == ((QString) combo->text(i)).lower()){
     combo->setCurrentItem(i);
     //printf("Found Font %s\n",string.data());
     found = true;
     break;
   }
 }

 
 number_of_entries =  size_combo->count(); 
 string.setNum(selFont.pointSize());
 combo = size_combo; 
 found = false;

 for (i = 0;i < number_of_entries - 1; i++){
   if ( string == (QString) combo->text(i)){
     combo->setCurrentItem(i);
     found = true;
     // printf("Found Size %s setting to item %d\n",string.data(),i);
     break;
   }
 }

 if (selFont.bold()){
   //weight_combo->setCurrentItem(0);
   weight_combo->setCurrentItem(1);
 }else
   weight_combo->setCurrentItem(0);

  if (selFont.italic())
   style_combo->setCurrentItem(1);
 else
   style_combo->setCurrentItem(0);

 // Re-create displayable charsets list
 KCharsets *charsets=KApplication::getKApplication()->getCharsets();
 const char * charset=charsets->name(selFont);
 QStrList lst=charsets->displayable(selFont.family());
 charset_combo->clear();
 i = 0;
 for(const char * chset=lst.first();chset;chset=lst.next(),++i) {
     charset_combo->insertItem( chset );
     if (strcmp(chset, charset) == 0) charset_combo->setCurrentItem(i);
 }
 charset_combo->insertItem( "any" );
 
}

bool KFontDialog::loadKDEInstalledFonts(){

  QString fontfilename;

  //TODO replace by QDir::homePath();

  fontfilename = KApplication::localkdedir() + "/share/config/kdefonts";

  QFile fontfile(fontfilename);

  if (!fontfile.exists())
    return false;

  if(!fontfile.open(IO_ReadOnly)){
    return false;
  }

  if (!fontfile.isReadable())
    return false;
  
  
  QTextStream t(&fontfile);

  while ( !t.eof() ) {
    QString s = t.readLine();
    s = s.stripWhiteSpace();
    if(!s.isEmpty())
      family_combo->insertItem( s ,-1 );
  }

  fontfile.close();

  
  return true;

}


void KFontDialog::fill_family_combo(){

  int numFonts;
  Display *kde_display;
  char** fontNames;
  char** fontNames_copy;
  QString qfontname;


    
  QStrList fontlist(TRUE);
  
  kde_display = XOpenDisplay( 0L );

  // now try to load the KDE fonts

  bool have_installed = loadKDEInstalledFonts();
  
  // if available we are done, the kde fonts are now in the family_combo

  if (have_installed)
    return;

  fontNames = XListFonts(kde_display, "*", 32767, &numFonts);
  fontNames_copy = fontNames;

  for(int i = 0; i < numFonts; i++){
    
    if (**fontNames != '-'){ 
      
      // The font name doesn't start with a dash -- an alias
      // so we ignore it. It is debatable whether this is the right
      // behaviour so I leave the following snippet of code around.
      // Just uncomment it if you want those aliases to be inserted as well.
      
      /*
      qfontname = "";
      qfontname = *fontNames;
      if(fontlist.find(qfontname) == -1)
          fontlist.inSort(qfontname);
      */

      fontNames ++;
      continue;
    };
      
    qfontname = "";
    qfontname = *fontNames;
    int dash = qfontname.find ('-', 1, TRUE); // find next dash

    if (dash == -1) { // No such next dash -- this shouldn't happen.
                      // but what do I care -- lets skip it.
      fontNames ++;
      continue;
    }

    // the font name is between the second and third dash so:
    // let's find the third dash:

    int dash_two = qfontname.find ('-', dash + 1 , TRUE); 

    if (dash == -1) { // No such next dash -- this shouldn't happen.
                      // But what do I care -- lets skip it.
      fontNames ++;
      continue;
    }

    // fish the name of the font info string

    qfontname = qfontname.mid(dash +1, dash_two - dash -1);

    if( !qfontname.contains("open look", TRUE)){
      if(qfontname != "nil"){
	if(fontlist.find(qfontname) == -1)
	  fontlist.inSort(qfontname);
      }
    }
  

    fontNames ++;

  }

  for(fontlist.first(); fontlist.current(); fontlist.next())
      family_combo->insertItem(fontlist.current(),-1);

  XFreeFontNames(fontNames_copy);
  XCloseDisplay(kde_display);


}


void KFontDialog::setColors(){
 
  /* this is to the the backgound of a widget to white and the
     text color to black -- some lables such as the one of the
     font manager really shouldn't follow colorschemes The
     primary task of those label is to display the text clearly
     an visibly and not to look pretty ...*/

  QPalette mypalette = (example_edit->palette()).copy();

  QColorGroup cgrp = mypalette.normal();
  QColorGroup ncgrp(black,cgrp.background(),
		    cgrp.light(),cgrp.dark(),cgrp.mid(),black,white);

  mypalette.setNormal(ncgrp);
  mypalette.setDisabled(ncgrp);
  mypalette.setActive(ncgrp);

  example_edit->setPalette(mypalette);
  example_edit->setBackgroundColor(white);
 
}


QString KFontDialog::getXLFD( const QFont& font )
{
  QFontInfo fi( font );

  QString xlfd = "-*-"; // foundry
  xlfd += fi.family(); // family
  xlfd += "-";
  switch( fi.weight() ) { // weight
  case QFont::Light:
	xlfd += "light-";
	break;
  case QFont::Normal:
	xlfd += "normal-";
	break;
  case QFont::DemiBold:
	xlfd += "demi-";
	break;
  case QFont::Bold:
	xlfd += "bold-";
	break;
  case QFont::Black:
	xlfd += "black-";
	break;
  default:
	xlfd += "*-";
  }
  if( fi.italic() )
	xlfd += "i-"; // slant
  else
	xlfd += "r-"; // slant
  xlfd += "*-"; // set width
  xlfd += "*-"; // pixels (we cannot know portably, because this
                // depends on the screen resolution 
  xlfd += fi.pointSize()*10; // points
  xlfd += "-";
  xlfd += "*-"; // horz. resolution
  xlfd += "*-"; // vert. resolution
  if( fi.fixedPitch() )
	xlfd += "m-"; // spacing: monospaced
  else
	xlfd += "p-"; // spacing: proportional
  xlfd += "*-"; // average width
  // charset
  switch( fi.charSet() ) {
  case QFont::AnyCharSet:
	xlfd += "*";
	break;
  case QFont::Latin1:
	xlfd += "iso8859-1";
	break;
  case QFont::Latin2:
	xlfd += "iso8859-2";
	break;
  case QFont::Latin3:
	xlfd += "iso8859-3";
	break;
  case QFont::Latin4:
	xlfd += "iso8859-4";
	break;
  case QFont::Latin5:
	xlfd += "iso8859-5";
	break;
  case QFont::Latin6:
	xlfd += "iso8859-6";
	break;
  case QFont::Latin7:
	xlfd += "iso8859-7";
	break;
  case QFont::Latin8:
	xlfd += "iso8859-8";
	break;
  case QFont::Latin9:
	xlfd += "iso8859-9";
	break;
  default:
	xlfd += "*";
  }

  return xlfd;
}
