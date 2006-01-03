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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
//----------------------------------------------------------------------
// KDE color selection dialog.

// layout management added Oct 1997 by Mario Weilguni
// <mweilguni@sime.com>

#ifndef KCOLORDIALOG_H
#define KCOLORDIALOG_H

#ifdef Q_WS_QWS
// FIXME(E): Do we need the KColorDialog extra functionality in Qt Embedded?
#include <qcolordialog.h>
#define KColorDialog QColorDialog
#else //UNIX, WIN32
#include <kdialogbase.h>
#include <qpixmap.h>
#include <q3gridview.h>

#include "kselect.h"

class QComboBox;
class QLineEdit;
class KListBox;
class KPalette;
class KColorCells;


/**
 * Widget for Hue/Saturation colour selection.
 *
 * The actual values can be fetched using the inherited xValue and yValue
 * methods.
 *
 * \image html khsselector.png "KDE Hue/Saturation Selection Widget"
 *
 * @see KXYSelector, KValueSelector, KColorDialog
 * @author Martin Jones (mjones@kde.org)
*/
class KDEUI_EXPORT KHSSelector : public KXYSelector
{
  Q_OBJECT

public:
  /**
   * Constructs a hue/saturation selection widget.
   */
  KHSSelector( QWidget *parent=0);

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
  void updateContents();
  QPixmap pixmap;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KHSSelectorPrivate;
  KHSSelectorPrivate *d;
};


class KValueSelectorPrivate;
/**
 * Widget for color value selection.
 *
 * @see KHSSelector, KColorDialog
 * @author Martin Jones (mjones@kde.org)
 */
class KDEUI_EXPORT KValueSelector : public KSelector
{
  Q_OBJECT

public:
  /**
   * Constructs a widget for color selection.
   */
  KValueSelector( QWidget *parent=0 );
  /**
   * Constructs a widget for color selection with a given orientation
   */
  KValueSelector( Qt::Orientation o, QWidget *parent = 0 );

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

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KValueSelectorPrivate;
  KValueSelectorPrivate *d;
};


/**
 * A color class that preserves both RGB and HSV values.
 *
 * This is
 * unlike QColor which only preserves RGB values and recalculates HSV
 * values. The QColor behavior leads to an accumulation of rounding
 * errors when working in the HSV color space.
 *
 * @author Waldo Bastian <bastian@kde.org>
 **/
class KDEUI_EXPORT KColor : public QColor
{
public:
  KColor();
  KColor( const KColor &col);
  KColor( const QColor &col);

  KColor& operator=( const KColor& col);

  bool operator==( const KColor& col) const;

  void setHsv(int _h, int _s, int _v);
  void setRgb(int _r, int _g, int _b);

  void rgb(int *_r, int *_g, int *_b) const;
  void hsv(int *_h, int *_s, int *_v) const;
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
 *
 * @author Waldo Bastian <bastian@kde.org>
 **/
class KDEUI_EXPORT KPaletteTable : public QWidget
{
  Q_OBJECT
public:
  KPaletteTable( QWidget *parent, int minWidth=210, int cols = 16);
  ~KPaletteTable();
  void addToCustomColors( const QColor &);
  void addToRecentColors( const QColor &);
  QString palette() const;
public slots:
  void setPalette(const QString &paletteName);
signals:
  void colorSelected( const QColor &, const QString & );
  void colorDoubleClicked( const QColor &, const QString & );

protected slots:
  void slotColorCellSelected( int );
  void slotColorCellDoubleClicked( int );
  void slotColorTextSelected( const QString &colorText );
  void slotSetPalette( const QString &_paletteName );
  void slotShowNamedColorReadError( void );

protected:
  void readNamedColor( void );

protected:
  QString i18n_namedColors;
  QComboBox *combo;
  KColorCells *cells;
  Q3ScrollView *sv;
  KListBox *mNamedColorList;
  KPalette *mPalette;
  int mMinWidth;
  int mCols;

private:

  virtual void setPalette(const QPalette& p) { QWidget::setPalette(p); }
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KPaletteTablePrivate;
  KPaletteTablePrivate *d;
};


/**
* A table of editable color cells.
*
* @author Martin Jones <mjones@kde.org>
*/
class KDEUI_EXPORT KColorCells : public Q3GridView
{
  Q_OBJECT
public:
  KColorCells( QWidget *parent, int rows, int cols );
  ~KColorCells();

  void setColor( int colNum, const QColor &col );
  QColor color( int indx ) const
  {	return colors[indx]; }
  int numCells() const
  {	return numRows() * numCols(); }

  void setShading(bool _shade) { shade = _shade; }

  void setAcceptDrags(bool _acceptDrags) { acceptDrags = _acceptDrags; }

  int getSelected() const
  {	return selected; }

  signals:
  void colorSelected( int col );
  void colorDoubleClicked( int col );

protected:
  virtual void paintCell( QPainter *painter, int row, int col );
  virtual void resizeEvent( QResizeEvent * );
  virtual void mouseReleaseEvent( QMouseEvent * );
  virtual void mousePressEvent( QMouseEvent * );
  virtual void mouseMoveEvent( QMouseEvent * );
  virtual void dragEnterEvent( QDragEnterEvent *);
  virtual void dropEvent( QDropEvent *);
  virtual void mouseDoubleClickEvent( QMouseEvent * );

  int posToCell(const QPoint &pos, bool ignoreBorders=false);

  QColor *colors;
  bool inMouse;
  QPoint mPos;
  int	selected;
  bool shade;
  bool acceptDrags;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KColorCellsPrivate;
  KColorCellsPrivate *d;
};

/**
 * @short A color displayer.
 *
 * The KColorPatch widget is a (usually small) widget showing
 * a selected color e. g. in the KColorDialog. It
 * automatically handles drag and drop from and on the widget.
 *
 */
class KDEUI_EXPORT KColorPatch : public QFrame
{
  Q_OBJECT
public:
  KColorPatch( QWidget *parent );
  virtual ~KColorPatch();

  void setColor( const QColor &col );

signals:
  void colorChanged( const QColor&);

protected:
  virtual void paintEvent    ( QPaintEvent * pe );
  virtual void mouseMoveEvent( QMouseEvent * );
  virtual void dragEnterEvent( QDragEnterEvent *);
  virtual void dropEvent( QDropEvent *);

private:
  QColor color;
  uint pixel;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KColorPatchPrivate;
  KColorPatchPrivate *d;
};

/**
 * @short A color selection dialog.
 *
 * <b>Features:</b>\n
 *
 * @li Color selection from a wide range of palettes.
 * @li Color selection from a palette of H vs S and V selectors.
 * @li Direct input of HSV or RGB values.
 * @li Saving of custom colors
 *
 * In most cases, you will want to use the static method KColorDialog::getColor().
 * This pops up the dialog (with an initial selection provided by you), lets the
 * user choose a color, and returns.
 *
 * Example:
 *
 * \code
 * 	QColor myColor;
 * 	int result = KColorDialog::getColor( myColor );
 *         if ( result == KColorDialog::Accepted )
 *            ...
 * \endcode
 *
 * @image html kcolordialog.png "KDE Color Dialog"
 *
 * The color dialog is really a collection of several widgets which can
 * you can also use separately: the quadratic plane in the top left of
 * the dialog is a KXYSelector. Right next to it is a KHSSelector
 * for choosing hue/saturation.
 *
 * On the right side of the dialog you see a KPaletteTable showing
 * up to 40 colors with a combo box which offers several predefined
 * palettes or a palette configured by the user. The small field showing
 * the currently selected color is a KColorPatch.
 *
 **/
class KDEUI_EXPORT KColorDialog : public KDialog
{
  Q_OBJECT

  public:
    /**
     * Constructs a color selection dialog.
     */
    KColorDialog( QWidget *parent = 0L, const char *name = 0L,
		  bool modal = false );
    /**
     * Destroys the color selection dialog.
     */
    ~KColorDialog();

    /**
     * Returns the currently selected color.
     **/
    QColor color() const;

    /**
     * Creates a modal color dialog, let the user choose a
     * color, and returns when the dialog is closed.
     *
     * The selected color is returned in the argument @p theColor.
     *
     * @returns QDialog::result().
     */
    static int getColor( QColor &theColor, QWidget *parent=0L );

    /**
     * Creates a modal color dialog, lets the user choose a
     * color, and returns when the dialog is closed.
     *
     * The selected color is returned in the argument @p theColor.
     *
     * This version takes a @p defaultColor argument, which sets the color
     * selected by the "default color" checkbox. When this checkbox is checked,
     * the invalid color (QColor()) is returned into @p theColor.
     *
     * @returns QDialog::result().
     */
    static int getColor( QColor &theColor, const QColor& defaultColor, QWidget *parent=0L );

    /**
     * Gets the color from the pixel at point p on the screen.
     */
    static QColor grabColor(const QPoint &p);

    /**
     * Call this to make the dialog show a "Default Color" checkbox.
     * If this checkbox is selected, the dialog will return an "invalid" color (QColor()).
     * This can be used to mean "the default text color", for instance,
     * the one with the KDE text color on screen, but black when printing.
     */
    void setDefaultColor( const QColor& defaultCol );

    /**
     * @return the value passed to setDefaultColor
     */
    QColor defaultColor() const;

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
    void slotColorDoubleClicked( const QColor &col, const QString &name );
    void slotColorPicker();
    void slotAddToCustomColors();
    void slotDefaultColorClicked();
    /**
     * Write the settings of the dialog to config file.
     **/
    void slotWriteSettings();

  private:
    /**
     * Read the settings for the dialog from config file.
     **/
    void readSettings();

    void setRgbEdit( const KColor &col );
    void setHsvEdit( const KColor &col );
    void setHtmlEdit( const KColor &col );
    void _setColor( const KColor &col, const QString &name=QString() );
    void showColor( const KColor &color, const QString &name );

  protected:
    virtual void mouseReleaseEvent( QMouseEvent * );
    virtual void keyPressEvent( QKeyEvent * );
    virtual bool eventFilter( QObject *obj, QEvent *ev );

  protected:
    virtual void virtual_hook( int id, void* data );
  private:
    class KColorDialogPrivate;
    KColorDialogPrivate *d;
};

#endif		// !Q_WS_QWS
#endif		// KCOLORDIALOG_H

