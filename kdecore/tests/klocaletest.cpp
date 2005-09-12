// klocaletest.cpp     -*- C++ -*-
//
// Author: Jacek Konieczny <jajcus@zeus.polsl.gliwice.pl>
//

#include <stdlib.h>
#include <stdio.h>

#include <qdatetime.h>
#include <qlabel.h>
#include <qdesktopwidget.h>

#include <kglobal.h>
#include <kglobalsettings.h>
#include "klocale.h"
#include <kaboutdata.h>
#include <kcmdlineargs.h>
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
  string+=KGlobal::locale()->languageList().join(":")+"\n";

  // This will show nothing, as there is no klocaletest.mo
  // but you can copy other *.mo file
  string+="Used language: ";
  string+=KGlobal::locale()->language()+"\n";
  string+="Locale encoding: ";
  string+=QString::fromLatin1(KGlobal::locale()->encoding())+"\n";

  string+="Localized date and time: ";
  string+=KGlobal::locale()->formatDateTime(QDateTime::currentDateTime());
  string+="\nLocalized monetary numbers: ";
  string+=KGlobal::locale()->formatMoney(1234567.89) + " / \n" +KGlobal::locale()->formatMoney(-1234567.89);
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
  KLocale::setMainCatalog("kdelibs");
  KAboutData about("klocaletest", "klocaletest", "version");
  KCmdLineArgs::init(argc, argv, &about);

  KApplication a;

  KGlobal::locale()->setLanguage(QString::fromLatin1("en_US"));
  KGlobal::locale()->setCountry(QString::fromLatin1("C"));
  KGlobal::locale()->setThousandsSeparator(QString::fromLatin1(","));

  QString formatted;
  formatted = KGlobal::locale()->formatNumber( 70 ); check("formatNumber(70)",formatted,"70.00");
  formatted = KGlobal::locale()->formatNumber( 70, 0 ); check("formatNumber(70, 0)",formatted,"70");
  formatted = KGlobal::locale()->formatNumber( 70.2 ); check("formatNumber(70.2)",formatted,"70.20");
  formatted = KGlobal::locale()->formatNumber( 70.24 ); check("formatNumber(70.24)",formatted,"70.24");
  formatted = KGlobal::locale()->formatNumber( 70.245 ); check("formatNumber(70.245)",formatted,"70.25"); /*rounded*/
  formatted = KGlobal::locale()->formatNumber(1234567.89123456789,8); check("formatNumber(1234567.89123456789,8)",formatted,"1,234,567.89123457");

  formatted = KGlobal::locale()->formatNumber("70"); check("formatNumber(\"70\")",formatted,"70.00");
  formatted = KGlobal::locale()->formatNumber("70", true, 2); check("formatNumber(\"70\", true, 2)",formatted,"70.00");
  formatted = KGlobal::locale()->formatNumber("70", true, 0); check("formatNumber(\"70\", true, 0)",formatted,"70");
  formatted = KGlobal::locale()->formatNumber("70.9123", true, 0); check("formatNumber(\"70.9123\", true, 0)",formatted,"71"); /* rounded */
  formatted = KGlobal::locale()->formatNumber("-70.2", true, 2); check("formatNumber(\"-70.2\", true, 2)",formatted,"-70.20");
  formatted = KGlobal::locale()->formatNumber("+70.24", true, 2); check("formatNumber(\"+70.24\", true, 2)",formatted,"70.24");
  formatted = KGlobal::locale()->formatNumber("70.245", true, 2); check("formatNumber(\"70.245\", true, 2)",formatted,"70.25"); /*rounded*/
  formatted = KGlobal::locale()->formatNumber("99.996", true, 2); check("formatNumber(\"99.996\", true, 2)",formatted,"100.00"); /*rounded*/
  formatted = KGlobal::locale()->formatNumber("12345678901234567.89123456789", false, 0); check("formatNumber(\"12345678901234567.89123456789\", false, 0)",formatted,"12,345,678,901,234,567.89123456789");



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
  num = KGlobal::locale()->readNumber( "1.12345678912", &ok );
        qDebug( "%s", QString::number( num, 'g', 12 ).latin1() ); // warning this is the only way to see all decimals
        check("readNumber(1.12345678912)",ok && num==1.12345678912?"yes":"no","yes");

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

  time = KGlobal::locale()->readTime( "11:22:33", KLocale::WithoutSeconds, &ok );
  check("readTime(\"11:22:33\", WithoutSeconds)", (!ok && !time.isValid()) ?
        "invalid" : "valid", "invalid");
  time = KGlobal::locale()->readTime( "11:22", KLocale::WithoutSeconds, &ok );
  check("readTime(\"11:22\", WithoutSeconds)", (ok && time == QTime(11, 22, 0)) ?
        "yes" : "no", "yes");

  KGlobal::locale()->setTimeFormat( "%H:%M %p" );
  time = QTime( 0, 22, 33 );
  QString timeStr = KGlobal::locale()->formatTime( time, true /*seconds*/, false /*duration*/ );
  check("formatTime(\"0:22\", as time)", timeStr, "00:22 am" );
  timeStr = KGlobal::locale()->formatTime( time, true /*seconds*/, true /*duration*/ );
  check("formatTime(\"0:22\", as duration)", timeStr, "00:22" );

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
