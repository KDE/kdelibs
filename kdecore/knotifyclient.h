/* This file is part of the KDE libraries
   Copyright (C) 2000 Charles Samuels <charles@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef _KNOTIFY_CLIENT
#define _KNOTIFY_CLIENT
#include <qstring.h>

class KInstance;
#undef None // X11 headers...

/**
 * This namespace provides a method for issuing events to a KNotifyServer
 * call KNotifyClient::event("eventname"); to issue it.
 * On installation, there should be a file called
 * $KDEDIR/share/apps/appname/eventsrc which contains the events.
 *
 * The file looks like this:
 * <pre>
 * [!Global!]
 * IconName=Filename (e.g. kdesktop, without any extension)
 * Comment=FriendlyNameOfApp
 *
 * [eventname]
 * Name=FriendlyNameOfEvent
 * Comment=Description Of Event
 * default_sound=filetoplay.wav
 * default_presentation=1
 *  ...
 * </pre>
 * default_presentation contains these ORed events:
 *	None=0, Sound=1, Messagebox=2, Logfile=4, Stderr=8
 *
 * You can also use the "nopresentation" key, with any the presentations
 * ORed.  Those that are in that field will not appear in the kcontrol
 * module.  This was intended for software like KWin to not allow a window-opening
 * that opens a window (e.g., allowing to disable KMessageBoxes from appearing)
 * If the user edits the eventsrc file manually, it will appear.  This only
 * affects the KcmNotify.
 *
 * You can also use the following events, which are system controlled
 * and do not need to be placed in your eventsrc:
 *
 *<ul>
 * <li>cannotopenfile
 * <li>notification
 * <li>warning
 * <li>fatalerror
 * <li>catastrophe
 *</ul>
 *
 * @author Charles Samuels <charles@kde.org>
 */


namespace KNotifyClient
{
    struct InstancePrivate;
	class InstanceStack;

    /**
     * Makes it possible to use @ref KNotifyClient with a @ref KInstance
     * that is not the application.
     *
     * Use like this:
     * <pre>
     * KNotifyClient::Instance(myInstance);
     * KNotifyClient::event("MyEvent");
     * </pre>
     *
     * @short Enables @ref KNotifyClient to use a different @ref KInstance
     */
    class Instance
    {
    public:
        /**
         * Constructs a @ref KNotifyClient::Instance to make @ref KNotifyClient use
         * the specified @ref KInstance for the event configuration.
	 * @param instance the instance for the event configuration
         */
        Instance(KInstance *instance);
        /**
         * Destructs the KNotifyClient::Instance and resets @ref KNotifyClient
         * to the previously used @ref KInstance.
         */
        ~Instance();
	/**
	 * Checks whether the system bell should be used.
	 * @returns true if this instance should use the System bell instead
	 * of KNotify.
	 */
	bool useSystemBell() const;
        /**
         * Returns the currently active @ref KInstance.
	 * @return the active KInstance
         */
        static KInstance *current();

	/**
	 * Returns the current KNotifyClient::Instance (not the KInstance).
	 * @return the active Instance
	 */
	static Instance *currentInstance();
	
    private:
		static InstanceStack *instances();
		InstancePrivate *d;
		static InstanceStack *s_instances;
    };


    /**
     * Describes the notification method.
     */
	enum {
		Default = -1,
		None = 0,
		Sound = 1,
		Messagebox = 2,
		Logfile = 4,
		Stderr = 8,
		PassivePopup = 16,
		Execute = 32
	};

	/**
	 * Describes the level of the error.
	 */
	enum {
		Notification=1,
		Warning=2,
		Error=4,
		Catastrophe=8
	};

	/**
	 * default events you can use
	 */
	enum StandardEvent {
		cannotOpenFile,
		notification,
		warning,
		fatalError,
		catastrophe
	};

	/**
	 * This starts the KNotify Daemon, if it's not already started.
	 * This will be useful for games that use sound effects. Run this
	 * at the start of the program, and there won't be a pause when it is
	 * first triggered.
	 * @return true if daemon is running (always true at the moment)
	 **/
	bool startDaemon();

	/**
	 * This should be the most used method in here.
	 * Call it by KNotifyClient::event("EventName");
	 * It will use KApplication::kApplication->dcopClient() to communicate to
	 * the server
	 * @param message The name of the event
	 * @param text The text to put in a dialog box.  This won't be shown if
	 *             the user connected the event to sound, only. Can be QString::null.
	 * @return true if successful, false otherwise
	 */
	bool event(const QString &message, const QString &text=QString::null);

	/**
	 * Allows to easily emit standard events.
	 * @param event The event you want to raise.
	 * @param text The text explaining the event you raise. Can be QString::null.
	 * @return true if successful, false otherwise
	 */
	bool event( StandardEvent event, const QString& text=QString::null );

	/**
	 * Will fire an event that's not registered.
	 * @param text The error message text, if applicable
	 * @param present The presentation method(s) of the event
	 * @param level The error message level, defaulting to "Default"
	 * @param sound The sound file to play if selected with @p present
	 * @param file The log file to play if selected with @p present
	 * @return true if successful, false otherwise
	 */
	bool userEvent(const QString &text=QString::null, int present=Default, int level=Default,
	                      const QString &sound=QString::null, const QString &file=QString::null);
	
	/**
	 * This is a simple substitution for QApplication::beep().
	 * It simply calls
	 * <pre>
	 * KNotifyClient::event( KNotifyClient::notification, reason );
	 * </pre>
	 * @param reason the reason, can be QString::null.
	 */
	void beep(const QString& reason=QString::null);

	/**
	 * Gets the presentation associated with a certain event name
	 * Remeber that they may be ORed:
	 * <pre>
	 * if (present & KNotifyClient::Sound) { [Yes, sound is a default] }	
	 * </pre>
	 * @param eventname the event name to check
	 * @return the presentation methods
	 */
	int getPresentation(const QString &eventname);
	
	/**
	 * Gets the default file associated with a certain event name
	 * The control panel module will list all the event names
	 * This has the potential for being slow.
	 * @param eventname the name of the event
	 * @param present the presentation method
	 * @return the associated file. Can be QString::null if not found.
	 */
	QString getFile(const QString &eventname, int present);
	
	/**
	 * Gets the default presentation for the event of this program.
	 * Remember that the Presentation may be ORed.  Try this:
	 * <pre>
	 * if (present & KNotifyClient::Sound) { [Yes, sound is a default] }
	 * </pre>
	 * @return the presentation methods
	 */
	int getDefaultPresentation(const QString &eventname);
	
	/**
	 * Gets the default File for the event of this program.
	 * It gets it in relation to present.
	 * Some events don't apply to this function ("Message Box")
	 * Some do (Sound)
	 * @param eventname the name of the event
	 * @param present the presentation method
	 * @return the default file. Can be QString::null if not found.
	 */
	QString getDefaultFile(const QString &eventname, int present);

	/**
	 * Shortcut to KNotifyClient::Instance::current() :)
	 * @returns the current KInstance.
	 */
	KInstance * instance();
};

#endif
