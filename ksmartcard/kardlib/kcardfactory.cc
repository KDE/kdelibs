/* This file is part of the KDE project 
 * 
 * Copyright (C) 2001 Fernando Llobregat <fernando.llobregat@free.fr > 
 * Copyright (C) 2001 George Staikos <staikos@kde.org> 
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
 
#include "kcardfactory.h" 
#include "kpcsc.h"
#include "kcarddb.h" 
#include "kcardimplementation.h"
 
#include <qfile.h> 
#include <qvariant.h> 
#include <qstring.h> 
#include <qstringlist.h> 

#include <dcopclient.h>
#include <klibloader.h> 
#include <kservicetype.h> 
#include <klocale.h>
#include <kapplication.h>
 
#include <kdebug.h> 
 
KCardFactory::KCardFactory() { 


  loadModules();
  


  
  
// for (_modulesMap::Iterator x=_modules.begin(); x!=_modules.end();++x) {

  
//   for (QMap< QString,QMap< QString,void*> >::Iterator y = x.data().begin();
//        y!=x.data().end();
//        ++y){

    
//     for(QMap< QString,void*> ::Iterator z=y.data().begin();
// 	z!=y.data().end();
// 	++z){
      
//       //      kdDebug()<<"MAIN-> " << "_type " << x.key() << "  _subType:"<< y.key() << "  _subSubType" << z.key() << endl;
      
	   
//     }
    
    
//   }
// }
 



} 
 
 
KCardFactory::~KCardFactory() { 
} 
 
 
KCardFactory *KCardFactory::_self = NULL; 
QStringList  KCardFactory::_implementationList = QStringList();
 
KCardFactory *KCardFactory::self() { 
	if (!_self) 
		_self = new KCardFactory; 
	return _self; 
} 
 


KCardImplementation * KCardFactory::getCard (const QString & slot,  
					     const QString & atr)
{ 

  KCardDB cardBase;
  KCardImplementation * impl=NULL;
  QString handler = cardBase.getModuleName(atr);
  

  if (handler==QString::null) {
   return NULL;
  }
  QStringList types = QStringList::split(",",handler);
  
  
  KService::Ptr serv = _modules[types[0]][types[1]][types[2]];

  
  
  impl= (KCardImplementation *)loadModule(serv);

  
  
  if (impl==NULL) return NULL;
  if (impl->init(slot)) return NULL;
  

  return impl;
  
} 

KCardImplementation * KCardFactory::getCard (const QString & slot)
{ 
  
  QByteArray retval;
  QCString   rettype;
  
  QByteArray dataATR;
  QDataStream argATR(dataATR,IO_WriteOnly);
  argATR << slot;
  
  kapp->dcopClient()->call("kded", "kardsvc", "getCardATR(QString)", 
			   dataATR, rettype, retval);
  
  
  QString cardATR;
  QDataStream _retReaderATR(retval, IO_ReadOnly);
  _retReaderATR>>cardATR;
  

  if (cardATR.isNull()){
    
    return NULL;
  }

  return getCard(slot,cardATR);
  
  
}

QStringList & KCardFactory::getImplementationList()const{

     return _implementationList;

} 
void *KCardFactory::loadModule(KService::Ptr svc) { 


  KCardImplementation * module=0;
  if (!svc || svc->library().isEmpty()) 
    return NULL; 
  
  QCString obj = svc->desktopEntryName().latin1(); 
  
  KLibLoader *loader = KLibLoader::self(); 
  
  QVariant v = svc->property("X-KDE-Factory"); 
  
  QString factory = v.isValid() ? v.toString() : QString::null; 
  if (factory.isEmpty()) 
    factory = svc->library(); 
  
  factory = "create_" + factory; 
  QString libname = "libkscard_" + svc->library(); 
  
  
  KLibrary *lib = loader->library(QFile::encodeName(libname)); 
  if (lib) { 
    void *create = lib->symbol(QFile::encodeName(factory)); 
    
	  if (create){
	    
	    KCardImplementation* (*func)(void); 
	    func = (KCardImplementation* (*)(void)) create;
	    module = func();
	    
	    if (module){
	      
	      return module; 
	    }
	    
	    loader->unloadLibrary(QFile::encodeName(factory)); 
	    return NULL; 
	    
	    
	  }
	  else { 
	    return NULL; 
	  } 
  }
}

 
int KCardFactory::loadModules() { 
  
  _implementationList.clear();
  
	KService::List kards = KServiceType::offers("KDESmartcard"); 
	for (KService::List::ConstIterator it = kards.begin();  
					    it != kards.end();  
					    ++it) { 
		KService::Ptr service = *it; 
		QString _type = service->property("X-KDE-Smartcard-Type").toString(); 
		QString _subType = service->property("X-KDE-Smartcard-SubType").toString(); 
		QStringList _subSubType = service->property("X-KDE-Smartcard-SubSubType").toStringList(); 


		if (_type == QString::null) 
			continue; 
 
		if (_subType == QString::null) 
			continue; 
 
		// subSubType can be NULL for instances where a manufacturer 
		// only makes one type of card or there is a driver which can 
		// handle all cards from that manufacturer. 
		// A better solution is to put them all in the property as 
		// a list though. 
 
		//void *f = loadModule(service); 
		
		
		for (QStringList::Iterator j = _subSubType.begin(); 
					    j != _subSubType.end(); 
								++j) { 
		  _modules[_type][_subType][*j] = service;
		  QString tp= _type +",";
		  tp+=_subType + ",";
		  tp+=_subSubType.join("-");
		  _implementationList.append(tp);
	
		} 


	}


	
	return 0; 
} 
 

 
 
