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

// $Id$
// $Log$
// Revision 1.55  1999/05/28 10:17:21  kulow
// several fixes to make --enable-final work. Most work is done by changing
// the order of the files in _SOURCES
//
// Revision 1.54  1999/05/23 00:53:59  kulow
// CVS_SILENT moving some header files that STL comes before Qt
//
// Revision 1.53  1999/05/22 20:54:13  ssk
// Minor doc reformat.
//
// Revision 1.52  1999/05/04 04:28:08  ssk
// Updated KToolBar description.
//
// Revision 1.51  1999/04/22 15:59:44  shausman
// - support QStringList for combos
//
// Revision 1.50  1999/03/06 18:03:37  ettrich
// the nifty "flat" feature of kmenubar/ktoolbar is now more visible:
// It has its own menu entry and reacts on simple LMP clicks.
//
// Revision 1.49  1999/03/01 23:35:26  kulow
// CVS_SILENT ported to Qt 2.0
//
// Revision 1.48  1999/02/10 19:51:23  koss
// *** empty log message ***
//
// Revision 1.46  1998/11/25 13:22:00  radej
// sven: Someone made some private things protected (was it me?).
//
// Revision 1.45  1998/11/21 19:27:20  radej
// sven: doubleClicked signal for buttons.
//
// Revision 1.44  1998/11/11 14:32:11  radej
// sven: *Bars can be made flat by MMB (Like in Netscape, but this works)
//
// Revision 1.43  1998/11/09 00:28:43  radej
// sven: Docs update (more to come)
//
// Revision 1.42  1998/11/06 12:54:54  radej
// sven: radioGroup is in. handle changed again (broken in vertical mode)
//
// Revision 1.41  1998/10/09 12:42:21  radej
// sven: New: (un) highlight sugnals, Autorepeat buttons, button down when
//       pressed. kdetest/kwindowtest updated. This is Binary COMPATIBLE.
//
// Revision 1.40  1998/09/15 05:56:47  antlarr
// I've added a setIconText function to change the state of a variable
// in KToolBar
//
// Revision 1.39  1998/09/01 20:22:24  kulow
// I renamed all old qt header files to the new versions. I think, this looks
// nicer (and gives the change in configure a sense :)
//
// Revision 1.38  1998/08/09 14:01:19  radej
// sven: reintroduced makeDisabledPixmap code, and dumped QIconSet. Fixed a bug
//       with paletteChange too.
//
// Revision 1.37  1998/08/06 15:39:03  radej
// sven: Popups & delayedPopups. Uses QIconSet. Needs Qt-1.4x
//
// Revision 1.36  1998/06/20 10:57:00  radej
// sven: mispelled something...
//
// Revision 1.35  1998/06/19 13:09:31  radej
// sven: Docs.
//
// Revision 1.34  1998/05/04 16:38:36  radej
// Bugfixes for moving + opaque moving
//
// Revision 1.33  1998/04/28 09:17:49  radej
// New moving and docking BINARY INCOMPATIBLE
//

#ifndef _KTOOLBAR_H
#define _KTOOLBAR_H

#include <qlist.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qbutton.h>
#include <qfont.h>
#include <qsize.h>
#include <qintdict.h>
#include <qstringlist.h>

//#include <qiconset.h>

#include "kcombo.h"

class KLineEdit;
class KToolBar;
class KToolBoxManager;

typedef QWidget Item;

enum itemType {
    ITEM_LINED = 0,
    ITEM_BUTTON = 1,
    ITEM_COMBO = 2,
    ITEM_FRAME = 3,
    ITEM_TOGGLE = 4,
    ITEM_ANYWIDGET=5
};


/**
 * A toolbar item. Used internally by KToolBar, use KToolBar methods
 * instead.
 * @internal
 * */
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


/**
 * A toolbar button. This is used internally by @ref KToolBar, use the 
 * KToolBar methods instead.
 * @internal
 */

class KToolBarButton : public QButton
 {
   Q_OBJECT

 public:
   KToolBarButton(const QPixmap& pixmap, int id, QWidget *parent,
                  const char *name=0L, int item_size = 26, const QString &txt=QString::null,
                  bool _mb = false);
   KToolBarButton(QWidget *parent=0L, const char *name=0L);
   ~KToolBarButton() {};
   void setEnabled(bool enable);

   virtual void setPixmap( const QPixmap & );
   virtual void setText ( const QString& text);
   void on(bool flag);
   void toggle();
   void beToggle(bool);
   bool ImASeparator () {return sep;};
   void youreSeparator () {sep = true;};
   QPopupMenu *popup () {return myPopup;};
   void setPopup (QPopupMenu *p);
   void setDelayedPopup (QPopupMenu *p);
   void setRadio(bool f);

 public slots:
   void modeChange();

 protected:
   void paletteChange(const QPalette &);
   void leaveEvent(QEvent *e);
   void enterEvent(QEvent *e);
   void drawButton(QPainter *p);
   bool eventFilter (QObject *o, QEvent *e);
   void showMenu();
   //void setIconSet (const QPixmap &);
   void makeDisabledPixmap();

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
   KToolBar *parentWidget;
   QString btext;
   QFont buttonFont;
   QPopupMenu *myPopup;
   bool delayPopup;
   QTimer *delayTimer;
   bool radio;

 protected slots:
     void ButtonClicked();
     void ButtonPressed();
     void ButtonReleased();
     void ButtonToggled();
     void slotDelayTimeout();

 signals:
     void clicked(int);
     void doubleClicked(int);
     void pressed(int);
     void released(int);
     void toggled(int);
     void highlighted (int, bool);
 };


 /**
  * KToolBar can be "floated", dragged and docked from its parent window.
  *
  * A KToolBar can contain standard or toggle buttons, line edit widgets,
  * combo boxes, frames or any developer-defined custom widget, with 
  * automatic full-width resize for each widget except buttons. Buttons 
  * can also be connected to popup menus with a delay option.
  * 
  * KToolBar can be used as a standalone widget, but @ref KTopLevelWidget 
  * provides easy factories and management of one or more toolbars.
  * Once you have a KToolBar object, you can insert items into it with the 
  * insert... methods, or remove them with the @ref removeItem method. This
  * can be done at any time; the toolbar will be automatically updated.
  * There are also many methods to set per-child properties like alignment
  * and toggle behaviour.
  *
  * KToolBar uses a global config group to load toolbar settings on
  * construction. It will reread this config group on a
  * @ref KApplication::apearanceChanged signal.
  * 
  * BUGS: Sometimes flickers on auto resize, no workaround has yet been
  * found for this.
  *
  * @short Floatable toolbar with auto resize.
  * @version $Id$
  * @author Stephan Kulow <coolo@kde.org>, Sven Radej <radej@kde.org>.
  */
 class KToolBar : public QFrame
  {

  Q_OBJECT

  friend class KToolBarButton;
  friend class KRadioGroup;

public:
  enum BarStatus{Toggle, Show, Hide};
  enum BarPosition{Top, Left, Bottom, Right, Floating, Flat};

  /**
   * Constructor.
   * Toolbar will read global-config file for item Size higlight
   * option and button type. However, you can pass desired height.
   * If you specify height here, config value has no effect.
   * Exception is if you set Icontext mode to 3 (icons under text) whic sets
   * size to minimum 40 pixels. For setting IconText mode, see
   * @ref #setIconText .
   * Setting size in constructor is not recomended.
   */
  KToolBar(QWidget *parent=0L, const char *name=0L, int _item_size = -1);

  /**
   * Destructor. If toolbar is floating it will cleanup itself.
   */
  virtual ~KToolBar();

  /**
   * Inserts KButton with pixmap. You should connect to one or more signals in
   * KToolBar: @ref #clicked , @ref #pressed , @ref #released ,
   * @ref highlighted  and
   * if toolbar is toggle button (@ref #setToggle ) @ref #toggled . Those
   * signals have id of a button that caused the signal.
   * If you want to bound an popup to button, see  @ref #setButton
   * @param index the position of the button. (-1 = at end).
   * @return Returns item index
   */
  int insertButton(const QPixmap& pixmap, int ID, bool enabled = true,
                   const QString& ToolTipText = QString::null, int index=-1 );
  /**
   * This is the same as above, but with specified signals and
   * slots to which this button will be connected. Button emits
   * signals pressed, clicked and released, and
   * if toolbar is toggle button ( @ref #setToggle ) @ref #toggled .
   * You can add more signals with @ref #addConnection .
   * @return Returns item index
   */
  int insertButton(const QPixmap& pixmap, int ID, const char *signal,
                   const QObject *receiver, const char *slot,
                   bool enabled = true,
                   const QString& tooltiptext = QString::null,
		   int index=-1 );

  /**
   * This inserts a button with popupmenu. Button will have small
   * trialngle. You have to connect to popup's signals. The
   * signals pressed, released, clikced or doubleClicked are NOT emmited by
   * this button (see @ref #setDelayedPopup for that).
   * You can add custom popups which inherit @ref QPopupMenu to get popups
   * with tables, drawings etc. Just don't fiddle with events there.
   */
  int insertButton(const QPixmap& pixmap, int id, QPopupMenu *popup,
                   bool enabled, const QString&_text, int index=-1);

  /**
   * Inserts a KLined. You have to specify signals and slots to
   * which KLineEdit will be connected. KLineEdit has all slots QLineEdit
   * has, plus signals @ref KLineEdit::completion and @ref KLineEdit::rotation
   * KLineEdit can be set to autoresize itself to full free width
   * in toolbar, that is to last right aligned item. For that,
   * toolbar must be set to full width (which it is by default).
   * @see #setFullWidth
   * @see #setItemAutoSized
   * @see KLineEdit
   * @return Returns item index
   */
  int insertLined (const QString& text, int ID,
                   const char *signal,
                   const QObject *receiver, const char *slot,
                   bool enabled = true,
                   const QString& toolTipText = QString::null,
		   int size = 70, int index =-1);

  /**
   * Inserts KComboBox with list. Can be writable, but cannot contain
   * pixmaps. By default inserting policy is AtBottom, i.e. typed items
   * are placed at the bottom of the list. Can be autosized.
   *
   * KCombo is almost the same thing as QComboBox.
   * @see #setFullWidth
   * @see #setItemAutoSized
   * @see KCombo
   * @return Returns item index
   */
  int insertCombo (QStrList *list, int id, bool writable,
                   const char *signal, QObject *recevier,
                   const char *slot, bool enabled=true,
                   const QString& tooltiptext=QString::null,
                   int size=70, int index=-1,
                   KCombo::Policy policy = KCombo::AtBottom);
		   
  /**
   * Inserts KComboBox with list. Can be writable, but cannot contain
   * pixmaps. By default inserting policy is AtBottom, i.e. typed items
   * are placed at the bottom of the list. Can be autosized.
   * KCombo is almost the same thing as QComboBox.
   * @see #setFullWidth
   * @see #setItemAutoSized
   * @see KCombo
   * @return Returns item index
   */
  int insertCombo (const QStringList &list, int id, bool writable,
                   const char *signal, QObject *recevier,
                   const char *slot, bool enabled=true,
                   const QString& tooltiptext=QString::null,
                   int size=70, int index=-1,
                   KCombo::Policy policy = KCombo::AtBottom);

  /**
   * Inserts KCombo with text. The rest is the same as above.
   * @see #setItemAutoSized
   * @see KCombo
   * @return Returns item index
   */
  int insertCombo (const QString& text, int id, bool writable,
                   const char *signal, QObject *recevier,
                   const char *slot, bool enabled=true,
                   const QString& tooltiptext=QString::null,
                   int size=70, int index=-1,
                   KCombo::Policy policy = KCombo::AtBottom);
  /**
   * Insert separator
   */
  int insertSeparator(int index=-1);

  /**
   * Insert line separator
   */
  int insertLineSeparator(int index=-1);

  /**
   * This function is deprecated and will be removed. Use @ref #insertWidget
   * to insert anything.
   * Inserts frame with specified width. You can get
   * pointer to this frame with @ref #getFrame
   * Frame can be autosized to full width.
   * @see #setItemAutoSized
   * @return Returns item index
   */
  int insertFrame(int id, int width, int index =-1);

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
   * Sets delayed popup to a button. Delayed popup is what you see in
   * netscape's Previous&next buttons: if you click them you go back,
   * or forth. If you press them long enough, you get a history-menu.
   * This is exactly what we do here.
   *
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
   * You may add popups wich are derived from QPopupMenu. You may
   * add popups that are already in menu bar or are submenus of other popups.
   */
  void setDelayedPopup (int id , QPopupMenu *_popup);

 /**
   * Makes a button autorepeat button. Toggle, buttons with menu or
   * delayed menu cannot be autorepeat. More, you can and will receive
   * only signals clicked, and not pressed or released.
   * When user presses this buton, you will receive signal clicked,
   * and if button is still pressed after some time, more clicks
   * in some interval. Since this uses @ref QButton::setAutoRepeat ,
   * I don't know how much is 'some'.
   */
  void setAutoRepeat (int id, bool flag=true);


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
   * #ref toggled.
   *
   * @see #setToggle
   */
  void setButton (int id, bool flag);

  /**
   * Returns true if button is on, false if button is off.
   * If button is not a toggle button returns false
   * @see #setToggle
   */
  bool isButtonOn (int id);

  /**
   * Sets text in Lined.
   * Cursor is set at end of text.
   */
  void setLinedText (int id, const QString& text);

  /**
   * Returns Lined text.
   * If you want to store this text, you have to deep-copy it somwhere.
   */
  QString getLinedText (int id);

  /**
   * Inserts text in combo id with at position index.
   */
  void insertComboItem (int id, const QString& text, int index);

  /**
   * Inserts list in combo id at position index
   */
  void insertComboList (int id, QStrList *list, int index);

  /**
   * Inserts list in combo id at position index
   */
  void insertComboList (int id, const QStringList &list, int index);

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
  void changeComboItem  (int id, const QString& text, int index=-1);

  /**
   * Clears combo id.
   * Does not delete it or hide it.
   */
  void clearCombo (int id);

  /**
   * Returns text of item index from Combo id.
   * index = -1 means current item
   */

  QString getComboItem (int id, int index=-1);

  /**
   * This returns pointer to Combo. Example:
   * <pre>
   * KCombo *combo = toolbar->getCombo(combo_id);
   * </pre>
   * That way you can get access to other public methods
   * that @ref KCombo provides. @ref KCombo is KDE enhancement
   * of @ref QComboBox . KCombo inherits QComboBox, so you can
   * use pointer to QComboBox too.
   */
  KCombo * getCombo(int id);

  /**
   * This returns pointer to KToolBarLined. Example:
   * <pre>
   * KLineEdit * lined = toolbar->getKTollBarLined(lined_id);
   * </pre>
   * That way you can get access to other public methods
   * that @ref KLineEdit provides. @ref KLineEdit is the same thing
   * as @ref QLineEdit plus completion signals.
   */
  KLineEdit * getLined (int id);

  /**
   * This returns a pointer to KToolBarButton. Example:
   * <pre>
   * KToolBarButton * button = toolbar->getButton(button_id);
   * </pre>
   * That way you can get access to other public methods
   * that @ref KToolBarButton provides. Using of this method is not
   * recomended.
   */
  KToolBarButton * getButton (int id);

  /**
   * Alignes item right.
   * This works only if toolbar is set to full width.
   * @see #setFullWidth
   */
  void alignItemRight (int id, bool right = true);

  /**
   * This function is deprecated and might be removed. Use @ref #insertWidget
   * and @ref #getWidget instead.<br>
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
   * autosized item.
   *
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
   * hiding means minimizing. Warning: kwm will not show minimized toolbar
   * on taskbar. Therefore hiding means hiding.
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
   * In 0xFE cases out of 0xFF you don't need to use this function.
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
   * Sets title for toolbar when it floats. Titles are however not (yet)
   * visible. You can't change toolbar's title while it's floating.
   */
  void setTitle (const QString& _title) {title = _title;}

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
   * Sets the kind of painting for buttons between : 0 (only icons),
   * 1 (icon and text, text is left from icons), 2 (only text),
   * and 3 (icons and text, text is under icons).
   */

  void setIconText(int it);

  /**
   * Redraw toolbar and resize it if resize is true.
   * You normaly don't have to call it, since it's called from
   * @ref KTopLevelWidget#updateRects or from resizeEvent. You can call it
   * if you manualy change width of inserted frame, or if you wish to force
   * toolbar to recalculate itself.
   *
   * You don't want to fiddle with this.
   * @ref KtopLevelWidget works closely with toolbar. If you want to
   * subclass KTopLevelWidget to change its resize policy, hear this:
   *
   * resizeEvent() in KTopLevelWidget just calls updateRects, which handles
   * children sizes. Call updateRects when you're done with your things.
   * 
   * If you want to handle everything yourself:
   * 
   * KToolBar manages itself by calling toolbar->@ref #updateRects (true).
   * It will autosize itself, but won't move itself.
   * You have to do the moving.
   * First setup & move anything that is above toolbars (menus...). Then
   * setup statusbars and other horizontal things on bottom. Then loop through
   * all HORIZONTAL toolbars, call their updateRects(true), _then_ take their
   * size, an move them (note that they size themselves according to parent
   * width()). After  you have looped through HORIZONTAL toolbars, calculate
   * the maximum height that vertical toolbars may have (this is your free
   * area height). Then loop through vertical toolbars,
   *
   * @ref #setMaxHeight (calculated_max_height) on them,
   * call their updateRects(true), and _then_ move them to their locations.
   * In 0xFE cases out of 0xFF you don't need to use this function.
   * @see KtopLevelWidget#updateRects
   */
  void updateRects(bool resize = false);

  /**
     * Returns minimal width for top-level window, so that toolbar
     * has only one row.
     */
  QSize sizeHint();

  void setFlat (bool flag);

signals:
    /**
     * Emits when button id is clicked.
     */
    void clicked(int id);

    /**
     * Emits when button id is double clicked. Note: you will always
     * recive two @ref #clicked , @ref #pressed and @ref #released signals.
     * There is no way to avoid it - at least no easy way.
     * If you need to resolve this all you can do is set up timers
     * which waits for @ref QApplication::doubleClickInterval to expire.
     * if in that time you don't get this signal, you may belive that
     * button was only clicked.
     * And please note that butons with popup menus do not emit this signal,
     * but those with delayed popup do.
     */
    void doubleClicked (int id);

    /**
     * Emits when button id is pressed.
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
     * This signal is emmited when item id gets highlighted/unhighlighted
     * (i.e when mouse enters/exits). Note that this signal is emited from
     * all buttons (normal, disabled and toggle) even when there is no visible
     * change in buttons (meaning, buttons do not raise when mouse enters).
     * Parameter isHighlighted is true when mouse enters and false when
     * mouse exits.
     */
    void highlighted(int id, bool isHighlighted);

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

  QString title;
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
  BarPosition movePos;      // Where was I moved to?
  bool mouseEntered;  // Did the mouse touch the cheese?
  bool horizontal;    // Do I stand tall?
  bool localResize;   // Am I trying to understand recursion?
  bool wasFullWidth;  // Was I loong when I was?
  bool haveAutoSized; // Do I have a problem?

  KToolBoxManager *mgr;
  bool buttonDownOnHandle;

protected:
  QPopupMenu *context;

  void drawContents ( QPainter *);
  void resizeEvent(QResizeEvent*);
  void paintEvent(QPaintEvent*);
  void closeEvent (QCloseEvent *);
  void mousePressEvent ( QMouseEvent *);
  void mouseMoveEvent ( QMouseEvent *);
  void mouseReleaseEvent ( QMouseEvent *);
  void init();
  void layoutVertical ();
  void layoutHorizontal ();
  void leaveEvent (QEvent *e);


private slots:
  void ButtonClicked(int);
  void ButtonDblClicked( int id );
  void ButtonPressed(int);
  void ButtonReleased(int);
  void ButtonToggled(int);
  void ButtonHighlighted(int,bool);

  void ContextCallback(int);
  void slotReadConfig ();
  void slotHotSpot (int i);


private:
   QPoint pointerOffset;
   QPoint parentOffset;
   int item_size;  // normal: 26
   int icon_text;  // 1 = icon+text, 0 icon+tooltip
   bool highlight; // yes/no
   QSize szh;      // Size for sizeHint
   bool fixed_size; // do not change the toolbar size
   bool transparent; // type of moving
  };


/*************************************************************************
 *                          KRadioGroup                                  *
 *************************************************************************/
 /**
  * KRadioGroup is class for group of radio butons in toolbar.
  * Take toggle buttons which you already inserted into toolbar,
  * create KRadioGroup instance and add them here.
  * All buttons will emit signals toggled (bool) (or you can
  * use sitgnal @ref #toggled (int id) from toolbar). When one button is set
  * down, all others are unset. All buttons emit signals - those who
  * "go down" and those who "go up".
  *
  * @author Sven Radej <radej@kde.org>
  * @short Class for group of radio butons in toolbar.
  */
class KRadioGroup : public QObject
{
  Q_OBJECT

public:
  /**
   * Constructor. Parent must be @ref KToolBar .
   */
  KRadioGroup (QWidget *_parent, const char *_name=0);
  /**
   * Destructor.
   */
  ~KRadioGroup () {};

  /**
   * Adds button to group. Button cannot be unset by mouse clicks (you
   * must press some other button tounset this one)
   */
  void addButton (int id);

  /**
   * Removes button from group, making it again toggle button (i.e.
   * You can unset it with mouse).
   */
  void removeButton (int id);

public slots:
  /**
   * Internal - nothing for you here.
   */
  void slotToggled (int);

private:
  QIntDict<KToolBarButton> buttons;
  KToolBar *tb;
};


#endif
