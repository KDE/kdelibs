/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMA_WEBVIEW_H
#define PLASMA_WEBVIEW_H

#include <plasma/plasma_export.h>
#include <QGraphicsWidget>

#include <kurl.h>

class QWebPage;
class QWebFrame;
class QKeyEvent;
class QGraphicsSceneDragDropEvent;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;
class QRect;

namespace Plasma
{

class WebViewPrivate;

/**
 * @class WebView plasma/widgets/webview.h <Plasma/Widgets/WebView>
 *
 * @short Provides a widget to display html content in Plasma.
 */
class PLASMA_EXPORT WebView : public QGraphicsWidget
{
    Q_OBJECT

    Q_PROPERTY(KUrl url READ url WRITE setUrl)
    Q_PROPERTY(QString html READ html WRITE setHtml)
    Q_PROPERTY(bool dragToScroll READ dragToScroll WRITE setDragToScroll)
    Q_PROPERTY(QPointF scrollPosition READ scrollPosition WRITE setScrollPosition)
    Q_PROPERTY(QSizeF contentsSize READ contentsSize)
    Q_PROPERTY(QRectF viewportGeometry READ viewportGeometry)
    Q_PROPERTY(qreal zoomFactor READ zoomFactor WRITE setZoomFactor)

    public:
        explicit WebView(QGraphicsItem *parent = 0);
        ~WebView();

        /**
         * Sets the URL to display. Loading may happen asynchronously.
         *
         * @param url the location of the content to load.
         */
        void setUrl(const KUrl &url);

        /**
         * @return the html content of the page
         */
        KUrl url() const;

        /**
         * Sets the html to be shown along with a base URL to be used
         * to resolve relative references.
         *
         * @param html the html (in utf8) to display in the content area
         * @param baseUrl the base url for relative references
         */
        void setHtml(const QByteArray &html, const KUrl &baseUrl = KUrl());

        /**
         * @return the html content of the page
         */
        QString html() const;

        /**
         * Sets the html to be shown along with a base URL to be used
         * to resolve relative references.
         *
         * @param html the html (in utf8) to display in the content area
         * @param baseUrl the base url for relative references
         */
        void setHtml(const QString &html, const KUrl &baseUrl = KUrl());

        /**
         * Reimplementation
         */
        QRectF geometry() const;

        /**
         * @return the size of the internal widget
         * @since 4.4
         */
        QSizeF contentsSize() const;

        /**
         * Sets the position of the webpage relative to this widget
         * @since 4.4
         */
        void setScrollPosition(const QPointF &position);

        /**
         * @return the position of the webpage relative to this widget
         * @since 4.4
         */
        QPointF scrollPosition() const;

        /**
         * The geometry of the area that actually displays the web page
         * @since 4.4
         */
        QRectF viewportGeometry() const;

        /**
         * The zoom factor of the page
         *
         * @since 4.4
         */
        qreal zoomFactor() const;

        /**
         * Sets the zoom factor of the page
         *
         * @since 4.4
         */
        void setZoomFactor(const qreal zoom);

        /**
         * Sets the page to use in this item. The owner of the webpage remains,
         * however if this WebView object is the owner of the current page,
         * then the current page is deleted
         *
         * @param page the page to set in this view
         */
        void setPage(QWebPage *page);

        /**
         * The QWebPage associated with this item. Useful when more
         * of the features of the full QWebPage object need to be accessed.
         */
        QWebPage *page() const;

        /**
         * The main web frame associated with this item.
         */
        QWebFrame *mainFrame() const;

        /**
         * Sets if the page can be scrolled around by dragging the contents with the mouse
         * @since 4.3
         */
        void setDragToScroll(bool drag);

        /**
         * @return true if the page can be scrolled by dragging the mouse
         * @since 4.3
         */
        bool dragToScroll();

        /**
         * Reimplementation
         */
        void setGeometry(const QRectF &geometry);

    public Q_SLOTS:
        /**
         * Loads the previous document in the list of documents built by navigating links.
         * @since 4.6
         */
        void back();

        /**
         * Loads the next document in the list of documents built by navigating links.
         * @since 4.6
         */
        void forward();

        /**
         * Reloads the current document.
         * @since 4.6
         */
        void reload();

        /**
         * Stops loading the document.
         * @since 4.6
         */
        void stop();

    Q_SIGNALS:
        /**
         * During loading progress, this signal is emitted. The values
         * are always between 0 and 100, inclusive.
         *
         * @param percent the estimated amount the loading is complete
         */
        void loadProgress(int percent);

        /**
         * This signal is emitted when loading is completed.
         *
         * @param success true if the content was loaded successfully,
         *                otherwise false
         */
        void loadFinished(bool success);

        /**
         * url displayed by the web page changed
         * @since 4.6
         */
        void urlChanged(const QUrl &url);

    protected:
        /**
         * Reimplementation
         */
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
        void wheelEvent(QGraphicsSceneWheelEvent *event);
        void keyPressEvent(QKeyEvent * event);
        void keyReleaseEvent(QKeyEvent * event);
        void focusInEvent(QFocusEvent * event);
        void focusOutEvent(QFocusEvent * event);
        void dragEnterEvent(QGraphicsSceneDragDropEvent * event);
        void dragLeaveEvent(QGraphicsSceneDragDropEvent * event);
        void dragMoveEvent(QGraphicsSceneDragDropEvent * event);
        void dropEvent(QGraphicsSceneDragDropEvent * event);
        QVariant itemChange(GraphicsItemChange change, const QVariant &value);
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;

    private:
        Q_PRIVATE_SLOT(d, void loadingFinished(bool success))

        WebViewPrivate * const d;
        friend class WebViewPrivate;
};

} // namespace Plasma

#endif // Multiple incluson guard

