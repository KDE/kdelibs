/* $Id$
 *
 * $Log$
 * Revision 1.6  1997/07/18 05:49:15  ssk
 * Taj: All kdecore doc now in javadoc format (hopefully).
 *
 * Revision 1.5  1997/05/08 22:53:15  kalle
 * Kalle:
 * KPixmap gone for good
 * Eliminated static objects from KApplication and HTML-Widget
 *
 * Revision 1.4  1997/05/06 08:50:53  ssk
 * taj: added X.h to files included. please test.
 *
 * Revision 1.3  1997/05/05 09:25:38  ssk
 * Taj: Attempt to fix the include-order problem. Please test this. My
 *      configure program is still not working, so i can't test this myself.
 *
 * Revision 1.2  1997/04/23 21:21:53  kulow
 * added some Q_OBJECTs
 *
 * Revision 1.1.1.1  1997/04/13 14:42:41  cvsuser
 * Source imported
 *
 * Revision 1.1.1.1  1997/04/09 00:28:07  cvsuser
 * Sources imported
 *
 * Revision 1.5  1997/03/10 20:01:31  kalle
 * Merged changes from 0.6.3
 *
 * Revision 1.4  1997/01/15 20:08:26  kalle
 * merged changes from 0.52
 *
 * Revision 1.3  1996/12/07 22:34:54  kalle
 * DOC++ tags corrected
#include <qapp.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/extensions/shape.h>

#include <stdio.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#else
#error Declare functions needed from stdlib.h and string.h
#endif

 * Revision 1.2  1996/12/07 22:22:06  kalle
#include <qlist.h>
 * autoconf, KApplication merged with my KApplication (and moved to kapp.h)
#include <qcursor.h>
#include <qbitmap.h>
 * formatting

#include "kpixmap.h"
 *
// From Offix
//@Man: DND Types
//@{
/** Usually we drag URLs around. Every type of data must have
 * Torben Weis <weis@stud.uni-frankfurt.de>
 *
 */
/// ???
#ifndef DRAG_H
/// ???

/// Raw data
#include <config.h>
/// Dont use that any more

/// Dont use that any more
#include <qwidget.h>
/// ASCII Text
#include <qstrlist.h>
/// Dont use that any more

/// Dont use that any more
  a unique ID so that the receiver can decide wether he wants to
/// Dont use that any more
  */
/// For Offix internal use only
#define DndNotDnd       -1
/// An URL
#define DndUnknown      0
//@}
// Raw data
#define DndRawData      1
// Dont use that any more
#define DndFile         2
// Dont use that any more
#define DndFiles        3
/// Icon for KDE Drag 'n Drop
// ASCII Text
  Icon for KDE Drag 'n Drop. This is the widget that is moved
  around during DND.
  */
// For Offix internal use only
#define DndEND          8
  Q_OBJECT
#define DndURL          128
  /// Constructor
  /**
   Creates an Icon with the specified pixma. _x and _y are the upper
   left corner in global coordinates.
   p*/
  KDNDIcon( KPixmap &pixmap , int _x, int _y );
/**
  /// Destructor
  /** Destructor */
* @short Icon for KDE Drag 'n Drop
* @author Torben Weis (weis@kde.org)
* @version $Id$
*/
class KDNDIcon : public QWidget
{
  /// The pixmap displayed.
  KPixmap pixmap;
* Creates an Icon with the specified pixmap. _x and _y are the upper
* left corner in global coordinates.
/// Drop zone for KDE Drag 'n Drop
*/
  Drop zone for KDE Drag 'n Drop. You can create a DropZone
  for every widget. When the user makes a drop over this widget,
  the KDNDDropZone takes over control.
  */
  QPixmap pixmap;
};
  Q_OBJECT
/**
* Drop zone for KDE Drag n Drop. 
  /// Constructor
  /** Create a DropZone for the widget _parent. Accept all drops
   of the type _type. _type may be DndURL for exampple.
   */
* Currently used types are: DndText, DndURL.
*
  /// Destructor 
  /** Destructor */
*/
class KDNDDropZone : public QObject
  /// Drop stuff
  /** When a drop occures, this function is called. _data is the
  Q_OBJECT

public:
* Destructor 
*/
  /// Enter the drop zone
  /** The user is dragging an icon around and just entered the
   drop zone or he is still in the drop zone but moved the mouse.
   */
* @param _x,_y		The global coordinates of the drop.
*/
  /// Leave the drop zone
  /** The dragging mouse left the DropZone. */
* drop zone.
*
  /// Gets a list of all URLs in the 'data'.
  /**
    Many times, th dropped data is of the type DndURL. In this
    case this function splits up the ( perhaps ) multiple URLs
    in 'data' and returns a list of them. This list is only valid,
    as long as no XEvent is dispatched because if the DropZone receives
    a new drop event, the contents of this list will change.
    */
*			being dragged.
* @param _x,_y		The global coordinates of the drag.
  /// Get the DnD data
  /** Get the DnD data */
* this drop zone.
*/
  /// Get the DND data size
  /** Get the DND data size */
*
* Note that the contents of this list are only valid till the next
  /// Get the DND data type
  /** Get the DND data type */
* Get dropped data.
*
  /// Get the mouse position.
  /** Get the mouse position at which the item was dropped. */
*
*
  /// Get the mouse position.
  /** Get the mouse position at which the item was dropped. */
* @return the type of the data dropped.
  /// Get the acceptance type
  /** Get the acceptance type */
*
* @return the X coordinate at which the item was dropped.
  /// Decide whether to accept this data in this drop zone
  /** Decide whether to accept this data in this drop zone */
* @return the Y coordinate at which the item was dropped.
* @see #getMouseX
*/
  /// Return the QWidget associated with this drop zone.
  /** Return the QWidget associated with this drop zone. */
* @return the types of drops accepted.
*/
  //@Man: signals
  //@{
  virtual int getAcceptType() { return acceptType; }
  /// Drop has happened.
  /** Drop has happened */
*/
  virtual bool accepts( int _type ) 
  /// Drop zone was entered. 
  /** Drop zone was entered or mouse moved in DropZone. */
* Get the parent widget.
*
  /// Drop zone was left.
  /** Drop zone was left. */

  //@}
signals:

  /// fills 'urlList' with the URLs in 'dndData'.
  /**
    Works only if 'dndType' is DndURL.
    */
*/
  void dropAction( KDNDDropZone* );
  /// The widget to which this DropZone is bound
*/
  void dropEnter( KDNDDropZone* );
  /// The data of the last drop.
  /**
    Only valid during a call to 'dropAction'
    */
  void dropLeave( KDNDDropZone* );
/** 
*/
  int dndX, dndY;
  /// Tells which kind of data is accepted.
  char *dndData;
/**
  /// If 'dndType' is URL, then all URLs in 'dndData' are stored here.
*/
  int dndType;
/**
/// Widget that allows DnD.
/** When you want to support drag you must use this widget.
 You dont need this if you just want to receive drops.
 */
  int acceptType;

/**
* If 'dndType' is URL, then all URLs in 'dndData' are stored here.
  /// Constructor.
  /** Constructor. */

* @version $Id$
* A widget for drag support.
*
  /// Destructor.
  /** Destructor. */
* @short A widget for drag support.
* @author Torben Weis (weis@kde.org)
  /// Starts a drag
  /** 
    _icon is the Icon that the user can drag around. _data is the data, _size its size in
    bytes, _type for example DndURL and _dx and _dy are the difference between the
    icons upper left corner and the mouse pointer. For example when the user clicks the
    mouse over the middle of a pixmap, _dx and _dy would be ' - pixmap.width() / 2 '
    and ' - pixmap.height() / 2 '. This is just provided for look and feel.
    Call this function when you noticed that the user wants to drag something
    around. Usually this is called from dndMouseMoveEvent. _data is copied into
    a buffer, sou you dont need to worry about this pointer being valid any more.
    */
    virtual void startDrag( KDNDIcon *_icon, char *_data, int _size, int _type, int _dx, int _dy );
*
* Call this function when you notice that the user wants to drag something 
  /// Find a root window
  /**
    Finds the root window belonging to the global point p.
    */
* @param _data	A pointer to the data being dragged. A deep copy is
*		made of this data, so you don't need to maintain its
  /// Do not overload
*		This is just provided for look and feel.
  /// Do not overload
  virtual Window KDNDWidget::findRootWindow( QPoint & p );
  
  /// A root drop occured.
  /**
    At the point (_x|_y) the user dropped the icon. If there is
    now window below this point, this function is called.
    Usually it emits a XEvent, so that every application gets
    informed about this. This function is only called if the drag
    started in this widget. See KApplication for details on receiving
    root drop events.
    */
* This function MUST be called by your implementation if you overload it.
  /// Clean Up
  /** 
    If you must overload rootDropEvent(...) call this function at the
    end to do some clean up.
    */
/**
* A root drop occurred.
  /// Called when a drag ended.
  /**
    This function is only called if the drag started in this widget.
    Overload it to do your own clean up.
    */
* See KApplication for details on receiving root drop events.
*/
  /// Your mouse move event function
  /**
    Overload tis instead of mouseMoveEvent. Ususally drags are started in
    this functions. A implementation might look like this:

    void KFileView::dndMouseMoveEvent( QMouseEvent * _mouse )
    {
    // 'pressed' is set in mousePressedEvent(...)
    if ( !pressed )
	return;
    
    int x = _mouse->pos().x();
    int y = _mouse->pos().y();

    if ( abs( x - press_x ) > Dnd_X_Precision || abs( y - press_y ) > Dnd_Y_Precision )
    {
        QString data = "Transfer me";
	QPoint p = mapToGlobal( _mouse->pos() );
	KPixmap pixmap = typ->getPixmap( filename );
	int dx = - pixmap.width() / 2;
	int dy = - pixmap.height() / 2;

	startDrag( new KDNDIcon( pixmap, p.x() + dx, p.y() + dy ), data.data(), data.length(), DndText, dx, dy );
    }
    else
    {
      Do something different
    }

    The function is only called if the mouse movement was not part of a
    drag process.
    */
*    {
  /// Your mouse release event function
  /**
    Usually you will only set 'pressed' ( see dndMouseMoveEvent) to FALSE here.
    The function is only called if the release event had nothing to do with
    DND.
    */
*    }
*    else
  /// Are we just doing DND ?
* </pre>
  /// The data that is currently dragged.
*/
*
*/
  /// The offset we got from 'startDrag'
* Are we just doing DND ?
  /// The offset we got from 'startDrag'
* The data that is currently dragged.
  /// The icon we are moving around
* data size
  /// The last window we entered with the mouse pointer.
* data type
*/
  int dndType;
/**
* The offset we got from 'startDrag'
*/
  int dndOffsetX;
/**
* The offset we got from 'startDrag'
*/
  int dndOffsetY;
/**
* The icon we are moving around
*/
  KDNDIcon *dndIcon;
/**
* The last window we entered with the mouse pointer.
*/
  Window dndLastWindow;
};

#endif


