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

 
#include <qfile.h> 
#include <qvariant.h> 
#include <qstring.h> 
#include <qstringlist.h> 
#include <klibloader.h> 
#include <kservicetype.h> 
 
#include <kdebug.h> 
 
KCardFactory::KCardFactory() { 

  //QMap< QString, QMap< QString, QMap< QString,void*> > > _modules;  

  loadModules();
  
//   _modulesMap::Iterator i;
  
  
//   for ( i=_modules.begin();i!=_modules.end();++i){

//     _modules[i.key().latin1()]::Iterator j;
//     for (  j=_modules[i.key().latin1()].begin();
// 	   j!=_modules[i.key().latin1()].end();
// 	   ++j){
      
//       kdDebug() << i.key().latin1() << endl;
      
//     }
    
//   }
  //  kdDebug() << _type << _subType << _subSubType.join("-") << endl;

} 
 
 
KCardFactory::~KCardFactory() { 
} 
 
 
KCardFactory *KCardFactory::_self = NULL; 
 
 
KCardFactory *KCardFactory::self() { 
	if (!_self) 
		_self = new KCardFactory; 
	return _self; 
} 
 
 
KCardImplementation * KCardFactory::getCard (KCardReader * /*selReader*/,  
					     KCardType selcardType,  
					     KCardATR atr){ 
 
  switch (selcardType){ 
 
 
  case KCardGSMType: 
    
    break; 
 
 
  case KCardProcessorType: 
 
    break; 
 
  default: 
    break; 
  } 
 
 
return NULL; 
} 
 
 
void *KCardFactory::loadModule(KService::Ptr svc) { 



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
		if (!create) { 
			loader->unloadLibrary(QFile::encodeName(factory)); 
			return NULL; 
		} 
		return create; 
	} else { 
		return NULL; 
	} 
} 
 
 
int KCardFactory::loadModules() { 
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
 
		void *f = loadModule(service); 
 
		
		for (QStringList::Iterator j = _subSubType.begin(); 
					    j != _subSubType.end(); 
								++j) { 
			_modules[_type][_subType][*j] = f; 
		} 
	} 
	
	return 0; 
} 
 

 
 
