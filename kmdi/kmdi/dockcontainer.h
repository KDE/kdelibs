/* This file is part of the KDE project
   Copyright (C) 2002, 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2002, 2004 Christoph Cullmann <cullmann@kde.org>

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

#ifndef _KMDI_DOCK_CONTAINER_
#define _KMDI_DOCK_CONTAINER_

#include <qwidget.h>
#include <qstringlist.h>
#include <kdockwidget.h>
#include <qmap.h>

# include <kdockwidget_p.h>

#include <qpushbutton.h>

class QWidgetStack;
class KMultiTabBar;
class KDockButton_Private;

namespace KMDI
{

class DockContainer: public QWidget, public KDockContainer
{
  Q_OBJECT

  public:
    DockContainer(QWidget *parent, QWidget *win, int position, int flags);
    virtual ~DockContainer();

    /** Get the KDockWidget that is our parent */
    KDockWidget *parentDockWidget();

    /**
     * Add a widget to this container.
     * \param w the KDockWidget object to add
     * \TODO Remove the extra parameters that we don't use?
     */
    virtual void insertWidget (KDockWidget *w, QPixmap, const QString &, int &);

    /**
     * Show a KDockWidget in our DockContainer
     * \param w the KDockWidget to show
     */
    virtual void showWidget (KDockWidget *w);

    /**
     * Set a tooltip for a widget
     *
     * \TODO Actually implement it? Right now, it looks just it
     * does exactly nothing
     */
    virtual void setToolTip (KDockWidget *, QString &);

    /**
     * Set a pixmap for one of our dock widgets
     * \param widget the KDockWidget to set the pixmap for
     * \param pixmap the pixmap you want to give the widget
     */
    virtual void setPixmap(KDockWidget* widget, const QPixmap& pixmap);

    /**
     * Undock a widget from the container. This function is called
     * when you've dragged a tab off the dock it's attached to. 
     * \param dwdg the KDockWidget to undock
     */
    virtual void undockWidget(KDockWidget* dwdg);

    /**
     * Remove a widget from the container. The caller
     * of this function is responsible for deleting the widget after calling
     * this function.
     */
    virtual void removeWidget(KDockWidget*);

    void hideIfNeeded();

    virtual void save(KConfig *,const QString& group_or_prefix);
    virtual void load(KConfig *,const QString& group_or_prefix);

    void setStyle(int);
  protected:
    bool eventFilter(QObject*,QEvent*);

  public slots:
    void init();
    void collapseOverlapped();
    void toggle();
    void nextToolView();
    void prevToolView();
  protected slots:
    void tabClicked(int);
    void delayedRaise();
    void changeOverlapMode();
  private:
    QWidget *m_mainWin;
    QWidgetStack *m_ws;
    KMultiTabBar *m_tb;
    int mTabCnt;
    int oldtab;
    int m_previousTab;
    int m_position;
    int m_separatorPos;
    QMap<KDockWidget*,int> m_map;
    QMap<int,KDockWidget*> m_revMap;
    QMap<KDockWidget*,KDockButton_Private*> m_overlapButtons;
    QStringList itemNames;
    QMap<QString,QString> tabCaptions;
    QMap<QString,QString> tabTooltips;
    int m_inserted;
    int m_delayedRaise;
    bool m_vertical;
    bool m_block;
    bool m_tabSwitching;
    QObject *m_dragPanel;
    KDockManager *m_dockManager;
    QMouseEvent *m_startEvent;
    enum MovingState {NotMoving=0,WaitingForMoveStart,MovingInternal,Moving} m_movingState;
  signals:
        void activated(DockContainer*);
        void deactivated(DockContainer*);
};

}

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
