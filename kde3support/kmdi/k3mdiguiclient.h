/* This file is part of the KDE libraries
  Copyright (C) 2003 Joseph Wenninger

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef K3MDIGUICLIENT_H
#define K3MDIGUICLIENT_H

#include <qobject.h>
#include <qpointer.h>
#include <kxmlguiclient.h>
#include <kaction.h>
#include <q3ptrlist.h>
#include "k3mdidefines.h"

class KMainWindow;
class KToolBar;
class K3MdiToolViewAccessor;
class K3MdiMainFrm;
class K3DockWidget;

namespace K3MDIPrivate
{

/**
 * A class derived from KXMLGUIClient that handles the various
 * K3MDI modes
 */
class KDE3SUPPORT_EXPORT K3MDIGUIClient : public QObject,
			public KXMLGUIClient
{
	Q_OBJECT
public:

	K3MDIGUIClient( K3MdiMainFrm *mdiMainFrm, bool showMDIModeAction, const char *name = 0 );
	virtual ~K3MDIGUIClient();

	/**
	 * Add a new tool view to this K3MDIGUIClient. Reads the shortcut
	 * for the tool view from the K3MDI application's config file and also
	 * adds a ToggleToolViewAction so that the visibility of the toolviews
	 * can be turned on and off
	 */
	void addToolView( K3MdiToolViewAccessor* );

private Q_SLOTS:

	/**
	 * The XMLGUIClient factory has added an XMLGUI client. Plug our actions
	 * in if we're the client that's been added.
	 */
	void clientAdded( KXMLGUIClient *client );
	/**
	 * Plug in the various toggle actions we have into the tool views menu
	 */
	void setupActions();

	/**
	 * Change the view mode. This will automatically change the view mode
	 * of the K3MdiMainFrm associated with this K3MDIGUIClient
	 */
	void changeViewMode( int id );

	/**
	 * One of our tool view toggle actions has been deleted. Redo the
	 * tool views menu
	 */
	void actionDeleted( QObject* );

	/**
	 * Updates the action that lets the user change the MDI mode to the
	 * correct value based on the current mode
	 */
	void mdiModeHasBeenChangedTo( K3Mdi::MdiMode );

Q_SIGNALS:
	/** Toggle the top tool dock */
	void toggleTop();

	/** Toggle the left tool dock */
	void toggleLeft();

	/** Toggle the right tool dock */
	void toggleRight();

	/** Toggle the bottom tool dock */
	void toggleBottom();

private:
	class K3MDIGUIClientPrivate;
	K3MDIGUIClientPrivate *d;
	K3Mdi::MdiMode m_mdiMode;

	QPointer<K3MdiMainFrm> m_mdiMainFrm;
	Q3PtrList<KAction> m_toolViewActions;
	Q3PtrList<KAction> m_documentViewActions;

	KActionMenu *m_docMenu;
	KActionMenu *m_toolMenu;
	KSelectAction *m_mdiModeAction;

	KActionMenu *m_gotoToolDockMenu;
};

/**
 * A KToggleAction specifically for toggling the showing
 * or the hiding of a K3MDI tool view
 */
class KDE3SUPPORT_EXPORT ToggleToolViewAction: public KToggleAction
{
	Q_OBJECT
public:

	ToggleToolViewAction( const QString& text, const KShortcut& cut = KShortcut(), K3DockWidget *dw = 0, K3MdiMainFrm *mdiMainFrm = 0,
	                      KActionCollection* parent = 0, const char* name = 0 );

	virtual ~ToggleToolViewAction();

private:

	K3DockWidget *m_dw;
	K3MdiMainFrm *m_mdiMainFrm;
protected Q_SLOTS:

	void slotToggled( bool );
	void anDWChanged();
	void slotWidgetDestroyed();
};


}

#endif 
// kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;
