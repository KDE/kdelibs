/*
 * Copyright Nadeem Hasan <nhasan@kde.org>
 * Licensed under the GNU General Public License version 2
 */

#include <QtGui/QApplication>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kinputdialog.h>
#include <kdebug.h>
#include <klistwidget.h>

#include <QtCore/QString>
#include <QtGui/QValidator>
#include <kcmdlineargs.h>


int main( int argc, char *argv[] )
{
  QApplication app( argc, argv );
//  app.setApplicationName("kinputdialogtest"); //since there is no kapp usage, set the application name directly
  KAboutData aboutData( "kinputdialogtest", 0, ki18n("kinputdialogtest"), "1.0" );
  KComponentData i( &aboutData );

  bool ok;
  QString svalue;
  int ivalue;
  double dvalue;

  svalue = KInputDialog::getText( "_caption", "_label:", "_value", &ok );
  kDebug() << "value1: " << svalue << ", ok: " << ok;

  QRegExpValidator validator( QRegExp( "[0-9]{3}\\-[0-9]{3}\\-[0-9]{4}" ), 0 );
  svalue = KInputDialog::getText( "_caption", "_label:", "_value", &ok, 0L,
    &validator );
  kDebug() << "value2: " << svalue << ", ok: " << ok;

  svalue = KInputDialog::getText( "_caption", "_label:", "_value", &ok, 0L, 0L,
    0L, "900.900.900.900" );
  kDebug() << "value1: " << svalue << ", ok: " << ok;

  ivalue = KInputDialog::getInteger( "_caption", "_label:", 64, 0, 255,
    16, 16, &ok );
  kDebug() << "value3: " << ivalue << ", ok: " << ok;

  ivalue = KInputDialog::getInteger( "_caption", "_label:", 100, 0, 255,
    10, 10, &ok );
  kDebug() << "value4: " << ivalue << ", ok: " << ok;

  dvalue = KInputDialog::getDouble( "_caption", "_label:", 10, 0, 100, 0.1,
    2, &ok );
  kDebug() << "value5: " << dvalue << ", ok: " << ok;

  dvalue = KInputDialog::getDouble( "_caption", "_label:", 10, 0, 100, 2, &ok );
  kDebug() << "value6: " << dvalue << ", ok: " << ok;

  QStringList list, slvalue;
  list << "Item 1" << "Item 2" << "Item 3" << "Item 4" << "Item 5";
  svalue = KInputDialog::getItem( "_caption", "_label:", list, 1, false, &ok );
  kDebug() << "value7: " << svalue << ", ok: " << ok;

  svalue = KInputDialog::getItem( "_caption", "_label:", list, 1, true, &ok );
  kDebug() << "value8: " << svalue << ", ok: " << ok;

  QStringList select;
  select << "Item 3";
  list << "Item 6" << "Item 7" << "Item 8" << "Item 9" << "Item 10";
  slvalue = KInputDialog::getItemList( "_caption", "_label:", list, select,
    false, &ok );
  kDebug() << "value9: " << slvalue << ", ok: " << ok;

  select << "Item 5";
  slvalue = KInputDialog::getItemList( "_caption", "_label:", list, select,
    true, &ok );
  kDebug() << "value10: " << slvalue << ", ok: " << ok;
}
