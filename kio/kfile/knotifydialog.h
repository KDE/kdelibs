/*
	Copyright (C) Charles Samuels <charles@altair.dhs.org>
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

#ifndef KNOTIFYDIALOG_H
#define KNOTIFYDIALOG_H

#include <kdialogbase.h>

class QCheckBox;
class QCheckListItem;
class QListView;
class QPushButton;
class QSlider;
class KURLRequester;

/** KNotifyDialog presents an interface for configuring an application's
 * KNotify events.
 *
 * Rather than requiring the user to wade through the entire list of
 * applications' events in KControl, your application can make the list of its
 * own notifications available here.
 */
class KNotifyDialog : public KDialogBase
{
Q_OBJECT

public:
	KNotifyDialog(QWidget *parent = 0, const char *name = 0);
	virtual ~KNotifyDialog();

	/** Execute a KNotifyDialog.
	 *
	 * @see exec for the return values.
	 */
	static int configure(QWidget *parent = 0, const char *name = 0);

private slots:
	void load();
	void slotApply();
	void slotDefault();
	void disableAllSounds();
	void enableAllSounds();

	void slotRequesterClicked( KURLRequester * );
	void externalClicked( bool on );

	void slotItemActivated( QListViewItem * );
	void slotFileChanged( const QString& text );
	void playSound();

private:
	enum
	{
		COL_FILENAME = 1
	};

	void updateView();

	QCheckBox *cbExternal;
	QListView *view;
	QLabel *lblFilename;
	KURLRequester *requester, *reqExternal;
	QPushButton *playButton, *soundButton;
	QSlider *volumeSlider;
	bool updating;

	class KNApplication;
	class KNEvent;
	class KNListViewItem;
	class KNCheckListItem;
	typedef QPtrList<KNotifyDialog::KNEvent> EventList;
	typedef QPtrListIterator<KNotifyDialog::KNEvent> KNEventListIterator;

	KNApplication *m_application;
	KNCheckListItem *currentItem;

	class Data;
	Data *d;
};

#endif
