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
 * Change Cursor Over Link (tm).
 *
 * There is a new signal executed(). It gets connected to either
 * QListView::clicked() or QListView::doubleClicked() depending on the KDE
 * wide Single Click/Double Click settings. It is strongly recomended that
 * you use this signal instead of the above mentioned. This way you don´t
 * need to care about the current settings.
 * If you want to get informed when the user selects something connect to the
 * QListView::selectionChanged() signal.
 *
 * Drag-and-Drop is supported with the signal @ref dropped, just setAcceptDrops (true)
 * and connect it to a suitable slot.
 * To see where you are dropping, @ref setDropVisualizer(true).
 *
 * KListView is drag-enabled, too: to benefit from that you've got derive from it.
 * Reimplement @ref dragObject() and (possibly) @ref startDrag(),
 * and @ref setDragEnabled(true).
 *
 * @version $Id$
 */
class KListView : public QListView
{
  Q_OBJECT
   
public:
  /**
   * Constructor.
   *
   * The parameters @p parent and @p name are handled by
   * @ref QListView, as usual.
   */
  KListView (QWidget *parent = 0, const char *name = 0);
  
  /**
   * Destructor.
   */
  virtual ~KListView();

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

  /**
   * @return a list containing the currently selected items.
   */
  QList<QListViewItem> selectedItems() const;

  /**
   * Arbitrarily move @p item to @p parent, positioned immediately after item @p after.
   */
  void moveItem(QListViewItem *item, QListViewItem *parent, QListViewItem *after);

  /**
   * @return the last item of this listview.
   */
  QListViewItem *lastItem() const;

  /**
   * For future expansions.
   * 
   * Do not use.
   * @deprecated
   */
  bool itemsMovable() const;

  /**
   * @return whether inplace-renaming has been enabled.
   *
   * @see #setItemsRenameable
   */
  bool itemsRenameable() const;

  /**
   * @return whether dragging is enabled.
   */
  bool dragEnabled() const;

  /**
   * @return true if AutoOpen is enabled.
   */
  bool autoOpen() const;

  /**
   * @return true if @p column is renamable.
   */
  bool isRenameable (int column) const;

  /**
   * @return true if drawing of the drop-visualizer has been enabled.
   */
  bool dropVisualizer() const;

  /**
   * @return the column for which tooltips are displayed (or -1 if none set).
   */
  int tooltipColumn() const;

  /**
   * For future expansions.
   * 
   * Do not use.
   * @deprecated
   */
  bool createChildren() const;

  /**
   * For future expansions.
   * 
   * Do not use.
   * @deprecated
   */
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
   * @param c is the column into which the user clicked.
   *
   * Note that you may not delete any @ref QListViewItem objects in slots
   * connected to this signal.
   *
   * This signal is more or less here for the sake of completeness.
   * You should normally not need to use this. In most cases it´s better
   * to use @ref executed instead.
   */
  void doubleClicked( QListViewItem *item, const QPoint &pos, int c );

  /**
   * This signal gets emitted whenever something acceptable is
   * dropped onto the listview.
   *
   * @param e is the drop-event itself (it has already been accepted)
   * @param after is the item after which the drop occured (or 0L, if
   * the drop was above all items)
   *
   * @see acceptDrop
   */
  void dropped(QDropEvent * e, QListViewItem *after);

  /**
   * For future expansions.
   * 
   * Do not use.
   * @deprecated
   */
  void moved();

  /**
   * This signal gets emitted when an item is renamed via in-place renaming.
   *
   * @param item is the renamed item.
   * @param str is the new value of column @p col.
   * @param col is the renamed column.
   */
  void itemRenamed(QListViewItem * item, const QString &str, int col);

  /**
   * Same as above, but without the extra information.
   */
  void itemRenamed(QListViewItem * item);

public slots:
  /**
   * Rename colum @p c of @p item.
   */
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
   * @deprecated
   */
  virtual void setItemsMovable(bool b);

  /**
   * Enables inplace-renaming of items.
   *
   * @see itemsRenameable
   * @see setRenameable
   */
  virtual void setItemsRenameable(bool b);
  virtual void setDragEnabled(bool b);
  virtual void setAutoOpen(bool b);
  virtual void setDropVisualizer(bool b);
  virtual void setTooltipColumn(int column);
  /**
   * For future expansion.
   * 
   * Do not use.
   * @deprecated
   * Highlight a parent if I drop into it's children
   */
  virtual void setDropHighlighter(bool b);

  /**
   * For future expansions.
   * 
   * Do not use.
   * @deprecated
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
   *
   * Note: @p p is assumed to be in contents coordinates.
   */
  inline bool below (QListViewItem* i, const QPoint& p)
  {
	return below (itemRect(i), contentsToViewport(p));
  }

  void emitExecute( QListViewItem *item, const QPoint &pos, int c );

  virtual void focusOutEvent( QFocusEvent *fe );
  virtual void leaveEvent( QEvent *e );

  /**
   * @return the tooltip for @p column of @p item.
   */
  virtual QString tooltip(QListViewItem* item, int column) const;

  /**
   * @return whether the tooltip for @p column of @p item shall be shown at point @p pos.
   */
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

  /**
   * @return a dragobject encoding the current selection.
   *
   * @see setDragEnabled
   */
  virtual QDragObject *dragObject() const;

  /**
   * @return true if the @p event provides some acceptable
   * format.
   */
  virtual bool acceptDrag (QDropEvent* event) const;

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
   * Highlight @p item.  painter may be null
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
