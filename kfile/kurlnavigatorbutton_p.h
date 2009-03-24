/*****************************************************************************
 * Copyright (C) 2006 by Peter Penz <peter.penz@gmx.at>                      *
 * Copyright (C) 2006 by Aaron J. Seigo <aseigo@kde.org>                     *
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

#ifndef KURLNAVIGATORBUTTON_P_H
#define KURLNAVIGATORBUTTON_P_H

#include "kurltogglebutton_p.h"
#include "kurlnavigatormenu_p.h"

#include <kio/global.h>
#include <kio/udsentry.h>
#include <kurl.h>

class KJob;
class KUrlNavigator;
class QDropEvent;
class QPaintEvent;

namespace KIO
{
    class Job;
}

/**
 * @brief Button of the URL navigator which contains one part of an URL.
 *
 * It is possible to drop a various number of items to an UrlNavigatorButton. In this case
 * a context menu is opened where the user must select whether he wants
 * to copy, move or link the dropped items to the URL part indicated by
 * the button.
 */
class KUrlNavigatorButton : public KUrlButton
{
    Q_OBJECT

public:
    explicit KUrlNavigatorButton(int index, KUrlNavigator* parent);
    virtual ~KUrlNavigatorButton();

    void setIndex(int index);
    int index() const;

    void setActive(bool active);
    bool isActive() const;

    /* Implementation note: QAbstractButton::setText() is not virtual,
     * but KUrlNavigatorButton needs to adjust the minimum size when
     * the text has been changed. KUrlNavigatorButton::setText() hides
     * QAbstractButton::setText() which is not nice, but sufficient for
     * the usage in KUrlNavigator.
     */
    void setText(const QString& text);

    /** @see QWidget::sizeHint() */
    virtual QSize sizeHint() const;

Q_SIGNALS:
    /**
     * Is emitted if URLs have been dropped
     * to the destination \a destination.
     */
    void urlsDropped(const KUrl& destination, QDropEvent* event);

protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void enterEvent(QEvent* event);
    virtual void leaveEvent(QEvent* event);
    virtual void dropEvent(QDropEvent* event);
    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dragMoveEvent(QDragMoveEvent* event);
    virtual void dragLeaveEvent(QDragLeaveEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);

private Q_SLOTS:
    void updateNavigatorUrl();
    void startPopupDelay();
    void stopPopupDelay();
    void startListJob();
    void entriesList(KIO::Job* job, const KIO::UDSEntryList& entries);
    void listJobFinished(KJob* job);
    void urlsDropped(QAction* action, QDropEvent* event);

private:
    int arrowWidth() const;
    bool isAboveArrow(int x) const;
    bool isTextClipped() const;
    void updateMinimumWidth();

private:
    int m_index;
    bool m_hoverArrow;
    QTimer* m_popupDelay;
    KIO::Job* m_listJob;
    QStringList m_subdirs;
    KUrlNavigatorMenu* m_dirsMenu;
};

inline int KUrlNavigatorButton::index() const
{
    return m_index;
}

#endif
