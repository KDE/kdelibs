// test.cpp     -*- C++ -*-
//
// $Id$
// 
// Author: Michael Wiedmann <mw@miwie.in-berlin.de>
//

#include <stdlib.h>

#include "klinetest.h"
#include <qapp.h>

#include "klinetest.moc"

Test::Test( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  setCaption("Testing KIntegerLine");

  QWidget *d = qApp->desktop();
  setGeometry((d->width()-600)>>1, (d->height()-400)>>1, 280, 100);

  createFields();
 
  qApp->setStyle( MotifStyle );
  show();
}

Test::~Test()
{
  ;
}

void Test::createFields()
{
  int x = 10, y = 10;
  m_ple1 = new KIntegerLine(this, "line1", KEditTypeOct);
  m_ple2 = new KIntegerLine(this, "line2", KEditTypeDec);
  m_ple3 = new KIntegerLine(this, "line3", KEditTypeHex);

  m_plb1 = new QLabel("Octal",       this, "_octal");
  m_plb2 = new QLabel("Decimal",     this, "_decimal");
  m_plb3 = new QLabel("Hexadecimal", this, "_dexa");

  m_plb  = new QLabel("Try: up-, down-arrow, Pg-Up and Pg-Down", this);

  QSize sz1, sz2, sz3;
  int w1, w2, w3, w_max, h;
  sz1 = m_plb1->sizeHint(); 
  sz2 = m_plb2->sizeHint(); 
  sz3 = m_plb3->sizeHint(); 

  w1 = sz1.width();
  w2 = sz2.width();
  w3 = sz3.width();
  h  = sz3.height();
  w_max = (w1 > w2) ? w1 : w2;
  w_max = (w_max > w3) ? w_max : w3;

  int delta = (20-h)>>1;
  m_plb1->setGeometry(x, y+delta,          w_max, h); 
  m_plb2->setGeometry(x, y+20+delta+delta, w_max, h); 
  m_plb3->setGeometry(x, y+40+delta+delta, w_max, h); 

  m_ple1->setGeometry(x+w_max+10, y, 100, 20);
  y += 20;
  m_ple2->setGeometry(x+w_max+10, y, 100, 20);
  y += 20;
  m_ple3->setGeometry(x+w_max+10, y, 100, 20);

  sz1 = m_plb->sizeHint();
  m_plb->setGeometry(x, y+30, sz1.width(), sz1.height());

  m_ple1->setText("7");
  m_ple2->setText("10");
  m_ple3->setText("F");

  m_ple1->setFocus();
}

int main( int argc, char ** argv )
{
  QApplication a( argc, argv );
  Test m;

  a.setMainWidget( &m );

  m.show();

  return a.exec();
}

