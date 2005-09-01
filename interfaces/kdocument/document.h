#ifndef _KDOCUMENT_DOCUMENT_
#define _KDOCUMENT_DOCUMENT_

#include <kparts/part.h>

namespace KDocument {
	class View;
	class KDE_EXPORT Document : public KParts::ReadWritePart {
		Q_OBJECT
		public:
			Document(QObject *parent);
			virtual ~Document() {}
			virtual View *createView ( QWidget *parent ) = 0;
			virtual const QList<View*> &views () = 0;
			/**
			 * Get the global number of this document in your application.
			 * @return document number
			 */
			int documentNumber () const;
		private:
			/**
			 * Private d-pointer
			 */
			class Private;
			class Private *d;
			/**
			 * document number
			 */
			int m_documentNumber;
	};
}


#endif

