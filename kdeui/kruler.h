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
  Q_PROPERTY( int minValue READ minValue WRITE setMinValue )
  Q_PROPERTY( int maxValue READ maxValue WRITE setMaxValue )
  Q_PROPERTY( int value READ value WRITE setValue )
  Q_PROPERTY( bool showTinyMarks READ showTinyMarks WRITE setShowTinyMarks )
  Q_PROPERTY( bool showLittleMarks READ showLittleMarks WRITE setShowLittleMarks )
  Q_PROPERTY( bool showMediumMarks READ showMediumMarks WRITE setShowMediumMarks )
  Q_PROPERTY( bool showBigMarks READ showBigMarks WRITE setShowBigMarks )
  Q_PROPERTY( bool showPointer READ showPointer WRITE setShowPointer )
  Q_PROPERTY( bool showEndLabel READ showEndLabel WRITE setShowEndLabel )
  Q_PROPERTY( int tinyMarkDistance READ tinyMarkDistance WRITE setTinyMarkDistance )
  Q_PROPERTY( int littleMarkDistance READ littleMarkDistance WRITE setLittleMarkDistance )
  Q_PROPERTY( int mediumMarkDistance READ mediumMarkDistance WRITE setBigMarkDistance )
  Q_PROPERTY( int bigMarkDistance READ bigMarkDistance WRITE setBigMarkDistance )
  Q_PROPERTY( double pixelPerMark READ pixelPerMark WRITE setPixelPerMark )
  Q_PROPERTY( bool lengthFixed READ lengthFixed WRITE setLengthFixed )
      
public:

/*
#define KRULER_ROTATE_TEST KRULER_ROTATE_TEST
#undef KRULER_ROTATE_TEST
#ifdef KRULER_ROTATE_TEST
  double xtrans, ytrans, rotate;
# warning tmporaer variablen eingeschaltet
#endif
*/

  /**
   * The types of units used.
   **/
  enum MetricStyle { Custom=0, Pixel, Inch, Millimetres, Centimetres, Metres };

  /**
   * The style (or look) of the ruler.
   **/
    //  enum PaintStyle { Flat, Raised, Sunken };

  /**
   * Constructs a horizontal ruler.
   */
  KRuler(QWidget *parent=0, const char *name=0);
  /**
   * Constructs a ruler with orientation @p orient.
   *
   * @p parent, @p name and @p f are passed to QFrame.
   * The default look is a raised widget
   * but may be changed with the inherited @ref QFrame methods.
   *
   * @param orient     Orientation of the ruler.
   * @param parent     Will be handed over to @ref QFrame.
   * @param name       Will be handed over to @ref QFrame.
   * @param f          Will be handed over to @ref QFrame.
   * @param allowLines Will be handed over to @ref QFrame.
   *
   **/
  KRuler(Orientation orient, QWidget *parent=0, const char *name=0, 
	 WFlags f=0);

  /**
   * Constructs a ruler with orientation @p orient and initial width @p widgetWidth.
   *
   * The width sets the fixed width of the widget. This is useful if you
   * want to draw the ruler bigger or smaller than the default size.
   * Note: The size of the marks doesn't change.
   * @p parent, @p name and @p f are passed to @ref QFrame.
   *
   * @param orient      Orientation of the ruler.
   * @param widgetWidth Fixed width of the widget.
   * @param parent      Will be handed over to @ref QFrame.
   * @param name        Will be handed over to @ref QFrame.
   * @param f           Will be handed over to @ref QFrame.
   * @param allowLines  Will be handed over to @ref QFrame.
   *
   */
  KRuler(Orientation orient, int widgetWidth, QWidget *parent=0, 
	 const char *name=0, WFlags f=0);

  /**
   * Destructor.
   */
  ~KRuler();

  /**
   * Sets the minimal value of the ruler pointer (default is 0).
   *
   * This method calls @ref update() so that the widget is painted after leaving
   * to the main event loop.
   *
   **/
  void setMinValue(int);

  /**
   * Returns the minimal value of the ruler pointer.
   **/
  inline int minValue() const;

  /**
   * Sets the maximum value of the ruler pointer (default is 100).
   *
   * This method calls @ref update() so that the widget is painted after leaving
   * to the main event loop.
   */
  void setMaxValue(int);
  
  /**
   * Returns the maximal value of the ruler pointer.
   */
  inline int maxValue() const;

  /**
   * Sets minimum and maximum values of the ruler pointer.
   *
   * This method calls @ref update() so that the widget is painted after leaving
   * to the main event loop.
   */
  void setRange(int min, int max);

  /**
   * Sets the value of the ruler pointer.
   *
   * The value is indicated by painting the ruler pointer at the
   * corresponding position.
   * This method calls @ref update() so that the widget is painted after leaving
   * to the main event loop.
   */
  void setValue(int);
  inline int value() const;

  /**
   * Sets the distance between tiny marks.
   *
   * This is mostly used in the English system (inches) with distance of 1. 
   */
  void setTinyMarkDistance(int);
  /**
   * Returns the distance between tiny marks.
   **/
  inline int tinyMarkDistance() const;

  /** 
   * Sets the distance between little marks.
   *
   * The default value is 1 in the metric system and 2 in the English (inches) system.
   */
  void setLittleMarkDistance(int);

  /** 
   * Returns the distance between little marks.
   */
  inline int littleMarkDistance() const;

  /**
   * Sets the distance between medium marks.
   *
   * For English (inches) styles it defaults to twice the little mark distance.
   * For metric styles it defaults to five times the little mark distance.
   **/
  void setMediumMarkDistance(int);
  inline int mediumMarkDistance() const;

  /** 
   * Sets distance between big marks.
   *
   * For English (inches) or metric styles it is twice the medium mark distance.
   **/
  void setBigMarkDistance(int);
  /**
   * Returns the distance between big marks.
   **/
  inline int bigMarkDistance() const;

  /**
   * Shows/hides tiny marks.
   **/
  void setShowTinyMarks(bool);
  bool showTinyMarks() const;
  /**
   * Shows/hides little marks.
   **/
  void setShowLittleMarks(bool);
  bool showLittleMarks() const;
  /**
   * Shows/hides medium marks.
   **/
  void setShowMediumMarks(bool);
  bool showMediumMarks() const;
  /**
   * Shows/hides big marks.
   **/
  void setShowBigMarks(bool);
  bool showBigMarks() const;
  /**
   * Shows/hides end marks.
   **/
  void setShowEndMarks(bool);
  bool showEndMarks() const;
  /**
   * Shows/hides the pointer.
   */
  void setShowPointer(bool);
  bool showPointer() const;

  //#### KDE4: The next 3 need to go.
  /**
   * @deprecated
   * This method has no effect other than an update. Do not use.
   **/
  void setValuePerLittleMark(int);

  /**
   * @deprecated
   * This method has no effect other than an update. Do not use.
   **/
  void setValuePerMediumMark(int);

  /**
   * @deprecated
   * This method has no effect other than an update. Do not use.
   */
  void setValuePerBigMark(int);

  /**
   * Show/hide number values of the little marks.
   *
   * Default is @p false.
   **/
    //  void setShowLittleMarkLabel(bool);

  /**
   * Show/hide number values of the medium marks.
   *
   * Default is @p false.
   **/
    //  void setShowMediumMarkLabel(bool);

  /**
   * Show/hide number values of the big marks.
   *
   * Default is @p false.
   **/
    //  void showBigMarkLabel(bool);

  /**
   * Show/hide number values of the end marks.
   *
   * Default is @p false.
   **/
   void setShowEndLabel(bool);
   bool showEndLabel() const;

  /**
   * Sets the label this is drawn at the beginning of the visible part
   * of the ruler to @p label
   **/
  void setEndLabel(const QString&);
  QString endLabel() const;

  /**
   * Sets up the necessary tasks for the provided styles.
   *
   * A convenience method.
   **/
  void setRulerMetricStyle(KRuler::MetricStyle);

  /**
   * Sets the number of pixels between two base marks.
   *
   * Calling this method stretches or shrinks your ruler.
   *
   * For pixel display (@ref MetricStyle) the value is 10.0 marks
   * per pixel ;-)
   * For English (inches) it is 9.0, and for centimetres ~2.835 -> 3.0 .
   * If you want to magnify your part of display, you have to
   * adjust the mark distance @p here.
   * Notice: The double type is only supported to give the possibility
   *         of having some double values.
   *         It should be used with care.  Using values below 10.0
   *         shows visible jumps of markpositions (e.g. 2.345).
   *         Using whole numbers is highly recommended.
   * To use @p int values use setPixelPerMark((int)your_int_value);
   * default: 1 mark per 10 pixels 
   */
  void setPixelPerMark(double rate);

  /**
   * Returns the number of pixels between two base marks.
   **/
  inline double pixelPerMark() const;

  /**
   * Sets the length of the ruler, i.e. the difference between
   * the begin mark and the end mark of the ruler.
   *
   * Same as (width() - offset())
   *
   * when the length is not locked, it gets adjusted with the
   * length of the widget.
   */
  void setLength(int);
  int length() const;

  /**
   * Locks the length of the ruler, i.e. the difference between
   * the two end marks doesn't change when the widget is resized.
   *
   * @param fix fixes the length, if true
   */
  void setLengthFixed(bool fix);
  bool lengthFixed() const;

  /**
   * Sets the number of pixels by which the ruler may slide up or left.
   * The number of pixels moved is realive to the previous position.
   * The Method makes sense for updating a ruler, which is working with 
   * a scrollbar.
   *
   * This doesn't affect the position of the ruler pointer.
   * Only the visible part of the ruler is moved.
   *
   * @param count Number of pixel moving up or left relative to the previous position
   **/
  void slideUp(int count = 1);

  /**
   * Sets the number of pixels by which the ruler may slide down or right.
   * The number of pixels moved is realive to the previous position.
   * The Method makes sense for updating a ruler, which is working with 
   * a scrollbar.
   *
   * This doesn't affect the position of the ruler pointer.
   * Only the visible part of the ruler is moved.
   *
   * @param count Number of pixel moving up or left relative to the previous position
   **/
  void slideDown(int count = 1);

  /**
   * Sets the ruler slide offset.
   *
   * This is like @ref slideup() or @ref slidedown() with an absolute offset
   * from the start of the ruler.
   *
   * @param offset Number of pixel to move the ruler up or left from the beginning
   **/
  void setOffset(int offset);

  /**
   * Returns the current ruler offset.
   **/
  inline int offset() const;

  int endOffset() const;

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

  void slotEndOffset(int);

protected:
  /**
   * @reimplemented
   */
  virtual void drawContents(QPainter *);

private:
  void init();

  QRangeControl range;
  Orientation dir;
  int tmDist;
  int lmDist;
  int mmDist;
  int bmDist;
  int offset_;
  bool showtm; /* show tiny, little, medium, big, endmarks */
  bool showlm;
  bool showmm;
  bool showbm;
  bool showem;
  
  // #### KDE4 : All value* and show* below should be removed, unused
  
  int valuelm; /* value per little, medium, big mark */
  int valuemm;
  int valuebm;
  bool showValuelm;
  bool showValuemm;
  bool showValuebm;
  
  double ppm; /* pixel per mark */

  QString endlabel;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KRulerPrivate;
  KRulerPrivate *d;
};


int
KRuler::minValue() const
{ return range.minValue(); }

int 
KRuler::maxValue() const
{ return range.maxValue(); }

int 
KRuler::value() const
{ return range.value(); }

int 
KRuler::tinyMarkDistance() const
{ return tmDist; }

int 
KRuler::littleMarkDistance() const
{ return lmDist; }

int 
KRuler::mediumMarkDistance() const
{ return mmDist; }

int 
KRuler::bigMarkDistance() const
{ return bmDist; }

double 
KRuler::pixelPerMark() const
{ return ppm; }

int 
KRuler::offset() const
{ return offset_; }

#endif
