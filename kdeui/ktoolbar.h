#ifndef _KTOOLBAR_H
#define _KTOOLBAR_H
#include <qcombo.h>
#include <qframe.h>
#include <qstring.h> 
#include <kbutton.h> 
#include <qpixmap.h>
#include <qlist.h>
#include <qpopmenu.h>

#include "klined.h"
#include "kcombo.h"
#include "kfloater.h"

class KToolBarCombo : public KCombo
{
  Q_OBJECT

public:
  KToolBarCombo (bool wr, int ID,
				 QWidget *parent, char *name=0);

  ~KToolBarCombo ();
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

class KToolBarLined : public KLined
{
  Q_OBJECT
public:
  KToolBarLined (const char *text, int ID,
				 QWidget *parent, char *name=0);
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

class KToolBarButton : public KButton
{
  Q_OBJECT

public:
  KToolBarButton(QPixmap& pixmap,int ID, QWidget *parent, char *name=NULL);
  KToolBarButton(QWidget *parent=NULL, char *name=NULL);
  void enable(bool enable);
  void makeDisabledPixmap();
  QPixmap disabledPixmap;
  virtual void setPixmap( QPixmap & );
  int ID() {return id;};
  bool isRight () {return right;};
  void alignRight (bool flag) {right = flag;};

protected:
  void paletteChange(const QPalette &);

protected:
  int id;
  QPixmap enabledPixmap;
  bool right;

 protected slots:
 void ButtonClicked();
  void ButtonPressed();
  void ButtonReleased();

  signals:
  void clicked(int);
  void pressed(int);
  void released(int);
};


class KToolBar : public QFrame {
  Q_OBJECT

public:
  enum BarStatus{Toggle, Show, Hide};
  enum Position{Top, Left, Bottom, Right, Floating};

  KToolBar(QWidget *parent=NULL,char *name=NULL);
  virtual ~KToolBar();

  /**** INTERFACE *****/
  /** Inserts KButton with pixmap. 
	  You should connect to one or more signals in 
	  KToolBar: clicked (int id), pressed (int id), released(int id). Those 
	  signals have id of a button that cused the signal. 'index' is the position
	  of the button. (-1 = at end).	
  */
  int insertButton(QPixmap& pixmap, int ID, bool enabled = TRUE,
				   char *ToolTipText = NULL, int index=-1 );

  /** Same as above, but connects to desired slot.
	*/
  int insertButton(QPixmap& pixmap, int ID, const char *signal,
				   const QObject *receiver, const char *slot,
				   bool enabled = TRUE,
				   char *tooltiptext = NULL, int index=-1 );

  /** Inserts KLined. Connects to desired slot. 'enabled' has
	  no effect (yet). You can connect to more signals using addConnection().
  */
  int insertLined (const char *text, int ID,
				   const char *signal,
				   const QObject *receiver, const char *slot,
				   bool enabled = TRUE,
				   char *toolTipText = NULL, int size = 70, int index =-1);

  /** Inserts QComboBox with list. 
	  Can be writable, but cannot contain
	  pixmaps. By default inserting policy is AtBottom, i.e. typed items are
	  placed at the bottom of the list.
  */
  int insertCombo (QStrList *list, int id, bool writable,
				   const char *signal, QObject *recevier,
				   const char *slot, bool enabled=TRUE,
				   char *tooltiptext=NULL,
				   int size=70, int index=-1,
				   QComboBox::Policy policy = QComboBox::AtBottom);

  /** Inserts QComboBox with text. 
	  The rest is the same ("This is an overloaded 
	  function provided for convenience", (c) TrollTech).
  */
  int insertCombo (const char *text, int id, bool writable,
				   const char *signal, QObject *recevier,
				   const char *slot, bool enabled=TRUE,
				   char *tooltiptext=NULL,
				   int size=70, int index=-1,
				   QComboBox::Policy policy = QComboBox::AtBottom);

  /** Inserts... separator.
	*/
  int insertSeparator(int index=-1);

  /** This adds connection to items. Therefore it is important that you
	  know id of particular item.
  */
  void addConnection (int id, const char *signal,
					  const QObject *receiver, const char *slot);

  /** This sets button with id enabled.
	*/
  void setItemEnabled( int id, bool enabled );
  
  /** Sets button another pixmap.
	*/
  void setButtonPixmap( int id, QPixmap& _pixmap );

  /** A new text for line editor.
	*/
  void setLinedText (int id, const char *text);

  /** This returns pointer to text in line editor. Or 0 if you missed id. 
	  You should make a copy of text.
  */
  const char *getLinedText (int id);

  /** Inserts item *text, at position index (-1 = end) in combo with id, 
	  or does nothing if you miss id.
  */
  void insertComboItem (int id, const char *text, int index);

  /** Inserts list at position index (-1 = end) in combo with id. Does
	  nothing if you miss id.
  */
  void insertComboList (int id, QStrList *list, int index);

  /** Removes item index from combo id. Don't miss id.
	*/
  void removeComboItem (int id, int index);

  /** Changes item index in combo id.
	*/
  void changeComboItem  (int id, const char *text, int index);

  /** Clears combo. Does not remove combo itself. Does nothing if id  was
	  wrong.
  */
  void clearCombo (int id);

  /** Returns item index from combo id. Or 0 if id or index was wrong.
	*/
  const char *getComboItem (int id, int index);

  /** This alignes item to the right. It works only if toolbar is set to
	  full width with void fullWidth(TRUE); You can have many right aligned
	  items. What do I mean many? Probably it is not wise to to have toolbar
	  with 1000 buttons, 123 comboboxes, and 2^32 Lineeditors.
  */
  void alignItemRight (int id, bool right = TRUE); // Only with fullWidth

  /**   This sets item to autosize. Now, this is tricky:

		- You can have only one autosized item.
		- Only KLined or KCombo can be autosized
		- Autosized item must be the last left-aligned
		item. After that you must call setItemRight()
		for every item that is inserted.

		If you don't stick by these rules, the engine will not work properly.
		Auto sized means that he (item who is autosized) will resize itself till
		next right-aligned item. See toolbartest.
  */
  void setItemAutoSized (int id, bool yes = TRUE); // item (lined and combo only) gets max size
  
  // Toolbar itself
  /** If flag == TRUE toolbar will spread to full width of KTopLevelWidget.
	  If there are two toolbars? It works too.
  */
  void setFullWidth(bool flag = TRUE);    // Top and Bottom pos only

  /** if flag == TRUE toolbar is movable, and can become floating.
	*/
  void enableMoving(bool flag = TRUE);

  /** Sets position: Top, Left, Bottom, Right, Floating
	*/
  void setPos(Position pos);

  /** Returns position: Top, Left, Bottom, Right, Floating
	*/
  Position pos();

  /** stat: Toggle, Show, Hide. Returns TRUE if visible and FALSE if not
	  visible. There may some problems with this when toolbar floats.
	  Currently, floating toolbar can be minimized by WM and by enable(Hide).
  */
  bool enable(BarStatus stat);

  /** You don't need that unless you want to subclass KTopLevelWidget. It
	  sets maximal height vertical toolbar (i.e. when it is right or left)
	  can have.
  */
  void setMaxHeight (int h);  // Set max height for vertical toolbars

  /** Set titletext for floating
	*/
  void setTitle (const char *_title) {title = _title;}; // Set titletext for floating


  /** Floating is enabled by default.
	*/
  void enableFloating (bool arrrrrrgh);  // disable  floating... *#%$§
  
  /***** END INTERFACE *******/

  /****    OLD  INTERFACE  ****/

  int insertItem(QPixmap& pixmap, int ID, bool enabled = TRUE,
				 char *ToolTipText = NULL, int index=-1 );

  int insertItem(QPixmap& pixmap, int ID, const char *signal,
				 const QObject *receiver, const char *slot,
				 bool enabled = TRUE,
				 char *tooltiptext = NULL, int index=-1 );
  void setItemPixmap( int id, QPixmap& _pixmap );

  /******* END OLD INTERFACE *******/

private:
  QList <QWidget> items;
  int getType(QWidget *w);
  int getID (QWidget *w);
  bool isItemRight (QWidget *w);
  bool isItemAutoSized(QWidget *w);
  KFloater *Float;

  const char *title;
  bool fullWidth;
  Position position;
  bool moving;
  QWidget *Parent;           // My parent to know width & height
  int toolbarWidth;
  int toolbarHeight;

  int max_height;
  Position lastPosition;      // before floating

  bool noPaint;
  bool noSize;
  bool noUpdate;
  
protected:
  QPopupMenu *context;

  void drawContents ( QPainter *);
  void resizeEvent(QResizeEvent*);
  void paintEvent(QPaintEvent*);
  virtual void mousePressEvent ( QMouseEvent *);
  void init();
  void updateRects(bool resize = FALSE);
  void layoutVertical ();
  void layoutHorizontal ();
  
 protected slots:
 void ButtonClicked(int);
  void ButtonPressed(int);
  void ButtonReleased(int);
  void ContextCallback(int);
  void floatKilled ();

  signals:
  void clicked(int);
  void pressed(int);
  void released(int);
  void moved( Position );
};

#endif
