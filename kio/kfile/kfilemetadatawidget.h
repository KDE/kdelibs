/*****************************************************************************
 * Copyright (C) 2008 by Sebastian Trueg <trueg@kde.org>                     *
 * Copyright (C) 2009-2010 by Peter Penz <peter.penz@gmx.at>                 *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#ifndef KFILEMETADATAWIDGET_H
#define KFILEMETADATAWIDGET_H

#include <kio/kio_export.h>
#include <kfileitem.h>

#include <QList>
#include <QWidget>

class KUrl;

/**
 * @brief Shows the meta data of one or more file items.
 *
 * Meta data like name, size, rating, comment, ... are
 * shown as several rows containing a description and
 * the meta data value. It is possible for the user
 * to change specific meta data like rating, tags and
 * comment. The changes are stored automatically by the
 * meta data widget.
 *
 * @since 4.5
 */
class KIO_EXPORT KFileMetaDataWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KFileMetaDataWidget(QWidget* parent = 0);
    virtual ~KFileMetaDataWidget();

    /**
     * Sets the items for which the meta data should be shown.
     */
    void setItems(const KFileItemList& items);
    KFileItemList items() const;

    /**
     * If set to true, data such as the comment, tag or rating cannot be
     * changed by the user. Per default read-only is disabled.
     */
    void setReadOnly(bool readOnly);
    bool isReadOnly() const;

    /** @see QWidget::sizeHint() */
    virtual QSize sizeHint() const;

Q_SIGNALS:
    /**
     * Is emitted, if a meta data represents an URL that has
     * been clicked by the user.
     */
    void urlActivated(const KUrl& url);

protected:
    virtual bool event(QEvent* event);

private:
    class Private;
    Private* d;

    Q_PRIVATE_SLOT(d, void slotLoadingFinished())
    Q_PRIVATE_SLOT(d, void removeOutdatedRows())
    Q_PRIVATE_SLOT(d, void slotLinkActivated(QString))
    Q_PRIVATE_SLOT(d, void slotDataChangeStarted())
    Q_PRIVATE_SLOT(d, void slotDataChangeFinished())
};

#endif
