#ifndef _KDOCUMENT_VIEW_
#define _KDOCUMENT_VIEW_

#include <qwidget.h>
#include <kxmlguiclient.h>

namespace KDocument {

	/**
	 * A widget with actions representing a document.
	 *
	 * A View represents the document's content. All views of the same
	 * document are synchronized.
	 */
	class KDE_EXPORT View : public QWidget, public KXMLGUIClient {
		Q_OBJECT
		public:
			/**
			 * Constructor.
			 *
			 * Create a new view with parent widget @p parent.
			 * @param parent parent widget
			 */
			View(QWidget *parent):QWidget(parent),KXMLGUIClient() {}
			/**
			 * Virtual destructor.
			 */
			virtual ~View() {}
		signals:
			/**
			 * This signal is emitted whenever the @p view got the focus.
			 * @param view view that emitted the signal
			 * @see focusOut()
			 */
			void focusIn ( KDocument::View *view );

			/**
			 * This signal is emitted whenever the @p view lost the focus.
			 * @param view view that emitted the signal
			 * @see focusIn()
			 */
			void focusOut ( KDocument::View *view );

			/**
			 * This signal is emitted whenever the @p view sends an
			 * information with text @p message.
			 * @param view view that emitted the signal
			 * @param message the information message
			 */
			void informationMessage ( KDocument::View *view, const QString &message );
	};
}

#endif

// kate: space-indent off; indent-width 2; tab-width 4; replace-tabs off;