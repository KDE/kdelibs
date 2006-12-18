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

#ifndef KPAGEMODEL_H
#define KPAGEMODEL_H

#include <QtCore/QAbstractItemModel>

#include <kdelibs_export.h>

/**
 *  @short A base class for a model used by KPageView.
 *
 *  This class is an abstract base class which must be used to
 *  implement custom models for KPageView. Additional to the the standard
 *  Qt::ItemDataRoles it provides the two roles
 *
 *    @li HeaderRole
 *    @li WidgetRole
 *
 *  which are used to return a header string for a page and a QWidget
 *  pointer to the page itself.
 *
 *  <b>Example:</b>\n
 *
 *  \code
 *    KPageView *view = new KPageView( this );
 *    KPageModel *model = new MyPageModel( this );
 *
 *    view->setModel( model );
 *  \endcode
 *
 *  @see KPageView
 *  @author Tobias Koenig <tokoe@kde.org>
 */
class KDEUI_EXPORT KPageModel : public QAbstractItemModel
{
  Q_OBJECT

  public:
    /**
     * Additional KPageModel specific roles.
     *
     * @li HeaderRole - The data to be rendered as page header (usually text).
     * @li WidgetRole - The data which contains a pointer to the page widget.
     */
    enum Role
    {
      HeaderRole = Qt::UserRole + 1,
      WidgetRole
    };

    /**
     * Constructs a page model with the given parent.
     */
    explicit KPageModel( QObject *parent = 0 );

    /**
     * Destroys the page model.
     */
    virtual ~KPageModel();

  private:
    class Private;
    Private* const d;
};

#endif
