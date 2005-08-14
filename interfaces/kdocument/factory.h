#ifndef _KDOCUMENT_FACTORY_
#define _KDOCUMENT_FACTORY_

#include <kdocument/document.h>
#include <kparts/genericfactory.h>
#include <qobject.h>
#include <qstring.h>
namespace KDocument {

	class Factory {
		public:
			Factory(){}
			virtual ~Factory(){}
			virtual KDocument::Document *createDocument(QObject *parent,const QStringList&)=0;
	};


	template <class T> class GenericFactory: public KParts::GenericFactory<T>, Factory {
		public:
			GenericFactory(): KParts::GenericFactory<T>(),Factory() {}
			virtual ~GenericFactory(){}
			
			KDocument::Document *createDocument(QObject *parent,const QStringList &args) {
				return new T(parent,args);
			}
	};

	Factory *factory(KParts::Factory* _factory);
}
#endif

