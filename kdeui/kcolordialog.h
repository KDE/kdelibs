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
//----------------------------------------------------------------------
// KDE color selection dialog.

// layout managment added Oct 1997 by Mario Weilguni 
// <mweilguni@sime.com>

#ifndef __KCOLORDIALOG_H__
#define __KCOLORDIALOG_H__

#include <qcombobox.h>
#ifdef Q_WS_QWS
// FIXME(E): Do we need the KColorDialog extra functionality in Qt Embedded?
#include <qcolordialog.h>
#define KColorDialog QColorDialog
#else
#include <kdialogbase.h>
#include <qframe.h>
#include <qpixmap.h>
#include <qrangecontrol.h>
#include <qgridview.h>

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
  /**
   * @reimplemented
   */
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
  /**
   * Constructs a widget for color selection with a given orientation
   */
  KValueSelector( Orientation o, QWidget *parent = 0, const char *name = 0 );

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
  /**
   * @reimplemented
   */
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

  virtual void setPalette(const QPalette& p) { QWidget::setPalette(p); }
};


/**
* A table of editable colour cells.
* @author Martin Jones <mjones@kde.org>
* @version $Id$
*/
class KColorCells : public QGridView
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
  /**
   * @reimplemented
   */
  virtual void paintCell( QPainter *painter, int row, int col );
  /**
   * @reimplemented
   */
  virtual void resizeEvent( QResizeEvent * );
  /**
   * @reimplemented
   */
  virtual void mouseReleaseEvent( QMouseEvent * );
  /**
   * @reimplemented
   */
  virtual void mousePressEvent( QMouseEvent * );
  /**
   * @reimplemented
   */
  virtual void mouseMoveEvent( QMouseEvent * );
  /**
   * @reimplemented
   */
  virtual void dragEnterEvent( QDragEnterEvent *);
  /**
   * @reimplemented
   */
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
 * The KColorPatch widget is a (usually small) widget showing
 * a selected color e. g. in the @ref KColorDialog. It
 * automatically handles drag and drop from and on the widget.
 *
 * @short A color shower.
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
  /**
   * @reimplemented
   */
  virtual void drawContents( QPainter *painter );
  /**
   * @reimplemented
   */
  virtual void mouseMoveEvent( QMouseEvent * );
  /**
   * @reimplemented
   */
  virtual void dragEnterEvent( QDragEnterEvent *);
  /**
   * @reimplemented
   */
  virtual void dropEvent( QDropEvent *);

private:
  QColor color;
  uint pixel;
  int colContext;

  class KColorPatchPrivate;
  KColorPatchPrivate *d;

};

/** 
 * The KColorDialog provides a dialog for color selection.
 *
 * @sect Features:
 * 
 * @li Colour selection from a wide range of palettes.
 * @li Colour selection from a palette of H vs S and V selectors (similar to windoze).
 * @li Direct input of HSV or RGB values.
 * @li Saving of custom colors
 * 
 * In most cases, you will want to use the static method @ref KColorDialog::getColor().
 * This pops up the dialog (with an initial selection provided by you), lets the
 * user choose a color, and returns.
 *
 * Example:
 * 
 * <pre>
 * 	QColor myColor;
 * 	int result = KColorDialog::getColor( myColor );
 *         if ( result == KColorDialog::Accepted )
 *            ...
 * </pre>
 *
 * @image kcolordialog.png KColorDialog
 *
 * The color dialog is really a collection of several widgets which can
 * you can also use separately: the quadratic plane in the top left of
 * the dialog is a @ref KXYSelector. Right next to it is a @ref KHSSelector
 * for chosing hue/saturation.
 *
 * On the right side of the dialog you see a @ref KPaletteTable showing
 * up to 40 colors with a combo box which offers several predefined
 * palettes or a palette configured by the user. The small field showing
 * the currently selected color is a @ref KColorPatch.
 *
 * @short A color selection dialog.
 **/
class KColorDialog : public KDialogBase
{
  Q_OBJECT

  public:
    /** 
     * Construct a @ref KColorDialog.
     */
    KColorDialog( QWidget *parent = 0L, const char *name = 0L,
		  bool modal = FALSE );

    ~KColorDialog();

    /** 
     * Retrieve the currently selected color. 
     **/
    QColor color();
  
    /**
     * Create a modal color dialog, let the user choose a
     * color, and return when the dialog is closed.
     *
     * The selected color is returned in the argument @p theColor.
     *
     * @returns @ref QDialog::result().
     */
    static int getColor( QColor &theColor, QWidget *parent=0L );

    /**
     * Get the color from the pixel at point p on the screen.
     */
    static QColor grabColor(const QPoint &p);

  public slots:
    /** 
     * Preselects a color.
     */
    void setColor( const QColor &col );

  signals:
    /** 
     * Emitted when a color is selected.
     * Connect to this to monitor the color as it as selected if you are
     * not running modal.
     */
    void colorSelected( const QColor &col );

  private slots:
    void slotRGBChanged( void );
    void slotHSVChanged( void );
    void slotHtmlChanged( void );
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
   /**
    * @reimplemented
    */
    virtual void mouseReleaseEvent( QMouseEvent * );
   /**
    * @reimplemented
    */
    virtual void keyPressEvent( QKeyEvent * );

  private:
    class KColorDialogPrivate;
    KColorDialogPrivate *d;
};

#endif		// Q_WS_QWS
#endif		// __KCOLORDIALOG_H__

