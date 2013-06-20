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

#include <kdialog.h>
#include <QPixmap>
#include <QScrollArea>
#include <QTableWidget>
#include <kcolorchoosermode.h>

#include <kde4support_export.h>

/**
* A table of editable color cells.
*
* @author Martin Jones <mjones@kde.org>
*/
class KDE4SUPPORT_DEPRECATED_EXPORT KColorCells : public QTableWidget
{
  Q_OBJECT
  Q_PROPERTY(bool acceptDrags READ acceptDrags WRITE setAcceptDrags)
  Q_PROPERTY(bool shading READ shading WRITE setShading)

public:
  /** 
   * Constructs a new table of color cells, consisting of
   * @p rows * @p columns colors. 
   *
   * @param parent The parent of the new widget
   * @param rows The number of rows in the table
   * @param columns The number of columns in the table
   */
  KColorCells( QWidget *parent, int rows, int columns );
  ~KColorCells();

  /** Sets the color in the given index in the table */
  void setColor( int index, const QColor &col );
  /** Returns the color at a given index in the table */
  QColor color( int index ) const;
  /** Returns the total number of color cells in the table */
  int count() const;

  void setShading(bool shade);
  bool shading() const;

  void setAcceptDrags(bool acceptDrags);
  bool acceptDrags() const;

  /** Sets the currently selected cell to @p index */
  void setSelected(int index);
  /** Returns the index of the cell which is currently selected */
  int  selectedIndex() const;
  
Q_SIGNALS:
  /** Emitted when a color is selected in the table */
  void colorSelected( int index , const QColor& color );
  /** Emitted when a color in the table is double-clicked */
  void colorDoubleClicked( int index , const QColor& color );

protected:
  // the three methods below are used to ensure equal column widths and row heights
  // for all cells and to update the widths/heights when the widget is resized
  virtual int sizeHintForColumn(int column) const;
  virtual int sizeHintForRow(int column) const;
  virtual void resizeEvent( QResizeEvent* event );

  virtual void mouseReleaseEvent( QMouseEvent * );
  virtual void mousePressEvent( QMouseEvent * );
  virtual void mouseMoveEvent( QMouseEvent * );
  virtual void dragEnterEvent( QDragEnterEvent * );
  virtual void dragMoveEvent( QDragMoveEvent * );
  virtual void dropEvent( QDropEvent *);
  virtual void mouseDoubleClickEvent( QMouseEvent * );

  int positionToCell(const QPoint &pos, bool ignoreBorders=false) const;

private:
  class KColorCellsPrivate;
  friend class KColorCellsPrivate;
  KColorCellsPrivate *const d;
  
  Q_DISABLE_COPY(KColorCells)
};

/**
 * @short A color displayer.
 *
 * The KColorPatch widget is a (usually small) widget showing
 * a selected color e.g. in the KColorDialog. It
 * automatically handles drag and drop from and on the widget.
 *
 * \image html kcolorpatch.png "KDE Color Patch"
 */
class KDE4SUPPORT_DEPRECATED_EXPORT KColorPatch : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)
  
public:
  KColorPatch( QWidget *parent );
  virtual ~KColorPatch();

    /**
     * Get the currently displayed color
     */
    QColor color() const;

    /**
     * Set the color to display and update the display
     *
     * @param col color to display
     */
    void setColor( const QColor &col );

Q_SIGNALS:
    /**
     * This signal is emitted whenever the current color
     * changes due to a drop event
     */
    void colorChanged(const QColor&);

protected:
  virtual void paintEvent    ( QPaintEvent * pe );
  virtual void mouseMoveEvent( QMouseEvent * );
  virtual void dragEnterEvent( QDragEnterEvent *);
  virtual void dropEvent( QDropEvent *);

private:
  class KColorPatchPrivate;
  KColorPatchPrivate *const d;
  
  Q_DISABLE_COPY(KColorPatch)
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
 * To react to the color selection as it is being selected, the colorSelected() signal
 * can be used.  This can be used still in a modal way, for example:
 *
 * \code
 *  KColorDialog dialog(this);
 *  connect(&dialog, SIGNAL(colorSelected(const QColor &)), this, SLOT(temporarilyChangeColor(const QColor &)));
 *  QColor myColor;
 *  dialog.setColor(myColor);
 *  int result = dialog.exec();
 *
 *  if ( result == KColorDialog::Accepted )
 *    changeColor( dialog.color() );
 *  else
 *    temporarilyChangeColor(myColor); //change back to original color
 * \endcode
 *
 *
 * @image html kcolordialog.png "KDE Color Dialog"
 *
 * The color dialog is really a collection of several widgets which can
 * you can also use separately: the quadratic plane in the top left of
 * the dialog is a KXYSelector. Right next to it is a KHSSelector
 * for choosing hue/saturation.
 *
 * On the right side of the dialog you see a KColorTable showing
 * a number of colors with a combo box which offers several predefined
 * palettes or a palette configured by the user. The small field showing
 * the currently selected color is a KColorPatch.
 *
 **/
class KDE4SUPPORT_DEPRECATED_EXPORT KColorDialog : public KDialog
{
  Q_OBJECT
  Q_PROPERTY(bool isAlphaChannelEnabled READ isAlphaChannelEnabled WRITE setAlphaChannelEnabled)
  Q_PROPERTY(QColor defaultColor READ defaultColor WRITE setDefaultColor)
  Q_PROPERTY(QColor color READ color WRITE setColor)

  public:
    /**
     * Constructs a color selection dialog.
     */
    explicit KColorDialog( QWidget *parent = 0L, bool modal = false );
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
     * @param theColor if valid, specifies the color to be initially selected. On
     *                 return, holds the selected color.
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
     * @param theColor if valid, specifies the color to be initially selected. On
     *                 return, holds the selected color.
     * @param defaultColor color selected by the "default color" checkbox
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

    /**
     * When set to true, the user is allowed to change the alpha component of the color.
     * The default value is false.
     * @since 4.5
     */
    void setAlphaChannelEnabled(bool alpha);

    /**
     * Returns true when the user can change the alpha channel.
     * @since 4.5
     */
    bool isAlphaChannelEnabled() const;

  public Q_SLOTS:
    /**
     * Preselects a color.
     */
    void setColor( const QColor &col );

  Q_SIGNALS:
    /**
     * Emitted when a color is selected.
     * Connect to this to monitor the color as it as selected if you are
     * not running modal.
     */
    void colorSelected( const QColor &col );

  private:
    Q_PRIVATE_SLOT(d, void slotRGBChanged( void ))
    Q_PRIVATE_SLOT(d, void slotAlphaChanged( void ))
    Q_PRIVATE_SLOT(d, void slotHSVChanged( void ))
    Q_PRIVATE_SLOT(d, void slotHtmlChanged( void ))
    Q_PRIVATE_SLOT(d, void slotHSChanged( int, int ))
    Q_PRIVATE_SLOT(d, void slotVChanged( int ))
    Q_PRIVATE_SLOT(d, void slotAChanged( int ))
    Q_PRIVATE_SLOT(d, void slotColorSelected( const QColor &col ))
    Q_PRIVATE_SLOT(d, void slotColorSelected( const QColor &col, const QString &name ))
    Q_PRIVATE_SLOT(d, void slotColorDoubleClicked( const QColor &col, const QString &name ))
    Q_PRIVATE_SLOT(d, void slotColorPicker())
    Q_PRIVATE_SLOT(d, void slotAddToCustomColors())
    Q_PRIVATE_SLOT(d, void slotDefaultColorClicked())
    Q_PRIVATE_SLOT(d, void slotModeChanged( int id ))

    /**
     * Write the settings of the dialog to config file.
     **/
    Q_PRIVATE_SLOT(d, void slotWriteSettings())

  private:
    /**
     * Read the settings for the dialog from config file.
     **/
    void readSettings();

  protected:
    virtual void mouseMoveEvent( QMouseEvent * );
    virtual void mouseReleaseEvent( QMouseEvent * );
    virtual void keyPressEvent( QKeyEvent * );
    virtual bool eventFilter( QObject *obj, QEvent *ev );

  private:
    class KColorDialogPrivate;
    KColorDialogPrivate *const d;
    
    Q_DISABLE_COPY(KColorDialog)
};

#endif		// KCOLORDIALOG_H
