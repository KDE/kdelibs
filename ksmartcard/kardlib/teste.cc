/* This file is part of the KDE project
 *
 * Copyright (C) 2001 Fernando Llobregat <fernando.llobregat@free.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#include <klocale.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <dcopclient.h>
#include <qstringlist.h>

#include <kcardfactory.h>
#include <kdebug.h>


bool getReaders (QStringList & list){

  QByteArray data, retval;
  QCString rettype;

  

  if (!kapp->dcopClient()->call("kded", "kardsvc", "getSlotList ()", 
				data, rettype, retval))
      return false;
  
  
  
  list.clear();
  QDataStream _retReader(retval, IO_ReadOnly);
  _retReader>>list;

  return true;

}


int main(int argc, char **argv) {
  KAboutData aboutData("kcardtest", I18N_NOOP("KCardtest"),
		      "1.0", I18N_NOOP("KDE Smartcard test program"), 
		      KAboutData::License_GPL, "(c) 2001, Fer");
  aboutData.addAuthor("Fernando Llobregat", 0, "fernando.llobregat@free.fr");

  KCmdLineArgs::init( argc, argv, &aboutData );
  //KCmdLineArgs::addCmdLineOptions( options );
  //KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KApplication app(false,false);
  
  //kapp->dcopClient()->attach();
  QStringList lista;
  if (!getReaders(lista)){

    kdDebug() << "ERROR retrieving reader list" << endl;
    exit(1);
  }
  KCardFactory * one = KCardFactory::self();

  for (QStringList::Iterator i=lista.begin();i!=lista.end();++i){

    kdDebug() << *i <<endl;

  }

  
  
  
  //  app.exec();
  
  return 0;
}

