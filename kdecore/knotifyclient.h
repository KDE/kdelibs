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
 * <pre>
 * [!Global!]
 * appname=executable
 * friendly=FriendlyNameOfApp
 *
 * [eventname]
 * friendly=FriendlyNameOfEvent
 * description=DescriptionOfEvent
 * sound=/path/to/sound/file
 * presentation=1
 *  ...
 * </pre>
 * This class works, but is still in testing.
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
	 * @param message The event type to send, such as "Desktop1" for a virtual
	 *                desktop change
	 * @param text If you need to send off a message with your alert.  This
	 *             will happen for an error level of 2 or more.
	 * @param present How to present it.  If "Default" is chosen, the server
	 *                will decide according to the config
	 * @param client The DCOPClient to use.  Usually it pulls the one from
	 *               your KApplication.
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
	 * @param message The name of the event
	 * @param text The text to put in a dialog box.  This won't be shown if
	 *             the user connected the event to sound, only.
	 */
	static bool event(const QString &message, const QString &text=0);
	/**
	 * Will fire an event that's not registered.
	 * @param text The error message text, if applicable
	 * @param present The error message level, one again, defaulting to "Default"
	 * @param soundfile The sound file to play if selected with present
	 */
	static bool userEvent(const QString &text=0, Presentation present=Default,
	                      const QString &soundfile=0);
	
	/**
	 * Gets the sound file associated with a certain event name
	 * The control panel module will list all the event names
	 * This has the potential for being slow
	 */
	static QString soundFileByName(const QString &eventname);

private:
	/**
	 * Why does kdoc include this? This is an internal structure that's actually
	 * declared under "private"
	 * @internal
	 */
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
