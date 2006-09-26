/*
    This file is part of the KDE Libraries

    Copyright (C) 2006 Tobias Koenig (tokoe@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KPAGEVIEW_H
#define KPAGEVIEW_H

#include <QtGui/QWidget>

#include <kdelibs_export.h>

class KPageModel;

class QAbstractItemDelegate;
class QAbstractItemView;
class QModelIndex;

/**
 * @short A base class which can handle multiple pages.
 *
 * This class provides a widget base class which handles multiple
 * pages and allows the user to switch between these pages in
 * different ways.
 *
 * Currently, @p Auto, @p Plain, @p List, @p Tree and @p Tabbed face
 * types are available (@see KPageView).
 *
 * <b>Example:</b>\n
 *
 * \code
 *  KPageModel *model = new MyPageModel();
 *
 *  KPageView *view = new KPageView( this );
 *  view->setModel( model );
 *
 *  view->setFaceType( KPageView::List );
 * \endcode
 *
 * @author Tobias Koenig (tokoe@kde.org)
 */
class KDEUI_EXPORT KPageView : public QWidget
{
  Q_OBJECT
  Q_ENUMS( FaceType )
  Q_PROPERTY( FaceType faceType READ faceType WRITE setFaceType )

  public:
    /**
     * This enum is used to decide which type of navigation view
     * shall be used in the page view.
     *
     * @li Auto   - Depending on the number of pages in the model,
     *              the Plain (one page), the List (several pages)
     *              or the Tree face (nested pages) will be used.
     *              This is the default face type.
     * @li Plain  - No navigation view will be visible and only the
     *              first page of the model will be shown.
     *
     * @li List   - An icon list is used as navigation view.
     *
     * @li Tree   - A tree list is used as navigation view.
     *
     * @li Tabbed - A tab widget is used as navigation view.
     */
    enum FaceType
    {
      Auto,
      Plain,
      List,
      Tree,
      Tabbed
    };

    /**
     * Creates a page view with given parent.
     */
    explicit KPageView( QWidget *parent = 0 );

    /**
     * Destroys the page view.
     */
    virtual ~KPageView();

    /**
     * Sets the @param model of the page view.
     */
    void setModel( KPageModel *model );

    /**
     * Returns the model of the page view.
     */
    KPageModel* model() const;

    /**
     * Sets the face type of the page view.
     */
    void setFaceType( FaceType faceType );

    /**
     * Returns the face type of the page view.
     */
    FaceType faceType() const;

    /**
     * Sets the page with @param index to be the current page and emits
     * the @see currentPageChanged signal.
     */
    void setCurrentPage( const QModelIndex &index );

    /**
     * Returns the index for the current page or an invalid index
     * if no current page exists.
     */
    QModelIndex currentPage() const;

    /**
     * Sets the item @param delegate which can be used customize
     * the page view.
     */
    void setItemDelegate( QAbstractItemDelegate *delegate );

    /**
     * Returns the item delegate of the page view.
     */
    QAbstractItemDelegate* itemDelegate() const;

  Q_SIGNALS:
    /**
     * This signal is emitted whenever the current page changes.
     * The previous page index is replaced by the current index.
     */
    void currentPageChanged( const QModelIndex &current, const QModelIndex &previous );

  private Q_SLOTS:
    void pageSelected( const QModelIndex&, const QModelIndex& );
    void modelChanged();

  protected:
    /**
     * Returns the navigation view, depending on the current
     * face type.
     *
     * This method can be reimplemented to provide custom
     * navigation views.
     */
    virtual QAbstractItemView* createView();

    /**
     * Returns whether the page header should be visible.
     *
     * This method can be reimplemented for adapting custom
     * views.
     */
    virtual bool showPageHeader() const;

    /**
     * Returns the position where the navigation view should be
     * located according the page stack.
     *
     * This method can be reimplemented for adapting custom
     * views.
     */
    Qt::Alignment viewPosition() const;

  private:
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void rebuildGui() )
};

#endif
