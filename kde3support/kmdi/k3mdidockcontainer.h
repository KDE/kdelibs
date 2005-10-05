 /* This file is part of the KDE project
  Copyright (C) 2002 Christoph Cullmann <cullmann@kde.org>
  Copyright (C) 2002,2003 Joseph Wenninger <jowenn@kde.org>

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

#ifndef _K3MDI_DOCK_CONTAINER_
#define _K3MDI_DOCK_CONTAINER_

#include <qwidget.h>
#include <qstringlist.h>
#include <k3dockwidget.h>
#include <qmap.h>
#include <qdom.h>

# include <k3dockwidget_p.h>

#include <qpushbutton.h>

class Q3WidgetStack;
class KMultiTabBar;
class K3DockButton_Private;

class KDE3SUPPORT_EXPORT K3MdiDockContainer: public QWidget, public K3DockContainer
{
	Q_OBJECT

public:
	K3MdiDockContainer( QWidget *parent, QWidget *win, int position, int flags );
	virtual ~K3MdiDockContainer();

	/** Get the K3DockWidget that is our parent */
	K3DockWidget *parentDockWidget();

	/**
	 * Add a widget to this container
	 * \param w the K3DockWidget we are adding
	 */
	virtual void insertWidget ( K3DockWidget *w, QPixmap, const QString &, int & );

	/**
	 * Show a widget.
	 *
	 * The widget has to belong to this container otherwise
	 * it will not be shown
	 * \param w the K3DockWidget to show
	 */
	virtual void showWidget ( K3DockWidget *w );

	/**
	 * Set the tooltip for the widget.
	 * Currently, this method does nothing
	 */
	virtual void setToolTip ( K3DockWidget *, QString & );

	/**
	 * Set the pixmap for the widget.
	 */
	virtual void setPixmap( K3DockWidget* widget, const QPixmap& pixmap );

	/**
	 * Undock the widget from the container.
	 */
	virtual void undockWidget( K3DockWidget* dwdg );

	/**
	 * Remove a widget from the container. The caller of this function
	 * is responsible for deleting the widget
	 */
	virtual void removeWidget( K3DockWidget* );

	/**
	 * Hide the the dock container if the number of items is 0
	 */
	void hideIfNeeded();

	/**
	 * Save the config using a KConfig object
	 *
	 * The combination of the group_or_prefix variable and the parent
	 * dockwidget's name will be the group the configuration is saved in
	 * \param group_or_prefix the prefix to append to the parent dockwidget's name
	 */
	virtual void save( KConfig *, const QString& group_or_prefix );

	/**
	 * Load the config using a KConfig object
	 * 
	 * The combination of the group_or_prefix variable and the parent
	 * dockwidget's name will be the group the configuration is loaded from
	 * \param group_or_prefix the prefix to append to the parent dockwidget's name
	 */
	virtual void load( KConfig *, const QString& group_or_prefix );

	/**
	 * Save the config to a QDomElement
	 */
	virtual void save( QDomElement& );

	/**
	 * Load the config from a QDomElement
	 */
	virtual void load( QDomElement& );

	/**
	 * Set the style for the tabbar
	 */
	void setStyle( int );

protected:
	bool eventFilter( QObject*, QEvent* );

public slots:
	void init();
	void collapseOverlapped();
	void toggle();
	void nextToolView();
	void prevToolView();
protected slots:
	void tabClicked( int );
	void delayedRaise();
	void changeOverlapMode();
private:
	QWidget *m_mainWin;
	Q3WidgetStack *m_ws;
	KMultiTabBar *m_tb;
	int mTabCnt;
	int oldtab;
	int m_previousTab;
	int m_position;
	int m_separatorPos;
	QMap<K3DockWidget*, int> m_map;
	QMap<int, K3DockWidget*> m_revMap;
	QMap<K3DockWidget*, K3DockButton_Private*> m_overlapButtons;
	QStringList itemNames;
	QMap<QString, QString> tabCaptions;
	QMap<QString, QString> tabTooltips;
	int m_inserted;
	int m_delayedRaise;
	bool m_horizontal;
	bool m_block;
	bool m_tabSwitching;
	QObject *m_dragPanel;
	K3DockManager *m_dockManager;
	QMouseEvent *m_startEvent;
	enum MovingState {NotMoving = 0, WaitingForMoveStart, Moving} m_movingState;
signals:
	void activated( K3MdiDockContainer* );
	void deactivated( K3MdiDockContainer* );
};

#endif

// kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;
