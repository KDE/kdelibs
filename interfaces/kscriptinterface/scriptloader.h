#ifndef _script_loader_h
#define _script_loader_h
#include <qptrlist.h>
#include <kmainwindow.h>
#include <kaction.h>
#include <scriptinterface.h>

class ScriptLoader : virtual public QObject
{
	Q_OBJECT
	public:
		/* Defalut Constructor
		*/
		ScriptLoader(KMainWindow *parent=0);
		~ScriptLoader();
		/* Return the a KSelectAction with all of the scripts
		 * @returns KSelectAction containing access to all of the scripts
		 */
		KSelectAction *getScripts();
	public slots:
		/* Activate the current action.
		* @param QString actionName - the action that we wish to activate
		*/
		void runAction();
		/* Stop the currently running scripts operations
		*/
		void stopAction();
	signals:
		virtual void errors(QString messages);
		virtual void output(QString messages);
		virtual void done(int errorCode);
	private:
		QPtrList<KScriptInterface> m_scripts;
		KSelectAction *m_theAction;
		int m_currentSelection;
		KMainWindow *m_parent;
};
#endif
