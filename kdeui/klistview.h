/* This file is part of the KDE libraries
   Copyright (C) 2000 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2000 Charles Samuels <charles@kde.org>

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
#ifndef KLISTVIEW_H
#define KLISTVIEW_H

#include <qlistview.h>

#include <qlist.h>

class QDragObject;
/**
 * This Widget extends the functionality of QListView to honor the system
 * wide settings for Single Click/Double Click mode, Auto Selection and
 * Change Cursor over Link.
 *
 * There is a new signal executed(). It gets connected to either
 * QListView::clicked() or QListView::doubleClicked() depending on the KDE
 * wide Single Click/Double Click settings. It is strongly recomended that
 * you use this signal instead of the above mentioned. This way you don´t
 * need to care about the current settings.
 * If you want to get informed when the user selects something connect to the
 * QListView::selectionChanged() signal.
 **/
class KListView : public QListView
{
    Q_OBJECT

public:
    KListView( QWidget *parent = 0, const char *name = 0 );
    ~KListView();

  /**
   * This function determines whether the given coordinates are within the
   * execute area. The execute area is the part of an QListViewItem where mouse
   * clicks or double clicks respectively generate a executed() signal.
   * Depending on @ref QListView::allColumnsShowFocus() this is either the
   * whole item or only the first column.
   * @return true if point is inside execute area of an item, false in all
   * other cases including the case that it is over the viewport.
   */
  virtual bool isExecuteArea( const QPoint& point);

  QList<QListViewItem> selectedItems() const;
  void moveItem(QListViewItem *item, QListViewItem *parent, QListViewItem *after);


  QListViewItem *lastItem() const;

  /**
   * For future expansions.
   * 
   * Do not use.
   */
  bool itemsMovable() const;
  bool itemsRenameable() const;
  bool dragEnabled() const;
  bool autoOpen() const;
  bool isRenameable (int column) const;
  bool dropVisualizer() const;
  int tooltipColumn() const;

  /**
   * For future expansions.
   * 
   * Do not use.
   */
  bool createChildren() const;
  bool dropHighlighter() const;

signals:

  /**
   * This signal is emitted whenever the user executes an listview item.
   * That means depending on the KDE wide Single Click/Double Click
   * setting the user clicked or double clicked on that item.
   * @param item is the pointer to the executed listview item.
   *
   * Note that you may not delete any @ref QListViewItem objects in slots
   * connected to this signal.
   */
  void executed( QListViewItem *item );

  /**
   * This signal is emitted whenever the user executes an listview item.
   * That means depending on the KDE wide Single Click/Double Click
   * setting the user clicked or double clicked on that item.
   * @param item is the pointer to the executed listview item.
   * @param pos is the position where the user has clicked
   * @param c is the column into which the user clicked.
   *
   * Note that you may not delete any @ref QListViewItem objects in slots
   * connected to this signal.
   */
  void executed( QListViewItem *item, const QPoint &pos, int c );

  /**
   * This signal gets emitted whenever the user double clicks into the
   * listview.
   * @param item is the pointer to the clicked listview item.
   * @param pos is the position where the user has clicked, and
   * @param c the column into which the user clicked.
   *
   * Note that you may not delete any @ref QListViewItem objects in slots
   * connected to this signal.
   *
   * This signal is more or less here for the sake of completeness.
   * You should normally not need to use this. In most cases it´s better
   * to use @ref #executed instead.
   */
  void doubleClicked( QListViewItem *item, const QPoint &pos, int c );

  void dropped(QDropEvent * e, QListViewItem *after);

  /**
   * For future expansions.
   * 
   * Do not use.
   */
  void moved();

  void itemRenamed(QListViewItem * item, const QString &str, int col);
  void itemRenamed(QListViewItem * item);

public slots:
  virtual void rename(QListViewItem *item, int c);

  /**
   * is column renameable? Set it.  by default, all columns
   * are not renameable.  If you want more intelligent
   * selection, you'll have to derive from KListView,
   * and override @ref rename where you only call
   * KListView::rename if you want it renamed.
   **/
  void setRenameable (int column, bool yesno=true);

  /**
   * For future expansions.
   * 
   * Do not use.
   */
  virtual void setItemsMovable(bool b);
  virtual void setItemsRenameable(bool b);
  virtual void setDragEnabled(bool b);
  virtual void setAutoOpen(bool b);
  virtual void setDropVisualizer(bool b);
  virtual void setTooltipColumn(int column);
  /**
   * Highlight a parent if I drop into it's children
   **/
  virtual void setDropHighlighter(bool b);

  /**
   * For future expansions.
   * 
   * Do not use.
   */
  virtual void setCreateChildren(bool b);

protected slots:
  void slotOnItem( QListViewItem *item );
  void slotOnViewport();

  void slotSettingsChanged(int);

  /**
   * Auto selection happend.
   */
  void slotAutoSelect();

  /**
   * Repaint the rect where I was drawing the drop line.
   */
  void cleanDropVisualizer();

protected:
  /**
   * Determine whether a drop on this position (@p p) would count as
   * being above or below the QRect (@p rect).
   *
   * Note: @p p is assumed to be in viewport coordinates.
   */
  inline bool below (const QRect& rect, const QPoint& p)
  {
	return (p.y() > (rect.top() + (rect.bottom() - rect.top())/2));
  }

  /**
   * An overloaded version of below(const QRect&, const QPoint&).
   *
   * It differs from the above only in what arguments it takes.
   * Note that @p p is assumed to be in contents coordinates!
   */
  inline bool below (QListViewItem* i, const QPoint& p)
  {
	return below (itemRect(i), contentsToViewport(p));
  }

  void emitExecute( QListViewItem *item, const QPoint &pos, int c );

  virtual void focusOutEvent( QFocusEvent *fe );
  virtual void leaveEvent( QEvent *e );
  virtual QString tooltip(QListViewItem*, int column) const;
  virtual bool showTooltip(QListViewItem *item, const QPoint &pos, int column) const;

  /**
   * Draw a line when you drag it somewhere nice.
   */
  virtual void contentsDragMoveEvent (QDragMoveEvent *event);
  virtual void contentsMousePressEvent( QMouseEvent *e );
  virtual void contentsMouseMoveEvent( QMouseEvent *e );
  virtual void contentsMouseDoubleClickEvent ( QMouseEvent *e );
  virtual void contentsDragLeaveEvent (QDragLeaveEvent *event);
  virtual void contentsMouseReleaseEvent (QMouseEvent*);
  virtual void contentsDropEvent (QDropEvent*);
  virtual void contentsDragEnterEvent (QDragEnterEvent *);

  virtual QDragObject *dragObject() const;

  virtual bool acceptDrag(QDropEvent*) const;

  /**
   * Paint the drag line. If painter is null, don't try to :)
   *
   * If after == 0 then the marker should be drawn at the top.
   *
   * @return the rectangle that you painted to.
   */
  virtual QRect drawDropVisualizer (QPainter *p, QListViewItem *parent, QListViewItem *after);	

  /**
   * For future expansion. 
   *
   * Do not use.
   *
   * Highlight @arg item.  painter may be null
   * return the rect drawn to
   * @deprecated
   */
  virtual QRect drawItemHighlighter(QPainter *painter, QListViewItem *item);
  void cleanItemHighlighter ();

  virtual void startDrag();

  inline int dropVisualizerWidth () const { return mDropVisualizerWidth; }
  void setDropVisualizerWidth (int w);

private slots:
  void slotMouseButtonClicked( int btn, QListViewItem *item, const QPoint &pos, int c );
  void doneEditing(QListViewItem *item, int row);

private:
  /**
   * Where is the nearest QListViewItem that I'm going to drop?
   **/
  void findDrop(const QPoint &pos, QListViewItem *&parent, QListViewItem *&after);

private:
  class KListViewPrivate;
  class Tooltip;
  KListViewPrivate *d;

  QRect mOldDropVisualizer;
  int mDropVisualizerWidth;
};

#endif
