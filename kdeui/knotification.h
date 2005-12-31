/* This file is part of the KDE libraries
   Copyright (C) 2005 Olivier Goffart <ogoffart @ kde.org>

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


#ifndef KNOTIFICATION_H
#define KNOTIFICATION_H


#include <qpixmap.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qpair.h>
#include <kdemacros.h>

class QWidget;

/**
 * KNotification is used to notify some event to the user.
 *
 * It covers severals kind of notifications
 *
 * @li Interface feedback events:
 * For notifying the user that he/she just performed an operation, like maximizing a
 * window. This allows us to play sounds when a dialog appears.
 * This is an instant notification.  It ends automatically after a small timeout
 *
 * @li complex notifications:
 * Notify when one received a new message, or when something important happened
 * the user has to know.  This notification has a start and a end.  It start when
 * the event actually occurs, and finish when the message is acknowledged.
 * 
 * In order to do a notification, you need to create a description files, which contains 
 * default parametters of the notification, and use KNotification::event at the place of the
 * code where the notification occurs.
 * the returned KNotification pointer may be used to connect signals or slots
 *
 * \section file The global config file
 * On installation, there should be a file called 
 *  <em>$KDEDIR/share/apps/appname/appname.notifyrc</em>
 *  This file contains  mainly 3 parts
 *   <ol><li>\ref global "Global information"</li>
 *       <li>\ref context "Context information"</li>
 *       <li>\ref events "Information about every events"</li></ol>
 *  
 *  \subsection global Global information
 * The global part looks like that
 * <pre>
		   [Global]
		   IconName=Filename
		   Comment=Freindly Name of app
 * </pre>
 *   The icon filename is just the name, without extention,  it's found with the KIconLoader
 * 
 * \subsection context Context information
 * 
 * This part is only hints for the configuration widget
 *  <pre>
		   [Context/group]
		   Name=Group name
		   Comment=The name of the group of the contact

		   [Context/folder]
		   Name=Group name
 *  </pre>
 *  the second part of the groupname is the context identifier.
 *  It should not contains special characters.
 *  The Name field is the one the user will see (and which is translated)
 * 
 * \subsection events Description of Events
 * 
 * Now comes the most important,  the description of each events.
 * <pre>
		   [Event/newmail]
		   Name=New email
		   Comment=You have got a new email
		   Contexts=folder,group
		   Action=Sound|Popup

		   [Event/contactOnline]
		   Name=Contact goes online
		   Comment=One of your contact has been connected
		   Contexts=group
		   Sound=filetoplay.ogg
		   Action=None
 *  </pre>
 *   All you put there are the default value.
 *   Action is a bitmask of KNotification::NotifyPresentation
 * 
 *  Contexts is a comma separated list of possible context for this event.
 * 
 *  \section userfile The user's config file
 * 
 *  This is only an implementation detail, for your information.
 * 
 * On the config file, there is two parts:  the events configuration, and the context informations
 * \subsection context Context informations
 *  This is only hints for the configuration dialog. It contains both the internal id of the context, and the user visible string.
 *  <pre>
		   [Context/group]
		   Values=1:Friends,2:Work,3:Family
 *  </pre>
 * \subsection event Events configuration
 *   This contains the configuration of events for the user.
 *   It contains the same fields as the description file.
 *    The key of groups is in the form 
 *  <em>Event/&lt;EventName&gt;/&lt;ContextName&gt;/&lt;ContextValue&gt;</em>
 * <pre>
		   [Event/contactOnline]
		   Action=Sound
		   Sound=/usr/share/sounds/super.ogg

		   [Event/contactOnline/group/1]
		   Action=PassivePopup|Sound
 * </pre>
 * 
 * \section example Example of code
 * 
 * This portion of code will fire the event for the "contactOnline" event
 * 
 * @code
	KNotification::ContextList contexts;
	foreach( QString group , contact->groups() ) {
		contexts.append( qMakePair( QString("group") , group ) );
	}
    
	KNotification *notification=KNotification::event( "contactOnline",
			i18n("The contact <i>%1</i> has gone online").arg( contact->name() ),
			contact->pixmap() , 0l , QStringList( i18n( "Open chat" ) ) , contexts );
	connect(notify, SIGNAL(activated(unsigned int )), contact , SLOT(slotOpenChat()) );
 * @endcode
 * 
 * @author Olivier Goffart  \<ogoffart\@kde.org\>
 */
class KDE_EXPORT KNotification : public QObject
{
        Q_OBJECT

public:
	/**
	 * Sometimes, the user may want different notification for the same event, 
	 * depending the source of the event.  Example, you wan to be notified for mails
	 * that arrives in your folder "personal inbox" but not for those in "spam" folder
	 * 
	 * A notification context is a pair of two strings. 
	 * The first string is a key from what the context is.  example "group" or 
	 * "filter" (not translated).
	 * The second is the id of the context. In our example, the group id or the 
	 * filter id in the applications.
	 * Theses string are the one present in the config file, and are in theory not 
	 * shown in the user interface
	 * 
	 * the order of contexts in the list is is important, most important context 
	 * should be placed first. They are proceded in that order when the notification occurs.
	 *
	 * @see event
	 */
	typedef QList< QPair<QString,QString> > ContextList;

	enum NotificationFlags
	{
		/**
		 * When the notification is activated, raise the notification's widget.
		 *
		 * This will change the desktop, raise the window, and switch to the tab.
		 * @todo  doesn't works yet
		 */
		RaiseWidgetOnActivation=0x01,

		/**
		 * The notification will be automatically closed after a timeout.
		 */
		CloseOnTimeout=0x02,
		/**
		 * The notification will be automatically closed if the widget() becomes
		 * activated.
		 *
		 * If the widget is already activated when the notification occurs, the
		 * notification will be closed after a small timeout.
		 * @todo doesn't works yet
		 */
		CloseWhenWidgetActivated=0x04,

		/**
		 * @internal
		 * The event is a standard kde event, and not an event of the application
 		 */
		DefaultEvent=0x08
		
	};

	/**
	 * default events you can use in the event function
	 */
	enum StandardEvent {
		cannotOpenFile,
		notification,
		warning,
		fatalError,
		catastrophe
	};

	~KNotification();

    /**
	 * @brief the widget associated to the notification
	 *
	 * If the widget is destroyed, the notification will be automatically canceled.
	 * If the widget is activated, the notificaiton will be automatically closed if the flags said that
	 *
	 * When the notification is activated, the widget might be raised.
	 * Depending of the configuration, the taskbar entry of the window containing the widget may blink.
	 */
	QWidget *widget() const;
			  
   signals:
	/**
	 * Emit only when the default activation has occured
	 */
	void activated();
	/**
	 * Emit when an action has been activated.
	 * @param action will be 0 is the default aciton was activated, or any actiton id
	 */
	void activated(unsigned int action);

	/**
	 * Emit when the notification is closed. Both if it's activated or just ignored
	 */
	 void closed();

	/**
	 * The notification has been ignored
	 */
	void ignored();

public slots:
	/**
	 * @brief Active the action specified action
	 * If the action is zero, then the default action is activated
	 */
	void activate(unsigned int action=0);

	/**
	 * close the notification without activate it.
	 *
	 * This will delete the notification
	 */
	void close();

	/**
	 * @brief Raise the widget.
	 * This will change the desktop, activate the window, and the tab if needed.
	 */
	void raiseWidget();

	/**
	 * The notification will automatically be closed if all presentation are finished.
	 * if you want to show your own presentation in your application, you should use this
	 * function, so it will not be automatically closed when there is nothing to show.
	 * 
	 * don't forgot to deref, or the notification may be never closed if there is no timeout.
	 * @see ref
	 */
	void ref();
	/**
	 * remove a reference made with ref()
	 * the notification may be closed when calling this.
	 * @see ref
	 */
	void deref();


private:
	struct Private;
	Private *d;
	KNotification(QObject *parent=0L);
	/**
	 * recursive function that raise the widget. @p w
	 *
	 * @see raiseWidget()
	 */
	static void raiseWidget(QWidget *w);


public:
	/**
	 * @brief emit an event
	 *
	 * A popup may be showed, a sound may be played, depending the config.
	 *
	 * return a KNotification .  You may use that pointer to connect some signals or slot.
	 * the pointer is automatically deleted when the event is closed.
	 *
	 * Make sure you use one of the CloseOnTimeOut or CloseWhenWidgetActivated, if not,
	 * you have to close yourself the notification.
	 *
	 * @note the text is shown in a QLabel, you should make sure to escape the html is needed.
	 *
	 * @param eventId is the name of the event
	 * @param text is the text of the notification to show in the popup.
	 * @param pixmap is a picture which may be shown in the popup.
	 * @param widget is a widget where the notification reports to
	 * @param actions is a list of action texts.
	 * @param contexts is the lists of contexts, see ContextList
	 * @param flags is a bitmask of KNotificationFlags  
	 */
	static KNotification *event( const QString& eventId , const QString& text=QString::null,
			const QPixmap& pixmap=QPixmap(), QWidget *widget=0L,
			const QStringList &actions=QStringList(), ContextList contexts=ContextList() ,
			unsigned int flags=CloseOnTimeout);
	
	/**
	 * @brief emit standard an event
	 * @overload
	 *
	 * This will emit a standard event
	 *
	 * @param eventId is the name of the event
	 * @param text is the text of the notification to show in the popup.
	 * @param pixmap is a picture which may be shown in the popup.
	 * @param widget is a widget where the notification reports to
	 * @param flags is a bitmask of KNotificationFlags  
	 */
	static KNotification *event( StandardEvent eventId , const QString& text=QString::null,
								 const QPixmap& pixmap=QPixmap(), QWidget *widget=0L,
								unsigned int flags=CloseOnTimeout);
	
	/**
	 * This is a simple substitution for QApplication::beep()
	 * 
	 * @param reason a small text explaining what's happen (may be null)
	 * @param widget the widget where the notification refer to 
	 */
	static void beep( const QString& reason = QString::null , QWidget *widget=0L);

	/**
	 * @return the name of the event
	 */
	QString eventId() const;
	
	/**
	 * @return the notification text
	 */
	QString text() const ;
	
	/**
	 * @return the notification title
	 */
	QString title() const;


public:	
};



#endif
