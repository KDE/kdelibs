/*******************************************************************************
 *   Copyright (C) 2008-2009 by Peter Penz <peter.penz@gmx.at>                 *
 *                                                                             *
 *   This library is free software; you can redistribute it and/or             *
 *   modify it under the terms of the GNU Library General Public               *
 *   License as published by the Free Software Foundation; either              *
 *   version 2 of the License, or (at your option) any later version.          *
 *                                                                             *
 *   This library is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *   Library General Public License for more details.                          *
 *                                                                             *
 *   You should have received a copy of the GNU Library General Public License *
 *   along with this library; see the file COPYING.LIB.  If not, write to      *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 *   Boston, MA 02110-1301, USA.                                               *
 *******************************************************************************/

#ifndef KFILEPREVIEWGENERATOR_H
#define KFILEPREVIEWGENERATOR_H

#include <kfile_export.h>

#include <QtCore/QObject>

class KAbstractViewAdapter;
class KDirModel;
class QAbstractItemView;
class QAbstractProxyModel;

/**
 * @brief Generates previews for files of an item view.
 *
 * Per default a preview is generated for each item.
 * Additionally the clipboard is checked for cut items.
 * The icon state for cut items gets dimmed automatically.
 *
 * The following strategy is used when creating previews:
 * - The previews for currently visible items are created before
 *   the previews for invisible items.
 * - If the user changes the visible area by using the scrollbars,
 *   all pending previews get paused. As soon as the user stays
 *   on the same position for a short delay, the previews are
 *   resumed. Also in this case the previews for the visible items
 *   are generated first.
 *
 * @since 4.2
 */
class KFILE_EXPORT KFilePreviewGenerator : public QObject
{
    Q_OBJECT

public:
    /**
     * @param parent  Item view containing the file items where previews should
     *                be generated. It is mandatory that the item view specifies
     *                an icon size by QAbstractItemView::setIconSize() and that
     *                the model of the view (or the source model of the proxy model)
     *                is an instance of KDirModel. Otherwise no previews will be generated.
     */
    KFilePreviewGenerator(QAbstractItemView* parent);

    /** @internal */
    KFilePreviewGenerator(KAbstractViewAdapter* parent, QAbstractProxyModel* model);

    virtual ~KFilePreviewGenerator();

    /**
     * If \a show is set to true, a preview is generated for each item. If \a show
     * is false, the MIME type icon of the item is shown instead. Per default showing
     * the preview is turned on. Note that it is mandatory that the item view
     * specifies an icon size by QAbstractItemView::setIconSize(), otherwise
     * KFilePreviewGenerator::isPreviewShown() will always return false.
     */
    void setPreviewShown(bool show);
    bool isPreviewShown() const;

    /**
     * @deprecated Use KFilePreviewGenerator::updateIcons() instead.
     */
    void updatePreviews();

    /**
     * Updates the icons for all items. Usually it is only
     * necessary to invoke this method when the icon size of the abstract item view
     * has been changed by QAbstractItemView::setIconSize(). Note that this method
     * should also be invoked if previews have been turned off, as the icons for
     * cut items must be updated when the icon size has changed.
     * @since 4.3
     */
    void updateIcons();

    /** Cancels all pending previews. */
    void cancelPreviews();

    /**
     * Sets the list of enabled thumbnail plugins.
     * If the list is empty, all plugins will be enabled.
     * All plugins are enabled by default.
     *
     * Note that this method doesn't cause already generated previews
     * to be regenerated.
     *
     * For a list of available plugins, call KServiceTypeTrader::self()->query("ThumbCreator").
     *
     * @see enabledPlugins
     */
    void setEnabledPlugins(const QStringList& list);

    /**
     * Returns the list of enabled thumbnail plugins,
     * or an empty list if all plugins are enabled.
     *
     * @see setEnabledPlugins
     */
    QStringList enabledPlugins() const;

private:
    class Private;
    Private* const d; /// @internal
    Q_DISABLE_COPY(KFilePreviewGenerator)

    Q_PRIVATE_SLOT(d, void updateIcons(const KFileItemList&))
    Q_PRIVATE_SLOT(d, void updateIcons(const QModelIndex&, const QModelIndex&))
    Q_PRIVATE_SLOT(d, void addToPreviewQueue(const KFileItem&, const QPixmap&))
    Q_PRIVATE_SLOT(d, void slotPreviewJobFinished(KJob*))
    Q_PRIVATE_SLOT(d, void updateCutItems())
    Q_PRIVATE_SLOT(d, void dispatchIconUpdateQueue())
    Q_PRIVATE_SLOT(d, void pauseIconUpdates())
    Q_PRIVATE_SLOT(d, void resumeIconUpdates())
    Q_PRIVATE_SLOT(d, void resolveMimeType())
    Q_PRIVATE_SLOT(d, void requestSequenceIcon(const QModelIndex&, int))
    Q_PRIVATE_SLOT(d, void delayedIconUpdate())
    Q_PRIVATE_SLOT(d, void rowsAboutToBeRemoved(const QModelIndex&, int, int))
};

#endif
