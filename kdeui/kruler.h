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
 * A ruler widget.
 *
 * The vertical ruler looks similar to this:
 *
 *<pre>
 *    meters                       inches
 *
 *    ------   <--- end mark  ---> ------
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
 * There are tiny marks, little marks, medium marks, and big marks along the
 *  ruler.
 *
 * To receive mouse clicks or mouse moves,
 * the class has to be overloaded.
 *
 * For performance reasons, the public methods don't call @ref QWidget::repaint().
 * (Slots do, see documentation below.)
 * All the changed settings will be painted once after leaving
 * to the main event loop.
 * For performance painting the slot methods should be used,
 * they do a fast @ref QWidget::repaint() call after changing the values.
 * For setting multiple values like @ref minValue(), @ref maxValue(), @ref offset() etc.
 * using the public methods is recommended
 * so the widget will be painted only once when entering the main event loop.
 *
 * @short A ruler widget.
 * @author Jörg Habenicht
 */
class KRuler : public QFrame
{
Q_OBJECT
public:

  /**
   * Direction of the ruler.
   * Has to be defined when constructing the widget.
   * @see KRuler()
   * @short ruler Direction in window.
   */
  enum direction{ horizontal, vertical };

  /**
   * The types of units used.
   **/
  enum metric_style{ custom=0, pixel, inch, millimetres, centimetres, metres };

  /**
   * The style (or look) of the ruler.
   **/
  enum paint_style{ flat, raised, sunken };


  /**
   * Constructor, requires the ruler direction.
   *
   * @p parent, @p name, @p f and @p allowLines are passed to QFrame.
   * The default look is a raised widget
   * but may be changed with the inherited @ref QFrame methods.
   *
   * @param dir        Cirection of the ruler.
   * @param parent     Will be handed over to @ref QFrame.
   * @param name       Will be handed over to @ref QFrame.
   * @param f          Will be handed over to @ref QFrame.
   * @param allowLines Will be handed over to @ref QFrame.
   *
   **/
  KRuler(direction dir, QWidget *parent=0, const char *name=0, 
	 WFlags f=0, bool allowLines=TRUE);

  /**
   * Constructor, requires the direction of the ruler and an initial width.
   *
   * The width sets the fixed width of the widget. This is usefullif you
   * want to draw the ruler bigger or smaller than the default size.
   * Note: The size of the marks doesn't change.
   * @p parent, @p name, @p f and @p allowLines are passed to @ref QFrame.
   *
   * @param dir         Direction of the ruler.
   * @param widgetWidth Fixed width of the widget.
   * @param parent      Will be handed over to @ref QFrame.
   * @param name        Will be handed over to @ref QFrame.
   * @param f           Will be handed over to @ref QFrame.
   * @param allowLines  Will be handed over to @ref QFrame.
   *
   */
  KRuler(direction dir, int widgetWidth, QWidget *parent=0, 
	 const char *name=0, WFlags f=0, bool allowLines=TRUE);

  /**
   * Destructor.
   */
  ~KRuler();

  /**
   * Set the minimal value of the ruler pointer (default is 0).
   *
   * This method calls @ref update() so that the widget is painted after leaving
   * to the main event loop.
   *
   **/
  void setMinValue(int);

  /**
   * Retrieve the minimal value of the ruler pointer.
   **/
  inline int getMinValue() const;

  /**
   * Set the maximum value of the ruler pointer (default is 100).
   *
   * This method calls @ref update() so that the widget is painted after leaving
   * to the main event loop.
   */
  void setMaxValue(int);
  
  /** returns the maximal value of the ruler pointer.
   */
  inline int getMaxValue() const;

  /**
   * Set minimum and maxmimum values of the ruler pointer.
   *
   * This method calls @ref update() so that the widget is painted after leaving
   * to the main event loop.
   */
  void setRange(int min, int max);

  /**
   * Set the value of the ruler pointer.
   *
   * The value is indicated by painting the ruler pointer at the
   * corresponding position.
   * This method calls @ref update() so that the widget is painted after leaving
   * to the main event loop.
   */
  void setValue(int);
  inline int getValue() const;

  /**
   * Set distance between tiny marks.
   *
   * This is mostly used in the English system (inches) with distance of 1. 
   */
  void setTinyMarkDistance(int);
  /**
   * Retrieve the distance between tiny marks.
   **/
  inline int getTinyMarkDistance() const;

  /** 
   * Set the distance between little marks.
   *
   * The default value is 1 in the metric system and 2 in the English (inches) system.
   */
  void setLittleMarkDistance(int);

  /** 
   * Retrieve the distance between little marks.
   */
  inline int getLittleMarkDistance() const;

  /**
   * Set the distance between medium marks.
   *
   * For English (inches) styles it defaults to twice the little mark distance.
   * For metric styles it defaults to five times the little mark distance.
   **/
  void setMediumMarkDistance(int);
  inline int getMediumMarkDistance() const;

  /** 
   * Set distance between big marks.
   *
   * For English (inches) or metric styles it is twice the medium mark distance.
   **/
  void setBigMarkDistance(int);
  /**
   * Retrieve the distance between big marks.
   **/
  inline int getBigMarkDistance() const;

  /**
   * Show/hide tiny marks.
   **/
  void showTinyMarks(bool);
  /**
   * Show/hide little marks.
   **/
  void showLittleMarks(bool);
  /**
   * Show/hide medium marks.
   **/
  void showMediumMarks(bool);
  /**
   * Show/hide big marks.
   **/
  void showBigMarks(bool);
  /**
   * Show/hide end marks.
   **/
  void showEndMarks(bool);

  /**
   * Sets the value that is shown per little mark.
   *
   * For metric systems this is 1 per mark.
   * Note: If you set the value for little marks the values for 
   * medium and big marks are updated according to the
   * the values set in the setXXMarkDistance(int) methods.
   **/
  void setValuePerLittleMark(int);

  /**
   * Sets the value that is shown per medium mark.
   *
   * For metric systems this is 5.
   * Display of little marks is automatically disabled and
   * big marks are updated according to 
   * the values set in the @ref setBigMarkDistance() method.
   **/
  void setValuePerMediumMark(int);

  /**
   * Sets the value that is shown per big mark.
   *
   * For metric systems this is 1
   * Display of little and medium marks marks is disabled.
   */
  void setValuePerBigMark(int);

  /**
   * Show/hide number values of the little marks.
   *
   * Default is @p false.
   **/
  void showLittleMarkLabel(bool);

  /**
   * Show/hide number values of the medium marks.
   *
   * Default is @p false.
   **/
  void showMediumMarkLabel(bool);

  /**
   * Show/hide number values of the big marks.
   *
   * Default is @p false.
   **/
  void showBigMarkLabel(bool);

  /**
   * Show/hide number values of the end marks.
   *
   * Default is @p false.
   **/
  void showEndLabel(bool);

  /**
   * Set the label this is drawn at the beginning of the visible part
   * of the ruler.
   **/
  void setEndLabel(const QString&);

  /**
   * Set up the necessary tasks for the provided styles.
   *
   * A convenience method.
   **/
  void setRulerStyle(metric_style);
#if implemented
  inline metric_style getMetricRulerStyle() const;

  /** currently not implemented */
  void setRulerStyle(paint_style);
  /** currently not implemented */
  inline paint_style getPaintRulerStyle() const;
#endif

  /** currently not implemented */
  void setTickStyle(paint_style);

#if implemented
  /** currently not implemented */
  inline paint_style getTickStyle() const;
#endif

  /**
   * Set the number of pixels between two base marks.
   *
   * Calling this method stretches or shrinks your ruler.
   *
   * For pixel display (@ref metric_style) the value is 10.0 marks
   * per pixel ;-)
   * For English (inches) it is 9.0, and for centimetres ~2.835 -> 3.0 .
   * If you want to magnify your part of display, you have to
   * adjust the mark distance @bf here.
   * Notice: The double type is only supported to give the possibility
   *         of having some double values.
   *         It should be used with care.  Using values below 10.0
   *         shows visible jumps of markpositions (e.g. 2.345).
   *         Using whole numbers is highly recommended.
   * To use @p int values use setPixelPerMark((int)your_int_value);
   * default: 1 mark per 10 pixels 
   */
  void setPixelPerMark(double);
  /*  void setPixelPerMark(int); */
  /**
   * Retrieve the number of pixels between two base marks.
   **/
  inline double getPixelPerMark() const;

  /**
   * Set the number of pixels by which the ruler may slide up.
   *
   * This doesn't affect the position of the ruler pointer.
   * Only the visible part of the ruler is moved.
   **/
  void slideup(int count = 1);
  /**
   * Set the number of pixels by which the ruler may slide down.
   *
   * This doesn't affect the position of the ruler pointer.
   * Only the visible part of the ruler is moved.
   **/
  void slidedown(int count = 1);

  /**
   * Set ruler slide offset.
   *
   * This is like @ref slideup() or @ref slidedown() with an absolute offset
   * from the start of the ruler.
   **/
  void setOffset(int offset);
  /**
   * Get the current ruler offset.
   **/
  inline int getOffset() const;

public slots:

  /**
   * Sets the pointer to a new position.
   *
   * The offset is NOT updated.
   * @ref QWidget::repaint() is called afterwards.
   **/
  void slotNewValue(int);

  /**
   * Sets the ruler marks to a new position.
   *
   * The pointer is NOT updated.
   * @ref QWidget::repaint() is called afterwards.
   **/
  void slotNewOffset(int);

protected:
  virtual void drawContents(QPainter *);

private:

  direction dir;
  int tmDist;
  int lmDist;
  int mmDist;
  int bmDist;
  int offset;
  bool showtm; /* show tiny, little, medium, big, endmarks */
  bool showlm;
  bool showmm;
  bool showbm;
  bool showem;
  int valuelm; /* value per little, medium, big mark */
  int valuemm;
  int valuebm;
  bool showValuelm;
  bool showValuemm;
  bool showValuebm;
  double ppm; /* pixel per mark */

  QString endlabel;
  QRangeControl range;

  class KRestrictedLinePrivate;
  KRestrictedLinePrivate *d;
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

#if implemented
KRuler::metric_style 
KRuler::getMetricRulerStyle() const;

KRuler::paint_style 
KRuler::getPaintRulerStyle() const;

KRuler::paint_style 
KRuler::getTickStyle() const;
#endif

double 
KRuler::getPixelPerMark() const
{ return ppm; }

int 
KRuler::getOffset() const
{ return offset; }

#endif
