#ifndef _KNOTIFY_CLIENT
#define _KNOTIFY_CLIENT
#include <qobject.h>
#include <kapp.h>
#include <dcopclient.h>


/**
 * This class provides a method for issuing events to a KNotifyServer
 * call KNotifyClient::event("eventname"); to issue it.
 * On installation, there should be a file called 
 * $KDEDIR/share/apps/appname/eventsrc which contains the events.
 *
 * The file looks like this:
 * [eventname]
 * sound=/path/to/sound/file
 * presentation=1
 *  ...
 *
 * WARNING! This class is a class preview.  This class has already
 * been implemented for your use.  Please do use it, and even
 * install the eventsrc file, but don't expect it to work quite 
 * yet :)
 *
 * @author Charles Samuels <charles@altair.dhs.org>
 */
class KNotifyClient : public QObject
{
Q_OBJECT

public:
	enum Presentation
	{
		Default=-1,
		None=0,
		Sound=1,
		Messagebox=2,
		Logwindow=4,
		Logfile=8,
		Stderr=16
	};
	
public:
	/**
	 * The Default constructor.  You should have little use
	 * for this thanks to the @ref event method.
	 * @arg message The event type to send, such as "Desktop1" for a virtual
	 *              desktop change
	 * @arg text If you need to send off a message with your alert.  This
	 *           will happen for an error level of 2 or more.
	 * @arg present How to present it.  If "Default" is chosen, the server
	 *              will decide according to the config
	 * @arg client The DCOPClient to use.  Usually it pulls the one from
	 *             your KApplication.
	*/
	KNotifyClient(const QString &message, const QString &text=0,
	             Presentation present=Default,
	             const QString &soundfile=0,
	             DCOPClient* client=KApplication::kApplication()->dcopClient());

	virtual ~KNotifyClient();
	
public slots:
	/**
	 * If you need to send a message a whole lot sequentially, for reasons I 
	 * don't want to know, you might instanciate a KNotifyClient, and call
	 * this function when needed.
	 */
	bool send();

public: //static methods
	/**
	 * This should be the most used method in here.
	 * Call it by KNotifyClient::event("EventName");
	 * It will use KApplication::kApplication->dcopClient() to communicate to
	 * the server
	 * @arg message The name of the event
	 * @arg text The text to put in a dialog box.  This won't be shown if
	 *           the user connected the event to sound, only.
	 */
	static bool event(const QString &message, const QString &text=0);
	/**
	 * Will fire an event that's not registered.
	 * @arg text The error message text, if applicable
	 * @arg present The error message level, one again, defaulting to "Default"
	 * @arg soundfile The sound file to play if selected with present
	 */
	static bool userEvent(const QString &text=0, Presentation present=Default,
	                      const QString &soundfile=0);

private:
	struct Event
	{
		QString message;
		QString text;
		Presentation present;
		QString soundfile;
		DCOPClient *client;
	} *levent;
	
};

#endif
