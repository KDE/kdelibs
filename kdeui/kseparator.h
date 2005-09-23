/*
 *   Copyright (C) 1997  Michael Roth <mroth@wirlweb.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef __KSEPARATOR_H__
#define __KSEPARATOR_H__

#include <qframe.h>

#include <kdelibs_export.h>

/**
 * Standard horizontal or vertical separator.
 *
 * @author Michael Roth <mroth@wirlweb.de>
 */
class KDEUI_EXPORT KSeparator : public QFrame
{
  Q_OBJECT
  Q_PROPERTY( int orientation READ orientation WRITE setOrientation )
 public:
  /**
   * Constructor.
   * @param parent parent object.
   * @param f extra QWidget flags.
   **/
  KSeparator(QWidget* parent=0, Qt::WFlags f=0);

  /**
   * Constructor.
   * @param orientation Set the orientation of the separator.
   * Possible values are HLine or Horizontal and VLine or Vertical.
   * @param parent parent object.
   * @param f extra QWidget flags.
   **/
  KSeparator(int orientation, QWidget* parent=0, 
	     Qt::WFlags f=0);
  
  /**
   * Returns the orientation of the separator.
   * @return int Possible values are VLine and HLine.
   **/
  int orientation() const;
  
  /**
   * Set the orientation of the separator to @p orient
   *
   * @param orient Possible values are VLine and HLine.
   */
  void setOrientation(int orient);
  
  /**
   * The recommended height (width) for a horizontal (vertical) separator.
   **/
  virtual QSize sizeHint() const;

protected:
  void paintEvent(QPaintEvent* pe);

  virtual void virtual_hook( int id, void* data );
private:
  class KSeparatorPrivate* d;
};


#endif // __KSEPARATOR_H__
