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
 
        kaudioserver.cpp: Audio server starter for libmediatool/maudio
*/

#ifndef KNOTIFY_H
#define KNOTIFY_H

#include <qobject.h>
#include <qdict.h>
#include <dcopobject.h>

class KNotifyEntryPrivate;
class KNotify;

/**
   A KNotifyEntry describes a complete notification event. Such an event is uniqely
   identified by its notification name.
   
  notification 
   the notific */
class KNotifyEntry
{
public:
  /// An enum of all notification presentations. Use an OR operator (|) if you want
  /// multiple presentation.
  enum Presentation { None=0, Sound=1, Messagebox=2, Logwindow=4, Logfile=8, Stderr=16 };

  /** Constructs a notification with a certain presentation, a notification message
      and a sound file. Please note that the sound will only be played, when
      you specify Sound as part of presentation. */
  KNotifyEntry(unsigned int presentation, QString message, QCString soundFilename );
  /** Constructs a notification without sound presentation. Differs from the previous
      constructor only in the parameters it accepts. Specifying Sound as part of presentation
      is ignored */
  KNotifyEntry(unsigned int presentation, QString message);
  /** Destructor */
  ~KNotifyEntry();

  void setSound(bool YesNo, QCString soundFilename);
  void setMessagebox(bool YesNo);
  void setLogfile(bool YesNo);
  void setLogwindow(bool YesNo);
  void setMessage(QString& message);
  void setStderr(bool YesNo);

  bool sound();
  bool messagebox();
  bool logfile();
  bool logwindow();
  bool stderr();


private:
  bool i_b_sound;
  bool i_b_messagebox;
  bool i_b_logfile;
  bool i_b_logwindow;
  bool i_b_stderr;

  // For furter updates
  KNotifyEntryPrivate *p;

  friend KNotify;
};


class KNotify : public QObject, DCOPObject
{
Q_OBJECT
K_DCOP

public:
  KNotify();
  ~KNotify();

  /// Registers a new notification. After that it can be used by clients by using the event name "name"
  bool registerNotification(QString name, KNotifyEntry *notificationEntry);

  // !!! TODO  bool setLogfile(QCString& logfile);

protected:

  void processNotification(QString &val_s_event);
  bool notifyBySound( KNotifyEntry *ptr_event);
  bool notifyByMessagebox( KNotifyEntry *ptr_event);
  bool notifyByLogwindow( KNotifyEntry *ptr_event);
  bool notifyByLogfile( KNotifyEntry *ptr_event);
  bool notifyByStderr( KNotifyEntry *ptr_event);

  QDict<KNotifyEntry> *I_events;
};


#endif

