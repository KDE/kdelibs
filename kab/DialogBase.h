/* -*- C++ -*-
 * A dialog class that serves as a common base class for all dialogs kab creates.
 * Declaration
 * 
 * the KDE addressbook.
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class,
 *             Nana for debugging
 * $Revision$
 * 
 * This base class provides basic functionality needed by nearly all dialogs, as
 * the three standard buttons (OK, Apply, Cancel)  that are configurable in some
 * aspects already connected to the appropriate slots:
 * OK: 
 *     accept()
 * Apply:
 *     applyPressed() (not defined by Qt, but declared virtual for overloading)
 *     Sends signal "apply" to be connected to your code.
 * Cancel:
 *     reject()
 * Additionally, the class inserts a base frame and an inner frame that are 
 * resized automatically if you call the resizeEvent function of this class in 
 * your resizeEvent. Also the buttons are placed this way. The inner frame is 
 * set inside the base frame above of the buttons.
 * Both the base and the inner frame get background tiles as a static image 
 * provided by the class. This way you can achieve that all your dialogs look
 * the same way all over your application. All dialog derived from this class 
 * get the background that is set centrally.
 */
#ifndef DialogBase_included
#define DialogBase_included

#include "stl_headers.h"
#include "DialogBaseData.h"
#include <qsize.h>
#include "debug.h"

class QPixmap;

class DialogBase : public DialogBaseData
{
  // ############################################################################
  Q_OBJECT
  // ----------------------------------------------------------------------------
public:
  DialogBase(QWidget* parent=0, const char* name=0, bool modal=true);
  virtual ~DialogBase();
  /** This returns whether the background tile is set or not.
   */
  static bool haveBackgroundTile();
  /** This returns a const pointer to the background tile if there is one.
   */
  static const QPixmap* getBackgroundTile();
  /** This method sets the background tile. If it is Null (0), the background
   *  image is deleted.
   */
  static void setBackgroundTile(const QPixmap*);
  /** The background mode - either showing the selected tile or the standard
   *  background color - can be switched independently for the outer and the
   *  inner frame.
   */
  void showBaseFrameTile(bool state); // color (false) or tile (true)
  void showMainFrameTile(bool state);
  /** The following methods enable the respective buttons. All buttons are 
   *  enabled by default.
   */
  void enableButtonOK(bool state);
  void enableButtonApply(bool state);
  void enableButtonCancel(bool state);
  /** The following methods show or hide the respective buttons. All buttons are
   *  visible by default:
   */
  void showButtonOK(bool state);
  void showButtonApply(bool state);
  void showButtonCancel(bool state);
  /** This method returns the contents rectangle of the inner frame. Place your
   *  widgets inside this rectangle, and use it to set up their geometry.
   *  Be carefull: The rectangle is only valid after resizing the dialog, as it
   *  is a result of the resizing process. If you need the "overhead" the dialog
   *  needs for its element, use getBorderWidth.
   */
  QRect getContentsRect();
  /** This method returns the border widths in all directions that the dialog
   *  needs for itselfes. Respect this, or get bad looking results. 
   *  The references are upper left x, upper left y, lower left x and lower left
   *  y. The results are differences in pixels from the dialogs corners.
   */
  void getBorderWidths(int& ulx, int& uly, int& lrx, int& lry) const;
  /** With this method it is more easier to calculate a size hint for a dialog 
   *  derived from DialogBase if you now the width and height of the elements you
   *  add to the widget. The rectangle returned is calculated so that all 
   *  elements exactly fit into it. Thus, you may set it as a minimum size for
   *  the resulting dialog.
   */
  QSize calculateSize(int w, int h);
  /** This method is meant to make the management of simple layouts easier. You 
   *  may assign (and remove) a main widget to the dialog. This main widget will 
   *  be placed exactly fitting into the frame rectangle of the inner frame. 
   *  This is usefull if the widget you insert is managed by a Qt layout manager,
   *  for example (or by your own resizeEvent handler). Make sure to set the 
   *  minimum size of the dialog (using one of the methods above). If the main
   *  widget provided by you has a minimum size the dialogs minimum size is 
   *  automatically adapted to this size, no need to bother...
   *  If you set a main widget and there is already one, this previous main 
   *  widget will NOT be deleted. This way you may switch between different 
   *  widgets for example by hiding the old one and setting the new one as main.
   *  The dialog does not manipulate the visibility state of your widget, only 
   *  its geometry.
   *  If you replace a previous main widget, the old one is still visible. Hide
   *  it, or place it somewhere else.
   */
  void setMainWidget(QWidget*);
  QWidget* getMainWidget(); // returns zero if there is none
  // ----------------------------------------------------------------------------
protected:
  static QPixmap *tile;
  static const int Grid;
  static DialogBase *dummy;
  QWidget *main;
  bool mainTile, baseTile; 
  // one static object to be able to send signals from static members:
  void emitBackgroundChanged();
  // events:
  void resizeEvent(QResizeEvent*);
  // overloaded slot from base class:
  void applyPressed();
  // ----------------------------------------------------------------------------
signals:
  void apply();
  void backgroundChanged();
  // ----------------------------------------------------------------------------
public slots:
  // called when the background changed
  void updateBackground();
  // a method that is called on creation and when the main widget changes:
  void initializeGeometry(); // eventually selects minimum size
  // call this to set the dialog description (text on top):
  void setDesc(const char*);
  // ############################################################################
};

#endif // DialogBase_included



