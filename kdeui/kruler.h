/* -*- c++ -*- */
/* This file is part of the KDE libraries
    Copyright (C) 1998 Jörg Habenicht (j.habenicht@europemail.com)

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

#ifndef KRULER_H
#define KRULER_H

#include <qframe.h>
#include <qrangecontrol.h>

/**
 * The vertical ruler looks similar to this:
 *
 *<pre>
 *    meters                       inches
 *
 *    ------   <----end mark-----> ------
 *        --                            -
 *        --   <---little mark--->     --
 *        --                            -
 *        --                          ---
 *       ---   <---medium mark          -
 *        --                           --
 *        --        tiny mark---->      -
 *        --                         ----
 *        --                            -
 *      ----   <-----big mark          --
 *        --                            -
 *      |>--   <--ruler pointer-->   |>--
 *
 * </pre>
 *
 * To receive mouse clicks or mouse moves,
 * the class has to be overloaded.
 *
 * For performance reasons, the public methods doesn't call repaint(),
 * than update except the public slot methods as described.
 * so all the changed settings will be painted once after leaving
 * to the main event loop.
 * For performance painting the slot methods should be used,
 * they do a fast repaint(..) call after changing the values.
 * For setting multiple values like minValue(), maxValue(), offset() etc.
 * using the public methods is recommended,
 * so the widget will be painted only once when entering the main event loop.
 *
 * @short A ruler widget
 * @author Jörg Habenicht
 */
class KRuler : public QFrame
{
Q_OBJECT
public:

  /**
   * direction of the ruler.
   * Has to be defined when constructing the widget.
   * see KRuler();
   * @short ruler direction in window
   */
  enum direction{ horizontal, vertical };


  enum metric_style{ custom=0, pixel, inch, millimetres, centimetres, metres };
  enum paint_style{ flat, raised, sunken };


  /* create new ruler with "dir" direction
   * ruler will be raised panel frame.
   */

  /**
   * Constructor, requires the direction to paint to.
   * parent, name, f and allowLines are handed through to QFrame.
   * The default look is a raised widget,
   * but may be changed with the QFrame methods.
   *
   * @param dir        direction of the ruler
   * @param parent     will be handed over to QFrame
   * @param name       will be handed over to QFrame
   * @param f          will be handed over to QFrame
   * @param allowLines will be handed over to QFrame
   *
   * @short Constructor
   */
KRuler(direction dir, QWidget *parent=0, const char *name=0, WFlags f=0, bool allowLines=TRUE);

  /**
   * Constructor, requires the direction to paint to and an initial width.
   * The width sets the fixed width of the widget. This is usefull, if you
   * want to draw the ruler bigger or smaller than the default size.
   * Note: The size of the marks doesn't change.
   * parent, name, f and allowLines are handed through to QFrame
   *
   * @param dir         direction of the ruler
   * @param widgetWidth fixed width of the widget
   * @param parent      will be handed over to QFrame
   * @param name        will be handed over to QFrame
   * @param f           will be handed over to QFrame
   * @param allowLines  will be handed over to QFrame
   *
   * @short Constructor
   */
KRuler(direction dir, int widgetWidth, QWidget *parent=0, const char *name=0, WFlags f=0, bool allowLines=TRUE);

  /**
   * Destructor.
   */
~KRuler();

  /** minimum value reachable, default: 0
   * calls update(), so the widget is painted after leaving
   * the main event loop
   *
   * @short set the minimal value of the ruler pointer
   */
  void setMinValue(int);

  /**
   * returns the minimal value of the ruler pointer
   *
   * @short get the minimal value of the ruler pointer
   */
  inline int getMinValue() const;

  /** maximum value of the ruler pointer, default: 100 .
   * calls update(), so the widget is painted after leaving
   * the main event loop
   *
   * @short set the maximal value of the ruler pointer
   */
  void setMaxValue(int);
  
  /** returns the maximal value of the ruler pointer.
   */
  inline int getMaxValue() const;

  /** sets minimal and maxmal values of the ruler pointer.
   * calls update(), so the widget is painted after leaving
   * to the main event loop.
   */
  void setRange(int min, int max);

  /** set the value of the ruler pointer,
   * the value is shown with ruler pointer painted.
   * calls update(), the widget is painted after leaving
   * to the main event loop.
   */
  void setValue(int);
  inline int getValue() const;

  /** distance of tiny marks.
   * mostly used in inch system with distance of 1. 
   */
  void setTinyMarkDistance(int);
  inline int getTinyMarkDistance() const;

  /** set the paint distance of the little marks.
   * value is 1 in metric system and 2 in inch system.
   */
  void setLittleMarkDistance(int);
  inline int getLittleMarkDistance() const;

  /** set the paint distance of the medium marks.
   * for inch styles it is twice the little mark distance,
   * for metric styles it is five times.
   * default: 5 */
  void setMediumMarkDistance(int);
  inline int getMediumMarkDistance() const;

  /** set the paint distance of the big marks.
   * for inch or metric styles it is twice the medium mark distance.
   * default: 10 */
  void setBigMarkDistance(int);
  inline int getBigMarkDistance() const;

  /* base marks are shown constantly */
  void showTinyMarks(bool);
  void showLittleMarks(bool);
  void showMediumMarks(bool);
  void showBigMarks(bool);
  void showEndMarks(bool);

  /** sets the value that is shown per little mark.
   * for metric systems this is 1 per mark.
   * Note: If you set the value for little marks the values for 
   * medium and big marks are updated accordingly in respect to
   * the values set in "setXXMarkDistance(int)" */
  void setValuePerLittleMark(int);

  /** sets the value that is shown per little mark.
   * for metric systems this is 5
   * display of little marks is disabled,
   * big marks are updated */
  void setValuePerMediumMark(int);

  /** sets the value that is shown per little mark.
   * for metric systems this is 1
   * display of little and medium marks marks is disabled,
   */
  void setValuePerBigMark(int);

  /** paint number values of the little marks.
   * default: false */
  void showLittleMarkLabel(bool);

  /** paint number values of the medium marks.
   * default: false */
  void showMediumMarkLabel(bool);

  /** paint number values of the big marks.
   * default: true */
  void showBigMarkLabel(bool);

  /** paint number values of the little marks.
   * default: true */
  void showEndLabel(bool);

  /** label which is drawn at the beginning of the visible part
   * of the ruler */
  void setEndLabel(const char *);

  /** convenience method:
   * sets up the necessary tasks for the provided styles */
  void setRulerStyle(metric_style);
  inline metric_style getMetricRulerStyle() const;

  /** currently not implemented */
  void setRulerStyle(paint_style);
  /** currently not implemented */
  inline paint_style getPaintRulerStyle() const;
  /** currently not implemented */
  void setTickStyle(paint_style);
  /** currently not implemented */
  inline paint_style getTickStyle() const;

  /** Set the amount of pixel between two base marks.
   * Calling this method your are able to stretch or shrink your ruler.
   *
   * For pixel display the value is 10.0 marks per pixel ;-)
   * for inches it is 9.0, and for centimetres ~2,835 -> 3.0 .
   * If you like to magnify your part of display, you have to
   * adjust HERE your markdistance.
   * Notice: the floatingpoint type is only supported to give the possibility
   *         of having some floatingpoint values.
   *         It should be used with care, using values below 10.0
   *         shows visible jumps of markpositions (e.g. 2.345).
   *         Using whole numbers is highly recommended.
   * To use int values use setPixelPerMark((int)your_int_value);
   * default: 1 mark per 10 pixels 
   */
  void setPixelPerMark(double);
  /*  void setPixelPerMark(int); */
  inline double getPixelPerMark() const;

  /** the ruler may slide up and down "count" pixels
   * this doesn't affect the position of the ruler pointer,
   * only the visual part of the ruler is moved */
  void slideup(int count = 1);
  void slidedown(int count = 1);

  /** set ruler slide to "offset" from beginning.
   * like "slideup" or "slidedown" with an absolute offset */
  void setOffset(int);
  inline int getOffset() const;

public slots:

  /** sets the pointer to a new position.
   * the offset is NOT updated.
   * repaint() is called afterwards */
  void slotNewValue(int);

  /** sets the ruler marks to a new position.
   * the pointer is NOT updated.
   * repaint() is called afterwards */
  void slotNewOffset(int);

protected:
  virtual void drawContents(QPainter *);

private:

  direction dir;
  int tmDist, lmDist, mmDist, bmDist, offset;
  bool showtm, showlm, showmm, showbm, showem; /* show tiny, little, medium, big, endmarks */
  int valuelm, valuemm, valuebm; /* value per little, medium, big mark */
  bool showValuelm, showValuemm, showValuebm;
  double ppm; /* pixel per mark */

  QString endlabel;
  QRangeControl range;
};


int
KRuler::getMinValue() const
{ return range.minValue(); }

int 
KRuler::getMaxValue() const
{ return range.maxValue(); }

int 
KRuler::getValue() const
{ return range.value(); }

int 
KRuler::getTinyMarkDistance() const
{ return tmDist; }

int 
KRuler::getLittleMarkDistance() const
{ return lmDist; }

int 
KRuler::getMediumMarkDistance() const
{ return mmDist; }

KRuler::metric_style 
KRuler::getMetricRulerStyle() const;

KRuler::paint_style 
KRuler::getPaintRulerStyle() const;

KRuler::paint_style 
KRuler::getTickStyle() const;

double 
KRuler::getPixelPerMark() const
{ return ppm; }

int 
KRuler::getOffset() const
{ return offset; }

#endif
