// kcharsetstest.cpp     -*- C++ -*-
//
// $Id$
// 
// Author: Jacek Konieczny <jajcus@zeus.polsl.gliwice.pl>
//

#include <stdlib.h>

#include "kcharsetstest.h"
#include <kapp.h>
#include <kcharsets.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlineedit.h>

#include "kcharsetstest.h"

Test::Test( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  setCaption("Testing KCharsets");

  QWidget *d = qApp->desktop();
  setGeometry((d->width()-320)>>1, (d->height()-160)>>1, 320, 160);

  createFields();
  show();
}

Test::~Test()
{
  ;
}

void Test::createFields()
{
  KCharsets *charsets=KApplication::getKApplication()->getCharsets();
  QStrList displayable=charsets->displayable();
  QStrList available=charsets->available();
 
  QFont fnt=font();		//
  fnt.setFamily("courier");	// R E M O V E    M E !
  setFont(fnt);			//
  
  int y=10,dy=30,h=25;
  int label_x=10,rest_x=105,label_w=100,rest_w=200;
  
  inputCharsetLabel=new QLabel("inputCharset",this); 
  inputCharsetLabel->setGeometry(label_x,y,label_w,h);
  inputCharsetCombo=new QComboBox(this);
  for(const char *ch=available.first();ch;ch=available.next())
    inputCharsetCombo->insertItem(ch);
  inputCharsetCombo->setGeometry(rest_x,y,rest_w,h);
  connect(inputCharsetCombo,SIGNAL(activated(int)),SLOT(convert(int)));
  y+=dy;
  inputLabel=new QLabel("input",this); 
  inputLabel->setGeometry(label_x,y,label_w,h);
  inputEdit=new QLineEdit(this);
  inputEdit->setGeometry(rest_x,y,rest_w,h);
  connect(inputEdit,SIGNAL(returnPressed()),SLOT(convert()));
  y+=dy; 
  outputCharsetLabel=new QLabel("outputCharset",this); 
  outputCharsetLabel->setGeometry(label_x,y,label_w,h);
  outputCharsetCombo=new QComboBox(this);
  connect(outputCharsetCombo,SIGNAL(activated(int)),SLOT(convert(int)));
  for(const char *ch=available.first();ch;ch=available.next())
    outputCharsetCombo->insertItem(ch);
  outputCharsetCombo->setGeometry(rest_x,y,rest_w,h);
  y+=dy;
  displayCharsetLabel=new QLabel("displayCharset",this); 
  displayCharsetLabel->setGeometry(label_x,y,label_w,h);
  displayCharsetCombo=new QComboBox(this);
  connect(displayCharsetCombo,SIGNAL(activated(int)),SLOT(changeDisplayCharset(int)));
  for(const char *ch=displayable.first();ch;ch=displayable.next())
    displayCharsetCombo->insertItem(ch);
  displayCharsetCombo->setGeometry(rest_x,y,rest_w,h);
  y+=dy;
  outputLabel=new QLabel("output",this); 
  outputLabel->setGeometry(label_x,y,label_w,h);
  outputEdit=new QLineEdit(this);
  outputEdit->setGeometry(rest_x,y,rest_w,h); 
}

void Test::convert(){

  QFont fnt=inputEdit->font();
  KCharsets *charsets=KApplication::getKApplication()->getCharsets();
  charsets->setQFont(fnt,inputCharsetCombo->currentText());
  inputEdit->setFont(fnt);

  QString text=inputEdit->text();
  KCharsetConverter converter(inputCharsetCombo->currentText(),
                              outputCharsetCombo->currentText(),
			      KCharsetConverter::INPUT_AMP_SEQUENCES
			      |KCharsetConverter::OUTPUT_AMP_SEQUENCES);
  if (converter.ok())
    outputEdit->setText(converter.convert(text));
  else
    outputEdit->setText("Error!");
}

void Test::changeDisplayCharset(int){

  QFont fnt=outputEdit->font();
  KCharsets *charsets=KApplication::getKApplication()->getCharsets();
  charsets->setQFont(fnt,displayCharsetCombo->currentText());
  outputEdit->setFont(fnt);
}
  
int main( int argc, char ** argv )
{
  KApplication a( argc, argv );
  Test m;

  a.setMainWidget( &m );

  m.show();

  return a.exec();
}

#include "kcharsetstest.moc"

