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
  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef KNOTIFYDIALOG_H
#define KNOTIFYDIALOG_H

#include <klistview.h>

#include <kdialogbase.h>
#include <kinstance.h>
#include <kglobal.h>

#include <knotifywidgetbase.h>

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
 */
class KNotifyDialog : public KDialogBase
{
    Q_OBJECT

public:
    /**
     * Set aboutData to 0L, if you want to add all events yourself with
     * @ref addApplicationEvents().
     */
    KNotifyDialog( QWidget *parent = 0, const char *name = 0,
                   bool modal = true,
                   const KAboutData *aboutData =
                   KGlobal::instance()->aboutData() );
    virtual ~KNotifyDialog();

    /**
     * Execute a KNotifyDialog.
     *
     * @see exec for the return values.
     */
    static int configure( QWidget *parent = 0, const char *name = 0,
                          const KAboutData *aboutData =
                          KGlobal::instance()->aboutData() );

    virtual void addApplicationEvents( const char *appName, bool show = true );
    virtual void addApplicationEvents( const QString& path, bool show = true );
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
    typedef QPtrList<Application> ApplicationList;
    class Event;
    typedef QPtrList<Event> EventList;
    typedef QPtrListIterator<Application> ApplicationListIterator;
    typedef QPtrListIterator<Event> EventListIterator;


    /**
     * @internal
     */
    class KNotifyWidget : public KNotifyWidgetBase
    {
        Q_OBJECT

    public:
        KNotifyWidget( QWidget* parent = 0, const char* name = 0,
                       bool handleAllApps = false );
        ~KNotifyWidget();

        KListView * eventsView() {
            return m_listview;
        }

        ApplicationList& apps() { return m_apps; }
        /**
         * Returns 0L if no application events could be found
         */
        Application * addApplicationEvents( const QString& path );

        void reload( bool revertToDefaults = false );

        void sort( bool ascending = true );
        void setCurrentApplication( Application *app );

    public slots:
        virtual void clear();
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

    protected slots:
        void playSound();

    private slots:
        void slotEventChanged( QListViewItem * );
        void soundToggled( bool on );
        void loggingToggled( bool on );
        void executeToggled( bool on );
        void messageBoxChanged();
        void stderrToggled( bool on );

        void soundFileChanged( const QString& text );
        void logfileChanged( const QString& text );
        void commandlineChanged( const QString& text );

        void openSoundDialog( KURLRequester * );
        void openLogDialog( KURLRequester * );
        void openExecDialog( KURLRequester * );

        void updateWidgets( const Event& event );

        void enableAll();

    private:
        QString makeRelative( const QString& );
        void addToView( const EventList& events );
        void widgetChanged( int what, bool on, QWidget *buddy = 0L );
        void selectItem( QListViewItem *item );

        ApplicationList m_apps;

    };


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


    /**
     * @internal
     */
    class Application
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
    class ListViewItem : public QListViewItem
    {
    public:
        ListViewItem( QListView *view, Event *event )
            : QListViewItem( view, event->text() ),
              m_event( event ) {}

    Event& event() { return *m_event; }

    private:
        Event * m_event;
    };

};


#endif
