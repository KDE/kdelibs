/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)

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
//-----------------------------------------------------------------------------
// KDE color selection dialog.

// layout managment added Oct 1997 by Mario Weilguni 
// <mweilguni@sime.com>

#ifndef __KCOLORDLG_H__
#define __KCOLORDLG_H__

#include <kdialogbase.h>
#include <qtableview.h>
#include <qframe.h>
#include <qrangecontrol.h>
#include <qpixmap.h>
#include <qcombobox.h>
#include "kselect.h"

class QLineEdit;

/**
* Widget for Hue/Saturation selection.
* The actual values can be fetched using the inherited xValue and yValue
* methods.
*
* @see KXYSelector, KValueSelector, KColorDialog
* @author Martin Jones (mjones@kde.org)
* @version $Id$
*/
class KHSSelector : public KXYSelector
{
  Q_OBJECT
public:
  KHSSelector( QWidget *parent );

private:
  void drawPalette();

protected:
  virtual void resizeEvent( QResizeEvent * );
  virtual void drawContents( QPainter *painter );

protected:
  QPixmap pixmap;
};

/**
* Dialog for colour value selection.
*
* @see KHSSelector, KColorDialog
* @author Martin Jones (mjones@kde.org)
* @version $Id$
*/
class KValueSelector : public KSelector
{
  Q_OBJECT
public:
  KValueSelector( QWidget *parent );

  void setHue( int h )	{	hue = h; }
  void setSaturation( int s )	{	sat = s; }

  void drawPalette();

protected:
  virtual void resizeEvent( QResizeEvent * );
  virtual void drawContents( QPainter *painter );

protected:
  int hue;
  int sat;
  QPixmap pixmap;
};


/**
* A table of editable colour cells.
* @author Martin Jones <mjones@kde.org>
* @version $Id$
*/
class KColorCells : public QTableView
{
  Q_OBJECT
public:
  KColorCells( QWidget *parent, int rows, int cols );
  ~KColorCells();

  void setColor( int colNum, const QColor &col );
  QColor color( int indx )
  {	return colors[indx]; }
  int numCells()
  {	return numRows() * numCols(); }
	
  int getSelected()
  {	return selected; }

  signals:
  void colorSelected( int col );

protected:
  virtual void paintCell( QPainter *painter, int row, int col );
  virtual void resizeEvent( QResizeEvent * );
  virtual void mouseReleaseEvent( QMouseEvent * );
  virtual void mousePressEvent( QMouseEvent * );
  virtual void mouseMoveEvent( QMouseEvent * );
  virtual void dragEnterEvent( QDragEnterEvent *);
  virtual void dropEvent( QDropEvent *);

  int posToCell(const QPoint &pos, bool ignoreBorders=false);

  QColor *colors;
  bool inMouse;
  QPoint mPos;
  int	selected;
  
};

/**
 * 
 */
class KColorPatch : public QFrame
{
  Q_OBJECT
public:
  KColorPatch( QWidget *parent );
  virtual ~KColorPatch();

  void setColor( const QColor &col );

signals:
  void colorChanged( const QColor&);

protected:
  virtual void drawContents( QPainter *painter );

  virtual void mouseMoveEvent( QMouseEvent * );
  virtual void dragEnterEvent( QDragEnterEvent *);
  virtual void dropEvent( QDropEvent *);

private:
  QColor color;
  uint pixel;
  int colContext;
};

/** 
 * KDE Color Selection dialog
 *
Features:

@li Colour selection from a standard system palette.
@li Colour selection from a Palette of H vs S and V selectors (similar to windoze).
@li Direct input of HSV or RGB values.
@li Saving of custom colors

Simplest use:

<pre>
	QColor myColor;
	int result = KColorDialog::getColor( myColor );
</pre>

 */
class KColorDialog : public KDialogBase
{
  Q_OBJECT

  public:
    /** 
     * Construct a KColorDialog 
     */
    KColorDialog( QWidget *parent = 0L, const char *name = 0L,
		  bool modal = FALSE );

    /** 
     * Retrieve the currently selected color. 
     */
    QColor color() { return selColor; }
  
    /**
     * This is probably the function you are looking for.
     * Just call this to pop up dialog get the selected color.
     * returns result().
     */
    static int getColor( QColor &theColor, QWidget *parent=0L );

  public slots:
    /** 
     * Preselect a color 
     */
    void setColor( const QColor &col );

  signals:
    /** 
     * Notify when a color is selected.
     * Connect to this to monitor the color as it as selected if you are
     * not running modal.
     */
    void colorSelected( const QColor &col );

  private slots:
    void slotRGBChanged( void );
    void slotHSVChanged( void );
    void slotHSChanged( int, int );
    void slotVChanged( int );
    void slotSysColorSelected( int );
    void slotCustColorSelected( int );
    void slotAddToCustom( void );
    void slotWriteSettings( void );

  private:
    void readSettings( void );
    void setRgbEdit( void );
    void setHsvEdit( void );

  private:
    KColorCells *sysColorCells;
    KColorCells *custColorCells;
    QLineEdit *hedit;
    QLineEdit *sedit;
    QLineEdit *vedit;
    QLineEdit *redit;
    QLineEdit *gedit;
    QLineEdit *bedit;
    KColorPatch *patch;
    KHSSelector *palette;
    KValueSelector *valuePal;
    QColor selColor;
};




/**
 * Combobox for colours.
 */
class KColorCombo : public QComboBox
{
	Q_OBJECT
public:
	KColorCombo( QWidget *parent, const char *name = 0L );

	void setColor( const QColor &col );

public slots:
	void slotActivated( int index );
	void slotHighlighted( int index );

signals:
	void activated( const QColor &col );
	void highlighted( const QColor &col );

protected:
	virtual void resizeEvent( QResizeEvent *re );

private:
	void addColors();
	QColor customColor;
	QColor color;
};

#endif		// __KCOLORDLG_H__

