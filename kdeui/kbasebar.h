/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Sven Radej (radej@kde.org)
              (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
              (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
              
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    */

// Do not add this to makefiles or stuff cuz it is unused. I commited this
// so I can track changes and revisions. If you want to try this, mail me
// and I'll send you my working files.

// KBaseBar is/will be a base class for KToolBar and KToolBar. In future
// KToolBar and KMenuBar and (future) KToolBox will inherit this class.
// This is not yet finished and present things do not depend on it
// Expect big changes here. Most of stuff comes from KToolBar,
// So it inherits it's copyrights.
// New stuff is: buttons with popups, delayed popups and smarter
// management (?)
// This notice will be removed later.

// $Id$
// $Log$

#ifndef _KBASEBAR_H
#define _KBASEBAR_H

#include <qlist.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qpopmenu.h>
#include <qbutton.h>
#include <qfont.h>
#include <qsize.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

class KBaseBar;
class KToolBoxManager;

#define Item QWidget

enum itemType {
    ITEM_LINED = 0,
    ITEM_BUTTON = 1,
    ITEM_COMBO = 2,
    ITEM_FRAME = 3,
    ITEM_TOGGLE = 4,
    ITEM_ANYWIDGET=5
};


/**
 * This is a generic item for use in KMenuBar and in KToolBar
 */

class KBaseBarItem
{
public:
  KBaseBarItem (Item *_item, itemType _type, int _id,
                bool _myItem=true);
  ~KBaseBarItem ();
    
  void resize (int w, int h) { item->resize(w, h); };
  void move(int x, int y) { item->move(x, y); };
  void show () { item->show(); };
  void hide () { item->hide(); };
  void setEnabled (bool enable) { item->setEnabled(enable); };
  bool isEnabled () { return item->isEnabled(); };
  int ID() { return id; };
  void setID(int newId) {id == newId;};
  bool isRight () { return right; };
  void alignRight  (bool flag) { right = flag; };
  void autoSize (bool flag) { autoSized = flag; };
  bool isAuto ()  { return autoSized; };
  int width() { return item->width(); };
  int height() { return item->height(); };
  int x() { return item->x(); };
  int y() { return item->y(); };
  int winId () { return item->winId(); };
  
  Item *getItem() { return item; };
  
private:
  int id;
  bool right;
  bool autoSized;
  Item *item;
  itemType type;
  bool myItem;
};

/**
 * This is internal button for use in KMenuBar and in KToolBar
 */

class KBaseBarButton : public QButton
 {
   Q_OBJECT

 public:
   KBaseBarButton(const QPixmap& pixmap, int id, QWidget *parent,
                  const char *name=0L, int item_size = 26, const char *txt=0,
                  bool _mb = false);
   KBaseBarButton(QWidget *parent=0L, const char *name=0L);
   ~KBaseBarButton() {};
   void setEnabled(bool enable);
   void makeDisabledPixmap();
   
   virtual void setPixmap( const QPixmap & );
   virtual void setText ( const char *text);
   void on(bool flag);
   void toggle();
   void beToggle(bool);
   bool ImASeparator () {return sep;};
   void youreSeparator () {sep = true;};
   QPopupMenu *popup () {return myPopup;};
   void setPopup (QPopupMenu *p);
   void setDelayedPopup (QPopupMenu *p);
   
 public slots:
   void modeChange();
   
 protected:
   void paletteChange(const QPalette &);
   void leaveEvent(QEvent *e);
   void enterEvent(QEvent *e);
   void drawButton(QPainter *p);
   bool eventFilter (QObject *o, QEvent *e);
   void showMenu();
     
 private:
   bool toolBarButton;
   bool sep;
   QPixmap enabledPixmap;
   QPixmap disabledPixmap;
   int icontext;
   int highlight;
   bool raised;
   int id;
   int _size;
   KBaseBar *parentWidget;
   QString btext;
   QFont buttonFont;
   QPopupMenu *myPopup;
   bool delayPopup;
   QTimer *delayTimer;
   
 protected slots:
     void ButtonClicked();
     void ButtonPressed();
     void ButtonReleased();
     void ButtonToggled();
     void slotDelayTimeout();

 signals:
     void clicked(int);
     void pressed(int);
     void released(int);
     void toggled(int);
 };

/**
 * KBaseBar is a self resizing, floatable widget.
 * It is usually managed from KTopLevelWidget, but can be
 * used even if you don't use KTopLevelWidget. If you want
 * to handle this without KTopLevelWidget, see updateRects .<BR>
 * KBaseBar is a base class for @ref KToolBar and @ref KMenuBar and is
 * useless unless you want to make some other *Bar.
 * @short KDE Toolbar widget
 * @author Stephan Kullow <coolo@kde.org> Maintained by Sven Radej <radej@kde.org>
 */
 class KBaseBar : public QFrame
  {

  Q_OBJECT

      friend class KBaseBarButton; // this friend has a class
  
public:
  enum BarStatus{Toggle, Show, Hide};
  enum BarPosition{Top, Left, Bottom, Right, Floating};

  /**
   * Constructor.
   * Bar will read global-config file for intem Size higlight
   * option and button type.
   */
  KBaseBar(QWidget *parent=0L, const char *name=0L, int _item_size = -1);

  /**
   * Destructor. If bar is floating it will cleanup itself.
   * You MUST delete toolbar before exiting. (well I'm a bit confused
   * 'bout that...)
   */
  virtual ~KBaseBar();

  /**
   * Inserts KButton with pixmap. You should connect to one or more signals in
   * KBaseBar: @ref #clicked , @ref #pressed , @ref #released , and
   * if toolbar is toggle button (@ref #setToggle ) @ref #toggled . Those
   * signals have id of a button that caused the signal.
   * If you want to bound an popup to button, see  @ref #setButton
   * @param index the position of the button. (-1 = at end).
   * @return Returns item index
   */
  int insertButton(const QPixmap& pixmap, int ID, bool enabled = true,
                   const char *ToolTipText = 0L, int index=-1 );
  /**
   * This is the same as above, but with specified signals and
   * slots to which this button will be connected. Button emits
   * signals pressed, clicked and released, and
   * if toolbar is toggle button ( @ref #setToggle ) @ref #toggled .
   * You can add more signals with @ref #addConnection .
   * If you want to bound an popup to button  @ref #setButton
   * @return Returns item index
   */
  int insertButton(const QPixmap& pixmap, int ID, const char *signal,
                   const QObject *receiver, const char *slot,
                   bool enabled = true,
                   const char *tooltiptext = 0L, int index=-1 );

  /**
   * This inserts a button with popupmenu. If button is in toolbar a small
   * trialngle will be drawn. You have to connect to popup's signals. The
   * signals pressed, released or clikced are NOT emmited by this button
   * (see @ref #setDelayedPopup for that).
   * You can add custom popups which inherit @ref QPopupMenu to get popups
   * with tables, drawings etc. Just don't fiddle with events there.
   */
  int insertButton(const QPixmap& pixmap, int id, QPopupMenu *popup,
                   bool enabled, const char *_text, int index=-1);


  /**
   * Insert separator
   */
  virtual int insertSeparator(int index=-1);

  /**
   * This adds connection to items. Therefore it is important that you
   * know id of particular item. Nothing happens if you miss id.
   */
  void addConnection (int id, const char *signal,
                      const QObject *receiver, const char *slot);
  /**
   * Enables/disables item.
   */
  void setItemEnabled( int id, bool enabled );

  /**
   * True if item is enabled..
   */
  bool isItemEnabled( int id );

  
  /**
   * Sets button pixmap.
   * Can be used while button is visible.
   */
  void setButtonPixmap( int id, const QPixmap& _pixmap);

  /**
   * Sets button text (tooltip text for icontext=0).
   * Can be used while button is visible.
   */
  void setButtonText(int id, const char *text);
  /**
   * Sets delayed popup to a button. Delayed popup is what you see in
   * netscape's Previous&next buttons: if you click them you go back,
   * or forth. If you press them long enough, you get a history-menu.
   * This is exactly what we do here. <BR>
   * You will insert normal button with connection (or use signals from
   * toolbar):
   * <pre>
   * bar->insertButton(pixmap, id, const SIGNAL(clicked ()), this,
   *     		SLOT (slotClick()), true, "click or wait for popup");
   * </pre> And then add a delayed popup:
   * <pre>
   * bar->setDelayedPopup (id, historyPopup); </pre>
   *
   * Don't add delayed popups to buttons which have normal popups.
   *
   * You may add popups wich are derived from popupMenu.
   */
  void setDelayedPopup (int id , QPopupMenu *_popup);
  
  /**
   * Makes button a toggle button if flag is true
   */
  void setToggle (int id, bool flag = true);

  /**
   * If button is toggle (@ref #setToggle must be called first)
   * button state will be toggled. This will also cause toolbar to
   * emit signal @ref #toggled wit parameter id. You must connect to
   * this signal, or use @ref #addConnection to connect directly to
   * button-signal toggled.
   */
  void toggleButton (int id);

  /**
   * If button is toggle (@ref #setToggle must be called first)
   * this will set him to state flag. This will also emit signal
   * #ref toggled . <BR>
   * If button is not toggle, calling with flag = false will
   * unhighlight this button.
   * You will want to do this if you want buttons to have popups, but
   * you now can add a button with popup directly with @ref #insertButton .
   *
   * @see #setToggle
   */
  void setButton (int id, bool flag);

  /**
   * Returns true if button is on, false if button is off.
   * If button is not a toggle button, or not button at all
   * returns false
   * @see #setToggle
   */
  bool isButtonOn (int id);
  
  /**
   * This returns a pointer to KBaseBarButton. Example:
   * <pre>
   * KButton * button = toolbar->getButton(button_id);
   * </pre>
   * That way you can get access to other public methods
   * that @ref KButton provides.
   */  
  KBaseBarButton * getButton (int id);

  /**
   * Alignes item right.
   * This works only if toolbar is set to full width.
   * @see #setFullWidth
   */
  void alignItemRight (int id, bool right = true);

  /**
   * Sets item autosized. This works only if toolbar is set to full width.
   * ONLY ONE item can be autosized, and it has to be
   * the last left-aligned item. Items that come after this must be right
   * aligned. Items that can be right aligned are Lineds, Frames, Widgets and
   * Combos. Auto sized item will resize itself whenever toolbar geometry
   * changes, to last right-aligned item (or till end of toolbar if there
   * are no right aligned items
   * @see #setFullWidth
   * @see #alignItemRight
   */
  void setItemAutoSized (int id, bool yes = true);

  /**
   * Removes item id.
   * Item is deleted. Toolbar is redrawn after it.
   */
  void removeItem (int id);

  /**
   * Clears the whole bar. Bar is redrawn after it.
   */
  void clear ();

  /**
   * Sets accel for item id.
   */
  void setAccel (int key, int id);

  /**
   * Returns accel for item id.
   */
  int accel (int id);
  
  
  /**
   * Hides item.
   */
  void hideItem (int id);

  /**
   * shows item.
   */
  void showItem (int id);

  
  /**
   * Sets toolbar to full parent width (or to value set by setMaxWidth).
   * You have to call this function if you want to have right aligned items or
   * autosized item. <BR>
   * The toolbar is set to full width by default.
   * @see #alignItemRight
   * @see #setItemAutoSized
   */
  void setFullWidth(bool flag = true);    // Top and Bottom pos only

  /**
   * Enables or disables moving of toolbar.
   */
  void enableMoving(bool flag = true);

  /**
   * Sets position of toolbar
   * @see #BarPosition
   */
  void setBarPos (BarPosition bpos);

  /**
   * Returns position of toolbar
   */
  BarPosition barPos() {return position;};

  /**
   * This shows, hides, or toggles toolbar. If toolbar floats,
   * hiding means minimizing.
   * @see #BarStatus
   */
  bool enable(BarStatus stat);

  /**
   * Sets maximal height of vertical (Right or Left) toolbar. You normaly
   * do not have to call it, since it's called from
   * @ref KTopLevelWidget#updateRects
   * If you reimplement @ref KTopLevelWidget#resizeEvent or
   * KTopLevelWidget#updateRects,
   * be sure to call this function with maximal height toolbar can have.
   * @see #updateRects
   */
  void setMaxHeight (int h);  // Set max height for vertical toolbars

  /**
   * Sets maximal width of horizontal (top or bottom) toolbar. This works
   * only for horizontal toolbars (at Top or Bottom), and has no effect
   * otherwise. Has no effect when toolbar is floating.
   */
  void setMaxWidth (int dw);

  /**
   * Sets title for toolbar when it floats.
   * You can't change toolbar's title while it's floating.
   */
  void setTitle (const char *_title) {title = _title;};

  /**
   * Enables or disables floating.
   * Floating is enabled by default.
   * This only disables menu entry Floating in popup menu, so
   * toolbar can still be moved by @ref #setBarPos or by dragging.
   * This function is obsolete and do not use it. If you want to make
   * toolbar static use @ref enableMoving
   */
  void enableFloating (bool arrrrrrgh);

  /**
   * Redraw toolbar and resize it if resize is true.
   * You normaly don't have to call it, since it's called from
   * @ref KTopLevelWidget#updateRects or from resizeEvent. You can call it
   * if you manualy change width of inserted frame, or if you wish to force
   * toolbar to recalculate itself. <BR>
   * You don't want to fiddle with this.
   * @ref KtopLevelWidget works closely with toolbar. If you want to
   * subclass KTopLevelWidget to change its resize policy, hear this: <BR>
   * <BR>
   * resizeEvent() in KTopLevelWidget just calls updateRects, which handles
   * children sizes. Call updateRects when you're done with your things. <BR>
   * <BR>
   * If you want to handle everything yourself:<BR>
   * <BR>
   * KBaseBar manages itself by calling toolbar->@ref #updateRects (true).
   * It will autosize itself, but won't move itself.
   * You have to do the moving. <BR>
   * First setup & move anything that is above toolbars (menus...). Then
   * setup statusbars and other horizontal things on bottom. Then loop through
   * all HORIZONTAL toolbars, call their updateRects(true), _then_ take their
   * size, an move them (note that they size themselves according to parent
   * width()). After  you have looped through HORIZONTAL toolbars, calculate
   * the maximum height that vertical toolbars may have (this is your free
   * area height). Then loop through vertical toolbars,
   * @ref #setMaxHeight (calculated_max_height) on them,
   * call their updateRects(true), and _then_ move them to their locations.
   * @see KTopLevelWidget::updateRects
   */
  virtual void updateRects(bool resize = false);

  /**
     * Returns minimal width for top-level window, so that toolbar
     * has only one row.
     */
  QSize sizeHint();

  /**
   * Just returns height the bar would have if it had width w.
   */
  int heightForWidth (int w);
  
  /**
   * Just returns width the bar would have if it had height h.
   */
  int widthForHeight (int h);
  
signals:
    /**
     * Emits when button id is clicked.
     */
    void clicked(int id);

    /**
     * Emits when button id is pressed. See @ref setButton for binding
     * popups to buttons.
     */
    void pressed(int);

    /**
     * Emits when button id is released.
     */
    void released(int);

    /**
     * Emits when toggle button changes state
     * Emits also if you change state
     * with @ref #setButton or @ref #toggleButton
     * If you make a button normal again, with
     * @ref #setToggle (false), this signal won't
     * be emited.
     */
    void toggled(int);
    
    /**
     * Emits when toolbar changes its position, or when
     * item is removed from toolbar. This is normaly connected to
     * @ref KTopLevelWidget::updateRects.
     * If you subclass @ref KTopLevelWidget and reimplement
     * @ref KTopLevelWidget::resizeEvent or
     * @ref KTopLevelWidget::updateRects, be sure to connect to
     * this signal. You can connect this signal to slot that
     * doesn't take parameter.
     * @see #updateRects
     */
    void moved( BarPosition );

    /**
     * Internal. This signal is emited when toolbar detects changing of
     * following parameters:
     * highlighting, button-size, button-mode. This signal is
     * internal, aimed to buttons.
     */
    void modechange ();

protected:
    
  QList<KBaseBarItem> items;
  bool wasFullWidth;  // Was I loong when I was?
  bool haveAutoSized; // Do I have a problem?
  bool fullWidth;
  const char *title;
  BarPosition position;
  
private:
  
  bool moving;
  QWidget *Parent;
  int toolbarWidth;
  int toolbarHeight;

  int oldX;
  int oldY;
  int oldWFlags;
  
  int max_width;
  int max_height;
  
  BarPosition lastPosition; // Where was I last time I was?
  BarPosition movePos;      // Where was I moved to?
  bool mouseEntered;  // Did the mouse touch the cheese?
  bool horizontal;    // Do I stand tall?
  bool localResize;   // Am I trying to understand recursion?
  
  KToolBoxManager *mgr;
  
protected:
  QPopupMenu *context;

  void drawContents ( QPainter *);
  void resizeEvent(QResizeEvent*);
  void paintEvent(QPaintEvent*);
  void closeEvent (QCloseEvent *);
  void mousePressEvent ( QMouseEvent *);
  void init();
  void layoutVertical ();
  void layoutHorizontal ();
  void leaveEvent (QEvent *e);
  
protected slots:
  void ButtonClicked(int);
  void ButtonPressed(int);
  void ButtonReleased(int);
  void ButtonToggled(int);
  void ContextCallback(int);
  void slotReadConfig ();
  void slotHotSpot (int i);
  
protected:
  void mouseMoveEvent(QMouseEvent*);
  void mouseReleaseEvent ( QMouseEvent *);

  int item_size;  // normal: 26
  int icon_text;  // 1 = icon+text, 0 icon+tooltip
  bool fixed_size; // do not change the toolbar size
  
private:
   QPoint pointerOffset;
   QPoint parentOffset;
   bool highlight; // yes/no
   QSize szh;      // Size for sizeHint
   bool transparent; // type of moving
};
#endif
