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

bool check(QString txt, QString a, QString b)
{
  if (a.isEmpty())
     a = QString::null;
  if (b.isEmpty())
     b = QString::null;
  if (a == b) {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
  }
  else {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
    exit(1);
  }
  return true;
}

bool checkDate(QString txt, QDate a, QDate b)
{
  if (a == b) {
    kdDebug() << txt << " : checking '" << a.toString() << "' against expected value '" << b.toString() << "'... " << "ok" << endl;
  }
  else {
    kdDebug() << txt << " : checking '" << a.toString() << "' against expected value '" << b.toString() << "'... " << "KO !" << endl;
    exit(1);
  }
  return true;
}

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

  KGlobal::locale()->setLanguage(QString::fromLatin1("en_US"));
  KGlobal::locale()->setCountry(QString::fromLatin1("C"));
  KGlobal::locale()->setThousandsSeparator(QString::fromLatin1(","));
  double num;
  bool ok;
  num = KGlobal::locale()->readNumber( "12,1", &ok ); check("readNumber(12,1)",ok?"yes":"no","no");
  num = KGlobal::locale()->readNumber( "12,100", &ok ); check("readNumber(12,100)",ok?"yes":"no","yes");
  num = KGlobal::locale()->readNumber( "12,100000,000", &ok ); check("readNumber(12,100000,000)",ok?"yes":"no","no");
  num = KGlobal::locale()->readNumber( "12,100000000", &ok ); check("readNumber(12,100000000)",ok?"yes":"no","no");
  num = KGlobal::locale()->readNumber( "12,100000,000", &ok ); check("readNumber(12,100000,000)",ok?"yes":"no","no");
  num = KGlobal::locale()->readNumber( "12,,100,000", &ok ); check("readNumber(12,,100,000)",ok?"yes":"no","no");
  num = KGlobal::locale()->readNumber( "12,1000,000", &ok ); check("readNumber(12,1000,000)",ok?"yes":"no","no");
  num = KGlobal::locale()->readNumber( "12,0000000,000", &ok ); check("readNumber(12,0000000,000)",ok?"yes":"no","no");
  num = KGlobal::locale()->readNumber( "12,0000000", &ok ); check("readNumber(12,0000000)",ok?"yes":"no","no");
  num = KGlobal::locale()->readNumber( "12,146,131.12", &ok ); check("readNumber(12,146,131.12)",ok?"yes":"no","yes");

  QDate date;
  date.setYMD( 2002, 5, 3 );
  checkDate("readDate( 3, 5, 2002 )",date,KGlobal::locale()->readDate( KGlobal::locale()->formatDate( date ) ) );
  date = QDate::currentDate();
  checkDate("readDate( QDate::currentDate() )",date,KGlobal::locale()->readDate( KGlobal::locale()->formatDate( date ) ) );

  QTime time;
  time = KGlobal::locale()->readTime( "11:22:33", &ok );
  check("readTime(\"11:22:33\")", (ok && time == QTime(11, 22, 33)) ?
        "yes" : "no", "yes");
  time = KGlobal::locale()->readTime( "11:22", &ok );
  check("readTime(\"11:22\")", (ok && time == QTime(11, 22, 0)) ?
        "yes" : "no", "yes");
  time = KGlobal::locale()->readTime( "foo", &ok );
  check("readTime(\"foo\")", (!ok && !time.isValid()) ?
        "invalid" : "valid", "invalid");

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
