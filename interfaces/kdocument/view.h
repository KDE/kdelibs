#ifndef _KDOCUMENT_VIEW_
#define _KDOCUMENT_VIEW_

#include <qwidget.h>
#include <kxmlguiclient.h>

namespace KDocument {
	class View : public QWidget, public KXMLGUIClient {
		Q_OBJECT
		public:
			View(QWidget *parent):QWidget(parent),KXMLGUIClient() {}
			virtual ~View() {};
		signals:
			void focusIn ( View *view );
			void focusOut ( View *view );
			void informationMessage ( View *view, const QString &message );
	};
}

#endif

