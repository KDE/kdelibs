/*
    $Id$

    Requires the Qt widget libraries, available at no cost at 
    http://www.troll.no
       
    Copyright (C) 1996 Bernd Johannes Wuebben   
                       wuebben@math.cornell.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
    $Log$
    Revision 1.1.1.1  1997/04/09 00:28:09  cvsuser
    Sources imported

    Revision 1.1  1997/03/15 22:40:42  kalle

    Revision 1.4  1997/05/03 19:37:02  kulow
    Coolo: Again a little bug in acinclude
    included the moc file in kfontdialog

    Revision 1.3  1997/05/02 16:46:39  kalle
    Kalle: You may now override how KApplication reacts to external changes
    KButton uses the widget default palette

#include "kfontdialog.h"
#include "kfontdialog.moc"
    *** empty log message ***

    Revision 1.1  1997/01/04 17:36:44  wuebben
    Initial revision


*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kfontdialog.moc"
#include "stdio.h"
KFontDialog::KFontDialog( QWidget *parent, const char *name,  bool modal)
    : QDialog( parent, name, modal )
#include <qstrlist.h> 
#include <qfile.h>
                                   "ISO-8859-2",
  setCaption("Select Font");
                                   "ISO-8859-4",
                                   "ISO-8859-5",
                                   "ISO-8859-6",
  box1->setTitle("Requested Font");
                                   "ISO-8859-8"};

		   ,130);
                                             QFont::AnyCharSet,
  box1->setTitle("Actual Font");
					     QFont::ISO_8859_3,
  box1->setGeometry(XOFFSET,140,SIZE_X -  XOFFSET
		   ,110);
  family_label->setText("Family:");
					     QFont::ISO_8859_7,
					     QFont::ISO_8859_8};
                   
  actual_family_label->setText("Family:");
  family_label->setGeometry(3*XOFFSET,8*YOFFSET,LABLE_LENGTH,LABLE_HEIGHT);
   bool modal, const QStrList* fontlist)  : QDialog( parent, name, modal )
{

  actual_family_label->setGeometry(3*XOFFSET,160,40,LABLE_HEIGHT);
  setCaption( klocale->translate("Select Font") );
  size_label->setText("Size:");
  actual_family_label_data->setGeometry(3*XOFFSET +50 ,160,110,LABLE_HEIGHT);
  actual_family_label = new QLabel(this,"afamily");
  actual_family_label->setText(klocale->translate("Family:"));
  actual_family_label->setGeometry(3*XOFFSET,200,50,LABLE_HEIGHT);
  actual_size_label->setText("Size:");
			  8*YOFFSET,LABLE_LENGTH,LABLE_HEIGHT);
  actual_family_label_data->setGeometry(3*XOFFSET +60 ,200,110,LABLE_HEIGHT);

  charset_label = new QLabel(this,"charset");
  actual_size_label->setGeometry(3*XOFFSET,160 +LABLE_HEIGHT ,
				 LABLE_LENGTH,LABLE_HEIGHT);
    			    11*YOFFSET - COMBO_ADJUST +65 , LABLE_LENGTH +10,
			     LABLE_HEIGHT);
  weight_label->setText("Weight:");
  actual_charset_label = new QLabel(this,"acharset");
  actual_charset_label->setText(klocale->translate("Charset:"));
  actual_charset_label->setGeometry(3*XOFFSET,200 - LABLE_HEIGHT,50,LABLE_HEIGHT);

  actual_weight_label->setText("Weight:");
  actual_charset_label_data->setGeometry(3*XOFFSET +60 ,200 - LABLE_HEIGHT,110,LABLE_HEIGHT);

  size_label = new QLabel(this,"size");
  size_label->setText(klocale->translate("Size:"));
  actual_weight_label->setGeometry(3*XOFFSET,160 + 2*LABLE_HEIGHT ,
				 LABLE_LENGTH,LABLE_HEIGHT);

  actual_size_label = new QLabel(this,"asize");
  style_label->setText("Style:");
  actual_size_label->setGeometry(3*XOFFSET,200 +LABLE_HEIGHT ,
				 LABLE_LENGTH +10,LABLE_HEIGHT);

  actual_size_label_data = new QLabel(this,"asized");
  actual_size_label_data->setGeometry(3*XOFFSET +60 ,200 + LABLE_HEIGHT
				      ,110,LABLE_HEIGHT);
			   15*YOFFSET + LABLE_HEIGHT 
  actual_style_label->setText("Style:");
  weight_label->setText(klocale->translate("Weight:"));
  weight_label->setGeometry(3*XOFFSET,15*YOFFSET + LABLE_HEIGHT -20 
			  ,LABLE_LENGTH,LABLE_HEIGHT);

  actual_style_label->setGeometry(3*XOFFSET,160 + 3*LABLE_HEIGHT ,
				 LABLE_LENGTH,LABLE_HEIGHT);
  actual_weight_label->setGeometry(3*XOFFSET,200 + 2*LABLE_HEIGHT ,
  family_combo = new QComboBox(TRUE, this, "Family" );
  family_combo->insertItem( "Times" );
  family_combo->insertItem( "Helvetica" );
  family_combo->insertItem( "Utopia" );
  family_combo->insertItem( "New Century Schoolbook" );
  family_combo->insertItem( "Lucida" );
  family_combo->insertItem( "Lucidabright" );
  family_combo->insertItem( "Lucidatypewriter" );
  family_combo->insertItem( "Charter" );
  family_combo->insertItem( "Clean" );
  family_combo->insertItem( "Courier" );
  family_combo->insertItem( "Gothic" );
  family_combo->insertItem( "Symbol" );
  family_combo->insertItem( "Terminal" );
  family_combo->insertItem( "Fixed" );
  actual_weight_label_data = new QLabel(this,"aweightd");
  actual_weight_label_data->setGeometry(3*XOFFSET +60 ,200 + 2*LABLE_HEIGHT
  family_combo->setInsertionPolicy(QComboBox::NoInsertion);


  style_label = new QLabel(this,"style");
  style_label->setText(klocale->translate("Style:"));
  style_label->setGeometry(6*XOFFSET + LABLE_LENGTH + 12*XOFFSET + 
  //  QToolTip::add( family_combo, "Select Font Family" );


  size_combo = new QComboBox( TRUE, this, "Size" );
			    ,8*YOFFSET - COMBO_ADJUST -5 ,4* LABLE_LENGTH,COMBO_BOX_HEIGHT);
  connect( family_combo, SIGNAL(activated(const char *)),
      
    }
  }
  else{
    fill_family_combo();
  }


  charset_combo = new QComboBox( TRUE, this, klocale->translate("Charset") );
  charset_combo->setGeometry(6*XOFFSET + LABLE_LENGTH
			    ,11*YOFFSET - COMBO_ADJUST +60 ,
			     4* LABLE_LENGTH,COMBO_BOX_HEIGHT);
  for(int i=0;i<CHARSETS_COUNT;i++)
      charset_combo->insertItem( charsetsStr[i] );

  charset_combo->setInsertionPolicy(QComboBox::NoInsertion);
  connect( charset_combo, SIGNAL(activated(int)),
	   SLOT(charset_chosen_slot(int)) );
  // QToolTip::add( charset_combo, "Select Font Weight" );

  size_combo = new QComboBox( true, this, klocale->translate("Size") );
  size_combo->insertItem( "4" );
  size_combo->setInsertionPolicy(QComboBox::NoInsertion);
  size_combo->insertItem( "11" );
  size_combo->insertItem( "12" );
  size_combo->insertItem( "13" );
  size_combo->insertItem( "14" );
  size_combo->insertItem( "15" );
			    ,8*YOFFSET - COMBO_ADJUST 
  size_combo->insertItem( "17" );
  size_combo->insertItem( "18" );
  weight_combo = new QComboBox( TRUE, this, "Weight" );
  weight_combo->insertItem( "normal" );
  weight_combo->insertItem( "bold" );
  size_combo->insertItem( "24" );
  size_combo->insertItem( "26" );
  size_combo->insertItem( "28" );
  size_combo->insertItem( "32" );
  size_combo->insertItem( "48" );
			    ,19*YOFFSET - COMBO_ADJUST
  //  QToolTip::add( weight_combo, "Select Font Weight" );
  // we may want to allow the user to choose another size, since I
  style_combo = new QComboBox( TRUE, this, "Style" );
  style_combo->insertItem( "roman" );
  style_combo->insertItem( "italic" );

  size_combo->setGeometry(10*XOFFSET + 6*LABLE_LENGTH
			    ,8*YOFFSET - COMBO_ADJUST -5
			  ,2*LABLE_LENGTH + 20,COMBO_BOX_HEIGHT);
  connect( size_combo, SIGNAL(activated(const char *)),
			    ,19*YOFFSET- COMBO_ADJUST
  // QToolTip::add( style_combo, "Select Font Style" );


  cancel_button = new QPushButton("Cancel",this);
  weight_combo->insertItem( klocale->translate("normal") );
  weight_combo->insertItem( klocale->translate("bold") );
  weight_combo->setGeometry(6*XOFFSET + LABLE_LENGTH
			    ,19*YOFFSET - COMBO_ADJUST -20
  ok_button = new QPushButton( "Ok", this );
  cancel_button->setGeometry( 3*XOFFSET +100, OKBUTTONY, 80, BUTTONHEIGHT );
  connect( weight_combo, SIGNAL(activated(const char *)),
	   SLOT(weight_chosen_slot(const char *)) );
  // QToolTip::add( weight_combo, "Select Font Weight" );
  style_combo = new QComboBox( TRUE, this, klocale->translate("Style") );
  style_combo->insertItem( klocale->translate("italic") );
  style_combo->setGeometry(10*XOFFSET + 6*LABLE_LENGTH
			    ,19*YOFFSET- COMBO_ADJUST - 20
			   ,2*LABLE_LENGTH + 20,COMBO_BOX_HEIGHT);
  example_label->setGeometry(200,160,190, 80);
  example_label->setText("Dolor Ipse");
  //  example_label->setAutoResize(TRUE);
  //QToolTip::add( style_combo, "Select Font Style" );
  

  cancel_button = new QPushButton( klocale->translate("Cancel"),this);

  cancel_button->setGeometry( 3*XOFFSET +100, OKBUTTONY +40, 80, BUTTONHEIGHT );
  connect( cancel_button, SIGNAL( clicked() ), SLOT( reject() ) );
  example_label = new QLabel(this,"examples");

  example_label->setFont(selFont);

	KFontDialog dlg( NULL, "Font Selector", TRUE );
  example_label->setAlignment(AlignCenter);
  // let's initialize the display if possible
  if(family_combo->count() != 0){
    this->setFont(QFont(family_combo->text(0),12,QFont::Normal));
  }

  
  setColors();
  connect(KApplication::getKApplication(),SIGNAL(kdisplayPaletteChanged()),
	  this,SLOT(setColors()));

}


void KFontDialog::charset_chosen_slot(int index){

  selFont.setCharSet(charsetsIds[index]);
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


void KFontDialog::setFont( const QFont& aFont){

  selFont = aFont;
  setCombos();
  display_example(selFont);
  if ( weight_string == QString("normal"))
    selFont.setBold(FALSE);
  if ( weight_string == QString("bold"))
       selFont.setBold(TRUE);

  selFont.setFamily(family);
  //display_example();
  emit fontSelected(selFont);
}

void KFontDialog::size_chosen_slot(const char* size){
  
  QString size_string = size;
  if ( style_string == QString("roman"))
    selFont.setItalic(FALSE);
  if ( style_string == QString("italic"))
    selFont.setItalic(TRUE);
}

void KFontDialog::weight_chosen_slot(const char* weight){

  QString weight_string = weight;

  if ( weight_string == QString(klocale->translate("normal")))
    selFont.setBold(false);
  if ( weight_string == QString(klocale->translate("bold")))
       selFont.setBold(true);
  // display_example();
  emit fontSelected(selFont);

void KFontDialog::style_chosen_slot(const char* style){


  QString style_string = style;

    actual_weight_label_data->setText("Bold");
    selFont.setItalic(false);
    actual_weight_label_data->setText("Normal");
    selFont.setItalic(true);
  //  display_example();
    actual_style_label_data->setText("italic");
}
    actual_style_label_data->setText("roman");

void KFontDialog::display_example(const QFont& font){

  QString string;
  int i;

 QFont::CharSet charset=selFont.charSet();
  actual_size_label_data->setText(string);

 int number_of_entries; 
    actual_weight_label_data->setText(klocale->translate("Bold"));
  else
    actual_weight_label_data->setText(klocale->translate("Normal"));
 
  if (info.italic())
 found = FALSE;
  else
 for (int i = 0;i < number_of_entries - 1; i++){
   if ( string == (QString) combo->text(i)){
  for(i = 0;i<CHARSETS_COUNT;i++)
     //     printf("Found Font %s\n",string.data());
     found = TRUE;
      break;
    }
  
}

void KFontDialog::setCombos(){

 QString string;
 found = FALSE;
 int number_of_entries, i=0; 
 for (int i = 0;i < number_of_entries - 1; i++){

 number_of_entries =  family_combo->count(); 
     found = TRUE;
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
    qfontname = qfontname.mid(dash +1, dash_two - dash -1);

    if( !qfontname.contains("open look", TRUE)){
      if(qfontname != "nil"){
	if(fontlist.find(qfontname) == -1)
	  fontlist.inSort(qfontname);
      }
    }
  

    fontNames ++;

  }
		    cgrp.light(),cgrp.dark(),cgrp.mid(),black,white);

  mypalette.setNormal(ncgrp);
  mypalette.setDisabled(ncgrp);
  mypalette.setActive(ncgrp);

  example_label->setPalette(mypalette);
  example_label->setBackgroundColor(white);
 
}

