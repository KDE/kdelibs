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

#ifndef KNOTIFYDIALOGPRIVATE_H
#define KNOTIFYDIALOGPRIVATE_H

#include <qlistview.h>
#include <qptrlist.h>
#include <qstring.h>

class KConfig;

class KNotifyDialog::KNApplication
{
public:
	KNApplication( const QString &path );
	~KNApplication();

	QString text() const { return m_description; }
	QString icon() const { return m_icon; }
	EventList * eventList();
	void save();

	bool isValid(void) {return valid;}

private:
	void loadEvents();

	QString m_icon;
	QString m_description;
	EventList *m_events;

	KConfig *kc; // The file that defines the events.
	KConfig *config; // The file that contains the settings for the events.

	bool valid;
};

class KNotifyDialog::KNEvent
{
friend class KNApplication;

public:
	QString text() const { return description; }

	int presentation;
	int dontShow;
	QString logfile;
	QString soundfile;

private:
	KNEvent() {
	presentation = 0;
	dontShow = 0;
	}
	QString name;
	QString description;
	QString configGroup;
};

class KNotifyDialog::KNListViewItem : public QObject, public QListViewItem
{
Q_OBJECT

public:
	KNListViewItem( QListView *parent, KNEvent *e );
	void itemChanged( KNCheckListItem * );

signals:
	void changed();
	void soundActivated( KNEvent * );
	void logActivated( KNEvent * );
	void otherActivated( KNEvent * );

private:
	KNEvent *event;
	KNCheckListItem *stderrItem, *msgboxItem, *soundItem, *logItem;

};

class KNotifyDialog::KNCheckListItem : public QCheckListItem
{
public:
	KNCheckListItem( QListViewItem *parent, KNEvent *e, int type,
	                 const QString& text );
	int eventType() const { return _eventType; }
	KNEvent *event;

protected:
	virtual void stateChange( bool on );
	const int _eventType;
};

#endif
