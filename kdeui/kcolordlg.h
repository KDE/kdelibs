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

#include <qcombobox.h>
#include <kdialogbase.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qrangecontrol.h>
#include <qtableview.h>

#include "kselect.h"

class QLineEdit;
class KListBox;
class KPalette;
class KColorCells;


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
  /**
   * Constructs a hue/saturation selection widget.
   */
  KHSSelector( QWidget *parent=0, const char *name=0 );

  void updateContents();
    
protected:
  /**
   * Draws the contents of the widget on a pixmap,
   * which is used for buffering.
   */
  virtual void drawPalette( QPixmap *pixmap );
    
  virtual void resizeEvent( QResizeEvent * );
  /**
   * Reimplemented from KXYSelector. This drawing is
   * buffered in a pixmap here. As real drawing
   * routine, drawPalette() is used.
   */
  virtual void drawContents( QPainter *painter );

private:
  QPixmap pixmap;

  class KHSSelectorPrivate;
  KHSSelectorPrivate *d;
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
  /**
   * Constructs a widget for color selection.
   */
  KValueSelector( QWidget *parent=0, const char *name=0 );

  int hue() const
        { return _hue; }
  void setHue( int h )
        { _hue = h; }
  int saturation() const
        { return _sat; }
  void setSaturation( int s )
        { _sat = s; }
  void updateContents();
    
protected:
  /**
   * Draws the contents of the widget on a pixmap,
   * which is used for buffering.
   */
  virtual void drawPalette( QPixmap *pixmap );

  virtual void resizeEvent( QResizeEvent * );
  /**
   * Reimplemented from KSelector. The drawing is
   * buffered in a pixmap here. As real drawing
   * routine, drawPalette() is used.
   */
  virtual void drawContents( QPainter *painter );

private:
  int _hue;
  int _sat;
  QPixmap pixmap;

  class KValueSelectorPrivate;
  KValueSelectorPrivate *d;
};


class KColor : public QColor
{
public:
  KColor();
  KColor( const KColor &col);
  KColor( const QColor &col);

  KColor& operator=( const KColor& col);

  bool operator==( const KColor& col) const;

  void setHsv(int _h, int _s, int _v);
  void setRgb(int _r, int _g, int _b);

  void rgb(int *_r, int *_g, int *_b);
  void hsv(int *_h, int *_s, int *_v);
protected:
  int h;
  int s;
  int v;
  int r;
  int g;
  int b;

private:
  class KColorPrivate;
  KColorPrivate *d;

};

/**
 * A color palette in table form.
 * @author Waldo Bastian <bastian@kde.org>
 * @version $Id$
 **/
class KPaletteTable : public QWidget
{
  Q_OBJECT
public:
  KPaletteTable( QWidget *parent, int minWidth=210, int cols = 16);
  ~KPaletteTable();
  void addToCustomColors( const QColor &);
  void addToRecentColors( const QColor &);
  QString palette();
public slots:
  void setPalette(const QString &paletteName);
signals:
  void colorSelected( const QColor &, const QString & );

protected slots:
  void slotColorCellSelected( int );
  void slotColorTextSelected( const QString &colorText );
  void slotSetPalette( const QString &_paletteName );
  void slotShowNamedColorReadError( void );

protected:
  void readNamedColor( void );

protected:
  QString i18n_customColors;
  QString i18n_recentColors;
  QString i18n_namedColors;
  QComboBox *combo;
  KColorCells *cells;
  QScrollView *sv;
  KListBox *mNamedColorList;
  KPalette *mPalette;
  int mMinWidth;
  int mCols;

private:
  class KPaletteTablePrivate;
  KPaletteTablePrivate *d;
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

  void setShading(bool _shade) { shade = _shade; }
  
  void setAcceptDrags(bool _acceptDrags) { acceptDrags = _acceptDrags; }
	
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
  bool shade;  
  bool acceptDrags;

private:
  class KColorCellsPrivate;
  KColorCellsPrivate *d;
};

/**
 * Displays and signals a single colour selection.
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

  class KColorPatchPrivate;
  KColorPatchPrivate *d;

};

/** 
 * Color selection dialog.
 *
 * @sect Features:
 * 
 * @li Colour selection from a wide range of palettes.
 * @li Colour selection from a Palette of H vs S and V selectors (similar to windoze).
 * @li Direct input of HSV or RGB values.
 * @li Saving of custom colors
 * 
 * @sect Simplest use:
 * 
 * <pre>
 * 	QColor myColor;
 * 	int result = KColorDialog::getColor( myColor );
 *         if ( result == KColorDialog::Accepted )
 *            ...
 * </pre>
 * 
 **/
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
     **/
    QColor color();
  
    /**
     * This is probably the function you are looking for.
     * Just call this to pop up dialog get the selected color.
     * returns @ref result().
     */
    static int getColor( QColor &theColor, QWidget *parent=0L );

    /**
     * Get the color from the pixel at point p on the screen.
     */
    static QColor grabColor(const QPoint &p);

  public slots:
    /** 
     * Preselect a color.
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
    void slotColorSelected( const QColor &col );
    void slotColorSelected( const QColor &col, const QString &name );
    void slotColorPicker();
    void slotAddToCustomColors();
    /**
     * Write the settings of the dialog to config file.
     **/
    void slotWriteSettings();

  private:
    /**
     * Read the settings for the dialog from config file.
     **/
    void readSettings();

    void setRgbEdit( void );
    void setHsvEdit( void );
    void setHtmlEdit( void );
    void _setColor( const KColor &col, const QString &name=QString::null );

  protected:
    virtual void mouseReleaseEvent( QMouseEvent * );
    virtual void keyPressEvent( QKeyEvent * );

  private:
    KPaletteTable *table;
    bool bRecursion;
    bool bEditRgb;
    bool bEditHsv;
    bool bColorPicking;
    QLabel *colorName;
    QLineEdit *htmlName;
    QLineEdit *hedit;
    QLineEdit *sedit;
    QLineEdit *vedit;
    QLineEdit *redit;
    QLineEdit *gedit;
    QLineEdit *bedit;
    KColorPatch *patch;
    KHSSelector *hsSelector;
    KPalette *palette;
    KValueSelector *valuePal;
    KColor selColor;

    class KColorDialogPrivate;
    KColorDialogPrivate *d;
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

    class KColorComboPrivate;
    KColorComboPrivate *d;
};

#endif		// __KCOLORDLG_H__

