/*
   Copyright (c) 1997 Christian Esken (esken@kde.org)
             (c) 2005 Allan Sandfeld Jensen (kde@carewolf.com)

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef KNOTIFY_H
#define KNOTIFY_H

#include <qobject.h>
#include <knotifyclient.h>
#include <dcopobject.h>
#include <qwindowdefs.h>

class KNotifyPrivate;
class KProcess;

class KNotify : public QObject, public DCOPObject
{
Q_OBJECT
K_DCOP

public:
    KNotify( bool useArts );
    ~KNotify();

    enum PlayingFinishedStatus
    {
        PlayedOK = 0,        // success, all following mean failure
        NoSoundFile,
        FileAlreadyPlaying,
        NoSoundSupport,
        PlayerBusy,
        Aborted,
        Unknown = 5000
    };

    bool event(QEvent* );

protected:
k_dcop:
/*
	// deprecated
	void notify(const QString &event, const QString &fromApp,
                         const QString &text, QString sound, QString file,
                         int present, int level);

	// deprecated
	void notify(const QString &event, const QString &fromApp,
                         const QString &text, QString sound, QString file,
                         int present, int level, int winId);
*/
    void notify(const QString &event, const QString &fromApp,
                const QString &text, QString sound, QString file,
                int present, int level, int winId = 0, int eventId = 1);


    void reconfigure();
    void setVolume( int volume );
    void sessionReady(); // from ksmserver

private:
    bool notifyBySound(const QString &sound, const QString &appname, int eventId);
    bool notifyByMessagebox(const QString &text, int level, WId winId);
    bool notifyByLogfile(const QString &text, const QString &file);
    bool notifyByStderr(const QString &text);
    bool notifyByPassivePopup(const QString &text, const QString &appName,
                              WId winId );
    bool notifyByExecute(const QString &command,
                         const QString& event,
                         const QString& fromApp,
                         const QString& text,
                         int winId,
                         int eventId );
    bool notifyByTaskbar( WId winId );

    void soundFinished( int eventId, PlayingFinishedStatus reason );

    WId checkWinId( const QString& appName, WId senderWinId );

    /**
      * checks if eventname is a global event (exists in config/eventsrc)
      **/
    bool isGlobal(const QString &eventname);

private slots:
    void slotPlayerProcessExited( KProcess *proc );

private:
    KNotifyPrivate* d;
    void loadConfig();
};


#endif

