// klocaletest.cpp     -*- C++ -*-
//
// $Id$
//
// Author: Jacek Konieczny <jajcus@zeus.polsl.gliwice.pl>
//

#include <stdlib.h>
#include <stdio.h>

#include <qdatetime.h>
#include <qlabel.h>

#include <kglobal.h>
#include <kglobalsettings.h>
#include "klocale.h"
#include <kapplication.h>
#include <kcharsets.h>
#include <kdebug.h>

#include "klocaletest.h"

Test::Test( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  setCaption("Testing KLocale");

  QWidget *d = qApp->desktop();
  setGeometry((d->width()-320)>>1, (d->height()-160)>>1, 420, 420);

  createFields();
  show();
}

Test::~Test()
{
  ;
}

void Test::createFields()
{
  QString string;

  string+="Selected languages: ";
  string+=KGlobal::locale()->languages()+"\n";

  // This will show nothing, as there is no klocaletest.mo
  // but you can copy other *.mo file
  string+="Used language: ";
  string+=KGlobal::locale()->language()+"\n";
  string+="Locale encoding: ";
  string+=QString::fromLatin1(KGlobal::locale()->encoding())+"\n";

  string+="Localized date and time: ";
  string+=KGlobal::locale()->formatDateTime(QDateTime::currentDateTime());
  string+="\nLocalized number: ";
  string+=KGlobal::locale()->formatNumber(1234567.89);
  string+="\nLocalized monetary numbers: ";
  string+=KGlobal::locale()->formatMoney(1234567.89) + " / " +KGlobal::locale()->formatMoney(-1234567.89);
  // This will not work
  // but you can copy other *.mo file
  string+="\nSome localized strings:\n";
  string+=QString::fromLatin1("Yes = ")+i18n("Yes")+"\n";
  string+=QString::fromLatin1("No = ")+i18n("No")+"\n";
  string+=QString::fromLatin1("Help = ")+i18n("Help")+"\n";
  string+=QString::fromLatin1("Cancel = ")+i18n("Cancel")+"\n";

  label=new QLabel(string,this,"Label");
  label->setGeometry(10,10,400,400);
  label->setFont(KGlobalSettings::generalFont());
  label->show();
}

int main( int argc, char ** argv )
{
  KLocale::setMainCatalogue("kdelibs");
  KApplication a( argc, argv, "klocaletest" );

  kdDebug() << "setLanguage C\n";
  KGlobal::locale()->setLanguage(QString::fromLatin1("C"));
  kdDebug() << "C: " << i18n("yes") << " " << i18n("QAccel", "Space") << endl;

  kdDebug() << "setLanguage de\n";
  KGlobal::locale()->setLanguage(QString::fromLatin1("de"));
  kdDebug() << "de: " << i18n("yes") << " " << i18n("QAccel", "Space") << endl;
  Test m;

  a.setMainWidget( &m );

  m.show();

  return a.exec();
}

#include "klocaletest.moc"
