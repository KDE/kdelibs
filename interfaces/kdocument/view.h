#ifndef _KDOCUMENT_VIEW_
#define _KDOCUMENT_VIEW_

#include <qwidget.h>
#include <kxmlguiclient.h>

namespace KDocument {
	class KDE_EXPORT View : public QWidget, public KXMLGUIClient {
		Q_OBJECT
		public:
			View(QWidget *parent):QWidget(parent),KXMLGUIClient() {}
			virtual ~View() {};
		signals:
			void focusIn ( KDocument::View *view );
			void focusOut ( KDocument::View *view );
			void informationMessage ( KDocument::View *view, const QString &message );
	};
}

#endif

