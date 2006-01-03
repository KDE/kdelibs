/*
  Copyright (C) 2000,2002 Carsten Pfeiffer <pfeiffer@kde.org>
  Copyright (C) 2002 Neil Stevens <neil@qualityassistant.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation;

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library,  If not, write to the Free Software Foundation,
  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KNOTIFYDIALOG_H
#define KNOTIFYDIALOG_H

#include <klistview.h>
#include <kdialogbase.h>
#include <kinstance.h>
#include <kglobal.h>

#include "knotifywidgetbase.h"

class QShowEvent;

namespace KNotify
{
    class KNotifyWidget;
}

/**
 * KNotifyDialog presents an interface for configuring an application's
 * KNotify events.
 *
 * Rather than requiring the user to wade through the entire list of
 * applications' events in KControl, your application can make the list
 * of its own notifications available here.
 *
 * Typical usage is calling the static configure() method:
 * \code
 * (void) KNotifyDialog::configure( someParentWidget );
 * \endcode
 *
 * @since 3.1
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KIO_EXPORT KNotifyDialog : public KDialogBase
{
    Q_OBJECT

public:
    /**
     * If you want a non-modal dialog, you need to instantiate KNotifyDialog
     * yourself instead of using the configure() method.
     *
     * KDE4.0 modal default will be false.
     * 
     * @param parent The parent widget for the dialog
     * @param name The widget name
     * @param modal If true, this will be a modal dialog, otherwise non-modal.
     * @param aboutData A pointer to a KAboutData object. KAboutData::appName()
     *                  will be used to find the KNotify events (in the eventsrc file).
     *                  Set this to 0L if you want to add all events yourself with
     * addApplicationEvents().
     */
    KNotifyDialog( QWidget *parent = 0, const char *name = 0,
                   bool modal = true,
                   const KAboutData *aboutData =
                   KGlobal::instance()->aboutData() );
    /**
     * Destroys the KNotifyDialog
     */
    virtual ~KNotifyDialog();

    /**
     * Convenience method to create exec() a modal KNotifyDialog.
     *
     * @param parent The parent widget for the dialog
     * @param name The widget name
     * @param aboutData A pointer to a KAboutData object. KAboutData::appName()
     *                  will be used to find the KNotify events (in the eventsrc file).
     * @see exec for the return values.
     * @return The value of QDialog::exec()
     */
    static int configure( QWidget *parent = 0, const char *name = 0,
                          const KAboutData *aboutData = KGlobal::instance()->aboutData() );

    /**
     * With this method, you can add the KNotify events of one eventsrc
     * files to the view.
     * KNotifyDialog can handle events for multiple applications (i.e. eventsrc files).
     * Successive calls with a different @p appName will add them.
     * @param appName The application's name, i.e. the name passed to the 
     *                KApplication constructor or KAboutData.
     * @see clearApplicationEvents()
     */
    virtual void addApplicationEvents( const char *appName );

    /**
     * With this method, you can add the KNotify events of one eventsrc
     * files to the view.
     * KNotifyDialog can handle events for multiple applications (i.e. eventsrc files).
     * Successive calls with a different @p path will add them.
     * @param path The absolute or relative path to the eventsrc file to be configured.
     *             A relative path would be e.g. "kwin/eventsrc".
     * @see clearApplicationEvents()
     */
    virtual void addApplicationEvents( const QString& path );

    /**
     * Removes all the events added with addApplicationEvents()
     * @see addApplicationEvents()
     */
    virtual void clearApplicationEvents();

private slots:
    void slotDefault();

private:
    enum
    {
        COL_FILENAME = 1
    };

    void updateView();

    KNotify::KNotifyWidget * m_notifyWidget;

    class Private;
    Private *d;
};


namespace KNotify
{
    class Application;
    class Event;
    class ListViewItem;
    typedef QList<Event*> EventList;
    typedef QListIterator<Application*> ApplicationListIterator;
    typedef QListIterator<Event*> EventListIterator;

    /**
     * @internal
     */
    class KIO_EXPORT Application
    {
    public:
        Application( const QString &path );
        ~Application();

        QString text() const { return m_description; }
        QString icon() const { return m_icon; }
        const EventList& eventList();
        void reloadEvents( bool revertToDefaults = false );
        void save();

        QString appName() const { return m_appname; }

    private:
        QString m_icon;
        QString m_description;
        QString m_appname;
        EventList *m_events;

        KConfig *kc; // The file that defines the events.
        KConfig *config; // The file that contains the settings for the events
    };


    class KIO_EXPORT ApplicationList : public QList<Application*>
    {
		public:
		virtual ~ApplicationList(){}
		private:
        virtual int compareItems ( Q3PtrCollection::Item item1,
                                   Q3PtrCollection::Item item2 )
        {
            return (static_cast<Application*>( item1 )->text() >=
                static_cast<Application*>( item2 )->text()) ? 1 : -1;
        }
    };

    /**
     * @internal
     */
    class KIO_EXPORT KNotifyWidget : public KNotifyWidgetBase
    {
        Q_OBJECT

    public:
        KNotifyWidget( QWidget* parent = 0, const char* name = 0,
                       bool handleAllApps = false );
        ~KNotifyWidget();

        KListView * eventsView() {
            return m_listview;
        }

        void addVisibleApp( Application *app );
        ApplicationList& visibleApps() { return m_visibleApps; }
        ApplicationList& allApps() { return m_allApps; }

        /**
         * Returns 0L if no application events could be found
         * The returned pointer must be freed by the caller (easiest done
         * by putting it into an ApplicationList with setAutoDelete( true )).
         */
        Application * addApplicationEvents( const QString& path );

        void resetDefaults( bool ask );
        void sort( bool ascending = true );

    public slots:
        /**
         * Clears the view and all the Application events.
         */
        virtual void clear();
        /**
         * Clears only the view and the visible Application events.
         * E.g. useful if you want to set new visible events with
         * addVisibleApp()
         */
        virtual void clearVisible();
        virtual void save();
        virtual void showAdvanced( bool show );
        void toggleAdvanced();


    signals:
        void changed( bool hasChanges );

    protected:
        /**
         * May return 0L, if there is no current event selected.
         */
        Event * currentEvent();
        virtual void showEvent( QShowEvent * );
        virtual void enableAll( int what, bool enable );

        virtual bool eventFilter ( QObject * watched, QEvent * event ); 

        void reload( bool revertToDefaults = false );

    protected slots:
        void playSound();

    private slots:
        void slotItemClicked( Q3ListViewItem *item, const QPoint& point, 
                              int col );
        void slotEventChanged( Q3ListViewItem * );
        void soundToggled( bool on );
        void loggingToggled( bool on );
        void executeToggled( bool on );
        void messageBoxChanged();
        void stderrToggled( bool on );
        void taskbarToggled( bool on );

        void soundFileChanged( const QString& text );
        void logfileChanged( const QString& text );
        void commandlineChanged( const QString& text );

        void openSoundDialog( KURLRequester * );
        void openLogDialog( KURLRequester * );
        void openExecDialog( KURLRequester * );

        void enableAll();

    private:
        void updateWidgets( ListViewItem *item );
        void updatePixmaps( ListViewItem *item );

        static QString makeRelative( const QString& );
        void addToView( const EventList& events );
        void widgetChanged( Q3ListViewItem *item,
                            int what, bool on, QWidget *buddy = 0L );
        void selectItem( Q3ListViewItem *item );

        ApplicationList m_visibleApps;
        ApplicationList m_allApps;

        class Private;
        Private *d;

    };


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


    /**
     * @internal
     */
    class Event
    {
        friend class Application;

    public:
        QString text() const { return description; }

        int presentation;
        int dontShow;
        QString logfile;
        QString soundfile;
        QString commandline;

        const Application *application() const { return m_app; }

    private:
        Event( const Application *app ) {
            presentation = 0;
            dontShow = 0;
            m_app = app;
        }
        QString name;
        QString description;
        QString configGroup;

        const Application *m_app;
    };

    /**
     * @internal
     */
    class ListViewItem : public Q3ListViewItem
    {
    public:
        ListViewItem( Q3ListView *view, Event *event );

        Event& event() { return *m_event; }
        virtual int compare (Q3ListViewItem * i, int col, bool ascending) const;

    private:
        Event * m_event;
    };

}


#endif
