/*
   Copyright (c) 1997 Christian Esken (esken@kde.org)
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/



#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <dcopclient.h>

#include <qmessagebox.h>

#include "knotify.h"
#include "knotify.moc"

#include <iostream.h>

#include "qiomanager.h"
#include "dispatcher.h"
#include "soundserver.h"

SimpleSoundServer *server;

class KNotifyEntryPrivate
{
public:
  KNotifyEntryPrivate() {};
  ~KNotifyEntryPrivate() {};

  QCString	i_s_soundFilename;
  QString	i_s_message;
};

KApplication   *globalKapp;

int main(int argc, char **argv)
{
  globalKapp = new KApplication( argc, argv, "knotify" );
  // setup dcop communication
  if ( !kapp->dcopClient()->isAttached() ) {
    kapp->dcopClient()->registerAs("knotify",false);
  }
  // setup mcop communication
  QIOManager qiomanager;
  Dispatcher dispatcher(&qiomanager);

  // obtain an object reference to the soundserver
  server = SimpleSoundServer::_fromString("global:Arts_SimpleSoundServer");
  if(!server)
    cerr << "artsd not running - no sound notifications" << endl;

  KNotify *l_dcop_notify = new KNotify();

  KNotifyEntry *l_event1 = new KNotifyEntry(KNotifyEntry::Messagebox				, i18n("Switched to Desktop 1"));
  l_dcop_notify->registerNotification( (const char*)"Desktop1", l_event1);
  KNotifyEntry *l_event2 = new KNotifyEntry(KNotifyEntry::Stderr				, i18n("Switched to Desktop 2"));
  l_dcop_notify->registerNotification( (const char*)"Desktop2", l_event2);
  KNotifyEntry *l_event3 = new KNotifyEntry( KNotifyEntry::Messagebox | KNotifyEntry::Stderr	, i18n("Switched to Desktop 3"));
  l_dcop_notify->registerNotification( (const char*)"Desktop3", l_event3);
  KNotifyEntry *l_event4 = new KNotifyEntry(KNotifyEntry::None					, i18n("Switched to Desktop 4"));
  l_dcop_notify->registerNotification( (const char*)"Desktop4", l_event4);
  KNotifyEntry *l_event5 = new KNotifyEntry(KNotifyEntry::Sound					, i18n("Switched to Desktop 5"));
#warning "(Stefan) - you probably need to change the path to the wav here"
  l_event5->setSound(true,"/usr/share/sounds/KDE_Startup.wav");
  l_dcop_notify->registerNotification( (const char*)"Desktop5", l_event5);


  
  int l_i_ret = globalKapp->exec();
  return l_i_ret;
}



KNotify::KNotify() : QObject(), DCOPObject("Notify")
{
  // I expect we only have few items in the dict, so
  // I choose a small hash table [I like primes.html :-) ].
  I_events = new QDict<KNotifyEntry>(43);


  // We define an internal notifaction message, that is always there.
  // So we can safely log internal and usage errors.
  KNotifyEntry *l_event = new KNotifyEntry(KNotifyEntry::Messagebox, i18n("Application error. Unknown event used."));
  I_events->insert( (const char*)"KNotify usage", l_event);
}


KNotify::~KNotify()
{
  delete I_events;
}


bool KNotify::registerNotification(QString name, KNotifyEntry *notificationEntry)
{
  I_events->insert( name, notificationEntry);
  return true;
}


bool KNotify::process(const QCString &fun, const QByteArray &data,
		   QCString& /*replyType*/, QByteArray& /*replyData*/ )
{
  if ( fun == "notify(QString)" ) {
    QDataStream dataStream( data, IO_ReadOnly );
    QString l_s_event;
    dataStream >> l_s_event;
    
    processNotification(l_s_event);
 
    return true;
  }
  return false;
}



void KNotify::processNotification(QString &val_s_event)
{
  // This varible saves us from infinite recursions, in case
  // an error occurs while processing an internal error message
  static bool internalNotificationRunning = false;



  // Lokup the notification event
  KNotifyEntry *l_event = (*I_events)[val_s_event];
  if ( l_event == 0 ) {

    // The event is not registered. We will do a notification about this.
    if ( internalNotificationRunning ) {
      // Ouch: We are already processing an internal error. We give up here.
      cerr << "KDE notification system internal error: " \
	"KNotify::processNotification( " << val_s_event << ")";
    }
    else {
      internalNotificationRunning = true;
      QString l_s_tmp;
      l_s_tmp = "KNotify usage";
      processNotification(l_s_tmp);
      internalNotificationRunning = false;
    }
  } // -<- event not registered

  else {
    
    // The event is registered. Now do the notification
    if (l_event->sound() )		{ notifyBySound(l_event); }
    if (l_event->messagebox() )		{ notifyByMessagebox(l_event); }
    if (l_event->logfile() )		{ notifyByLogfile(l_event); }
    if (l_event->logwindow() )		{ notifyByLogwindow(l_event); }
    if (l_event->stderr() )		{ notifyByStderr(l_event); }
  }

}


bool KNotify::notifyBySound( KNotifyEntry *ptr_event)
{
  if(server) server->play((const char *)ptr_event->p->i_s_soundFilename);
  return true;
}

bool KNotify::notifyByMessagebox( KNotifyEntry *ptr_event)
{
  QMessageBox *l_qmb_notification;
  l_qmb_notification = new QMessageBox( i18n("Notification"), ptr_event->p->i_s_message, \
					QMessageBox::Information, QMessageBox::Ok | QMessageBox::Default , 0, 0, \
					0, 0, false);


  l_qmb_notification->show();
  return true;
}

bool KNotify::notifyByLogwindow( KNotifyEntry */*ptr_event*/)
{
  cerr << "notifyByLogwindow(): Not implemented\n";
  return true;
}

bool KNotify::notifyByLogfile( KNotifyEntry */*ptr_event*/)
{
  cerr << "notifyByLogfile(): Not implemented\n";
  return true;
}

bool KNotify::notifyByStderr( KNotifyEntry *ptr_event)
{
  cerr << "KDE Notification system: " << ptr_event->p->i_s_message << "\n";
  return true;
}




KNotifyEntry::KNotifyEntry(unsigned int presentation, QString message)
{
  p = new KNotifyEntryPrivate;
  setSound(false, QCString(""));
  setMessagebox(presentation & Messagebox);
  setLogfile(presentation & Logfile);
  setLogwindow(presentation & Logwindow);
  setStderr(presentation & Stderr);
  setMessage(message);
}

KNotifyEntry::KNotifyEntry(unsigned int presentation, QString message, QCString soundFilename)
{
  p = new KNotifyEntryPrivate;
  setSound(presentation & Sound, soundFilename);
  setMessagebox(presentation & Messagebox);
  setLogfile(presentation & Logfile);
  setLogwindow(presentation & Logwindow);
  setStderr(presentation & Stderr);
  setMessage(message);
}

KNotifyEntry::~KNotifyEntry()
{
  delete p;
}



void KNotifyEntry::setSound(bool YesNo, QCString soundFilename)
{
  i_b_sound = YesNo;
  p->i_s_soundFilename = soundFilename;
}
void KNotifyEntry::setMessagebox(bool YesNo)
{ i_b_messagebox = YesNo; }
void KNotifyEntry::setLogfile(bool YesNo)
{ i_b_logfile = YesNo; }
void KNotifyEntry::setLogwindow(bool YesNo)
{ i_b_logwindow = YesNo; }
void KNotifyEntry::setStderr(bool YesNo) 
{ i_b_stderr = YesNo; }
void KNotifyEntry::setMessage(QString& message)
{ p->i_s_message = message; }
 
 
 
bool KNotifyEntry::sound()
{ return i_b_sound; }
bool KNotifyEntry::messagebox()
{ return i_b_messagebox; }
bool KNotifyEntry::logfile()
{ return i_b_logfile; }
bool KNotifyEntry::logwindow()
{ return i_b_logwindow; }
bool KNotifyEntry::stderr() 
{ return i_b_stderr; }

