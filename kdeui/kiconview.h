/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef KICONVIEW_H
#define KICONVIEW_H

#include <qcursor.h>
#include <qiconview.h>

/**
 * This Widget extends the functionality of QIconView to honor the system
 * wide settings for Single Click/Double Click mode, Auto Selection and
 * Change Cursor over Link.
 *
 * There is a new signal executed(). It gets connected to either
 * @ref QIconView::clicked() or @ref QIconView::doubleClicked() depending on the KDE
 * wide Single Click/Double Click settings. It is strongly recomended that
 * you use this signal instead of the above mentioned. This way you don´t
 * need to care about the current settings.
 * If you want to get informed when the user selects something connect to the
 * @ref QIconView::selectionChanged() signal.
 *
 * @short A variant of QIconView that honors KDE's system-wide settings.
 **/
class KIconView : public QIconView
{
  friend class KIconViewItem;
  Q_OBJECT
  Q_ENUMS( Mode )
  Q_PROPERTY( Mode mode READ mode WRITE setMode )

public:
  KIconView( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );

  ~KIconView();

  /**
   * KIconView has two different operating modes. Execute mode is depending
   * on the configuration of single-click or double-click where the signal
   * executed() will be emitted upon click/double-click.
   * In Select mode, this signal will not be emitted.
   *
   * Default is Execute mode.
   */
  enum Mode { Execute, Select };

  /**
   * Sets the mode to Execute or Select.
   * @li In Execute mode, the signal executed()
   * will be emitted when the user clicks/double-clicks an item.
   * @li Select mode is
   * the normal QIconView mode.
   *
   * Default is Execute.
   */
  void setMode( Mode m );

  /**
   * @returns the current Mode, either Execute or Select.
   */
  Mode mode() const;

signals:

  /**
   * This signal is emitted whenever the user executes an iconview item.
   * That means depending on the KDE wide Single Click/Double Click
   * setting the user clicked or double clicked on that item.
   * @param item is the pointer to the executed iconview item.
   *
   * Note that you may not delete any @ref QIconViewItem objects in slots
   * connected to this signal.
   */
  void executed( QIconViewItem *item );

  /**
   * This signal is emitted whenever the user executes an iconview item.
   * That means depending on the KDE wide Single Click/Double Click
   * setting the user clicked or double clicked on that item.
   * @param item is the pointer to the executed iconview item.
   * @param pos is the position where the user has clicked
   *
   * Note that you may not delete any @ref QIconViewItem objects in slots
   * connected to this signal.
   */
  void executed( QIconViewItem *item, const QPoint &pos );

  /**
   * This signal gets emitted whenever the user double clicks into the
   * iconview.
   * @param item is the pointer to the clicked iconview item.
   * @param pos is the position where the user has clicked, and
   *
   * Note that you may not delete any @ref QIconViewItem objects in slots
   * connected to this signal.
   *
   * This signal is more or less here for the sake of completeness.
   * You should normally not need to use this. In most cases it´s better
   * to use @ref #executed() instead.
   */
  void doubleClicked( QIconViewItem *item, const QPoint &pos );

protected slots:
  void slotOnItem( QIconViewItem *item );
  void slotOnViewport();

  void slotSettingsChanged(int);

  /**
   * Auto selection happend.
   */
  void slotAutoSelect();

protected:
  void emitExecute( QIconViewItem *item, const QPoint &pos );

  virtual void focusOutEvent( QFocusEvent *fe );
  virtual void leaveEvent( QEvent *e );
  virtual void contentsMousePressEvent( QMouseEvent *e );
  virtual void contentsMouseDoubleClickEvent ( QMouseEvent * e );
  virtual void contentsMouseReleaseEvent( QMouseEvent *e );

  QCursor oldCursor;
  bool m_bUseSingle;
  bool m_bChangeCursorOverItem;

  QIconViewItem* m_pCurrentItem;

  QTimer* m_pAutoSelect;
  int m_autoSelectDelay;

private slots:
  void slotMouseButtonClicked( int btn, QIconViewItem *item, const QPoint &pos );

private:
  /**
   * @internal. For use by KIconViewItem.
   */
  QFontMetrics *itemFontMetrics() const;
  /**
   * @internal. For use by KIconViewItem.
   */
  QPixmap selectedIconPixmap( QPixmap *pix, const QColor &col ) const;

  class KIconViewPrivate;
  KIconViewPrivate *d;
};

class KWordWrap;
/**
 * KIconViewItem exists to improve the word-wrap functionality of QIconViewItem
 * Use KIconViewItem instead of QIconViewItem for any iconview item you might have :)
 *
 * @short A variant of QIconViewItem that wraps words better.
 * @author David Faure <david@mandrakesoft.com>
 */
class KIconViewItem : public QIconViewItem
{
public:
    // Need to redefine all the constructors - I want Java !
    KIconViewItem( QIconView *parent )
        : QIconViewItem( parent ) { init(); } // We need to call it because the parent ctor won't call our reimplementation :(((
    KIconViewItem( QIconView *parent, QIconViewItem *after )
        : QIconViewItem( parent, after ) { init(); }
    KIconViewItem( QIconView *parent, const QString &text )
        : QIconViewItem( parent, text ) { init(); }
    KIconViewItem( QIconView *parent, QIconViewItem *after, const QString &text )
        : QIconViewItem( parent, after, text ) { init(); }
    KIconViewItem( QIconView *parent, const QString &text, const QPixmap &icon )
        : QIconViewItem( parent, text, icon ) { init(); }
    KIconViewItem( QIconView *parent, QIconViewItem *after, const QString &text, const QPixmap &icon )
        : QIconViewItem( parent, after, text, icon ) { init(); }
    KIconViewItem( QIconView *parent, const QString &text, const QPicture &picture )
        : QIconViewItem( parent, text, picture ) { init(); }
    KIconViewItem( QIconView *parent, QIconViewItem *after, const QString &text, const QPicture &picture )
        : QIconViewItem( parent, after, text, picture ) { init(); }
    virtual ~KIconViewItem();
protected:
    void init();
    virtual void calcRect( const QString& text_ = QString::null );
    virtual void paintItem( QPainter *p, const QColorGroup &c );
private:
    KWordWrap* m_wordWrap;
};

// To be moved somewhere else....
/**
 * Word-wrap algorithm that takes into account beautifulness ;)
 * That means: not letting a letter alone on the last line,
 * breaking at punctuation signs, etc.
 *
 * Usage: call the static method, @ref formatText, with the text to
 * wrap and the constraining rectangle etc., it will return an instance of KWordWrap
 * containing internal data, result of the word-wrapping.
 * From that instance you can retrieve the boundingRect, and invoke drawing.
 *
 * This design allows to call the word-wrap algorithm only when the text changes
 * and not every time we want to know the bounding rect or draw the text.
 */
class KWordWrap
{
public:
    /**
     * Main method for wrapping text.
     *
     * @param fm Font metrics, for the chosen font. Better cache it, creating a QFontMetrics is expensive.
     * @param r Constraining rectangle. Only the width and height matter.
     * @param flags - currently unused.
     * @param str The text to be wrapped.
     * @param len Length of text to wrap (default is -1 for all).
     * @return a KWordWrap instance. The caller is responsible for storing and deleting the result.
     */
    static KWordWrap* formatText( QFontMetrics &fm, const QRect & r, int flags, const QString & str, int len = -1 );

    /**
     * @return the bounding rect, calculated by formatText.
     */
    QRect boundingRect() const { return m_boundingRect; }

    /**
     * @return the original string, with '\n' inserted where
     * the text is broken by the wordwrap algorithm.
     */
    QString wrappedString() const; // gift for Dirk :)

    /**
     * Draw the text that has been previously wrapped, at position x,y.
     * Flags are for alignment, e.g. AlignHCenter. Default is AlignAuto.
     */
    void drawText( QPainter *painter, int x, int y, int flags = Qt::AlignAuto ) const;

private:
    QValueList<int> m_breakPositions;
    QValueList<int> m_lineWidths;
    QRect m_boundingRect;
    QString m_text;
};

#endif
