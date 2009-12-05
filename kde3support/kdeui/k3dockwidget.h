/* This file is part of the KDE libraries
   Copyright (C) 2000 Max Judin <novaprint@mtu-net.ru>
   Copyright (C) 2000 Falk Brettschneider <falk@kdevelop.org>
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

/*
   activities:
   -----------
   05/2001 -               : useful patches, bugfixes by Christoph Cullmann <crossfire@babylon2k.de>,
                             Joseph Wenninger <jowenn@bigfoot.com> and  Falk Brettschneider
   03/2001 - 05/2001       : maintained and enhanced by Falk Brettschneider <falk@kdevelop.org>
   03/2000                 : class documentation added by Falk Brettschneider <gigafalk@yahoo.com>
   10/1999 - 03/2000       : programmed by Max Judin <novaprint@mtu-net.ru>

   C++ classes in this file:
   -------------------------
   - K3DockWidgetAbstractHeader     - minor helper class
   - K3DockWidgetAbstractHeaderDrag - minor helper class
   - K3DockWidgetHeaderDrag         - drag panel in a dockwidget title bar
   - K3DockWidgetHeader             - dockwidget title bar containing the drag panel
   - K3DockTabGroup                 - minor helper class
   - K3DockWidget                   - IMPORTANT CLASS: the one and only dockwidget class
   - K3DockManager                  - helper class
   - K3DockMainWindow               - IMPORTANT CLASS: a special KMainWindow that can have dockwidgets
   - K3DockArea                     - like K3DockMainWindow but inherits just QWidget

   IMPORTANT Note: This file compiles also in Qt-only mode by using the NO_KDE2 precompiler definition!
*/


#ifndef KDOCKWIDGET_H
#define KDOCKWIDGET_H

#define _KDOCKWIDGET_2_2_

#include <kde3support_export.h>
#include <QtCore/QPoint>
#include <Qt3Support/Q3PtrList>
#include <QtGui/QFrame>
#include <QtXml/QDomElement>
#include <QtGui/QTabWidget>

#ifndef NO_KDE2
#include <kxmlguiwindow.h>
#include <netwm_def.h>
#else

#include <Qt3Support/Q3MainWindow>
#include "exportdockclass.h"
#include "dummykmainwindow.h"
#endif

class K3DockSplitter;
class K3DockManager;
class K3DockWidget;
class K3DockButton_Private;
class K3DockWidgetPrivate;
class K3DockWidgetHeaderPrivate;
class K3DockArea;

class Q3PopupMenu;
class QVBoxLayout;
class QHBoxLayout;
class QPixmap;

#ifndef NO_KDE2
class KConfigGroup;
#else
#endif

class K3DockContainer;

namespace K3MDI
{
  class MainWindow;
}

/**
 * An abstract base clase for all dockwidget headers (and member of the dockwidget class set).
 * See the class description of K3DockWidgetHeader!
 * More or less a minor helper class for the dockwidget class set.
 *
 * @author Max Judin (documentation: Falk Brettschneider).
 */
class KDE3SUPPORT_EXPORT K3DockWidgetAbstractHeader : public QFrame
{
  Q_OBJECT
public:

  /**
   * Constructs this.
   *
   * @param parent the parent widget (usually a dockwidget)
   * @param name   the object instance name
   */
  K3DockWidgetAbstractHeader( K3DockWidget* parent, const char* name = 0L );

  /**
   * Destructs this.
   */
  virtual ~K3DockWidgetAbstractHeader(){}

  /**
   * Provides things concerning to switching to toplevel mode. Must be overridden by an inheriting class.
   */
  virtual void setTopLevel( bool ){}

#ifndef NO_KDE2
  /**
   * Provides saving the current configuration. Must be overridden by an inheriting class.
   */
  virtual void saveConfig( KConfigGroup* ){}

  /**
   * Provides loading the current configuration.  Must be overridden by an inheriting class
   */
  virtual void loadConfig( KConfigGroup* ){}
#endif

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class K3DockWidgetAbstractHeaderPrivate;
  K3DockWidgetAbstractHeaderPrivate *d;
};

/**
 * An abstract class for all dockwidget drag-panels of a dockwidgets (and member of the dockwidget class set).
 * See the class description of K3DockWidgetHeaderDrag!
 * More or less a minor helper class for the dockwidget class set.
 *
 * @author Max Judin (documentation: Falk Brettschneider).
 */
class KDE3SUPPORT_EXPORT K3DockWidgetAbstractHeaderDrag : public QFrame
{
  Q_OBJECT
public:

  /**
   * Constructs this.
   *
   * @param parent the parent widget (usually a dockwidget header)
   * @param dock   the dockwidget where it belongs to
   * @param name   the object instance name
   */
  K3DockWidgetAbstractHeaderDrag( K3DockWidgetAbstractHeader* parent,
                                 K3DockWidget* dock, const char* name = 0L );

  /**
   * Destructs this.
   */
  virtual ~K3DockWidgetAbstractHeaderDrag();

  /**
   * @return the dockwidget where this belongs to
   */
  K3DockWidget* dockWidget() const;

private:
  /**
   * the dockwidget where this belongs to
   */
  K3DockWidget* dw;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class K3DockWidgetAbstractHeaderDragPrivate;
  K3DockWidgetAbstractHeaderDragPrivate *d;
};

/**
 * This special widget is the panel one can grip with the mouses (and member of the dockwidget class set).
 * The widget for dragging, so to speak.
 * Usually it is located in the K3DockWidgetHeader.
 * More or less a minor helper class for the dockwidget class set.
 *
 * @author Max Judin (documentation: Falk Brettschneider).
 */
class KDE3SUPPORT_EXPORT K3DockWidgetHeaderDrag : public K3DockWidgetAbstractHeaderDrag
{
  Q_OBJECT
public:

  /**
   * Constructs this.
   *
   * @param parent the parent widget (usually a dockwidget header)
   * @param dock   the dockwidget where it belongs to
   * @param name   the object instance name
   */
  K3DockWidgetHeaderDrag( K3DockWidgetAbstractHeader* parent, K3DockWidget* dock,
                         const char* name = 0L );

  /**
   * Destructs this.
   */
  virtual ~K3DockWidgetHeaderDrag();

protected:

  /**
   * Draws the drag panel (a double line)
   */
  virtual void paintEvent( QPaintEvent* );

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class K3DockWidgetHeaderDragPrivate;
  K3DockWidgetHeaderDragPrivate *d;
};

/**
 * The header (additional bar) for a K3DockWidget s (and member of the dockwidget class set).
 * It have got the buttons located there. And it is for recording and reading the button states.
 * More or less a minor helper class for the dockwidget class set.
 *
 * @author Max Judin (documentation: Falk Brettschneider).
 */
class KDE3SUPPORT_EXPORT K3DockWidgetHeader : public K3DockWidgetAbstractHeader
{
  Q_OBJECT
public:

  /**
   * Constructs this.
   *
   * @param parent the parent widget (usually a dockwidget)
   * @param name   the object instance name
   */
  K3DockWidgetHeader( K3DockWidget* parent, const char* name = 0L );

  /**
   * Destructs this.
   */
  virtual ~K3DockWidgetHeader();

  /**
   * Hides the close button and stay button when switching to toplevel or vice versa shows them.
   *
   * @param t toplevel or not
   */
  virtual void setTopLevel( bool t);

  /**
   * Sets the drag panel of this header.
   *
   * @param nd A pointer to the new drag panel
   */
  void setDragPanel( K3DockWidgetHeaderDrag* nd );

  /**
   * Get the drag panel of this header.
   */
   K3DockWidgetHeaderDrag *dragPanel();

  bool dragEnabled() const;
  void setDragEnabled(bool b);
  void showUndockButton(bool show);

  void forceCloseButtonHidden(bool enable=true);
#ifndef NO_KDE2
  /**
   * Saves the current button state to a KDE config container object.
   *
   * @param c the configuration safe
   */
  virtual void saveConfig( KConfigGroup* c);

  /**
   * Loads the current button state from a KDE config container object.
   *
   * @param c the configuration safe
   */
  virtual void loadConfig( KConfigGroup* c);
#endif

   /**
    * add an arbitrary button to the dockwidget header
    * NOT PART OF THE PUBLIC API (you don't have access the class definition anyways, without special
    * header file copying. (don't do it))
    */
    void addButton(K3DockButton_Private*);

   /**
    * remove an arbtrary button from the dockwidget header
    * NOT PART OF THE PUBLIC API (you don't have access the class definition anyways, without special
    * header file copying. (don't do it))
    */
    void removeButton(K3DockButton_Private*);



protected Q_SLOTS:
  /**
   * Sets dragging the dockwidget off when the stay button is pressed down and vice versa.
   */
  void slotStayClicked();

protected:

  /**
   * A layout manager for placing the embedded buttons (close and stay)
   */
  QHBoxLayout* layout;

  /**
   * a little button for closing (undocking and hiding) the dockwidget
   */
  K3DockButton_Private* closeButton;

  /**
   * a little button for enabling/disabling dragging the dockwidget with the mouse
   */
  K3DockButton_Private* stayButton;

  /**
   * a little button for dock back the dockwidget to its previous dockwidget
   */
  K3DockButton_Private* dockbackButton;

  /**
   * the drag panel (double line)
   */
  K3DockWidgetHeaderDrag* drag;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  K3DockWidgetHeaderPrivate *d;
};

/**
 * It just hides the special implementation of a dockwidget tab groups (and is member of the dockwidget class set).
 * An abstraction what it is currently.
 * In general it is like QTabWidget but is more useful for the dockwidget class set.
 * More or less a minor helper class for the dockwidget class set.
 *
 * @author Max Judin (documentation: Falk Brettschneider).
 */
class KDE3SUPPORT_EXPORT K3DockTabGroup : public QTabWidget
{
  Q_OBJECT
public:
  /**
   * Constructs this. It just calls the method of the base class.
   */
  K3DockTabGroup( QWidget *parent = 0, const char *name = 0 );

  /**
   * Destructs a K3DockTabGroup.
   */
  virtual ~K3DockTabGroup();

  QWidget *transientTo();
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class K3DockTabGroupPrivate;
  K3DockTabGroupPrivate *d;
};


/**
 * Floatable widget that can be dragged around with the mouse and
 * encapsulate the actual widgets (and member of the dockwidget class
 * set).
 *
 * You just grip the double-lined panel, tear it off its parent
 * widget, drag it somewhere and let it loose. Depending on the
 * position where you leave it, the dockwidget becomes a toplevel
 * window on the desktop (floating mode) or docks to a new widget
 * (dock mode). Note: A K3DockWidget can only be docked to a
 * K3DockWidget.
 *
 * If you want to use this kind of widget, your main application
 * window has to be a K3DockMainWindow.  That is because it has
 * got several additional dock management features, for instance a
 * K3DockManager that has an overview over all dockwidgets and and
 * a dockmovemanager (internal class) that handles the dock process.
 *
 * Usually you create an K3DockWidget that covers the actual widget in this way:
 * \code
 * ...
 * K3DockMainWindow* mainWidget;
 * ...
 * K3DockWidget* dock = 0L;
 * dock = mainWidget->createDockWidget( "Any window caption", nicePixmap, 0L, i18n("window caption")); // 0L==no parent
 * QWidget* actualWidget = new QWidget( dock);
 * dock->setWidget( actualWidget); // embed it
 * dock->setToolTipString(i18n("That's me")); // available when appearing as tab page
 * ...
 * \endcode
 *
 * See K3DockMainWindow how a dockwidget is docked in.
 *
 *
 * @author Max Judin (documentation: Falk Brettschneider).
 */
class KDE3SUPPORT_EXPORT K3DockWidget: public QWidget
{
  Q_OBJECT
friend class K3DockManager;
friend class K3DockSplitter;
friend class K3DockMainWindow;
friend class K3DockArea;

public:
  /**
   * Construct a dockwidget.
   *
   * Initially, docking to another and docking to this is allowed for
   * every @p DockPosition.  It is supposed to be no (tab) group. It will
   * taken under control of its dockmanager.
   *
   * @param dockManager The responsible manager (dock helper)
   * @param name        Object instance name
   * @param pixmap      An icon (for instance shown when docked centered)
   * @param parent      Parent widget
   * @param strCaption  Title of the dockwidget window (shown when toplevel)
   * @param strTabPageLabel The title of the tab page (shown when in tab page mode), if it is "", only the icon will be shown, if it is 0L, the label is set to strCaption
   * @param f Qt::WidgetFlags widget flags
   */
  K3DockWidget( K3DockManager* dockManager, const char* name,
               const QPixmap &pixmap, QWidget* parent = 0L, const QString& strCaption = QString(),
               const QString& strTabPageLabel = QLatin1String( " " ), Qt::WFlags f = 0);

  /**
   * Destructs a dockwidget.
   */
  virtual ~K3DockWidget();

  /**
   * The possible positions where a dockwidget can dock to another dockwidget
   */
  enum DockPosition
  {
    DockNone   = 0,
    DockTop    = 0x0001,
    DockLeft   = 0x0002,
    DockRight  = 0x0004,
    DockBottom = 0x0008,
    DockCenter = 0x0010,
    DockDesktop= 0x0020,
    DockToSpecialSites=0x0040,
    DockCorner = DockTop | DockLeft | DockRight | DockBottom,
    DockFullSite = DockCorner | DockCenter,
    DockFullDocking = DockFullSite | DockDesktop
  };

  /**
   * This is a key method of this class! Use it to dock dockwidgets to
   * another dockwidget at the right position within its
   * K3DockMainWindow or a toplevel dockwidget.
   *
   *
   * If the target is null, it will become a toplevel dockwidget at position pos;
   * Note: Docking to another dockwidget means exactly:
   * A new parent dockwidget will be created, that replaces the target dockwidget and contains another single helper widget (tab widget or panner)
   * which contains both dockwidgets, this and the target dockwidget. So consider parent<->child relationships change completely during such actions.
   *
   * @param  target The dockwidget to dock to
   * @param  dockPos One of the DockPositions this is going to dock to
   * @param  spliPos The split relation (in percent, or percent*100 in high resolution) between both dockwidgets, target and this
   * @param  pos The dock position, mainly of interest for docking to the desktop (as toplevel dockwidget)
   * @param  check Only for internal use;
   * @param  tabIndex The position index of the tab widget (when in tab page mode), -1 (default) means append
   * @return result The group dockwidget that replaces the target dockwidget and will be grandparent of target and @p this.
   *
   * @note Since KDE 3.5 the splitter position @p spliPos is always a value between [0..100]. If
   *       the value is > 100, it will be treated like the old code and normalized to a value between
   *       0 and 100. Example: If the value was 4000, it will be fixed to 40. In short: The old and
   *       the new behavior both work, so it is compatible with older KDE-versions.
   */
  K3DockWidget* manualDock( K3DockWidget* target, DockPosition dockPos, int spliPos = 50, QPoint pos = QPoint(0,0), bool check = false, int tabIndex = -1);

  /**
   * Specify where it is either possible or impossible for this to dock to another dockwidget.
   *
   * @param pos An OR'ed set of @p DockPositions
   */
  void setEnableDocking( int pos );

  /**
   * @return Where it is either possible or impossible for this to dock to another dockwidget (an OR'ed set of DockPositions).
   */
  int enableDocking() const;

  /**
   * Specify where it is either possible or impossible for another dockwidget to dock to this.
   *
   * @param pos An OR'ed set of @p DockPositions
   */
  void setDockSite( int pos );

  /**
   * @return There it is either possible or impossible for another dockwidget to dock to this (an OR'ed set of @p DockPositions).
   */
  int dockSite() const;

  /**
   * Sets the embedded widget.
   *
   * A QLayout takes care about proper resizing, automatically.
   *
   * @param w The pointer to the dockwidget's child widget.
   */
  void setWidget( QWidget* w);

  /**
   * Get the embedded widget.
   *
   * @return The pointer to the dockwidget's child widget, 0L if there's no such child.
   */
  QWidget* getWidget() const;

  /**
   * Sets the header of this dockwidget.
   *
   * A QLayout takes care about proper resizing, automatically.
   * The header contains the drag panel, the close button and the stay button.
   *
   * @param ah A base class pointer to the dockwidget header
   */
  void setHeader( K3DockWidgetAbstractHeader* ah);

  /**
   * get the pointer to the header widget
   */
  K3DockWidgetAbstractHeader *getHeader();

  /**
   * Normally it simply shows the dockwidget.
   *
   * But additionally, if it is docked to a tab widget (@p DockCenter), it is set as the active (visible) tab page.
   */
  void makeDockVisible();

  /**
   * @return If it may be possible to hide this.
   *
   * There are reasons that it's impossible:
   * @li It is a (tab) group.
   * @li It is already invisible ;-)
   * @li The parent of this is the K3DockMainWindow.
   * @li It isn't able to dock to another widget.
   */
  bool mayBeHide() const;

  /**
   * @return If it may be possible to show this.
   * There are reasons that it's impossible:
   * @li It is a (tab) group.
   * @li It is already visible ;-)
   * @li The parent of this is the @p K3DockMainWindow.
   */
  bool mayBeShow() const;

  /**
   * @return The dockmanager that is responsible for this.
   */
  K3DockManager* dockManager() const;

  /**
   * Stores a string for a tooltip.
   *
   * That tooltip string has only a meaning when this dockwidget is shown as tab page.
   * In this case the tooltip is shown when one holds the mouse cursor on the tab page header.
   * Such tooltip will for instance be useful, if you use only icons there.
   * Note: Setting an empty string switches the tooltip off.
   *
   * @param ttStr A string for the tooltip on the tab.
   */
  void setToolTipString(const QString& ttStr);

  /**
   * @return The tooltip string being shown on the appropriate tab page header when in dock-centered mode.
   */
  const QString& toolTipString() const;

  /**
   * @return result @p true, if a dockback is possible, otherwise @p false.
   */
  bool isDockBackPossible() const;

  /**
   * Sets a string that is used for the label of the tab page when in tab page mode
   * @param label The new tab page label.
   */
  void setTabPageLabel( const QString& label);

  /**
   * @return A string that is used for the label of the tab page when in tab page mode.
   */
  const QString& tabPageLabel() const;

  /**
   * Catches and processes some QWidget events that are interesting for dockwidgets.
   */
  virtual bool event( QEvent * );

  /**
   * Add dockwidget management actions to QWidget::show.
   */
  virtual void show();
  /**
   * @return the parent widget of this if it inherits class K3DockTabGroup
   */
  K3DockTabGroup* parentDockTabGroup() const;

  QWidget *parentDockContainer() const;

#ifndef NO_KDE2

  /**
   * Sets the type of the dock window
   *
   * @param windowType is type of dock window
   */
  void setDockWindowType (NET::WindowType windowType);

#endif

  void setDockWindowTransient (QWidget *parent, bool transientEnabled);

  /**
   * Returns the widget this dockwidget is set transient to, otherwise 0
   */
   QWidget *transientTo();

  /**
   * Lookup the nearest dockwidget docked left/right/top/bottom to this one or return 0
   *
   * @param pos is the position the wanted widget is docked to this one
   */
  K3DockWidget *findNearestDockWidget(DockPosition pos);

  /**
   * Allows changing the pixmap which is used for the caption or dock tabs
   *
   * @param pixmap is the pixmap to set
   */
   void setPixmap(const QPixmap& pixmap=QPixmap());

  /**
   * Returns the dockwidget's associated caption/dock tab pixmap
   */
   const QPixmap& pixmap() const;

  /**
   * @return the current dock position.
   */
  K3DockWidget::DockPosition currentDockPosition() const;

public Q_SLOTS:
  /**
   * subject to changes. It doesn't completely work yet without small hacks from within the calling application (Perhaps
   * KDE 3.1.x oder 3.2
   * width is in pixel. It only affects a widget, which is placed directly into a horizontal K3DockSplitter
   **/
  void setForcedFixedWidth(int);
  /**
   * subject to changes. It doesn't completely work yet without small hacks from within the calling application (Perhaps
   * KDE 3.1.x oder 3.2
   * height is in pixel. It only affects a widget, which is placed directly into a vertical K3DockSplitter
   **/
  void setForcedFixedHeight(int);
  void restoreFromForcedFixedSize();

  int forcedFixedWidth();
  int forcedFixedHeight();

  /**
   * Docks a dockwidget back to the dockwidget that was the neighbor
   widget before the current dock position.
  */
  void dockBack();

  /**
   * Toggles the visibility state of the dockwidget if it is able to be shown or to be hidden.
   */
  void changeHideShowState();

  /**
   * Undocks this. It means it becomes a toplevel widget framed by the system window manager.
   * A small panel at the top of this undocked widget gives the possibility to drag it into
   * another dockwidget by mouse (docking).
   */
  void undock();

  /**
   * Docks the widget to the desktop (as a toplevel widget)
   */
  void toDesktop( );

protected:
  friend class K3MdiMainFrm;
  friend class K3MDI::MainWindow;
  /**
   * Checks some conditions and shows or hides the dockwidget header (drag panel).
   * The header is hidden if:
   * @li the parent widget is the K3DockMainWindow
   * @li this is a (tab) group dockwidget
   * @li it is not able to dock to another dockwidget
   */
  void updateHeader();

  void setLatestK3DockContainer(QWidget *);
  QWidget *latestK3DockContainer();

  void setFormerBrotherDockWidget(K3DockWidget *);

Q_SIGNALS:
  /**
   *is emitted after the setWidget method has finished
   */
   void widgetSet(QWidget*);

  /**
   * Emitted when another dock widget is docking to this.
   *
   * @param dw the dockwidget that is docking to this
   * @param dp the DockPosition where it wants to dock to
   */
  void docking( K3DockWidget* dw, K3DockWidget::DockPosition dp);

  /**
   * Signals that the dock default position is set.
   */
  void setDockDefaultPos();

  /**
   * Emitted when the close button of the panel ( K3DockWidgetHeader) has been clicked.
   */
  void headerCloseButtonClicked();

  /**
   * Emitted when the dockback button of the panel ( K3DockWidgetHeader) has been clicked.
   */
  void headerDockbackButtonClicked();

  /**
   * Emitted when the widget processes a close event.
   */
  void iMBeingClosed();
  /**
   * Emitted when the widget has undocked.
   */
  void hasUndocked();

protected Q_SLOTS:

  /** Does several things here when it has noticed that the former brother widget (closest neighbor) gets lost.
   * The former brother widget is needed for a possible dockback action, to speak with the Beatles:
   * "To get back to where you once belonged" ;-)
   */
  void loseFormerBrotherDockWidget();

  virtual void paintEvent(QPaintEvent*);

  virtual void mousePressEvent(QMouseEvent*);
  virtual void mouseReleaseEvent(QMouseEvent*);
  virtual void mouseMoveEvent(QMouseEvent*);
  virtual void leaveEvent(QEvent*);
protected:
  friend class K3DockWidgetHeader;
  /**
   * earlier closest neighbor widget, so it's possible to dock back to it.
   */
  K3DockWidget* formerBrotherDockWidget;
  /**
   * the current dock position.
   */
  DockPosition currentDockPos;
  /**
   * the former dock position when it really was at another position before.
   */
  DockPosition formerDockPos;
  /**
   * a string used as tooltip for the tab page header when in dock-centered mode.
   */
  QString toolTipStr;
  /**
   * a string used as title of the tab page when in tab page mode
   */
  QString tabPageTitle;

private:
  /**
   * Sets the caption (window title) of the given tab widget.
   *
   * @param g the group (tab) widget
   */
  void setDockTabName( K3DockTabGroup* g);

  /**
   * Reparent to s or set this to the K3DockMainWindow's view if s is that dockmainwindow.
   * If s is O, simply move the widget.
   *
   * @param s the target widget to reparent to
   * @param p the point to move to (if it doesn't reparent)
   */
  void applyToWidget( QWidget* s, const QPoint& p  = QPoint(0,0) );

  /**
   * A base class pointer to the header of this dockwidget
   */
  K3DockWidgetAbstractHeader* header;

  /**
   * the embedded widget
   */
  QWidget* widget;

  /**
   * the layout manager that takes care about proper resizing and moving the embedded widget and the header
   */
  QVBoxLayout* layout;

  /**
   * the responsible dockmanager
   */
  K3DockManager* manager;

  /**
   * an icon for the tab widget header
   */
  QPixmap* pix;

  /**
   * Information about the ability for docking to another dockwidget.
   */
  int eDocking;

  /**
   * Information which site of this dockwidget is free for docking of other dockwidgets.
   */
  int sDocking;

  /**
   * Previous side (left,right,top,bottom) where this dockwidget was before a dragging action, none if it wasn't dragged before.
   */
  K3DockWidget::DockPosition prevSideDockPosBeforeDrag;

  // GROUP data
  QString firstName;
  QString lastName;
  Qt::Orientation splitterOrientation;
  bool isGroup;
  bool isTabGroup;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  K3DockWidgetPrivate *d;
};

/**
 * The manager that knows all dockwidgets and handles the dock process (and member of the dockwidget class set).
 * More or less a helper class for the K3DockWidget class set but of interest for some functionality
 * that can be called within a K3DockMainWindow or a K3DockWidget .
 *
 * An important feature is the ability to read or save the current state of all things concerning to
 * dockwidgets to KConfig .
 *
 * The dockmanager is also often used when a certain dockwidget or a child of such dockwidget must be found.
 *
 * @author Max Judin (documentation: Falk Brettschneider).
 */
class KDE3SUPPORT_EXPORT K3DockManager: public QObject
{
  Q_OBJECT
friend class K3DockWidget;
friend class K3DockMainWindow;

public:
    enum EnReadDockConfigMode {
        Unknown,
        WrapExistingWidgetsOnly,
        RestoreAllDockwidgets
    };

public:
  /**
   * Constructs a dockmanager. Some initialization happen:
   * @li It installs an event filter for the main window,
   * @li a control list for dock objects
   * @li a control list for menu items concerning to menus provided by the dockmanager
   * @li Some state variables are set
   *
   * @param mainWindow the main window controlled by this
   * @param name the internal QOject name
   */
  K3DockManager( QWidget* mainWindow, const char* name = 0L );

  /**
   * Destructs a dockmanager.
   */
  virtual ~K3DockManager();

  void dumpDockWidgets();

#ifndef NO_KDE2
  /**
   * Saves the current state of the dockmanager and of all controlled widgets.
   * State means here to save the geometry, visibility, parents, internal object names, orientation,
   * separator positions, dockwidget-group information, tab widget states (if it is a tab group) and
   * last but not least some necessary things for recovering the dockmainwindow state.
   *
   * @param c the KDE configuration saver
   * @param group the name of the section in KConfig
   */
  void writeConfig( KConfig* c = 0L, const QString &group = QString() );

  /**
   * Like writeConfig but reads the whole stuff in.
   *
   * In order to restore a window configuration
   * from a config file, it looks up widgets by name
   * (QObject::name) in the childDock variable of
   * K3DockManager. This list in turn contains all
   * K3DockWidgets (according to the K3DockWidget constructor).
   * So in principle, in order to restore a window layout,
   * one must first construct all widgets, put each of them in a
   * K3DockWidget and then call readConfig(). And for all that
   * to work, each widget must have a unique name.
   *
   * @param c the KDE configuration saver
   * @param group the name of the section in KConfig
   */
  void readConfig ( KConfig* c = 0L, const QString &group = QString() );
#endif

  void setMainDockWidget2(K3DockWidget *);

  /**
   * Saves the current dock window layout into a DOM tree below the given element.
   */
  void writeConfig(QDomElement &base);
  /**
   * Reads the current dock window layout from a DOM tree below the given element.
   */
  void readConfig(QDomElement &base);

  /**
   * Shows all encapsulated widgets of all controlled dockwidgets and shows all dockwidgets which are
   * parent of a dockwidget tab group.
   */
  void activate();

  /**
   * It's more or less a method that catches several events which are interesting for the dockmanager.
   * Mainly mouse events during the drag process of a dockwidgets are of interest here.
   *
   * @param object the object that sends the event
   * @param event the event
   * @return the return value of the method call of the base class method
   */
  virtual bool eventFilter( QObject * object, QEvent * event );

  /**
   * This method finds out what a widgets' dockwidget is. That means the dockmanager has a look at all
   * dockwidgets it knows and tells you when one of those dockwidgets covers the given widget.
   *
   * @param w any widget that is supposed to be encapsulated by one of the controlled dockwidgets
   * @return the dockwidget that encapsulates that widget, otherwise 0
   */
  K3DockWidget* findWidgetParentDock( QWidget* w) const;

  /**
   * Works like makeDockVisible() but can be called for widgets that covered by a dockwidget.
   *
   * @param w the widget that is encapsulated by a dockwidget that turns to visible.
   */
  void makeWidgetDockVisible( QWidget* w );

  /**
   * @return the popupmenu for showing/hiding dockwidgets
   */
  Q3PopupMenu* dockHideShowMenu() const;

  /**
   * @param dockName an internal QObject name
   * @return the dockwidget that has got that internal QObject name
   */
  K3DockWidget* getDockWidgetFromName( const QString& dockName );

  /**
   * Enables opaque resizing. Opaque resizing defaults to KGlobalSettings::opaqueResize().
   * Call this method before you create any dock widgets!
   */
  void setSplitterOpaqueResize(bool b=true);

  /**
   * Returns true if opaque resizing is enabled, false otherwise.
   */
  bool splitterOpaqueResize() const;

  /**
   * Try to preserve the widget's size. Works like KeepSize resize mode
   * of QSplitter. Off by default.
   * Call this method before you create any dock widgets!
   */
  void setSplitterKeepSize(bool b=true);

  /**
   * Returns true if the KeepSize is enabled, false otherwise.
   */
  bool splitterKeepSize() const;

  /**
   * Operate the splitter with a higher resolution. Off by default.
   * Call this method before you create any dock widgets!
   * If high resolution is used all splitter position parameters
   * are percent*100 instead of percent.
   * @note Since KDE 3.5 this is ignored. Internally the splitter always
   *       calcualtes in high resolution values. For KDE 4, this will be removed.
   */
  void setSplitterHighResolution(bool b=true);

  /**
   * Returns true if the splitter uses the high resolution, false otherwise.
   */
  bool splitterHighResolution() const;

  void setSpecialLeftDockContainer(K3DockWidget* container);
  void setSpecialTopDockContainer(K3DockWidget* container);
  void setSpecialRightDockContainer(K3DockWidget* container);
  void setSpecialBottomDockContainer(K3DockWidget* container);

  void removeFromAutoCreateList(K3DockWidget* pDockWidget);
  void finishReadDockConfig();
  void setReadDockConfigMode(int mode);

Q_SIGNALS:

  /**
   * Signals changes of the docking state of a dockwidget. Usually the dock-toolbar will be updated then.
   */
  void change();

  /**
   * Signals a dockwidget is replaced with another one.
   */
  void replaceDock( K3DockWidget* oldDock, K3DockWidget* newDock );

  /**
   * Signals a dockwidget without parent (toplevel) is shown.
   */
  void setDockDefaultPos( K3DockWidget* );

private Q_SLOTS:

  /**
   * Clears the popupmenu for showing/hiding dockwidgets and fills it with the current states of all controlled dockwidgets.
   */
  void slotMenuPopup();

  /**
   * This method assumes a menuitem of the popupmenu for showing/hiding dockwidgets is selected and toggles that state.
   *
   * @param id the popupmenu id of the selected menuitem
   */
  void slotMenuActivated( int id);

  /* clears the old drawn drag rectangle (oldDragRect) from screen and
   * draws the new current drag rectangle (dragRect) depending on the current mouse position.
   * This highlights the dockwidget which is the currently chosen target during a dock action.
   */
  void drawDragRectangle();

private:

  /**
   * A data structure containing data about every dockwidget that is under control.
   */
  struct MenuDockData
  {
    MenuDockData( K3DockWidget* _dock, bool _hide )
    {
      dock = _dock;
      hide = _hide;
    }
    ~MenuDockData(){}

    K3DockWidget* dock;
    bool hide;
  };

  /**
   * Finds the K3DockWidget at the position given as parameter
   *
   * @param pos global (desktop) position of the wanted dockwidget
   * @return the dockwidget at that position
   */
  K3DockWidget* findDockWidgetAt( const QPoint& pos );

  /**
   * Finds the QWidget recursively at the position given as parameter
   *
   * @param w a variable where the method puts the QWidget at that position (instead of a return value)
   * @param p the parent widget where the recursive search should start from
   * @param pos global (desktop) position of the wanted dockwidget
   */
  void findChildDockWidget( QWidget*& w, const QWidget* p, const QPoint& pos );

  /**
   * Finds all dockwidgets which are child, grandchild and so on of p.
   *
   * @param p the parent widget where the recursive search starts from
   * @param l the widget list that contains the search result after the return of this method
   */
  void findChildDockWidget( const QWidget* p, QWidgetList*& l);

  /**
   * Sets a dockwidget in drag mode.
   */
  void startDrag( K3DockWidget* );

  /**
   * Moves a dockwidget that is in drag mode.
   *
   * @param d the dockwidget which is dragged
   * @param pos the new position of the dragged dockwidget
   */
  void dragMove( K3DockWidget* d, QPoint pos );

  /**
   * Aborts the drag mode. Restores the cursor and hides the drag indicator.
   */
  void cancelDrop();

  /**
   * Finishes the drag mode. If the user let it drop on an other dockwidget, it will possibly be docked (if allowed),
   * if the user drops it outside of the application window it becomes toplevel.
   */
  void drop();

// class members

  /**
   * Usually the K3DockMainWindow but not necessarily.
   */
  QWidget* main;

  /**
   * The dockwidget that is being dragged at the moment
   */
  K3DockWidget* currentDragWidget;

  /**
   * The target dockwidget where the currentDragWidget is dropped
   */
  K3DockWidget* currentMoveWidget; // widget where mouse moving

  /**
   * It is of interest during the dock process. Then it contains all child dockwidgets.
   */
  QWidgetList* childDockWidgetList;

  /**
   * The dockposition where the dockwidget would be docked to, if we dropped it here.
   */
  K3DockWidget::DockPosition curPos;

  /**
   * A QList of all objects that are important for docking.
   * Some serve as group widgets of dockwidgets, others encapsulate normal widgets.
   */
  QObjectList* childDock;

  /**
   * Contains dockwidgets that are created automatically by the dockmanager. For internal use.
   */
  QObjectList* autoCreateDock;

  /**
   * For storing the width during the dragging of a dockwidget.
   */
  int storeW;

  /**
   *  For storing the height during the dragging of a dockwidget.
   */
  int storeH;

  /**
   * State variable if there is a drag process active.
   */
  bool dragging;

  /**
   * State variable if there is an undock process active
   */
  bool undockProcess;

  /**
   * The dockmanager sets it to true if the user cancels the drag by moving the cursor
   * on a invalid drop place
   */
  bool dropCancel;

  /**
   * A popup menu that contains one menuitem for each dockwidget that shows the current visibility state and
   * to show or hide the appropriate dockwidget.
   */
  Q3PopupMenu* menu;

  /**
   * An internal list containing data for the menuitems for the visibility popup menu.
   */
  Q3PtrList<MenuDockData> *menuData;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class K3DockManagerPrivate;
  K3DockManagerPrivate *d;
};

/**
 * A special kind of KMainWindow that is able to have dockwidget child widgets (and member of the dockwidget class set).
 *
 * The main widget should be a K3DockWidget where other K3DockWidget can be docked to
 * the left, right, top, bottom or to the middle.
 * Note: dock to the middle means to drop on a dockwidget and to unite them to a new widget, a tab control.
 *
 * Furthermore, the K3DockMainWindow has got the K3DockManager and some data about the dock states.
 *
 * If you've got some dockwidgets, you can dock them to the dockmainwindow to initialize a start scene:
 * Here an example:
 * \code
 * DockApplication::DockApplication( const char* name) : K3DockMainWindow( name)
 * {
 *   ...
 *   K3DockWidget* mainDock;
 *   mainDock = createDockWidget( "Falk's MainDockWidget", mainPixmap, 0L, "main_dock_widget");
 *   AnyContentsWidget* cw = new AnyContentsWidget( mainDock);
 *   mainDock->setWidget( cw);
 *   // allow others to dock to the 4 sides
 *   mainDock->setDockSite(K3DockWidget::DockCorner);
 *   // forbit docking abilities of mainDock itself
 *   mainDock->setEnableDocking(K3DockWidget::DockNone);
 *   setView( mainDock); // central widget in a KDE mainwindow
 *   setMainDockWidget( mainDock); // master dockwidget
 *   ...
 *   K3DockWidget* dockLeft;
 *   dockLeft = createDockWidget( "Intially left one", anyOtherPixmap, 0L, i18n("The left dockwidget"));
 *   AnotherWidget* aw = new AnotherWidget( dockLeft);
 *   dockLeft->setWidget( aw);
 *   dockLeft->manualDock( mainDock,              // dock target
 *                         K3DockWidget::DockLeft, // dock site
 *                         20 );                  // relation target/this (in percent)
 *   ...
 * \endcode
 *
 * Docking is fully dynamic at runtime. That means you can always move dockwidgets via drag and drop.
 *
 * And last but not least you can use the popupmenu for showing or hiding any controlled dockwidget
 * of this class and insert it to your main menu bar or anywhere else.
 *
 * @author Max Judin (documentation: Falk Brettschneider).
 */
class KDE3SUPPORT_EXPORT K3DockMainWindow : public KXmlGuiWindow
{
  Q_OBJECT

friend class K3DockManager;

public:

  /**
   * Constructs a dockmainwindow. It calls its base class constructor and does additional things concerning
   * to the dock stuff:
   * @li information about the dock state of this' children gets initialized
   * @li a dockmanager is created...
   * @li ...and gets initialized
   * @li the main dockwidget is set to 0
   *
   * @param parent Parent widget for the dock main widget
   * @param name internal object name
   * @param f Qt::WidgetFlags widget flags
   */
  K3DockMainWindow( QWidget* parent = 0L, const char *name = 0L, Qt::WFlags f = Qt::Window);

  /**
   * Destructs a dockmainwindow.
   */
  virtual ~K3DockMainWindow();

  /**
   * Returns the dockmanager of this. (see K3DockManager)
   * @return pointer to the wanted dockmanager
   */
  K3DockManager* manager() const;

  /**
   * Sets a new main dockwidget.
   * Additionally, the toolbar is re-initialized.
   *
   * @param dockwidget dockwidget that become the new main dockwidget
   */
  void setMainDockWidget( K3DockWidget* dockwidget);

  /**
   * Returns the main dockwidget.
   *
   * @return pointer to the main dockwidget
   */
  K3DockWidget* getMainDockWidget() const;

  /**
   * This is one of the most important methods!
   * The K3DockMainWindow creates a new dockwidget object here that usually should encapsulate the user's widget.
   * The new dockwidget is automatically taken under control by the dockmanager of the dockmainwindow.
   *
   * @param name   QObject name (default dockwidget caption)
   * @param pixmap window icon (for instance shown when docked as tabwidget entry)
   * @param parent parent widget for the new dockwidget
   * @param strCaption  window title (shown when toplevel)
   * @param strTabPageLabel title of the tab page (visible when in tab page mode), if it is "", only the icon will be shown; if it is 0L, the label is set to strCaption
   * @return    a pointer to the new created dockwidget
   */
  K3DockWidget* createDockWidget( const QString& name, const QPixmap &pixmap, QWidget* parent = 0L,
    const QString& strCaption = QString(), const QString& strTabPageLabel = QLatin1String( " " ) );

  /**
   * Saves the current dock window layout into a DOM tree below the given element.
   */
  void writeDockConfig(QDomElement &base);
  /**
   * Reads the current dock window layout from a DOM tree below the given element.
   */
  void readDockConfig(QDomElement &base);

#ifndef NO_KDE2
  /**
   * It writes the current dock state in the given section of KConfig.
   *
   * @param c     KDE class for saving configurations
   * @param group name of section to write to
   */
  void writeDockConfig( KConfig* c = 0L, const QString &group = QString() );

  /**
   * It reads the current dock state from the given section of KConfig.
   *
   * @param c     KDE class for saving configurations
   * @param group name of section to read from
   */
  void readDockConfig ( KConfig* c = 0L, const QString &group = QString() );
#endif

  /**
   * It runs through all dockwidgets which are under control of the dockmanager and calls show() for every
   * encapsulated widget and show() for the dockwidget itself if it is not in tab mode.
   * Additionally, if the main dockwidget is not a QDialog, it will be shown.
   */
  void activateDock();

  /**
   * Returns a popup menu that contains entries for all controlled dockwidgets making hiding and showing
   * them possible.
   *
   * @return the wanted popup menu
   */
  Q3PopupMenu* dockHideShowMenu() const;

  /**
   * This method shows the given dockwidget.
   * The clue is that it also considers the dockwidget could be a tab page
   * and must set to be the activate one.
   *
   * @param dock the dockwidget that is to be shown
   */
  void makeDockVisible( K3DockWidget* dock );

  /**
   * This method hides the given dockwidget.
   *
   * @param dock the dockwidget that is to be shown
   */
  void makeDockInvisible( K3DockWidget* dock );

  /**
   * This is an overloaded member function, provided for convenience.
   * It differs from the above function only in what argument(s) it accepts.
   */
  void makeWidgetDockVisible( QWidget* widget );

  /**
   * This method calls the base class method.
   * If the given widget inherits K3DockWidget, applyToWidget(this) is called.
   *
   * @param widget any widget that should become the main view
   */
  void setView( QWidget * widget );

Q_SIGNALS:
  /**
  * Signals a certain dockwidget is undocked now.
  */
  void dockWidgetHasUndocked(K3DockWidget*);

protected:

  /**
   * A pointer to the main dockwidget (where one can manualDock() to
   */
  K3DockWidget* mainDockWidget;

  /**
   * A pointer to the manager for the dock process
   */
  K3DockManager* dockManager;

protected Q_SLOTS:
  /**
  * Called whenever one of the dockwidgets of this has been undocked.
  */
  void slotDockWidgetUndocked();

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class K3DockMainWindowPrivate;
  K3DockMainWindowPrivate *d;
};

class KDE3SUPPORT_EXPORT K3DockArea : public QWidget
{
  Q_OBJECT

friend class K3DockManager;

public:


  K3DockArea( QWidget* parent = 0L, const char *name = 0L);

  virtual ~K3DockArea();

  K3DockManager* manager();


  void setMainDockWidget( K3DockWidget* );
  K3DockWidget* getMainDockWidget();

  K3DockWidget* createDockWidget( const QString& name, const QPixmap &pixmap, QWidget* parent = 0L,
    const QString& strCaption = QString(), const QString& strTabPageLabel = QLatin1String( " " ) );

  void writeDockConfig(QDomElement &base);
  void readDockConfig(QDomElement &base);

#ifndef NO_KDE2
  void writeDockConfig( KConfig* c = 0L, const QString &group = QString() );
  void readDockConfig ( KConfig* c = 0L, const QString &group = QString() );
#endif



  void activateDock();
  Q3PopupMenu* dockHideShowMenu();
  void makeDockVisible( K3DockWidget* dock );
  void makeDockInvisible( K3DockWidget* dock );
  void makeWidgetDockVisible( QWidget* widget );
  //void setView( QWidget* );

Q_SIGNALS:
  /**
  * Signals a certain dockwidget is undocked now.
  */
  void dockWidgetHasUndocked(K3DockWidget*);

protected:

  K3DockWidget* mainDockWidget;
  K3DockManager* dockManager;

protected Q_SLOTS:
  void slotDockWidgetUndocked();

public:
	virtual void resizeEvent(QResizeEvent *);

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class K3DockMainWindowPrivate;
  K3DockMainWindowPrivate *d;
};


#endif


