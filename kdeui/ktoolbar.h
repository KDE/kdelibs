/* This file is part of the KDE libraries
    Copyright (C) 1997 Stephan Kulow (coolo@kde.org)
              (C) 1997 Sven Radej (sven.radej@iname.com)

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
#include <qlist.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qpopmenu.h>

#include "kbutton.h"
#include "kcombo.h"
#include "klined.h"

/**
 * This is internal class for use in toolbar
 *
 * @short Internal Combobox class for toolbar
 */
class KToolBarCombo : public KCombo
 {
   Q_OBJECT

 public:
   KToolBarCombo (bool wr, int ID,
                  QWidget *parent, const char *name=0);

   void enable (bool enable);
   int ID() {return id;};
   bool isRight () {return right;};
   void alignRight (bool flag) {right = flag;};
   void autoSize (bool flag) {autoSized = flag;};
   bool isAuto ()  {return autoSized;};
   
 protected:
   int id;
   bool right;
   bool autoSized;
 };
/**
 * This is internal class for use in toolbar
 *
 * @short Internal Frame class for toolbar
 */
class KToolBarFrame : public QFrame
 {
   Q_OBJECT

 public:
   KToolBarFrame (int ID, QWidget *parent, const char *name=0);

   void enable (bool enable);
   int ID() {return id;};
   bool isRight () {return right;};
   void alignRight (bool flag) {right = flag;};
   void autoSize (bool flag) {autoSized = flag;};
   bool isAuto ()  {return autoSized;};
   
 protected:
   int id;
   bool right;
   bool autoSized;
 };
     
/**
 * This is internal class for use in toolbar
 *
 * @short Internal Lined class for toolbar
 */
class KToolBarLined : public KLined
 {
   Q_OBJECT
 public:
   KToolBarLined (const char *text, int ID,
                  QWidget *parent, const char *name=0);
   void enable (bool enable);
   int ID() {return id;};
   bool isRight () {return right;};
   void alignRight (bool flag) {right = flag;};
   void autoSize (bool flag) {autoSized = flag;};
   bool isAuto ()  {return autoSized;};
   
 protected:
   int id;
   bool right;
   bool autoSized;
};

/**
 * This is internal class for use in toolbar
 *
 * @short Internal Button class for toolbar
 */
class KToolBarButton : public KButton
 {
   Q_OBJECT

 public:
   KToolBarButton(const QPixmap& pixmap,int ID, QWidget *parent, const char *name=0L);
   KToolBarButton(QWidget *parent=0L, const char *name=0L);
   void enable(bool enable);
   void makeDisabledPixmap();
   QPixmap disabledPixmap;
   virtual void setPixmap( const QPixmap & );
   int ID() {return id;};
   bool isRight () {return right;};
   void alignRight (bool flag) {right = flag;};
   void on(bool flag);
   void toggle();
   void beToggle(bool);
   
 protected:
   void paletteChange(const QPalette &);
   void leaveEvent(QEvent *e);
   void enterEvent(QEvent *e);
   void drawButton(QPainter *p);
   
 protected:
   int id;
   QPixmap enabledPixmap;
   bool right;

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
 *
 * KToolBar is a self resizing, floatable widget.
 * It is usually managed from KTopLevelWidget, but can be
 * used even if you don't use KTopLevelWidget. If you want
 * to handle this without or with subclassed KTopLevelWidget,
 * see @ref #updateRects.
 * KToolBar can contain buttons (see @ref #insertButton ), Line inputs
 * (see @ref #insertKLined ), Combo Boxes, (see @ref #insertCombo ) and frames
 * (see @ref #insertFrame ). Combos, Frames and Lineds can
 * be autosized to full width. Items can be right aligned.
 * Toolbar can float, and autoresizes itself. This may lead to
 * some flickering, but there is no way to solve it (as far as I
 * know).
 * You normaly use toolbar from subclassed KTopLevelWidget. When
 * you create toolbar object, insert items that you want to be in it.
 * Items can be inserted or removed (see @ref #removeItem ) later, when toolbar
 * is displayed. It will updte itself.
 * Then set their propperties (see @ref #alignItemRight, @ref #setItemAutoSized,
 * @ref #setToggle ...) After that set the toolbar itself (see @ref #setFullWidth,
 * @ref #enable, @ref #setBarPos ...). Then simply do addToolbar (toolbar),
 * and you're on. See how it's done in testtoolbar.
 * @short KDE Toolbar widget
 * @author Maintained by Sven Radej <a9509961@unet.univie.ac.at> 
 */
 class KToolBar : public QFrame
  {

  Q_OBJECT

public:
  enum BarStatus{Toggle, Show, Hide};
  enum BarPosition{Top, Left, Bottom, Right, Floating};

  /**
   * Constructor
   */
  KToolBar(QWidget *parent=0L, const char *name=0L);

  /**
   * Destructor. If toolbar is floating it will cleanup itself.
   * You MUST delete toolbar before exiting.
   */
  virtual ~KToolBar();

  /**
   * Inserts KButton with pixmap. You should connect to one or more signals in
   * KToolBar: @ref #clicked, @ref #pressed, @ref #released, and
   * if toolbar is toggle button (@ref setToggle ) @ref toggled. Those
   * signals have id of a button that caused the signal.
   * @param index the position of the button. (-1 = at end).
   * @return Returns item index
   */
  int insertButton(const QPixmap& pixmap, int ID, bool enabled = TRUE,
                   const char *ToolTipText = 0L, int index=-1 );
  /**
   * This is the same as above, but with specified signals and
   * slots to which this button will be connected. KButton emits
   * signals pressed, clicked and released, and
   * if toolbar is toggle button (@ref setToggle ) @ref toggled.
   * You can add more signals with @ref #addConnection.
   * @return Returns item index
   */
  int insertButton(const QPixmap& pixmap, int ID, const char *signal,
                   const QObject *receiver, const char *slot,
                   bool enabled = TRUE,
                   const char *tooltiptext = 0L, int index=-1 );
  /**
   * Inserts a KLined. You have to specify signals and slots to
   * which KLined will be connected. KLined has all slots QLineEdit
   * has, plus signals @ref #completion and @ref #rotation
   * KLined can be set to autoresize itself to full free width
   * in toolbar, that is to last right aligned item. For that,
   * toolbar must be set to full width.
   * @see #setFullWidth
   * @see #setItemAutoSized
   * @return Returns item index
   */
  int insertLined (const char *text, int ID,
                   const char *signal,
                   const QObject *receiver, const char *slot,
                   bool enabled = TRUE,
                   const char *toolTipText = 0L, int size = 70, int index =-1);

  /**
   * Inserts QComboBox with list. Can be writable, but cannot contain pixmaps. By
   * default inserting policy is AtBottom, i.e. typed items are placed at the bottom
   * of the list. Can be autosized
   * @see #setFullWidth
   * @see #setItemAutoSized
   * @see QComboBox
   * @return Returns item index
   */
  int insertCombo (QStrList *list, int id, bool writable,
                   const char *signal, QObject *recevier,
                   const char *slot, bool enabled=TRUE,
                   const char *tooltiptext=0L,
                   int size=70, int index=-1,
                   QComboBox::Policy policy = QComboBox::AtBottom);

  /**
   * Inserts ComboBox with text. The rest is the same as above.
   * @see #setItemAutoSized
   * @see QComboBox
   * @return Returns item index
   */
  int insertCombo (const char *text, int id, bool writable,
                   const char *signal, QObject *recevier,
                   const char *slot, bool enabled=TRUE,
                   const char *tooltiptext=0L,
                   int size=70, int index=-1,
                   QComboBox::Policy policy = QComboBox::AtBottom);
  /**
   * Insert separator
   */
  int insertSeparator(int index=-1);

  /**
   * Inserts frame with specified width. You can get pointer
   * to this frame with @ref #getFrame
   * Frame can be autosized.
   * @see #setItemAutoSized
   * @return Returns item index
   */
  int insertFrame(int width, int id, int index =-1);

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
   * Makes button a toggle button if flag is TRUE
   */
  void setToggle (int id, bool flag = TRUE);

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
   * @see #setToggle
   */
  void setButton (int id, bool flag);

  /**
   * Returns TRUE if button is on, FALSE if button is off.
   * If button is not a toggle button, or not button at all
   * returns FALSE
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
   * This returns pointer to Combo. Use it as
   * KCombo *combo = toolbar->getCombo(combo_id);
   * That way you can get access to other public methods
   * that KCombo provides. KCombo is the same thing
   * as QComboBox plus two signals
   */
  KToolBarCombo * getCombo(int id);
  
  /**
   * Alignes item right.
   * This works only if toolbar is set to full width.
   * @see #setFullWidth
   */
  void alignItemRight (int id, bool right = TRUE);

  /**
   * Sets item autosized. This works only if toolbar is set to full width.
   * ONLY ONE item can be autosized, and it has to be
   * the last left-aligned item. Items that come after this must be right
   * aligned. Items that can be right aligned are Lineds, Frames, and
   * Combos. Auto sized item will resize itself whenever toolbar geometry
   * changes, to last right-aligned item (or till end of toolbar if there
   * are no right aligned items
   * @see #setFullWidth
   * @see #alignItemRight
   */
  void setItemAutoSized (int id, bool yes = TRUE);

  /**
   * Returns pointer to inserted frame, or 0 if id is wrong, or
   * if item id is not a frame, or if there is no frame inserted.
   * You can use QFrame *frame = toolbar->getframe (frameid);
   * to get frame. You can do with this frame whatever you want,
   * except changing its height (hardcoded 24). If you change its width
   * you will probbably have to call toolbar->@ref #updateRects (TRUE)
   * @see QFrame::QFrame
   * @see #updateRects
   */
  KToolBarFrame * getFrame (int id);

  /**
   * Removes item id.
   * Item is deleted. Toolbar is redrawn after it.
   */
  void removeItem (int id);
  
  /**
   * Sets toolbar to full parent width (or to value set by setMaxWidth).
   * You have to call this function if you want to have right aligned items or
   * autosized item.
   * @see #alignItemRight
   * @see #setItemAutoSized
   */
  void setFullWidth(bool flag = TRUE);    // Top and Bottom pos only

  /**
   * Enables or disables moving of toolbar.
   */
  void enableMoving(bool flag = TRUE);

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
   * do not have to call it, since it's called from @ref KTopLevelWidget::updateRects
   * If you reimplement @ref KTopLevelWidget::resizeEvent or updateRects,
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
   * toolbar can still be moved by @ref #setBarPos
   */
  void enableFloating (bool arrrrrrgh);

  /**
   * Redraw toolbar and resize it if resize is TRUE.
   * You normaly don't have to call it, since it's called from
   * @ref KTopLevelWidget::updateRects. You can call it if you manualy
   * change width of inserted frame, or if you wish to force toolbar to
   * recalculate itself.
   *
   * KtopLevelWidget is changed a bit to work with this toolbar. Interface
   * to KTopLevelWidget is same as before. If you want to subclass
   * KTopLevelWidget to change its resize policy, hear this:
   *
   * resizeEvent () in KTopLevelWidget just calls updateRects, which handles
   * children sizes. Call updateRects when you're done with your things.
   *
   * If you want to handle everything yourself:
   *
   * KToolBar manages itself by calling toolbar->@ref #updateRects (TRUE).
   * It will autosize itself, but won't move itself. You have to do the moving.
   *
   * First setup & move anything that is above toolbars (menus...). Then
   * setup statusbars and other horizontal things on bottom. Then loop through
   * all HORIZONTAL toolbars, call their updateRects(TRUE), _then_ take their
   * size, an move them (note that they size themselves according to parent
   * width()). After  you have looped through HORIZONTAL toolbars, calculate
   * the maximum height that vertical toolbars may have (this is your free
   * area height). Then loop through vertical toolbars,
   * @ref #setMaxHeight (calculated_max_height) on them,
   * call their updateRects(TRUE), and _then_ move them to their locations.
   */
  void updateRects(bool resize = FALSE);
  
  // OLD  INTERFACE
  
  /**
   * This is provided for compatibility with old KToolBar. Writes
   * a warning, and calls @ref #InsertButton
   *
   */
  int insertItem(const QPixmap& pixmap, int ID, bool enabled = TRUE,
               char *ToolTipText = 0L, int index=-1 );

  /**
   * This is provided for compatibility with old KToolBar. Writes
   * a warning, and calls @ref #InsertButton
   */
  int insertItem(const QPixmap& pixmap, int ID, const char *signal,
               const QObject *receiver, const char *slot,
               bool enabled = TRUE,
               char *tooltiptext = 0L, int index=-1 );

  /**
   * This is provided for compatibility with old KToolBar. Writes
   * a warning, and calls @ref #setButtonPixmap
   */
  void setItemPixmap( int id, const QPixmap& _pixmap );

signals:
    /**
     * Emits when button id is clicked
     */
    void clicked(int id);

    /**
     * Emits when button id is pressed
     */
    void pressed(int);

    /**
     * Emits when button id is pressed
     */
    void released(int);

    /**
     * Emits when toggle button changes state
     * Emits also if you change state
     * with @ref #setButton or @ref #toggleButton
     * If you make a button normal again, with
     * @ref #setToggle (FALSE), this signal won't
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
  
private:
    
  QList <QWidget> items;
  int getType(QWidget *w);
  int getID (QWidget *w);
  bool isItemRight (QWidget *w);
  bool isItemAutoSized(QWidget *w);

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
  
  BarPosition lastPosition;
  
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
  
protected slots:
  void ButtonClicked(int);
  void ButtonPressed(int);
  void ButtonReleased(int);
  void ContextCallback(int);


protected:
  void mouseMoveEvent(QMouseEvent*);
  void mouseReleaseEvent ( QMouseEvent *);

private:
   QPoint pointerOffset;
   QPoint parentOffset;
};
#endif
