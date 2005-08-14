#ifndef _KDOCUMENT_DOCUMENT_
#define _KDOCUMENT_DOCUMENT_

#include <kparts/part.h>

namespace KDocument {
	class View;
	class Document : public KParts::ReadWritePart {
		Q_OBJECT
		public:
			Document(QObject *parent): KParts::ReadWritePart(parent) {}
			virtual ~Document() {}
			virtual View *createView ( QWidget *parent ) = 0;
			virtual const QList<View*> &views () = 0;

	};
}


#endif

