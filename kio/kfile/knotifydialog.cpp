/*
	Copyright (C) 2000 Carsten Pfeiffer <pfeiffer@kde.org>
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
#include <kconfig.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <knotifydialog.h>
#include "knotifydialog_private.h"
#include <kstandarddirs.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qstring.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qvgroupbox.h>

int KNotifyDialog::configure(QWidget *parent, const char *name)
{
	KNotifyDialog dialog(parent, name);
	return dialog.exec();
}

KNotifyDialog::KNotifyDialog(QWidget *parent, const char *name)
	: KDialogBase(parent, name, true, i18n("Notification Settings"),
	              Ok | Apply | Cancel | Default, Ok, true)
	, m_application(0)
{
	resize(600, 400);

	updating = true;
	currentItem = 0L;

	QFrame *box = makeMainWidget();
	QVBoxLayout *lay = new QVBoxLayout( box, 0, KDialog::spacingHint() );
	lay->setAutoAdd(true);

	view = new QListView( box );
	view->addColumn(i18n("Events"));
	view->addColumn(i18n("Filename"));
	view->setSelectionMode( QListView::Single );
	view->setRootIsDecorated( true );
	view->setSorting( -1 );

	QHBox *hbox = new QHBox( box );
	hbox->setSpacing( KDialog::spacingHint() );
	lblFilename = new QLabel( i18n("&Filename: "), hbox );
	requester = new KURLRequester( hbox );
	lblFilename->setBuddy( requester );
	connect( requester, SIGNAL( openFileDialog( KURLRequester * )),
	         SLOT( slotRequesterClicked( KURLRequester * )));

	playButton = new QPushButton( hbox );
	playButton->setFixedSize( requester->button()->size() );
	playButton->setPixmap( SmallIcon("player_play") );
	QToolTip::add( playButton, i18n("Play the given sound") );
	playButton->hide();

	connect( playButton, SIGNAL( clicked() ), SLOT( playSound() ));
	connect(requester, SIGNAL( textChanged( const QString& )),
	        SLOT( slotFileChanged( const QString& )) );
	connect( view, SIGNAL( currentChanged( QListViewItem * )),
	         SLOT( slotItemActivated( QListViewItem * )));

	hbox = new QHBox( box );
	hbox->setSpacing( KDialog::spacingHint() );
	cbExternal = new QCheckBox( i18n("Use e&xternal player: "), hbox );
	reqExternal = new KURLRequester( hbox );
	reqExternal->completionObject()->setMode( KURLCompletion::ExeCompletion );
	connect( cbExternal, SIGNAL( toggled( bool )),
             SLOT( externalClicked( bool )));

	hbox = new QHBox( box );
	hbox->setSpacing( KDialog::spacingHint() );
	QLabel *l = new QLabel( i18n( "&Volume: " ), hbox );
	volumeSlider = new QSlider( hbox );
	volumeSlider->setOrientation( Horizontal );
	volumeSlider->setRange( 0, 100 );
	l->setBuddy( volumeSlider );

	soundButton = new QPushButton( box );
	// this is configured in updateView

	qApp->processEvents(); // let's show up

	// reading can take some time
	QTimer::singleShot( 0, this, SLOT( load() ));
	updating = false;
};

KNotifyDialog::~KNotifyDialog()
{
	delete m_application;
}

/**
 * Clears the view and iterates over all apps, creating listview-items
 */
void KNotifyDialog::updateView()
{
	bool save_updating = updating;
	updating = true;
	view->clear();
	bool soundsDisabled = true;

	QPixmap icon = SmallIcon("idea");

	for(KNEventListIterator it( *m_application->eventList() ); it.current(); ++it)
	{
		if(it.current()->presentation & KNotifyClient::Sound) soundsDisabled = false;

		KNListViewItem *eItem = new KNListViewItem( view, it.current() );
		eItem->setPixmap( 0, icon );
	}

	updating = save_updating;

	soundButton->disconnect(this);
	if(soundsDisabled)
	{
		soundButton->setText( i18n("&Enable All Sounds") );
		connect(soundButton, SIGNAL(clicked()), this, SLOT(enableAllSounds()));
	}
	else
	{
		soundButton->setText( i18n("&Disable All Sounds") );
		connect(soundButton, SIGNAL(clicked()), this, SLOT(disableAllSounds()));
	}
}

/**
 * Someone typing in the url-requester -> update the listview item and its
 * event.
 */
void KNotifyDialog::slotFileChanged( const QString& text )
{
	playButton->setEnabled( !text.isEmpty() );

	if ( !currentItem )
		return;

	KNEvent *event = currentItem->event;
	QString *itemText = 0L;

	if ( currentItem->eventType() == KNotifyClient::Sound )
		itemText = &(event->soundfile);
	else if ( currentItem->eventType() == KNotifyClient::Logfile )
		itemText = &(event->logfile);

	if ( itemText && *itemText != text )
	{
		*itemText = text;
	}

	currentItem->setText( COL_FILENAME, text );
}

void KNotifyDialog::playSound()
{
	KAudioPlayer::play( requester->url() );
}

namespace
{
// returns e.g. "kwin/eventsrc" from a given path
// "/opt/kde2/share/apps/kwin/eventsrc"
QString makeRelative( const QString& fullPath )
{
	int slash = fullPath.findRev( '/' ) - 1;
	slash = fullPath.findRev( '/', slash );

	if ( slash < 0 )
		return QString::null;

	return fullPath.mid( slash+1 );
}
}

void KNotifyDialog::load()
{
	bool save_updating = updating;
	updating = true;

	setEnabled( false );
	setCursor( KCursor::waitCursor() );
	currentItem = 0L;

	KConfig *kc = new KConfig( "knotifyrc", true, false );
	kc->setGroup( "Misc" );
	cbExternal->setChecked( kc->readBoolEntry( "Use external player", false ));
	reqExternal->setURL( kc->readEntry( "External player" ));
	reqExternal->setEnabled( cbExternal->isChecked() );
	volumeSlider->setValue( kc->readNumEntry( "Volume", 100 ) );
	static_cast<QHBox *>( volumeSlider->parent() )->setEnabled( !cbExternal->isChecked() );
	delete kc;

	requester->clear();
	requester->setEnabled( false );
	lblFilename->setEnabled( false );
	playButton->hide();

	view->clear();

	// find and load events for this application
	delete m_application;
	QString path = KGlobal::dirs()->findResource("appdata", "eventsrc");
	m_application = new KNApplication(makeRelative(path));
	if(!m_application->isValid())
	{
		KMessageBox::sorry(this, i18n("This application has no notifications."));
		done(Accepted);
	}

	updateView();
	setEnabled( true );
	setCursor( KCursor::arrowCursor() );

	updating = save_updating;
}

void KNotifyDialog::slotApply()
{
	// see kdelibs/arts/knotify/knotify.cpp
	KConfig *kc = new KConfig( "knotifyrc", false, false );
	kc->setGroup( "Misc" );
	kc->writeEntry( "External player", reqExternal->url() );
	kc->writeEntry( "Use external player", cbExternal->isChecked() );
	kc->writeEntry( "Volume", volumeSlider->value() );
	kc->sync();
	delete kc;

	m_application->save();
	if ( !kapp->dcopClient()->isAttached() )
		kapp->dcopClient()->attach();
	kapp->dcopClient()->send("knotify", "", "reconfigure()", "");
}

void KNotifyDialog::slotDefault()
{
	if (KMessageBox::warningContinueCancel(this,
		i18n("This will cause all notifications "
			 "to be reset to their defaults!"), i18n("Are you sure?"), i18n("Continue"))
		!= KMessageBox::Continue)
		return;

	QString path = KGlobal::dirs()->saveLocation("appdata", "./");
	path += QString::fromLatin1("eventsrc");
	QFile file(path);
	file.remove();

	load();
}

void KNotifyDialog::slotItemActivated( QListViewItem *i )
{
	bool enableButton = false;
	currentItem = dynamic_cast<KNCheckListItem *>( i );
	if ( currentItem )
	{
		const KNEvent *event = currentItem->event;

		if ( currentItem->eventType() == KNotifyClient::Sound )
		{
			requester->setURL( event->soundfile );
			enableButton = true;
			playButton->show();
			playButton->setEnabled( !event->soundfile.isEmpty() );
		}
		else if ( currentItem->eventType() == KNotifyClient::Logfile )
		{
			requester->setURL( event->logfile );
			enableButton = true;
			playButton->hide();
		}
		else
		{
			requester->lineEdit()->clear();
			playButton->hide();
		}
	}
	else
	{
		requester->lineEdit()->clear();
		playButton->hide();
	}

	requester->setEnabled( enableButton );
	lblFilename->setEnabled( enableButton );
}

void KNotifyDialog::externalClicked( bool on )
{
	if ( on )
		reqExternal->setFocus();
	reqExternal->setEnabled( on );
	static_cast<QHBox *>( volumeSlider->parent() )->setEnabled( !on );
}

void KNotifyDialog::slotRequesterClicked( KURLRequester *requester )
{
	static bool init = true;
	if ( !init )
		return;

	init = false;

	// find the first "sound"-resource that contains files
	QStringList soundDirs = KGlobal::dirs()->resourceDirs( "sound" );
	if ( !soundDirs.isEmpty() )
	{
		KURL soundURL;
		QDir dir;
		dir.setFilter( QDir::Files | QDir::Readable );
		QStringList::Iterator it = soundDirs.begin();
		while ( it != soundDirs.end() )
		{
			dir = *it;
			if ( dir.isReadable() && dir.count() > 2 )
			{
				soundURL.setPath( *it );
				requester->fileDialog()->setURL( soundURL );
				break;
			}
			++it;
		}
	}
}

void KNotifyDialog::disableAllSounds()
{
	for(KNEventListIterator event(*m_application->eventList()); event.current(); ++event)
	{
		(*event)->presentation &= ~KNotifyClient::Sound;
	}
	updateView();
}

void KNotifyDialog::enableAllSounds()
{
	for(KNEventListIterator event(*m_application->eventList()); event.current(); ++event)
	{
		if(!(*event)->soundfile.isNull())
			(*event)->presentation |= KNotifyClient::Sound;
	}
	updateView();
}

///////////////////////////////////////////////////////////////////

/**
 * Custom item that represents a KNotify-event
 * creates and handles checkable child-items
 */
KNotifyDialog::KNListViewItem::KNListViewItem( QListView *parent, KNEvent *e )
	: QListViewItem( parent, e->text() )
{
	event = e;

	if ( (e->dontShow & KNotifyClient::Stderr) == 0 )
	{
		stderrItem = new KNCheckListItem( this, event, KNotifyClient::Stderr,
		                                  i18n("Standard error output"));
		stderrItem->setOn( e->presentation & KNotifyClient::Stderr );
	}

	if ( (e->dontShow & KNotifyClient::Messagebox) == 0 )
	{
		msgboxItem = new KNCheckListItem(this, event, KNotifyClient::Messagebox,
		                                 i18n("Show messagebox"));
		msgboxItem->setOn( e->presentation & KNotifyClient::Messagebox );
	}

	if ( (e->dontShow & KNotifyClient::Sound) == 0 )
	{
		soundItem = new KNCheckListItem( this, event, KNotifyClient::Sound,
		                                 i18n("Play sound"));

		soundItem->setOn( e->presentation & KNotifyClient::Sound );
		soundItem->setText( COL_FILENAME, e->soundfile );
	}

	if ( (e->dontShow & KNotifyClient::Logfile) == 0 )
	{
		logItem = new KNCheckListItem( this, event, KNotifyClient::Logfile,
                                         i18n("Log to file"));
		logItem->setOn( e->presentation & KNotifyClient::Logfile  );
		logItem->setText( COL_FILENAME, e->logfile );
	}
}

/**
 * a child has changed -> update the KNEvent
 * not implemented as signal/slot to avoid lots of QObjects and connects
 */
void KNotifyDialog::KNListViewItem::itemChanged( KNCheckListItem *item )
{
	if ( item->isOn() )
		event->presentation |= item->eventType();
	else
		event->presentation &= ~item->eventType();
}



//////////////////////////////////////////////////////////////////////

/**
 * custom checkable item telling its parent when it was clicked
 */
KNotifyDialog::KNCheckListItem::KNCheckListItem( QListViewItem *parent, KNEvent *e, int t,
								  const QString& text )
	: QCheckListItem( parent, text, QCheckListItem::CheckBox ),
	  event( e ),
	  _eventType( t )
{
}

void KNotifyDialog::KNCheckListItem::stateChange( bool b )
{
	((KNListViewItem *) parent())->itemChanged( this );
	QCheckListItem::stateChange(b);
}

//////////////////////////////////////////////////////////////////////

KNotifyDialog::KNApplication::KNApplication( const QString &path )
	: valid(false)
{
	if(!path.contains('/')) return;
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
	valid = true;
}

KNotifyDialog::KNApplication::~KNApplication()
{
	delete config;
	delete kc;
	delete m_events;
}


KNotifyDialog::EventList * KNotifyDialog::KNApplication::eventList()
{
	if ( !m_events )
	{
		m_events = new EventList;
		m_events->setAutoDelete( true );
		loadEvents();
	}

	return m_events;
}

void KNotifyDialog::KNApplication::save()
{
	if ( !m_events )
		return;

	KNEventListIterator it( *m_events );
	KNEvent *e;
	while ( (e = it.current()) )
	{
		config->setGroup( e->configGroup );
		config->writeEntry( "presentation", e->presentation );
		config->writeEntry( "soundfile", e->soundfile );
		config->writeEntry( "logfile", e->logfile );

		++it;
	}
	config->sync();
}

void KNotifyDialog::KNApplication::loadEvents()
{
	KNEvent *e = 0L;

	const QString global = QString::fromLatin1("!Global!");
	const QString default_group = QString::fromLatin1("<default>");
	const QString name = QString::fromLatin1("Name");
	const QString comment = QString::fromLatin1("Comment");
	const QString unknown = i18n("Unknown Title");
	const QString nodesc = i18n("No Description");

	QStringList conflist = kc->groupList();
	for(QStringList::Iterator it = conflist.begin(); it != conflist.end(); ++it)
	{
		if ( (*it) != global && (*it) != default_group )
		{
			// event group
			kc->setGroup( *it );

			e = new KNEvent;
			e->name = kc->readEntry( name, unknown );
			e->description = kc->readEntry( comment, nodesc );
			e->configGroup = *it;

			if ( e->name.isEmpty() || e->description.isEmpty() )
			{
				delete e;
			}
			else
			{ // load the event
				int default_rep = kc->readNumEntry("default_presentation", 0 );
				QString default_logfile = kc->readEntry("default_logfile");
				QString default_soundfile = kc->readEntry("default_sound");
				config->setGroup(*it);
				e->presentation = config->readNumEntry("presentation", default_rep);
				e->dontShow = config->readNumEntry("nopresentation", 0 );
				e->logfile = config->readEntry("logfile", default_logfile);
				e->soundfile = config->readEntry("soundfile", default_soundfile);

				m_events->append( e );
			}
		}
	}

	return;
}

#include "knotifydialog.moc"
#include "knotifydialog_private.moc"
