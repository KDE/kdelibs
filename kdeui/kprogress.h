/*****************************************************************************
*                                                                            *
*  KProgress -- progress indicator widget for KDE by Martynas Kunigelis      *
*                                                                            *
*****************************************************************************/

#ifndef _KPROGRES_H
#define _KPROGRES_H

#include <qframe.h>
#include <qrangecontrol.h>

/** 
* KProgress is derived from QFrame and QRangeControl, so you can use all 
* the methods from those classes. The only difference is that setValue(int) 
* is now made a slot, so you can connect stuff to it. 
*
* None of the constructors take line step and page step as arguments, 
* so by default they're set to 1 and 10 respectively.
*
* The Blocked style ignores the textEnabled() setting and displays
* no text, since it looks truly ugly and there are other reasons. Signal 
* percentageChanged(int) is emmitted whenever the value changes so you
* can setup a different widget to display percentage and connect the
* signal to it. 
*
* @short A Progress indicator widget
* @author Martynas Kunigelis
* @version $Id$
*/
class KProgress : public QFrame, public QRangeControl {
	Q_OBJECT
public:
  /** 
  * Possible values for orientation 
  */
  enum Orientation { Horizontal, Vertical };

  /** 
  * Possible values for bar style. 
  *
  * Solid means one continuous progress bar, Blocked means a 
  * progress bar made up of several blocks. 
  */ 
  enum BarStyle { Solid, Blocked };

  /** 
  * Construct a default KProgress bar. Orientation is horizontal. 
  */
  KProgress(QWidget *parent=0, const char *name=0);

  /** 
  * Construct a KProgress bar with an orientation. 
  */
  KProgress(Orientation, QWidget *parent=0, const char *name=0);

  /** 
  * Construct a KProgress bar with minimum, maximum and initial value. 
  */
	KProgress(int minValue, int maxValue, int value, Orientation, 
				QWidget *parent=0, const char *name=0);
	
  /** 
  * Destructor 
  */
  ~KProgress();
	
  /** 
  * Set the progress bar style. Allowed values are Solid and Blocked. 
  */
  void setBarStyle(BarStyle style);  
	
  /** 
  * Set the color of the progress bar. 
  */
  void setBarColor(const QColor &); 
  
  /** 
  * Set a pixmap to be shown in the progress bar. 
  */
  void setBarPixmap(const QPixmap &);

  /** 
  * Set the orientation of the progress bar. 
  * Allowed values are Horizonzal and Vertical. 
  */
  void setOrientation(Orientation);

  /**
  * If this is set to TRUE, the progress text will be displayed.
  *
  */
  void setTextEnabled(bool);
	
  /** 
  * Retrieve the bar style. 
  */
  BarStyle barStyle() const;

  /** 
  * Retrieve the bar color. 
  */
  const QColor &barColor() const;

  /** 
  * Retrieve the bar pixmap. 
  */
  const QPixmap *barPixmap() const;

  /** 
  * Retrieve the orientation. 
  */
  Orientation orientation() const;

  /**
  * Returns TRUE if progress text will be displayed, FALSE otherwise.
  */
  bool textEnabled() const;

  /**
   * Returns the recommended width for vertical progress bars or
   * the recommended height for vertical progress bars
   */
  virtual QSize sizeHint() const;

  virtual QSizePolicy sizePolicy() const;
			
public slots:
	void setValue(int);
	void advance(int);
	
signals:
	void percentageChanged(int);
	
protected:
	void valueChange();
	void rangeChange();
	void styleChange( GUIStyle );
	void paletteChange( const QPalette & );
	void drawContents( QPainter * );
	
private:
	QPixmap		*bar_pixmap;
	QColor		bar_color;
	QColor		bar_text_color;
	QColor		text_color;
	QRect fr;
	BarStyle	bar_style;
	Orientation orient;
	bool		text_enabled;
	void initialize();
	int recalcValue(int);
	void drawText(QPainter *);
	void adjustStyle();
};


#endif
