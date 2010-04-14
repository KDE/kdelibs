/*****************************************************************************
 * Copyright (C) 2009 by Peter Penz <peter.penz@gmx.at>                      *
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

#ifndef KFILEMETADATACONFIGURATIONWIDGET_H
#define KFILEMETADATACONFIGURATIONWIDGET_H

#include <kio/kio_export.h>
#include <kfileitem.h>

#include <QWidget>


/**
 * @brief Widget which allows to configure which meta data should be shown
 *        in the KFileMetaDataWidget.
 *
 * Note that the class is marked as internal and the API might change.
 * It is planned to make the class public in future KDE SC's, after the API has
 * been stabilized.
 *
 * @see KFileMetaDataWidget
 * @since 4.5
 * @internal
 */
// TODO: A signal is required which indicates configuration changes. Otherwise
// it would not be possible for an application developer to enable/disable an
// "Apply" button of a dialog.
class KIO_EXPORT KFileMetaDataConfigurationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KFileMetaDataConfigurationWidget(QWidget* parent = 0);
    virtual ~KFileMetaDataConfigurationWidget();

    /**
     * Sets the items, for which the visibility of the meta data should
     * be configured. Note that the visibility of the meta data is not
     * bound to the items itself, the items are only used to determine
     * which meta data should be configurable. For example when a JPEG image
     * is set as item, it will be configurable which EXIF data should be
     * shown. If an audio file is set as item, it will be configurable
     * whether the artist, album name, ... should be shown.
     */
    void setItems(const KFileItemList& items);
    KFileItemList items() const;

    /**
     * Saves the modified configuration.
     */
    void save();

    /** @see QWidget::sizeHint() */
    virtual QSize sizeHint() const;

protected:
    virtual bool event(QEvent* event);

private:
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT(d, void loadMetaData())
    Q_PRIVATE_SLOT(d, void slotLoadingFinished())
};

#endif
