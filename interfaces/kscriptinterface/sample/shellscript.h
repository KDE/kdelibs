#ifndef __shellscript_h__
#define __shellscript_h__
#include <scriptinterface.h>
#include <kprocess.h>

class ShellScript : public KScriptInterface
{
	Q_OBJECT
public:
	ShellScript(QObject *parent, const char *name, const QStringList &args);
	virtual ~ShellScript();
	/**
		Return the current script code data
		@returns QString containing the currenly runable code
	**/
	QString Script() const;
	/**
		Sets the parent object of the script to the passed in
		QObject.  This is used to access public data members of
		the main application.  This is handy if your script runner
		contains an object twin.
	**/
	void setParent( QObject *parent);
	/**
		Sets the path to the actual script that we are going to embed.
	**/
	void setScript( QString PathToCode );
public slots:
	void runScript();
	void stopScript();
	bool status();
private slots:
	void goodExit(KProcess *proc);
private:
	KProcess *m_script;
};

#endif
