/* This file is part of the KDE libraries
    Copyright (C) 1997 Alexander Sanda (alex@darkstar.ping.at)

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
/*****************************************************************************
*                                                                            *
*  KPanner -- panner widget for KDE by Alexander Sanda                       *
*             modifications by Paul Kendall for absolute pixel positions     *
*                                                                            *
*                                                                            *
*****************************************************************************/

#ifndef _KPANNER_H
#define _KPANNER_H

#include <qwidget.h>
#include <qframe.h>

/// KPanner - resizeable screen regions
/** Kpanner is a simple widget, very similar to Motifs XmPanedWindow.
  Basically, KPanner creates two child widgets, separated by a dragable
  bar. The user can now resize the window areas by repositioning this bar.

How to use:
-----------

The class name is KPanner.

KPanner *p = new KPanner(parent, name, flags, i_size);

flags can be a combination of the following (or zero):

KPanner::O_VERTICAL (default)
KPanner::O_HORIZONTAL
KPanner::U_PERCENT (default)
KPanner::U_ABSOLUTE


i_size is the location of the separator.

The flag O_VERTICAL specifies that the separator is vertical and you
therefore have a left and right pane.  The O_HORIZONTAL flag means that
you have an upper and a lower pane.

U_ABSOLUTE specifies that the location is in pixels and U_PERCENT means
the location is measured in percent of full widget size, e.g. if you
specify a value of 50, the client widgets will have exactly the same size.  

To obtain the panners child widgets (which is necessary, because they
will act as parents for the widgets to manage), KPanner provides two
class methods:

QWidget *child0()  returns the leftmost (or topmost for horizontal
                   panners) child.

QWidget *child1()  returns the rightmost (or bottom for horizontal
                   panners) child */   
class KPanner : public QWidget  {
  Q_OBJECT
public:
  /// some flags
  /** some flags */
  enum {O_VERTICAL = 0, O_HORIZONTAL = 1, U_PERCENT = 0, U_ABSOLUTE = 2};
    
  KPanner(QWidget *parent = 0, const char *name = 0, unsigned flags = 0, int i_size = 0);

  ~KPanner();
  QWidget      *child0(), *child1();

  /// Set window division.
  /**
	(0 <= int <= 100)
	
	set the separator, so that it will divide the clients at i_size percent
	of full widget size. For vertical panners, this is measured from the
	left edge, and for horizontal panners it is measured from the top edge.
	
	If the value for i_size is invalid (less than 0 or greater than 100), it
	will be set to 50.
	
	This also applies to the value of i_size used with the constructor.
	*/
  void         setSeparator(int);

  /// Set panner to an absoulte pixel value.
  /**
	Same as setSeparator(), but i_size is interpreted as an absolute (pixel)
	value. The i_size parameter must be greater than current l_limit and less
	than current u_limit.
	*/
  void         setAbsSeparator(int);

  /// Set the range in which the panner may be moved.
  /**
	Set lower and upper limits as a range for the divider widget. The
	divider cannot be moved outside of these limits. The values are pixel
	values. The lower limit (l_limit) must be a positive integer between 0
	and the maximum width (height) of the panner. The upper limit (u_limit)
	may be either positive (if you want to measure it from the left (upper)
	edge), or negative (if you want to measure it from the right (lower)
	edge).  If the panner was created in U_PERCENT mode, use values 0 to 100
    also negatives for u_limit from right edge.
    To clear the limits, set both l_limit and u_limit to zero.
	*/
  void         setLimits(int, int);

  /// Get the maximum pixel value that the panner can be moved to.
  /**
    Get the maximum pixel value that the separator bar can be moved to.
	*/
  int          getMaxValue();
    
  /// Returns the current position of the separator as a percentage.
  /**
    Get the current position of the separator bar as a percentage.
    This can be used to save the position to the config file so the panner
    can be reset back to a save state.  Or if the panner is turned off &
    on again, the previous setting can be restored.
	*/
  int          getSeparator();
    
  /// Returns the current pixel position of the separator.
  /**
    Get the current pixel position of the separator bar.
    This can be used to save the position to the config file so the panner
    can be reset back to a save state.  Or if the panner is turned off &
    on again, the previous setting can be restored.
	*/
  int          getAbsSeparator();
    
private:
  enum {P_ORIENTATION = 1, P_UNITS = 2};
  virtual void resizeEvent(QResizeEvent *);
  bool         eventFilter(QObject *, QEvent *);
  void         setDividerGeometry(int);
  bool         checkRange(int &);
  int          u_coord, delta, old_coord;
  unsigned     u_flags;
  QWidget      *cw0, *cw1;
  QFrame       *divider;
  int          l_limit, u_limit, pos;
    
  signals:
  void positionChanged();
};

/*
 * $Id$
 *
 * $Log$
 * Revision 1.2  1997/05/02 16:46:41  kalle
 * Kalle: You may now override how KApplication reacts to external changes
 * KButton uses the widget default palette
 * new kfontdialog version 0,5
 * new kpanner by Paul Kendall
 * new: KIconLoader
 *
 * Revision 1.1.1.1  1997/04/13 14:42:43  cvsuser
 * Source imported
 *
 * Revision 1.1.1.1  1997/04/09 00:28:09  cvsuser
 * Sources imported
 *
 * Revision 1.3  1997/03/09 17:17:25  kalle
 * Umformatiert
 *
 * Revision 1.2  1997/03/09 16:47:45  kalle
 * Documentation added
 *
 * Revision 1.1  1997/03/09 16:41:58  kalle
 * Initial revision
 *
 */

#endif

