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

#include <dcopclient.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kaudioplayer.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <knotifydialog.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qptrlist.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qvbox.h>

using namespace KNotify;

//
// I don't feel like subclassing KComboBox and find ways to insert that into
// the .ui file...
//
namespace KNotify
{
    class SelectionCombo
    {
    public:
        //
        // Mind the order in fill() and type()
        //
        static void fill( KComboBox *combo )
        {
            combo->insertItem( i18n("Sounds") );
            combo->insertItem( i18n("Logging") );
            combo->insertItem( i18n("Program Execution") );
            combo->insertItem( i18n("Message Windows") );
            combo->insertItem( i18n("Passive Windows") );
            combo->insertItem( i18n("Standard Error Output") );
        }

        static int type( KComboBox *combo )
        {
            switch( combo->currentItem() )
            {
                case 0:
                    return KNotifyClient::Sound;
                case 1:
                    return KNotifyClient::Logfile;
                case 2:
                    return KNotifyClient::Execute;
                case 3:
                    return KNotifyClient::Messagebox;
                case 4:
                    return KNotifyClient::PassivePopup;
                case 5:
                    return KNotifyClient::Stderr;
            }

            return KNotifyClient::None;
        }
    };
};


int KNotifyDialog::configure( QWidget *parent, const char *name,
                              const KAboutData *aboutData )
{
    KNotifyDialog dialog( parent, name, true, aboutData );
    return dialog.exec();
}

KNotifyDialog::KNotifyDialog( QWidget *parent, const char *name, bool modal,
                              const KAboutData *aboutData )
    : KDialogBase(parent, name, modal, i18n("Notification Settings"),
                  Ok | Apply | Cancel | Default, Ok, true )
{
    QVBox *box = makeVBoxMainWidget();

    m_notifyWidget = new KNotifyWidget( box, "knotify widget" );

    if ( aboutData )
        addApplicationEvents( aboutData->appName() );

    connect( this, SIGNAL( okClicked() ), m_notifyWidget, SLOT( save() ));
    connect( this, SIGNAL( applyClicked() ), m_notifyWidget, SLOT( save() ));
};

KNotifyDialog::~KNotifyDialog()
{
}

void KNotifyDialog::addApplicationEvents( const char *appName, bool show )
{
    addApplicationEvents( QString::fromUtf8( appName ) +
                          QString::fromLatin1( "/eventsrc" ), show );
}

void KNotifyDialog::addApplicationEvents( const QString& path, bool show )
{
    Application *app = m_notifyWidget->addApplicationEvents( path );
    if ( app && show )
    {
        m_notifyWidget->setCurrentApplication( app );
        m_notifyWidget->sort();
    }
}

void KNotifyDialog::clearApplicationEvents()
{
    m_notifyWidget->clear();
}

void KNotifyDialog::slotDefault()
{
    if (KMessageBox::warningContinueCancel(this,
        i18n("This will cause the notifications "
             "to be reset to their defaults!"), i18n("Are you sure?"), i18n("Continue"))
        != KMessageBox::Continue)
        return;

    m_notifyWidget->reload( true ); // defaults
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


#define COL_EXECUTE 0
#define COL_STDERR  1
#define COL_MESSAGE 2
#define COL_LOGFILE 3
#define COL_SOUND   4
#define COL_EVENT   5

// simple access to all knotify-handled applications
KNotifyWidget::KNotifyWidget( QWidget *parent, const char *name,
                              bool handleAllApps )
    : KNotifyWidgetBase( parent, name ? name : "KNotifyWidget" )
{
    m_apps.setAutoDelete( true );

    layout()->setMargin( 0 );
    layout()->setSpacing( KDialogBase::spacingHint() );

    if ( !handleAllApps )
    {
        m_affectAllApps->hide();
        m_playerButton->hide();
    }

    SelectionCombo::fill( m_comboEnable );
    SelectionCombo::fill( m_comboDisable );

    m_listview->setFullWidth( true );
    m_listview->setAllColumnsShowFocus( true );

    QPixmap pexec = SmallIcon("exec");
    QPixmap psound = SmallIcon("sound");
    QPixmap plogfile = SmallIcon("log");
    QPixmap pmessage = SmallIcon("info");
    QPixmap pstderr = SmallIcon("terminal");

    int w = KIcon::SizeSmall + 6;
    
    QHeader *header = m_listview->header();
    header->setLabel( COL_EXECUTE, pexec,    QString::null, w );
    header->setLabel( COL_STDERR,  pstderr,  QString::null, w );
    header->setLabel( COL_MESSAGE, pmessage, QString::null, w );
    header->setLabel( COL_LOGFILE, plogfile, QString::null, w );
    header->setLabel( COL_SOUND,   psound,   QString::null, w );
    
    m_playButton->setPixmap( SmallIcon( "1rightarrow" ) );
    connect( m_playButton, SIGNAL( clicked() ), SLOT( playSound() ));

    connect( m_listview, SIGNAL( currentChanged( QListViewItem * ) ),
             SLOT( slotEventChanged( QListViewItem * ) ));

    connect( m_playSound, SIGNAL( toggled( bool )),
             SLOT( soundToggled( bool )) );
    connect( m_logToFile, SIGNAL( toggled( bool )),
             SLOT( loggingToggled( bool )) );
    connect( m_execute, SIGNAL( toggled( bool )),
             SLOT( executeToggled( bool )) );
    connect( m_messageBox, SIGNAL( toggled( bool )),
             SLOT( messageBoxChanged() ) );
    connect( m_passivePopup, SIGNAL( toggled( bool )),
             SLOT( messageBoxChanged() ) );
    connect( m_stderr, SIGNAL( toggled( bool )),
             SLOT( stderrToggled( bool ) ) );

    connect( m_soundPath, SIGNAL( textChanged( const QString& )),
             SLOT( soundFileChanged( const QString& )));
    connect( m_logfilePath, SIGNAL( textChanged( const QString& )),
             SLOT( logfileChanged( const QString& ) ));
    connect( m_executePath, SIGNAL( textChanged( const QString& )),
             SLOT( commandlineChanged( const QString& ) ));

    connect( m_soundPath, SIGNAL( openFileDialog( KURLRequester * )),
             SLOT( openSoundDialog( KURLRequester * )));
    connect( m_logfilePath, SIGNAL( openFileDialog( KURLRequester * )),
             SLOT( openLogDialog( KURLRequester * )));
    connect( m_executePath, SIGNAL( openFileDialog( KURLRequester * )),
             SLOT( openExecDialog( KURLRequester * )));

    connect( m_extension, SIGNAL( clicked() ),
             SLOT( toggleAdvanced()) );

    connect( m_buttonEnable, SIGNAL( clicked() ), SLOT( enableAll() ));
    connect( m_buttonDisable, SIGNAL( clicked() ), SLOT( enableAll() ));

    showAdvanced( false );
}

KNotifyWidget::~KNotifyWidget()
{

}

void KNotifyWidget::toggleAdvanced()
{
    showAdvanced( m_logToFile->isHidden() );
}

void KNotifyWidget::showAdvanced( bool show )
{
    if ( show )
    {
        m_extension->setText( i18n("Fewer Op&tions") );
        QToolTip::add( m_extension, i18n("Hide advanced options") );

        m_logToFile->show();
        m_logfilePath->show();
        m_execute->show();
        m_executePath->show();
        m_messageBox->show();
        m_passivePopup->show();
        m_stderr->show();
        m_controlsBox->show();

        m_actionsBoxLayout->setSpacing( KDialog::spacingHint() );
    }
    else
    {
        m_extension->setText( i18n("More Op&tions") );
        QToolTip::add( m_extension, i18n("Show advanced options") );

        m_logToFile->hide();
        m_logfilePath->hide();
        m_execute->hide();
        m_executePath->hide();
        m_messageBox->hide();
        m_passivePopup->hide();
        m_stderr->hide();
        m_controlsBox->hide();

        m_actionsBoxLayout->setSpacing( 0 );
    }
}

Application * KNotifyWidget::addApplicationEvents( const QString& path )
{
    kdDebug() << "**** knotify: adding path: " << path << endl;

    QString relativePath = path;

    if ( path.at(0) == '/' && KStandardDirs::exists( path ) )
        relativePath = makeRelative( path );

    if ( !relativePath.isEmpty() )
    {
        Application *app = new Application( relativePath );
        m_apps.append( app );

        return app;
    }

    return 0L;
}

void KNotifyWidget::clear()
{
    m_apps.clear();
    m_listview->clear();
}

void KNotifyWidget::showEvent( QShowEvent *e )
{
    selectItem( m_listview->firstChild() );
    KNotifyWidgetBase::showEvent( e );
}

void KNotifyWidget::slotEventChanged( QListViewItem *item )
{
    bool on = (item != 0L);
    
    m_actionsBox->setEnabled( on );
    m_controlsBox->setEnabled( on );

    if ( !on )
        return;

    ListViewItem *lit = static_cast<ListViewItem*>( item );
    updateWidgets( lit );
}

void KNotifyWidget::updateWidgets( ListViewItem *item )
{
    bool enable;
    bool checked;

    blockSignals( true ); // don't emit changed() signals

    const Event& event = item->event();
    
    // sound settings
    m_playButton->setEnabled( !event.soundfile.isEmpty() );
    m_soundPath->setURL( event.soundfile );
    enable = (event.dontShow & KNotifyClient::Sound) == 0;
    checked = enable && !event.soundfile.isEmpty() &&
              (event.presentation & KNotifyClient::Sound);
    m_playSound->setEnabled( enable );
    m_playSound->setChecked( checked );
    m_soundPath->setEnabled( checked );


    // logfile settings
    m_logfilePath->setURL( event.logfile );
    enable = (event.dontShow & KNotifyClient::Logfile) == 0;
    checked = enable && !event.logfile.isEmpty()  &&
              (event.presentation & KNotifyClient::Logfile);
    m_logToFile->setEnabled( enable );
    m_logToFile->setChecked( checked );
    m_logfilePath->setEnabled( checked );


    // execute program settings
    m_executePath->setURL( event.commandline );
    enable = (event.dontShow & KNotifyClient::Execute) == 0;
    checked = enable && !event.commandline.isEmpty() &&
              (event.presentation & KNotifyClient::Execute);
    m_execute->setEnabled( enable );
    m_execute->setChecked( checked );
    m_executePath->setEnabled( checked );


    // other settings
    m_messageBox->setChecked(event.presentation & KNotifyClient::Messagebox);
    m_passivePopup->setChecked(event.presentation & KNotifyClient::PassivePopup);
    m_stderr->setChecked( event.presentation & KNotifyClient::Stderr );

    updatePixmaps( item );
    
    blockSignals( false );
}

void KNotifyWidget::updatePixmaps( ListViewItem *item )
{
    Event &event = item->event();
        
    if ( event.presentation & KNotifyClient::Execute )
        item->setPixmap( COL_EXECUTE, SmallIcon("exec") );
    if ( event.presentation & KNotifyClient::Sound )
        item->setPixmap( COL_SOUND, SmallIcon("sound") );
    if ( event.presentation & KNotifyClient::Logfile )
        item->setPixmap( COL_LOGFILE, SmallIcon("log") );
    if ( event.presentation & KNotifyClient::Messagebox )
        item->setPixmap( COL_MESSAGE, SmallIcon("info") );
    if ( event.presentation & KNotifyClient::Stderr )
        item->setPixmap( COL_STDERR, SmallIcon("terminal") );
}

void KNotifyWidget::setCurrentApplication( Application *app )
{
    if ( !app )
        return;

    m_listview->clear();
    addToView( app->eventList() );
}

void KNotifyWidget::addToView( const EventList& events )
{
    ListViewItem *item = 0L;

    EventListIterator it( events );

    QPixmap pexec = SmallIcon("exec");
    QPixmap psound = SmallIcon("sound");
    QPixmap plogfile = SmallIcon("log");
    QPixmap pmessage = SmallIcon("info");
    QPixmap pstderr = SmallIcon("terminal");

    for ( ; it.current(); ++it )
    {
        Event *event = it.current();
        item = new ListViewItem( m_listview, event );
        
        if ( event->presentation & KNotifyClient::Execute )
            item->setPixmap( COL_EXECUTE, pexec );
        if ( event->presentation & KNotifyClient::Sound )
            item->setPixmap( COL_SOUND, psound );
        if ( event->presentation & KNotifyClient::Logfile )
            item->setPixmap( COL_LOGFILE, plogfile );
        if ( event->presentation & KNotifyClient::Messagebox )
            item->setPixmap( COL_MESSAGE, pmessage );
        if ( event->presentation & KNotifyClient::Stderr )
            item->setPixmap( COL_STDERR, pstderr );
    }
}

void KNotifyWidget::widgetChanged( QListViewItem *item, 
                                   int what, bool on, QWidget *buddy )
{
    if ( signalsBlocked() )
        return;

    if ( buddy )
        buddy->setEnabled( on );

    Event &e = static_cast<ListViewItem*>( item )->event();
    if ( on )
    {
        e.presentation |= what;
        if ( buddy )
            buddy->setFocus();
    }
    else
        e.presentation &= ~what;

    emit changed( true );
}

void KNotifyWidget::soundToggled( bool on )
{
    QListViewItem *item = m_listview->currentItem();
    if ( !item )
        return;
    item->setPixmap( COL_SOUND, on ? SmallIcon("sound") : QPixmap() );
    widgetChanged( item, KNotifyClient::Sound, on, m_soundPath );
}

void KNotifyWidget::loggingToggled( bool on )
{
    QListViewItem *item = m_listview->currentItem();
    if ( !item )
        return;
    item->setPixmap( COL_LOGFILE, on ? SmallIcon("log") : QPixmap() );
    widgetChanged( item, KNotifyClient::Logfile, on, m_logfilePath );
}

void KNotifyWidget::executeToggled( bool on )
{
    QListViewItem *item = m_listview->currentItem();
    if ( !item )
        return;
    item->setPixmap( COL_EXECUTE, on ? SmallIcon("exec") : QPixmap() );
    widgetChanged( item, KNotifyClient::Execute, on, m_executePath );
}

void KNotifyWidget::messageBoxChanged()
{
    if ( signalsBlocked() )
        return;

    m_passivePopup->setEnabled( m_messageBox->isChecked() );

    QListViewItem *item = m_listview->currentItem();
    if ( !item )
        return;

    bool on = m_passivePopup->isEnabled();
    item->setPixmap( COL_MESSAGE, on ? SmallIcon("info") : QPixmap() );
    
    Event &e = static_cast<ListViewItem*>( item )->event();

    if ( m_messageBox->isChecked() ) {
	if ( m_passivePopup->isChecked() ) {
	    e.presentation |= KNotifyClient::PassivePopup;
	    e.presentation &= ~KNotifyClient::Messagebox;
	}
	else {
	    e.presentation &= ~KNotifyClient::PassivePopup;
	    e.presentation |= KNotifyClient::Messagebox;
	}
    }
    else {
        e.presentation &= ~KNotifyClient::Messagebox;
        e.presentation &= ~KNotifyClient::PassivePopup;
    }

    emit changed( true );
}

void KNotifyWidget::stderrToggled( bool on )
{
    QListViewItem *item = m_listview->currentItem();
    if ( !item )
        return;
    item->setPixmap( COL_STDERR, on ? SmallIcon("terminal") : QPixmap() );
    widgetChanged( item, KNotifyClient::Stderr, on );
}

void KNotifyWidget::soundFileChanged( const QString& text )
{
    if ( signalsBlocked() )
        return;

    m_playButton->setEnabled( !text.isEmpty() );
    currentEvent()->soundfile = text;
    emit changed( true );
}

void KNotifyWidget::logfileChanged( const QString& text )
{
    if ( signalsBlocked() )
        return;

    currentEvent()->logfile = text;
    emit changed( true );
}

void KNotifyWidget::commandlineChanged( const QString& text )
{
    if ( signalsBlocked() )
        return;

    currentEvent()->commandline = text;
    emit changed( true );
}

void KNotifyWidget::sort( bool ascending )
{
    m_listview->setSorting( COL_EVENT, ascending );
    m_listview->sort();
}

void KNotifyWidget::selectItem( QListViewItem *item )
{
    if ( item )
    {
        m_listview->setCurrentItem( item );
        item->setSelected( true );
        slotEventChanged( item );
    }
}

void KNotifyWidget::reload( bool revertToDefaults )
{
    m_listview->clear();
    ApplicationListIterator it( m_apps );
    for ( ; it.current(); ++it )
    {
        it.current()->reloadEvents( revertToDefaults );
        addToView( it.current()->eventList() );
    }

    m_listview->sort();
    selectItem( m_listview->firstChild()  );
}

void KNotifyWidget::save()
{
    kdDebug() << "save\n";

    ApplicationListIterator it( m_apps );
    while ( it.current() ) {
        (*it)->save();
        ++it;
    }

    if ( kapp )
    {
        if ( !kapp->dcopClient()->isAttached() )
            kapp->dcopClient()->attach();
        kapp->dcopClient()->send("knotify", "", "reconfigure()", "");
    }

    emit changed( false );
}

// returns e.g. "kwin/eventsrc" from a given path
// "/opt/kde3/share/apps/kwin/eventsrc"
QString KNotifyWidget::makeRelative( const QString& fullPath )
{
    int slash = fullPath.findRev( '/' ) - 1;
    slash = fullPath.findRev( '/', slash );

    if ( slash < 0 )
        return QString::null;

    return fullPath.mid( slash+1 );
}

Event * KNotifyWidget::currentEvent()
{
    QListViewItem *current = m_listview->currentItem();
    if ( !current )
        return 0L;

    return &static_cast<ListViewItem*>( current )->event();
}

void KNotifyWidget::openSoundDialog( KURLRequester *requester )
{
    static bool init = true;
    if ( !init )
        return;

    init = false;

    KFileDialog *fileDialog = requester->fileDialog();
    fileDialog->setCaption( i18n("Select a Sound File") );
    QStringList filters;
    filters << "audio/x-wav" << "audio/x-mp3" << "application/x-ogg"
            << "audio/x-adpcm";
    fileDialog->setMimeFilter( filters );

    // find the first "sound"-resource that contains files
    const Application *app = currentEvent()->application();
    QStringList soundDirs =
        KGlobal::dirs()->findDirs("data", app->appName() + "/sounds");
    soundDirs += KGlobal::dirs()->resourceDirs( "sound" );

    if ( !soundDirs.isEmpty() ) {
        KURL soundURL;
        QDir dir;
        dir.setFilter( QDir::Files | QDir::Readable );
        QStringList::ConstIterator it = soundDirs.begin();
        while ( it != soundDirs.end() ) {
            dir = *it;
            if ( dir.isReadable() && dir.count() > 2 ) {
                soundURL.setPath( *it );
                fileDialog->setURL( soundURL );
                break;
            }
            ++it;
        }
    }
}

void KNotifyWidget::openLogDialog( KURLRequester *requester )
{
    static bool init = true;
    if ( !init )
        return;

    init = false;

    KFileDialog *fileDialog = requester->fileDialog();
    fileDialog->setCaption( i18n("Select a Log File") );
    QStringList filters;
    filters << "text/x-log" << "text/plain";
    fileDialog->setMimeFilter( filters );
}

void KNotifyWidget::openExecDialog( KURLRequester *requester )
{
    static bool init = true;
    if ( !init )
        return;

    init = false;

    KFileDialog *fileDialog = requester->fileDialog();
    fileDialog->setCaption( i18n("Select a File to Execute") );
    QStringList filters;
    filters << "application/x-executable" << "application/x-shellscript"
            << "application/x-perl" << "application/x-python";
    fileDialog->setMimeFilter( filters );
}

void KNotifyWidget::playSound()
{
    KAudioPlayer::play( m_soundPath->url() );
}

void KNotifyWidget::enableAll()
{
    bool enable = (sender() == m_buttonEnable);
    enableAll( SelectionCombo::type(enable ? m_comboEnable : m_comboDisable),
               enable );
}

void KNotifyWidget::enableAll( int what, bool enable )
{
    ApplicationListIterator appIt( m_apps );
    for ( ; appIt.current(); ++appIt )
    {
        const EventList& events = appIt.current()->eventList();
        EventListIterator it( events );
        for ( ; it.current(); ++it )
        {
            if ( enable )
                it.current()->presentation |= what;
            else
                it.current()->presentation &= ~what;
        }
    }

    QListViewItem *item = m_listview->currentItem();
    if ( !item )
        item = m_listview->firstChild();
    selectItem( item );
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


//
// path must be "appname/eventsrc", i.e. a relative path
//
Application::Application( const QString &path )
{
    QString config_file = path;
    config_file[config_file.find('/')] = '.';
    m_events = 0L;
    config = new KConfig(config_file, false, false);
    kc = new KConfig(path, true, false, "data");
    kc->setGroup( QString::fromLatin1("!Global!") );
    m_icon = kc->readEntry(QString::fromLatin1("IconName"),
                           QString::fromLatin1("misc"));
    m_description = kc->readEntry( QString::fromLatin1("Comment"),
                                   i18n("No description available") );

    int index = path.find( '/' );
    if ( index >= 0 )
        m_appname = path.left( index );
    else
        kdDebug() << "Cannot determine application name from path: " << path << endl;
}

Application::~Application()
{
    delete config;
    delete kc;
    delete m_events;
}


const EventList&  Application::eventList()
{
    if ( !m_events ) {
        m_events = new EventList;
        m_events->setAutoDelete( true );
        reloadEvents();
    }

    return *m_events;
}


void Application::save()
{
    if ( !m_events )
        return;

    EventListIterator it( *m_events );
    Event *e;
    while ( (e = it.current()) ) {
        config->setGroup( e->configGroup );
        config->writeEntry( "presentation", e->presentation );
        config->writeEntry( "soundfile", e->soundfile );
        config->writeEntry( "logfile", e->logfile );
        config->writeEntry( "commandline", e->commandline );

        ++it;
    }
    config->sync();
}


void Application::reloadEvents( bool revertToDefaults )
{
    if ( m_events )
        m_events->clear();

    Event *e = 0L;

    QString global = QString::fromLatin1("!Global!");
    QString default_group = QString::fromLatin1("<default>");
    QString name = QString::fromLatin1("Name");
    QString comment = QString::fromLatin1("Comment");

    QStringList conflist = kc->groupList();
    QStringList::ConstIterator it = conflist.begin();

    while ( it != conflist.end() ) {
        if ( (*it) != global && (*it) != default_group ) { // event group
            kc->setGroup( *it );

            e = new Event( this );
            e->name = kc->readEntry( name );
            e->description = kc->readEntry( comment );
            e->configGroup = *it;

            if ( e->name.isEmpty() || e->description.isEmpty() )
                delete e;

            else { // load the event
                int default_rep = kc->readNumEntry("default_presentation", 0);
                QString default_logfile = kc->readEntry("default_logfile");
                QString default_soundfile = kc->readEntry("default_sound");
                QString default_commandline = kc->readEntry("default_commandline");
                config->setGroup(*it);
                e->dontShow = config->readNumEntry("nopresentation", 0 );

                if ( revertToDefaults )
                {
                    e->presentation = default_rep;
                    e->logfile = default_logfile;
                    e->soundfile = default_soundfile;
                    e->commandline = default_commandline;
                }

                else
                {
                    e->presentation = config->readNumEntry("presentation",
                                                           default_rep);
                    e->logfile = config->readEntry("logfile",
                                                   default_logfile);
                    e->soundfile = config->readEntry("soundfile",
                                                     default_soundfile);
                    e->commandline = config->readEntry("commandline",
                                                       default_commandline);
                }

                m_events->append( e );
            }
        }

        ++it;
    }

    return;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

ListViewItem::ListViewItem( QListView *view, Event *event )
    : QListViewItem( view ),
      m_event( event ) 
{
    setText( COL_EVENT, event->text() );
}

int ListViewItem::compare ( QListViewItem * i, int col, bool ascending ) const
{
    ListViewItem *item = static_cast<ListViewItem*>( i );
    int myPres = m_event->presentation;
    int otherPres = item->event().presentation;
    
    int action = 0;
    
    switch ( col )
    {
        case COL_EVENT: // use default sorting
            return QListViewItem::compare( i, col, ascending );

        case COL_EXECUTE:
            action = KNotifyClient::Execute;
            break;
        case COL_LOGFILE:
            action = KNotifyClient::Logfile;
            break;
        case COL_MESSAGE:
            action = (KNotifyClient::Messagebox | KNotifyClient::PassivePopup);
            break;
        case COL_SOUND:
            action = KNotifyClient::Sound;
            break;
        case COL_STDERR:
            action = KNotifyClient::Stderr;
            break;
    }
    
    if ( (myPres & action) == (otherPres & action) )
    {
        // default sorting by event
        return QListViewItem::compare( i, COL_EVENT, true );
    }
    
    if ( myPres & action )
        return -1;
    if ( otherPres & action )
        return 1;
    
    return 0;
}

#include "knotifydialog.moc"
