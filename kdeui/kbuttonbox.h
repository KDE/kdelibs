/* 
 * KButtonBox class
 *
 * A container widget for buttons. Uses Qt layout control to place the
 * buttons, can handle both vertical and horizontal button placement.
 *
 * (c) 1997 Mario Weilguni <mweilguni@sime.com>
 *
 * This file is published under the GNU General Library Public License.
 * See COPYING.LIB for details.
 *
 */

#ifndef __KBUTTONBOX__H__
#define __KBUTTONBOX__H__

#include <qwidget.h>
#include <qpushbt.h>
#include <qlist.h>
#include <qlayout.h>

/**
  * This class is used internally.
  */
class KButtonBoxItem {
public:
  QPushButton *button;
  bool noexpand;
  int stretch;
  int actual_size;
};


class KButtonBox : public QWidget {
  Q_OBJECT
public:
  enum { VERTICAL = 1, HORIZONTAL = 2 };

  /**
    * Creates an empty container for buttons. If _orientation is 
    * KButtonBox::VERTICAL, the buttons inserted with @see addButton
    * are layouted from top to bottom, otherwise they are layouted 
    * from left to right.
    */
  KButtonBox(QWidget *parent, int _orientation = HORIZONTAL, 
	     int border = 4, int _autoborder = 10);

  /**
    * The destructor is needed, otherwise gcc 2.7.2.1 may report an 
    * internal compiler error. It does nothing.
    */
  ~KButtonBox();

  /**
    * @return the minimum size needed to fit all buttons. This size is
    * calculated by the with/height of all buttons plus border/autoborder
    */
  virtual QSize sizeHint();

  virtual void resizeEvent(QResizeEvent *);

  /**
    * adds a new @see QPushButton and @return a pointer to the newly 
    * created button. If noexpand is FALSE, the width of the button is
    * adjusted to fit the other buttons (the maximum of all buttons is
    * taken). If noexpand is TRUE, the width of this button will be
    * set to the minimum width needed for the given text).
    */
  QPushButton *addButton(const char *text, bool noexpand = FALSE);

  /**
    * This adds a stretch to the buttonbox. @see QBoxLayout for details.
    * Can be used to separate buttons (i.e. if you add the buttons "Ok",
    * "Cancel", add a stretch and then add the button "Help", "Ok" and
    * "Cancel" will be left-aligned (or top-aligned for vertical) while
    * "Help" will be right-aligned (or bottom-aligned for vertical).
    */
  void addStretch(int scale = 1);

  /**
    * This function must be called ONCE after all buttons have been
    * inserted. It will start layout control.
    */
  void layout();

protected:
  /**
    * @return the best size for a button. Checks all buttons and takes
    * the maximum width/height.
    */
  QSize bestButtonSize();
  void  placeButtons();

protected:
  int _border, _autoborder;
  int orientation;
  bool activated;
  QBoxLayout *tl_layout;
  QList<KButtonBoxItem> buttons;
};

#endif
