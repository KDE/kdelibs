#ifndef __scriptinterface_h__
#define __scriptinterface_h__
#include <qobject.h>

class QString;

class KScriptInterface : public QObject
{
	Q_OBJECT
public:

	/**
		Return the current script code data
		@returns QString containing the currenly runable code
	**/
	virtual QString Script() const = 0;
	/**
		Sets the parent object of the script to the passed in
		QObject.  This is used to access public data members of
		the main application.  This is handy if your script runner
		contains an object twin.  
	**/
	virtual void setParent( QObject *parent) = 0;
	/**
		Sets the path to the actual script that we are going to embed.
	**/
	virtual void setScript( QString PathToCode ) = 0;

public slots:
	virtual void runScript() = 0;
	virtual void stopScript() =0;

signals:
	/**
		Emit a signal that contains the current error message from 
		a script output.
	**/
	virtual void errors(QString messages);
	virtual void output(QString messages);
	virtual void done(int errorcode);
	virtual void progress( int progress);
};

#endif
