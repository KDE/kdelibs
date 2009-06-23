/*****************************************************************************
 * Copyright (C) 2006 by Peter Penz <peter.penz@gmx.at>                      *
 * Copyright (C) 2006 by Aaron J. Seigo <aseigo@kde.org>                     *
 * Copyright (C) 2007 by Kevin Ottens <ervin@kde.org>                        *
 * Copyright (C) 2007 by Urs Wolfer <uwolfer @ kde.org>                      *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License version 2 as published by the Free Software Foundation.           *
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

#ifndef KURLNAVIGATOR_H
#define KURLNAVIGATOR_H

#include <kfile_export.h>

#include <kurl.h>
#include <QtGui/QWidget>

class KFilePlacesModel;
class KUrlComboBox;
class QMouseEvent;

/**
 * @brief Allows to navigate through the paths of an URL.
 *
 * The URL navigator offers two modes:
 * - Editable:     Represents the 'classic' mode, where the URL
 *                 is editable inside a line editor. By pressing RETURN
 *                 the URL will get activated.
 * - Non editable ("breadcrumb view"): The URL is represented by a
 *                 number of buttons, where each button represents a path
 *                 of the URL. By clicking on a button the path will get
 *                 activated. This mode also supports drag and drop of items.
 *
 * The mode can be changed by clicking on the empty area of the URL navigator.
 * It is recommended that the application remembers the setting
 * or allows to configure the default mode (see KUrlNavigator::setUrlEditable()).
 *
 * The URL navigator remembers the URL history during navigation and allows to go
 * back and forward within this history.
 *
 * In the non editable mode ("breadcrumb view") it can be configured whether
 * the full path should be shown. It is recommended that the application
 * remembers the setting or allows to configure the default mode (see
 * KUrlNavigator::setShowFullPath()).
 *
 * The typical usage of the KUrlNavigator is:
 * - Create an instance providing a places model and an URL.
 * - Create an instance of QAbstractItemView which shows the content of the URL
 *   given by the URL navigator.
 * - Connect to the signal KUrlNavigator::urlChanged() and synchronize the content of
 *   QAbstractItemView with the URL given by the URL navigator.
 */
class KFILE_EXPORT KUrlNavigator : public QWidget
{
    Q_OBJECT

public:
    /**
     * @param placesModel    Model for the places which are selectable inside a
     *                       menu. A place can be a bookmark or a device. If it is 0,
                             there is no places selector displayed.
     * @param url            URL which is used for the navigation or editing.
     * @param parent         Parent widget.
     */
    KUrlNavigator(KFilePlacesModel* placesModel, const KUrl& url, QWidget* parent);
    virtual ~KUrlNavigator();

    /** Returns the current URL. */
    // KDE5: return 'KUrl' instead of 'const KUrl&'
    const KUrl& url() const;

    /**
     * Returns the currently entered, but not accepted URL.
     * Attention: It is possible that the returned URL is not valid!
     */
    KUrl uncommittedUrl() const;

    /**
     * Returns the portion of the current URL up to the path part given
     * by \a index. Assuming that the current URL is /home/peter/Documents/Music,
     * then the following URLs are returned for an index:
     * - index <= 0: /home
     * - index is 1: /home/peter
     * - index is 2: /home/peter/Documents
     * - index >= 3: /home/peter/Documents/Music
     */
    KUrl url(int index) const;

    /**
     * Goes back one step in the URL history. The signals
     * KUrlNavigator::urlChanged() and KUrlNavigator::historyChanged()
     * are emitted if true is returned. False is returned if the beginning
     * of the history has already been reached and hence going back was not
     * possible. The history index (see KUrlNavigator::historyIndex()) is
     * increased by one if the operation was successful.
     */
    bool goBack();

    /**
     * Goes forward one step in the URL history. The signals
     * KUrlNavigator::urlChanged() and KUrlNavigator::historyChanged()
     * are emitted if true is returned. False is returned if the end
     * of the history has already been reached and hence going forward
     * was not possible. The history index (see KUrlNavigator::historyIndex()) is
     * decreased by one if the operation was successful.
     */
    bool goForward();

    /**
     * Goes up one step of the URL path and remembers the old path
     * in the history. The signals KUrlNavigator::urlChanged() and
     * KUrlNavigator::historyChanged() are emitted if true is returned.
     * False is returned if going up was not possible as the root has
     * been reached.
     */
    bool goUp();

    /**
     * Goes to the home URL and remembers the old URL in the history.
     * The signals KUrlNavigator::urlChanged()
     * and KUrlNavigator::historyChanged() are emitted.
     *
     * @see KUrlNavigator::setHomeUrl()
     */
    void goHome();

    /**
     * Sets the home URL used by KUrlNavigator::goHome(). If no
     * home URL is set, the default home path of the user is used.
     */
    void setHomeUrl(const QString& homeUrl);

    /**
     * Allows to edit the URL of the navigation bar if \a editable
     * is true, and sets the focus accordingly.
     * If \a editable is false, each part of
     * the URL is presented by a button for a fast navigation ("breadcrumb view").
     */
    void setUrlEditable(bool editable);

    /**
     * @return True, if the URL is editable within a line editor.
     *         If false is returned, each part of the URL is presented by a button
     *         for fast navigation ("breadcrumb view").
     */
    bool isUrlEditable() const;

    /**
     * Shows the full path of the URL even if a place represents a part of the URL.
     * Assuming that a place called "Pictures" uses the URL /home/user/Pictures.
     * An URL like /home/user/Pictures/2008 is shown as [Pictures] > [2008]
     * in the breadcrumb view, if showing the full path is turned off. If
     * showing the full path is turned on, the URL is shown
     * as [/] > [home] > [Pictures] > [2008].
     * @since 4.2
     */
    void setShowFullPath(bool show);
    
    /**
     * @return True, if the full path of the URL should be shown in the breadcrumb view.
     * @since 4.2
     */
    bool showFullPath() const;
    
    /**
     * Set the URL navigator to the active mode, if \a active
     * is true. The active mode is default. The inactive mode only differs
     * visually from the active mode, no change of the behavior is given.
     *
     * Using the URL navigator in the inactive mode is useful when having split views,
     * where the inactive view is indicated by an inactive URL
     * navigator visually.
     */
    void setActive(bool active);

    /**
     * Returns true, if the URL navigator is in the active mode.
     * @see KUrlNavigator::setActive()
     */
    bool isActive() const;

    /**
     * Sets the places selector visible, if \a visible is true.
     * The places selector allows to select the places provided
     * by the places model passed in the constructor. Per default
     * the places selector is visible.
     */
    void setPlacesSelectorVisible(bool visible);

    /** Returns true, if the places selector is visible. */
    bool isPlacesSelectorVisible() const;

    /** Returns the amount of items in the history. */
    int historySize() const;

    /**
     * Returns the history index of the current URL, where
     * 0 <= history index < KUrlNavigator::historySize(). As long
     * as KUrlNavigator::goBack() is not invoked, the history index
     * stays on 0.
     */
    int historyIndex() const;
    
    /**
     * @return URL for the history element with the index \a historyIndex.
     *         The history index 0 represents the most recent URL.
     * @since 4.3
     */
    KUrl historyUrl(int historyIndex) const;

    /**
     * Returns the saved root URL for the current URL
     * (see KUrlNavigator::saveRootUrl()).
     */    
    // KDE5: return 'KUrl' instead of 'const KUrl&'    
    const KUrl& savedRootUrl() const;

    /**
     * Returns the saved contents position of the upper left corner
     * for the current URL.
     */
    QPoint savedPosition() const;

    /**
     * Returns the used editor when the navigator is in the edit mode
     * (see KUrlNavigator::setUrlEditable()).
     */
    KUrlComboBox* editor() const;

    /**
     * If an application supports only some special protocols, they can be set
     * with \a protocols .
     */
    void setCustomProtocols(const QStringList &protocols);

    /**
     * Returns the custom protocols if they are set, QStringList() otherwise.
     */
    QStringList customProtocols() const;

public Q_SLOTS:
    /**
     * Sets the active URL to \a url. The old URL is added to the history.
     * The signals KUrlNavigator::urlChanged() and KUrlNavigator::historyChanged()
     * are emitted.
     */
    void setUrl(const KUrl& url);

    /**
     * Activates the URL navigator (KUrlNavigator::isActive() will return true)
     * and emits the signal KUrlNavigator::activated().
     */
    void requestActivation();

    /**
     * Saves the used root URL of the content for the current history element.
     * Saving the root URL might be useful if the item view is represented by a tree or
     * columns, where it is mandatory to restore the root URL to iterating
     * through the history.
     */
    void saveRootUrl(const KUrl& url);

    /**
     * Saves the coordinates of the contents for
     * the current history element. The contents of the URL is usually shown
     * inside an instance of QAbstractItemView. It is recommended to invoke this
     * slot whenever the upper left position of the QAbstractItemView has been
     * changed to be able to restore the position when going back in history.
     *
     * @see KUrlNavigator::savedPosition()
     */
    void savePosition(int x, int y);

    /* @see QWidget::setFocus() */
    void setFocus();

Q_SIGNALS:
    /**
     * Is emitted, if the URL navigator has been activated by
     * a user interaction.
     */
    void activated();

    /**
     * Is emitted, if the URL has been changed e. g. by
     * the user.
     * @see KUrlNavigator::setUrl()
     */
    void urlChanged(const KUrl& url);

    /**
     * Is emitted, if the editable state for the URL has been changed
     * (see KUrlNavigator::setUrlEditable()).
     */
    void editableStateChanged(bool editable);

    /**
     * Is emitted, if the history has been changed. Usually
     * the history is changed if a new URL has been selected.
     */
    void historyChanged();

    /**
     * Is emitted if the URLs \a urls have been dropped
     * to the destination \a destination.
     */
    // KDE5: remove, as the signal has been replaced by
    // urlsDropped(const KUrl& destination, QDropEvent* event)
    void urlsDropped(const KUrl::List& urls,
                     const KUrl& destination);
                     
    /**
     * Is emitted if a dropping has been done above the destination
     * \a destination. The receiver must accept the drop event if
     * the dropped data can be handled.
     *
     * @since 4.2
     */
    void urlsDropped(const KUrl& destination, QDropEvent* event);

    /**
     * This signal is emitted when the Return or Enter key is pressed.
     */
    void returnPressed();

protected:
    /*
     * If the Escape key is pressed, the navigation bar should switch
     * to the breadcrumb view.
     * @see QWidget::keyReleaseEvent()
     */
    virtual void keyReleaseEvent(QKeyEvent* event);

    /*
     * Paste the clipboard content as URL, if the middle mouse
     * button has been clicked.
     * @see QWidget::mouseReleaseEvent()
     */
    virtual void mouseReleaseEvent(QMouseEvent* event);

    /* @see QWidget::resizeEvent() */
    virtual void resizeEvent(QResizeEvent* event);
    
    virtual bool eventFilter(QObject* watched, QEvent* event);

private:
    Q_PRIVATE_SLOT(d, void slotReturnPressed())
    Q_PRIVATE_SLOT(d, void slotRemoteHostActivated())
    Q_PRIVATE_SLOT(d, void slotProtocolChanged(const QString& protocol))
    Q_PRIVATE_SLOT(d, void switchView())
    Q_PRIVATE_SLOT(d, void dropUrls(const KUrl& destination, QDropEvent*))
    Q_PRIVATE_SLOT(d, void openPathSelectorMenu())
    Q_PRIVATE_SLOT(d, void updateButtonVisibility())
    Q_PRIVATE_SLOT(d, void switchToBreadcrumbMode())
    Q_PRIVATE_SLOT(d, void updateContent())

private:
    class Private;
    Private* const d;

    Q_DISABLE_COPY(KUrlNavigator)
};

#endif
