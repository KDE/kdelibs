/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Sven Radej (sven@lisa.exp.univie.ac.at)
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

#ifndef _KTOOLBAR_H
#define _KTOOLBAR_H

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

//#include "kbutton.h"
#include "kcombo.h"
#include "klined.h"

class KToolBar;

#define Item QWidget

enum itemType {
    ITEM_LINED = 0,
    ITEM_BUTTON = 1,
    ITEM_COMBO = 2,
    ITEM_FRAME = 3,
    ITEM_TOGGLE = 4,
    ITEM_ANYWIDGET=5
};

class KToolBarItem
{
public:
  KToolBarItem (Item *_item, itemType _type, int _id,
                bool _myItem=true);
  ~KToolBarItem ();
    
  void resize (int w, int h) { item->resize(w, h); };
  void move(int x, int y) { item->move(x, y); };
  void show () { item->show(); };
  void hide () { item->hide(); };
  void setEnabled (bool enable) { item->setEnabled(enable); };
  bool isEnabled () { return item->isEnabled(); };
  int ID() { return id; };
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

class KToolBarButton : public QButton
 {
   Q_OBJECT

 public:
   KToolBarButton(const QPixmap& pixmap, int id, QWidget *parent,
                  const char *name=0L, int item_size = 26, const char *txt=0);
   KToolBarButton(QWidget *parent=0L, const char *name=0L);
   ~KToolBarButton() {};
   void setEnabled(bool enable);
   void makeDisabledPixmap();
   
   void setPixmap( const QPixmap & );
   void on(bool flag);
   void toggle();
   void beToggle(bool);
   bool ImASeparator () {return sep;};
   void youreSeparator () {sep = true;};

 public slots:
   void modeChange();
   
 protected:
   void paletteChange(const QPalette &);
   void leaveEvent(QEvent *e);
   void enterEvent(QEvent *e);
   void drawButton(QPainter *p);
   
 private:
   bool sep;
   QPixmap enabledPixmap;
   QPixmap disabledPixmap;
   int icontext;
   int highlight;
   bool raised;
   int id;
   int _size;
   KToolBar *parentWidget;
   QString btext;
   QFont buttonFont;
   
   protected slots:
     void ButtonClicked();
     void ButtonPressed();
     void ButtonReleased();
     void ButtonToggled();

 signals:
     void clicked(int);
     void pressed(int);
     void released(int);
     void toggled(int);
 };

/**
 * KToolBar is a self resizing, floatable widget.
 * It is usually managed from KTopLevelWidget, but can be
 * used even if you don't use KTopLevelWidget. If you want
 * to handle this without KTopLevelWidget, see updateRects .<BR>
 * KToolBar can contain buttons Line inputs Combo Boxes, frames, and
 * custom widgets.
 * Combos, Frames and Lineds, and Widgets can be  autosized to full width.
 * Any Item can be right aligned, and buttons can be toggle buttons. Item
 * height,  type of buttons (icon or icon+text), and option for highlighting
 * is adjustable on constructor invocation by reading global config file.
 * Toolbar will reread config-file when it recieves signal
 * Kapplication:appearanceChanged.
 * Toolbar can float, be dragged from and docked back to parent window.
 * It autoresizes itself. This may lead to
 * some flickering, but there is no way to solve it (as far as I
 * know). <BR>
 * If you want to bind popups to buttons, see setButton(). <BR>
 * You normaly use toolbar from subclassed KTopLevelWidget. When
 * you create toolbar object, insert items that you want to be in it.
 * Items can be inserted or removed ( removeItem() ) later, when toolbar
 * is displayed. It will update itself.
 * Then set their propperties ( alignItemRight , setItemAutoSized ,
 * setToggle ...) After that set the toolbar itself, enable ,setBarPos ...).
 * Then simply do addToolbar (toolbar),
 * and you're on. See how it's done in kwindowtest.
 * @short KDE Toolbar widget
 * @author Stephan Kullow <coolo@kde.org> Maintained by Sven Radej <sven@lisa.exp.univie.ac.at>
 */
 class KToolBar : public QFrame
  {

  Q_OBJECT

  friend class KToolBarButton;
public:
  enum BarStatus{Toggle, Show, Hide};
  enum BarPosition{Top, Left, Bottom, Right, Floating};

  /**
   * Constructor.
   * Toolbar will read global-config file for intem Size higlight
   * option and button type.
   */
  KToolBar(QWidget *parent=0L, const char *name=0L, int _item_size = -1);

  /**
   * Destructor. If toolbar is floating it will cleanup itself.
   * You MUST delete toolbar before exiting.
   */
  virtual ~KToolBar();

  /**
   * Inserts KButton with pixmap. You should connect to one or more signals in
   * KToolBar: @ref #clicked , @ref #pressed , @ref #released , and
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
   * Inserts a KLined. You have to specify signals and slots to
   * which KLined will be connected. KLined has all slots QLineEdit
   * has, plus signals @ref KLined::completion and @ref KLined::rotation
   * KLined can be set to autoresize itself to full free width
   * in toolbar, that is to last right aligned item. For that,
   * toolbar must be set to full width.
   * @see #setFullWidth
   * @see #setItemAutoSized
   * @see KLined
   * @return Returns item index
   */
  int insertLined (const char *text, int ID,
                   const char *signal,
                   const QObject *receiver, const char *slot,
                   bool enabled = true,
                   const char *toolTipText = 0L, int size = 70, int index =-1);

  /**
   * Inserts KComboBox with list. Can be writable, but cannot contain pixmaps. By
   * default inserting policy is AtBottom, i.e. typed items are placed at the bottom
   * of the list. Can be autosized
   * @see #setFullWidth
   * @see #setItemAutoSized
   * @see KCombo
   * @return Returns item index
   */
  int insertCombo (QStrList *list, int id, bool writable,
                   const char *signal, QObject *recevier,
                   const char *slot, bool enabled=true,
                   const char *tooltiptext=0L,
                   int size=70, int index=-1,
                   KCombo::Policy policy = KCombo::AtBottom);

  /**
   * Inserts KCombo with text. The rest is the same as above.
   * @see #setItemAutoSized
   * @see KCombo
   * @return Returns item index
   */
  int insertCombo (const char *text, int id, bool writable,
                   const char *signal, QObject *recevier,
                   const char *slot, bool enabled=true,
                   const char *tooltiptext=0L,
                   int size=70, int index=-1,
                   KCombo::Policy policy = KCombo::AtBottom);
  /**
   * Insert separator
   */
  int insertSeparator(int index=-1);

  /**
   * This function is deprecated and will be removed. Use @ref #insertWidget
   * to insert anything.
   * Inserts frame with specified width. If _frame is not specified,
   * it will be created. In that case you must not delete it. You can get
   * pointer to this frame with @ref #getFrame
   * Frame can be autosized to full width.
   * @see #setItemAutoSized
   * @return Returns item index
   */
  int insertFrame(int id, int width, QFrame *_frame=0, int index =-1);

  /**
   * Insert a user defined widget. Widget must have a QWidget for
   * base class.
   * Widget can be autosized to full width. If you forget about it, you
   * can get pointer to this widget with @ref #getWidget .
   * @see #setItemAutoSized
   * @return Returns item index
   */
  int insertWidget(int id, int width, QWidget *_widget, int index=-1);

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
   * Sets button pixmap.
   * Can be used while button is visible.
   */
  void setButtonPixmap( int id, const QPixmap& _pixmap );

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
   * You will want to do this if you want buttons to have popups.
   * This is what you do: <BR>
   * - Connect to signal @ref #pressed <BR>
   * - In slot, before you activate popup, call setButton(id, false); <BR>
   * - activate your popup <BR>
   * Example:
   * <pre>
   * toolbar->insertButton(pixmap, 1, SIGNAL(pressed()),
   *                this, SLOT(slotPopup()), true,
   *                "Press this to popup");
   * ...
   * class::slotPopup()
   * {
   *  toolbar->setButton(1, false)  // this turns the button off
   *  myPopupMenu->show();
   * }
   * </pre>
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
   * Sets text in Lined.
   * Cursor is set at end of text.
   */
  void setLinedText (int id, const char *text);

  /**
   * Returns Lined text.
   * If you want to store this text, you have o copy it somwhere
   */
  const char *getLinedText (int id);

  /**
   * Inserts text in combo id with at position index.
   */
  void insertComboItem (int id, const char *text, int index);

  /**
   * Inserts list in combo id at position index
   */
  void insertComboList (int id, QStrList *list, int index);

  /**
   * Removes item index from Combo id.
   */
  void removeComboItem (int id, int index);

  /**
   * Sets item index to be current item in Combo id.
   */
  void setCurrentComboItem (int id, int index);

  /**
   * Changes item index in Combo id to text.
   * index = -1 means current item (one displayed in the button).
   */
  void changeComboItem  (int id, const char *text, int index=-1);

  /**
   * Clears combo id.
   * Does not delete it or hide it.
   */
  void clearCombo (int id);

  /**
   * Returns text of item index from Combo id.
   * index = -1 means current item
   */

  const char *getComboItem (int id, int index=-1);

  /**
   * This returns pointer to Combo. Example:
   * <pre>
   * KCombo *combo = toolbar->getCombo(combo_id);
   * </pre>
   * That way you can get access to other public methods
   * that @ref KCombo provides. @ref KCombo is KDE enhancement
   * of @ref QComboBox plus two signals
   *
   */
  KCombo * getCombo(int id);
  
  /**
   * This returns pointer to KToolBarLined. Example:
   * <pre>
   * KLined * lined = toolbar->getKTollBarLined(lined_id);
   * </pre>
   * That way you can get access to other public methods
   * that @ref KLined provides. @ref KLined is the same thing
   * as @ref QLineEdit plus completion signals.
   */  
  KLined * getLined (int id);

  /**
   * This returns a pointer to KToolBarButton. Example:
   * <pre>
   * KButton * button = toolbar->getButton(button_id);
   * </pre>
   * That way you can get access to other public methods
   * that @ref KButton provides.
   */  
  KToolBarButton * getButton (int id);

  /**
   * Alignes item right.
   * This works only if toolbar is set to full width.
   * @see #setFullWidth
   */
  void alignItemRight (int id, bool right = true);

  /**
   * Returns pointer to inserted frame. Wrong ids are not tested.
   * Example:
   * <pre>
   * QFrame *frame = toolbar->getframe (frameid);
   * </pre>
   * You can do with this frame whatever you want,
   * except change its height (hardcoded). If you change its width
   * you will probbably have to call toolbar->@ref #updateRects (true)
   * @see QFrame
   * @see #updateRects
   */
  QFrame * getFrame (int id);

  /**
   * Returns pointer to inserted widget. Wrong ids are not tested.
   * You can do with this whatever you want,
   * except change its height (hardcoded). If you change its width
   * you will probbably have to call toolbar->@ref #updateRects (true)
   * @see QWidget
   * @see #updateRects
   */
  QWidget *getWidget (int id);
  
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
   * KToolBar manages itself by calling toolbar->@ref #updateRects (true).
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
   * @see KtopLevelWidget#updateRects
   */
  void updateRects(bool resize = false);

  /**
     * Returns minimal width for top-level window, so that toolbar
     * has only one row.
     */
  QSize sizeHint();
    
  // OLD  INTERFACE

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
  
private:
    
  QList<KToolBarItem> items;

  const char *title;
  bool fullWidth;
  BarPosition position;
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

  bool mouseEntered;  // Did the mouse touch the cheese?
  bool horizontal;    // Do I stand tall?
  bool localResize;   // Am I trying to understand recursion?
  bool wasFullWidth;  // Was I loong when I was?
  bool haveAutoSized; // Do I have a problem?
  
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

protected:
  void mouseMoveEvent(QMouseEvent*);
  void mouseReleaseEvent ( QMouseEvent *);

private:
   QPoint pointerOffset;
   QPoint parentOffset;
   int item_size;  // normal: 26
   int icon_text;  // 1 = icon+text, 0 icon+tooltip
   bool highlight; // yes/no
   QSize szh;      // Size for sizeHint
   bool fixed_size; // do not change the toolbar size 
};
#endif
