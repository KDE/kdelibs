/*****************************************************************************
 * Copyright (C) 2006 by Peter Penz <peter.penz@gmx.at>                      *
 * Copyright (C) 2006 by Aaron J. Seigo <aseigo@kde.org>                     *
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

#ifndef KURLNAVIGATORBUTTON_P_H
#define KURLNAVIGATORBUTTON_P_H

#include "kurlnavigatorbuttonbase_p.h"
#include "kurlnavigatormenu_p.h"

#include <kio/global.h>
#include <kio/udsentry.h>
#include <kurl.h>

#include <QPointer>

class KJob;
class QDropEvent;
class QPaintEvent;

namespace KIO
{
    class Job;
}

namespace KDEPrivate
{

/**
 * @brief Button of the URL navigator which contains one part of an URL.
 *
 * It is possible to drop a various number of items to an UrlNavigatorButton. In this case
 * a context menu is opened where the user must select whether he wants
 * to copy, move or link the dropped items to the URL part indicated by
 * the button.
 */
class KUrlNavigatorButton : public KUrlNavigatorButtonBase
{
    Q_OBJECT

public:
    explicit KUrlNavigatorButton(const QUrl& url, QWidget* parent);
    virtual ~KUrlNavigatorButton();

    void setUrl(const QUrl& url);
    QUrl url() const;

    /* Implementation note: QAbstractButton::setText() is not virtual,
     * but KUrlNavigatorButton needs to adjust the minimum size when
     * the text has been changed. KUrlNavigatorButton::setText() hides
     * QAbstractButton::setText() which is not nice, but sufficient for
     * the usage in KUrlNavigator.
     */
    void setText(const QString& text);

    /**
     * Sets the name of the sub directory that should be marked when
     * opening the sub directories popup.
     */
    void setActiveSubDirectory(const QString& subDir);
    QString activeSubDirectory() const;

    /** @see QWidget::sizeHint() */
    virtual QSize sizeHint() const;

    void setShowMnemonic(bool show);
    bool showMnemonic() const;

Q_SIGNALS:
    /**
     * Is emitted if URLs have been dropped
     * to the destination \a destination.
     */
    void urlsDropped(const QUrl& destination, QDropEvent* event);

    void clicked(const QUrl& url, Qt::MouseButton button);

    /**
     * Is emitted, if KUrlNavigatorButton::setUrl() cannot resolve
     * the text synchronously and KUrlNavigator::text() will return
     * an empty string in this case. The signal finishedTextResolving() is
     * emitted, as soon as the text has been resolved.
     */
    void startedTextResolving();

    /**
     * Is emitted, if the asynchronous resolving of the text has
     * been finished (see startTextResolving()).
     * KUrlNavigatorButton::text() contains the resolved text.
     */
    void finishedTextResolving();

protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void enterEvent(QEvent* event);
    virtual void leaveEvent(QEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void dropEvent(QDropEvent* event);
    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dragMoveEvent(QDragMoveEvent* event);
    virtual void dragLeaveEvent(QDragLeaveEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);

private Q_SLOTS:
    /**
     * Requests to load the sub-directories after a short delay.
     * startSubDirsJob() is invoked if the delay is exceeded.
     */
    void requestSubDirs();

    /**
     * Starts to load the sub directories asynchronously. The directories
     * are stored in m_subDirs by addEntriesToSubDirs().
     */
    void startSubDirsJob();

    /**
     * Adds the entries from the sub-directories job to m_subDirs. The entries
     * will be shown if the job has been finished in openSubDirsMenu() or
     * replaceButton().
     */
    void addEntriesToSubDirs(KIO::Job* job, const KIO::UDSEntryList& entries);

    /**
     * Is called after the sub-directories job has been finished and opens a menu
     * showing all sub directories.
     */
    void openSubDirsMenu(KJob* job);

    /**
     * Is called after the sub-directories job has been finished and replaces
     * the button content by the current sub directory (triggered by
     * the scroll wheel).
     */
    void replaceButton(KJob* job);

    void urlsDropped(QAction* action, QDropEvent* event);

    /**
     * Is called, if an action of a sub-menu has been triggered by
     * a click with the middle mouse-button.
     */
    void slotMenuActionClicked(QAction* action);

    void statFinished(KJob*);

private:
    /**
     * Cancels any request done by requestSubDirs().
     */
    void cancelSubDirsRequest();

    /**
     * @return Text without mnemonic characters.
     */
    QString plainText() const;

    int arrowWidth() const;
    bool isAboveArrow(int x) const;
    bool isTextClipped() const;
    void updateMinimumWidth();
    void initMenu(KUrlNavigatorMenu* menu, int startIndex);

private:
    bool m_hoverArrow;
    bool m_pendingTextChange;
    bool m_replaceButton;
    bool m_showMnemonic;
    int m_wheelSteps;
    QUrl m_url;

    QString m_subDir;
    QTimer* m_openSubDirsTimer;
    KIO::Job* m_subDirsJob;

    /// pair of name and display name
    QList<QPair<QString,QString> > m_subDirs;

    static QPointer<KUrlNavigatorMenu> m_subDirsMenu;
};

} // namespace KDEPrivate

#endif
