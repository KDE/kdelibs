/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 2007 Trolltech ASA
 * Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>
 * Copyright (C) 2008 Laurent Montel <montel@kde.org>
 * Copyright (C) 2008 Michael Howell <mhowell123@gmail.com>
 * Copyright (C) 2009 Dawit Alemayehu <adawit @ kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef KGRAPHICSWEBVIEW_H
#define KGRAPHICSWEBVIEW_H

#include <kdewebkit_export.h>

#include <QGraphicsWebView>

class QUrl;
template<class T> class KWebViewPrivate;

/**
 * @short A re-implementation of QGraphicsWebView that provides KDE integration.
 *
 * This is a drop-in replacement for QGraphicsWebView that provides full KDE
 * integration through the use of @ref KWebPage. It also provides signals that
 * capture middle, shift and ctrl mouse clicks on links and URL pasting from the
 * selection clipboard.
 *
 * The specific functionality provided by this class (over and above what
 * would be acheived by using KWebPage with a QGraphicsWebView) is that
 * scrolling * with the mouse wheel while holding down CTRL zooms the page (see
 * QGraphicsWebView::setZoomFactor) and several useful signals are emitted when
 * the user performs certain actions.
 *
 * @author Urs Wolfer <uwolfer @ kde.org>
 * @author Dawit Alemayehu <adawit @ kde.org>
 *
 * @since 4.4
 */
class KDEWEBKIT_EXPORT KGraphicsWebView : public QGraphicsWebView
{
    Q_OBJECT

public:
    /**
     * Constructs a KGraphicsWebView object with parent @p parent.
     *
     * Set @p createCustomPage to false to prevent the creation of a
     * @ref KWebPage object for KDE integration. Doing so allows you to
     * avoid unnecessary object creation and deletion if you are going to
     * use a subclass of KWebPage.
     *
     * @param parent            the parent object
     * @param createCustomPage  if @c true, the view's page is set to an
     *                          instance of KWebPage
     */
    explicit KGraphicsWebView(QGraphicsItem *parent = 0, bool createCustomPage = true);

    /**
     * Destroys the KGraphicsWebView.
     */
    ~KGraphicsWebView();

    /**
     * Returns true if access to remote content is allowed.
     *
     * By default access to remote content is allowed.
     *
     * @see setAllowExternalContent()
     * @see KWebPage::isExternalContentAllowed()
     */
    bool isExternalContentAllowed() const;

    /**
     * Set @p allow to false if you want to prevent access to remote content.
     *
     * If this function is set to false only resources on the local system
     * can be accessed through this class. By default fetching external content
     * is allowed.
     *
     * @see isExternalContentAllowed()
     * @see KWebPage::setAllowExternalContent(bool)
     */
    void setAllowExternalContent(bool allow);

Q_SIGNALS:
    /**
     * Emitted when a URL from the selection clipboard is pasted on this view.
     *
     * This is triggered when the user clicks on the page with the middle
     * mouse button when there is something in the global mouse selection
     * clipboard. This is typically only possible on X11.
     *
     * Uri filters are applied to the selection clipboard to generate @p url.
     *
     * If the content in the selection clipboard is not a valid url and a
     * default search engine is configured, @p url will be set to a query
     * to the default search engine.
     *
     * @param url   url generated from the selection clipboard content
     *
     * @deprecated  use selectionClipboardUrlPasted(QUrl, bool) instead
     * @see QClipboard
     */
#ifndef KDE_NO_DEPRECATED
    KDEWEBKIT_DEPRECATED void selectionClipboardUrlPasted(const QUrl &url);
#endif

    /**
     * Emitted when a URL from the selection clipboard is pasted on this view.
     *
     * This is triggered when the user clicks on the page with the middle
     * mouse button when there is something in the global mouse selection
     * clipboard. This is typically only possible on X11.
     *
     * Uri filters are applied to the selection clipboard to generate @p url.
     *
     * If the content in the selection clipboard is not a valid URL and a
     * default search engine is configured, @p searchText will be set to the
     * content of the clipboard (250 characters maximum) and @p url will be
     * set to a query to the default search engine.
     *
     * @param url         the URL generated from the selection clipboard content
     * @param searchText  content of the selection clipboard if it is not a
     *                    valid URL
     *
     * @see KUriFilter
     * @see QClipboard
     * @since 4.6
     */
    void selectionClipboardUrlPasted(const QUrl &url, const QString& searchText);

    /**
     * Emitted when a link is clicked with the left mouse button while SHIFT is
     * held down.
     *
     * A KDE user would typically expect this to result in the triggering of a
     * "save link as" action.
     *
     * @param url  the URL of the clicked link
     */
    void linkShiftClicked(const QUrl &url);

    /**
     * Emitted when a link is clicked with the middle mouse button or clicked
     * with the left mouse button while CTRL is held down.
     *
     * Typically, the user would expect this to result in the URL being opened
     * in a new tab or window.
     *
     * @param url  the URL of the clicked link
     */
    void linkMiddleOrCtrlClicked(const QUrl &url);

protected:
    /**
     * @reimp
     *
     * Reimplemented for internal reasons, the API is not affected.
     *
     * @see QWidget::wheelEvent
     * @internal
     */
    void wheelEvent(QGraphicsSceneWheelEvent *event);

    /**
     * @reimp
     *
     * Reimplemented for internal reasons, the API is not affected.
     *
     * @see QWidget::mousePressEvent
     * @internal
     */
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

    /**
     * @reimp
     *
     * Reimplemented for internal reasons, the API is not affected.
     *
     * @see QWidget::mouseReleaseEvent
     * @internal
     */
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    friend class KWebViewPrivate<KGraphicsWebView>;
    KWebViewPrivate<KGraphicsWebView> * const d;
};

#endif // KWEBVIEW_H
