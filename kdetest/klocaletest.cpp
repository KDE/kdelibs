// klocaletest.cpp     -*- C++ -*-
//
// $Id$
// 
// Author: Jacek Konieczny <jajcus@zeus.polsl.gliwice.pl>
//

#include <stdlib.h>

#include "klocaletest.h"
#include <kapp.h>
#include <kcharsets.h>
#include <qlabel.h>
#include <locale.h>
#include <time.h>
#include <stdio.h>

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

QString Test::showLocale(QString category){

  return category+"="+klocale->getLocale(category)+"\n";
}

void Test::createFields()
{
  QString string;
  
  string+="Selected languages: ";
  string+=klocale->languages()+"\n";

  // This will show nothing, as there is no klocaletest.mo
  // but you can copy other *.mo file
  string+="Used language: ";
  string+=klocale->language()+"\n";
  string+="Locale charset: ";
  string+=klocale->charset()+"\n";

  KCharset charset=klocale->charset();
  string+="Is it ok?: ";
  string+=charset.ok()?"yes":"no";
  string+="\n";
  if (charset.ok()){
    string+="Canonical name: ";
    string+=charset.name();
    string+="\n";
  }  
  string+="Locale categories: \n";
  string+=showLocale("LC_MESSAGES");
  string+=showLocale("LC_CTYPE");
  string+=showLocale("LC_COLLATE");
  string+=showLocale("LC_NUMERIC");
  string+=showLocale("LC_MONETARY");
  string+=showLocale("LC_TIME");
  string+="Localized date and time: ";

  // Don't use Qt time & date functions. They are not localized!
  char buffer[256];
  time_t curtime;
  struct tm *loctime;
  /* Get the current time. */
  curtime = time (NULL);
  /* Convert it to local time representation. */
  loctime = localtime (&curtime);
  /* Print it out in a nice format. */
  strftime (buffer, 256, "%c\n", loctime);
  string+=buffer;
  string+="Non-localized number: ";
  sprintf(buffer,"%6.4f\n",3.141592);
  string+=buffer;
  string+="Localized number: ";
  // localization of number representation is off by default
  klocale->enableNumericLocale(true);
  sprintf(buffer,"%6.4f\n",3.141592);
  string+=buffer;
  // remeber of disabling numeric localization after use
  // in no case localized number may be written to config file
  // or other machine parsed code
  klocale->enableNumericLocale(false);
  // This will not work
  // but you can copy other *.mo file
  string+="Some localized strings:\n";
  string+=QString("Yes = ")+i18n("Yes")+"\n";
  string+=QString("No = ")+i18n("No")+"\n";
  string+=QString("Help = ")+i18n("Help")+"\n";
  string+=QString("Cancel = ")+i18n("Cancel")+"\n";
  
  label=new QLabel(string,this,"Label");
  label->setGeometry(10,10,400,400);
  label->setFont(kapp->generalFont);
  label->show();
}

int main( int argc, char ** argv )
{
  KApplication a( argc, argv );
  Test m;

  a.setMainWidget( &m );

  m.show();

  return a.exec();
}
