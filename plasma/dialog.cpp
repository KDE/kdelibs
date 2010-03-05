/*
 * Copyright 2008 by Alessandro Diaferia <alediaferia@gmail.com>
 * Copyright 2007 by Alexis Ménard <darktears31@gmail.com>
 * Copyright 2007 Sebastian Kuegler <sebas@kde.org>
 * Copyright 2006 Aaron Seigo <aseigo@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "dialog.h"

#include <QPainter>
#include <QSvgRenderer>
#include <QResizeEvent>
#include <QMouseEvent>
#ifdef Q_WS_X11
#include <QX11Info>
#endif
#include <QBitmap>
#include <QTimer>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGraphicsSceneEvent>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsWidget>
#include <QApplication>
#include <QDesktopWidget>
#include <QVarLengthArray>

#include <kdebug.h>
#include <kwindowsystem.h>
#include <netwm.h>

#include "plasma/applet.h"
#include "plasma/animator.h"
#include "plasma/extenders/extender.h"
#include "plasma/private/extender_p.h"
#include "plasma/framesvg.h"
#include "plasma/theme.h"
#include "plasma/windoweffects.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

namespace Plasma
{

class DialogPrivate
{
public:
    DialogPrivate(Dialog *dialog)
            : q(dialog),
              background(0),
              view(0),
              resizeCorners(Dialog::NoCorner),
              resizeStartCorner(Dialog::NoCorner),
              moveTimer(0),
              aspectRatioMode(Plasma::IgnoreAspectRatio)
    {
    }

    ~DialogPrivate()
    {
    }

    void themeChanged();
    void adjustView();
    void updateResizeCorners();
    int calculateWidthForHeightAndRatio(int height, qreal ratio);

    Plasma::Dialog *q;

    /**
     * Holds the background SVG, to be re-rendered when the cache is invalidated,
     * for example by resizing the dialogue.
     */
    Plasma::FrameSvg *background;
    QGraphicsView *view;
    QWeakPointer<QGraphicsWidget> graphicsWidgetPtr;
    Dialog::ResizeCorners resizeCorners;
    QMap<Dialog::ResizeCorner, QRect> resizeAreas;
    int resizeStartCorner;
    QTimer *moveTimer;
    QTimer *adjustViewTimer;
    Plasma::AspectRatioMode aspectRatioMode;
};

void DialogPrivate::themeChanged()
{
    QGraphicsWidget *graphicsWidget = graphicsWidgetPtr.data();

    qreal topHeight;
    qreal leftWidth;
    qreal rightWidth;
    qreal bottomHeight;

    //kDebug() << leftWidth << topHeight << rightWidth << bottomHeight;
    if (Plasma::Theme::defaultTheme()->windowTranslucencyEnabled()) {
        q->clearMask();
    } else {
        q->setMask(background->mask());
    }

    FrameSvg::EnabledBorders borders = FrameSvg::AllBorders;

    Extender *extender = qobject_cast<Extender*>(graphicsWidget);
    Plasma::Applet *applet = 0;
    if (extender) {
        applet = extender->d->applet;
    } else if (graphicsWidget) {
        QObject *pw = graphicsWidget;

        while ((pw = pw->parent())) {
            applet = dynamic_cast<Plasma::Applet *>(pw);
            if (applet) {
                break;
            }
        }
    }
    if (applet) {
        background->getMargins(leftWidth, topHeight, rightWidth, bottomHeight);

        switch (applet->location()) {
        case BottomEdge:
            borders &= ~FrameSvg::BottomBorder;
            leftWidth = 0;
            rightWidth = 0;
            bottomHeight = 0;
        break;

        case TopEdge:
            borders &= ~FrameSvg::TopBorder;
            topHeight = 0;
            leftWidth = 0;
            rightWidth = 0;
        break;

        case LeftEdge:
            borders &= ~FrameSvg::LeftBorder;
            leftWidth = 0;
            rightWidth = 0;
        break;

        case RightEdge:
            borders &= ~FrameSvg::RightBorder;
            leftWidth = 0;
            rightWidth = 0;
        break;

        default:
        break;
        }
    }

    if (!extender && q->isVisible()) {
        //remove borders at the edge of the desktop
        QDesktopWidget *desktop = QApplication::desktop();
        QRect avail = desktop->availableGeometry(desktop->screenNumber(q));
        QRect dialogGeom = q->geometry();

        if (dialogGeom.left() <= avail.left()) {
            borders &= ~FrameSvg::LeftBorder;
        }
        if (dialogGeom.top() <= avail.top()) {
            borders &= ~FrameSvg::TopBorder;
        }
        //FIXME: that 2 pixels offset has probably something to do with kwin
        if (dialogGeom.right() + 2 > avail.right()) {
            borders &= ~FrameSvg::RightBorder;
        }
        if (dialogGeom.bottom() + 2 > avail.bottom()) {
            borders &= ~FrameSvg::BottomBorder;
        }
    }

    background->setEnabledBorders(borders);

    if (!extender) {
        background->getMargins(leftWidth, topHeight, rightWidth, bottomHeight);
    }

    q->setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);
    q->update();
}

void DialogPrivate::adjustView()
{
    QGraphicsWidget *graphicsWidget = graphicsWidgetPtr.data();
    if (view && graphicsWidget) {
        const int prevStartCorner = resizeStartCorner;
        resizeStartCorner = -1;
        QSize prevSize = q->size();
        /*
        kDebug() << "Widget size:" << graphicsWidget->size()
                 << "| Widget size hint:" << graphicsWidget->effectiveSizeHint(Qt::PreferredSize)
                 << "| Widget minsize hint:" << graphicsWidget->minimumSize()
                 << "| Widget maxsize hint:" << graphicsWidget->maximumSize()
                 << "| Widget bounding rect:" << graphicsWidget->sceneBoundingRect();
        */
        //set the sizehints correctly:
        int left, top, right, bottom;
        q->getContentsMargins(&left, &top, &right, &bottom);

        QDesktopWidget *desktop = QApplication::desktop();
        QSize maxSize = desktop->availableGeometry(desktop->screenNumber(q)).size();


        q->setMinimumSize(qMin(int(graphicsWidget->minimumSize().width()) + left + right, maxSize.width()),
                          qMin(int(graphicsWidget->minimumSize().height()) + top + bottom, maxSize.height()));
        q->setMaximumSize(qMin(int(graphicsWidget->maximumSize().width()) + left + right, maxSize.width()),
                          qMin(int(graphicsWidget->maximumSize().height()) + top + bottom, maxSize.height()));
        q->resize(qMin(int(graphicsWidget->size().width()) + left + right, maxSize.width()),
                  qMin(int(graphicsWidget->size().height()) + top + bottom, maxSize.height()));
        q->updateGeometry();

        //reposition and resize the view.
        //force a valid rect, otherwise it will take up the whole scene
        QRectF sceneRect(graphicsWidget->sceneBoundingRect());

        sceneRect.setWidth(qMax(qreal(1), sceneRect.width()));
        sceneRect.setHeight(qMax(qreal(1), sceneRect.height()));
        view->setSceneRect(sceneRect);

        view->resize(graphicsWidget->size().toSize());
        view->centerOn(graphicsWidget);

        //if the view resized and a border is disabled move the dialog to make sure it will still look attached to panel/screen edge
        qreal topHeight;
        qreal leftWidth;
        qreal rightWidth;
        qreal bottomHeight;

        background->getMargins(leftWidth, topHeight, rightWidth, bottomHeight);

        if (rightWidth == 0) {
            q->move(q->pos().x() + (prevSize.width() - q->size().width()), q->pos().y());
        }
        if (bottomHeight == 0) {
            q->move(q->pos().x(), q->pos().y() + (prevSize.height() - q->size().height()));
        }

        if (q->size() != prevSize) {
            //the size of the dialog has changed, emit the signal:
            emit q->dialogResized();
        }
        resizeStartCorner = prevStartCorner;
    }
}

int DialogPrivate::calculateWidthForHeightAndRatio(int height, qreal ratio)
{
    switch (aspectRatioMode) {
        case KeepAspectRatio:
            return qRound(height * ratio);
            break;
        case Square:
            return height;
            break;
        case ConstrainedSquare:
            return height;
            break;
        default:
            return -1;
    }
}

Dialog::Dialog(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f),
      d(new DialogPrivate(this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);
    d->background = new FrameSvg(this);
    d->background->setImagePath("dialogs/background");
    d->background->setEnabledBorders(FrameSvg::AllBorders);
    d->background->resizeFrame(size());

    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::transparent);
    setPalette(pal);
    WindowEffects::overrideShadow(winId(), true);

    d->adjustViewTimer = new QTimer(this);
    d->adjustViewTimer->setSingleShot(true);
    connect(d->adjustViewTimer, SIGNAL(timeout()), this, SLOT(adjustView()));

    connect(d->background, SIGNAL(repaintNeeded()), this, SLOT(update()));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
    d->themeChanged();

    setMouseTracking(true);
}

Dialog::~Dialog()
{
    delete d;
}

void Dialog::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    d->background->paintFrame(&p, e->rect(), e->rect());
}

void Dialog::mouseMoveEvent(QMouseEvent *event)
{
    if (d->resizeAreas[Dialog::NorthEast].contains(event->pos())) {
        setCursor(Qt::SizeBDiagCursor);
    } else if (d->resizeAreas[Dialog::NorthWest].contains(event->pos())) {
        setCursor(Qt::SizeFDiagCursor);
    } else if (d->resizeAreas[Dialog::SouthEast].contains(event->pos())) {
        setCursor(Qt::SizeFDiagCursor);
    } else if (d->resizeAreas[Dialog::SouthWest].contains(event->pos())) {
        setCursor(Qt::SizeBDiagCursor);
    } else if (!(event->buttons() & Qt::LeftButton)) {
        unsetCursor();
    }

    // here we take care of resize..
    if (d->resizeStartCorner != Dialog::NoCorner) {
        int newWidth;
        int newHeight;
        QPoint position;

        qreal aspectRatio = (qreal)width() / (qreal)height();

        switch(d->resizeStartCorner) {
            case Dialog::NorthEast:
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), height() - event->y()));
                newWidth = d->calculateWidthForHeightAndRatio(newHeight, aspectRatio);
                if (newWidth == -1) {
                    newWidth = qMin(maximumWidth(), qMax(minimumWidth(), event->x()));
                }
                position = QPoint(x(), y() + height() - newHeight);
                break;
            case Dialog::NorthWest:
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), height() - event->y()));
                newWidth = d->calculateWidthForHeightAndRatio(newHeight, aspectRatio);
                if (newWidth == -1) {
                    newWidth = qMin(maximumWidth(), qMax(minimumWidth(), width() - event->x()));
                }
                position = QPoint(x() + width() - newWidth, y() + height() - newHeight);
                break;
            case Dialog::SouthWest:
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), event->y()));
                newWidth = d->calculateWidthForHeightAndRatio(newHeight, aspectRatio);
                if (newWidth == -1) {
                    newWidth = qMin(maximumWidth(), qMax(minimumWidth(), width() - event->x()));
                }
                position = QPoint(x() + width() - newWidth, y());
                break;
            case Dialog::SouthEast:
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), event->y()));
                newWidth = d->calculateWidthForHeightAndRatio(newHeight, aspectRatio);
                if (newWidth == -1) {
                    newWidth = qMin(maximumWidth(), qMax(minimumWidth(), event->x()));
                }
                position = QPoint(x(), y());
                break;
             default:
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), height()));
                newWidth = d->calculateWidthForHeightAndRatio(newHeight, aspectRatio);
                if (newWidth == -1) {
                    newWidth = qMin(maximumWidth(), qMax(minimumWidth(), width()));
                }
                position = QPoint(x(), y());
                break;
        }

        if ((newWidth >= minimumSize().width()) && (newHeight >= minimumSize().height())) {
            setGeometry(QRect(position, QSize(newWidth, newHeight)));
        }
    }

    QWidget::mouseMoveEvent(event);
}

void Dialog::mousePressEvent(QMouseEvent *event)
{
    if (d->resizeAreas[Dialog::NorthEast].contains(event->pos())) {
        d->resizeStartCorner = Dialog::NorthEast;
    } else if (d->resizeAreas[Dialog::NorthWest].contains(event->pos())) {
        d->resizeStartCorner = Dialog::NorthWest;
    } else if (d->resizeAreas[Dialog::SouthEast].contains(event->pos())) {
        d->resizeStartCorner = Dialog::SouthEast;
    } else if (d->resizeAreas[Dialog::SouthWest].contains(event->pos())) {
        d->resizeStartCorner = Dialog::SouthWest;
    } else {
        d->resizeStartCorner = Dialog::NoCorner;
    }

    QWidget::mousePressEvent(event);
}

void Dialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (d->resizeStartCorner != Dialog::NoCorner) {
        d->resizeStartCorner = Dialog::NoCorner;
        unsetCursor();
        emit dialogResized();
    }

    QWidget::mouseReleaseEvent(event);
}

void Dialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
    }
}

bool Dialog::event(QEvent *event)
{
    if (event->type() == QEvent::Paint) {
        QPainter p(this);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(rect(), Qt::transparent);
    }

    return QWidget::event(event);
}

void Dialog::resizeEvent(QResizeEvent *e)
{
    d->background->resizeFrame(e->size());

    if (Plasma::Theme::defaultTheme()->windowTranslucencyEnabled()) {
        clearMask();
    } else {
        setMask(d->background->mask());
    }

    if (d->resizeStartCorner != -1 && d->view && d->graphicsWidgetPtr) {
        QGraphicsWidget *graphicsWidget = d->graphicsWidgetPtr.data();
        graphicsWidget->resize(d->view->size());

        QRectF sceneRect(graphicsWidget->sceneBoundingRect());
        sceneRect.setWidth(qMax(qreal(1), sceneRect.width()));
        sceneRect.setHeight(qMax(qreal(1), sceneRect.height()));
        d->view->setSceneRect(sceneRect);
        d->view->centerOn(graphicsWidget);
    }

    d->updateResizeCorners();
}

void DialogPrivate::updateResizeCorners()
{
    const int resizeAreaMargin = 20;
    const QRect r = q->rect();
    const FrameSvg::EnabledBorders borders = background->enabledBorders();

    // IMPLEMENTATION NOTE: we set resize corners for the corners set, but also
    // for the complimentary corners if we've cut out an edge of our SVG background
    // which implies we are up against an immovable edge (e.g. a screen edge)

    resizeAreas.clear();
    if (resizeCorners & Dialog::NorthEast ||
        (resizeCorners & Dialog::NorthWest && !(borders & FrameSvg::LeftBorder)) ||
        (resizeCorners & Dialog::SouthEast && !(borders & FrameSvg::BottomBorder))) {
        resizeAreas[Dialog::NorthEast] = QRect(r.right() - resizeAreaMargin, 0,
                                               resizeAreaMargin, resizeAreaMargin);
    }

    if (resizeCorners & Dialog::NorthWest ||
        (resizeCorners & Dialog::NorthEast && !(borders & FrameSvg::RightBorder)) ||
        (resizeCorners & Dialog::SouthWest && !(borders & FrameSvg::BottomBorder))) {
        resizeAreas[Dialog::NorthWest] = QRect(0, 0, resizeAreaMargin, resizeAreaMargin);
    }

    if (resizeCorners & Dialog::SouthEast ||
        (resizeCorners & Dialog::SouthWest && !(borders & FrameSvg::LeftBorder)) ||
        (resizeCorners & Dialog::NorthEast && !(borders & FrameSvg::TopBorder))) {
        resizeAreas[Dialog::SouthEast] = QRect(r.right() - resizeAreaMargin,
                                               r.bottom() - resizeAreaMargin,
                                               resizeAreaMargin, resizeAreaMargin);
    }

    if (resizeCorners & Dialog::SouthWest ||
        (resizeCorners & Dialog::SouthEast && !(borders & FrameSvg::RightBorder)) ||
        (resizeCorners & Dialog::NorthWest && !(borders & FrameSvg::TopBorder))) {
        resizeAreas[Dialog::SouthWest] = QRect(0, r.bottom() - resizeAreaMargin,
                                               resizeAreaMargin, resizeAreaMargin);
    }

}

void Dialog::setGraphicsWidget(QGraphicsWidget *widget)
{
    if (d->graphicsWidgetPtr) {
        d->graphicsWidgetPtr.data()->removeEventFilter(this);
    }

    d->graphicsWidgetPtr = widget;

    if (widget) {
        if (!layout()) {
            QVBoxLayout *lay = new QVBoxLayout(this);
            lay->setMargin(0);
            lay->setSpacing(0);
        }

        d->themeChanged();

        if (!d->view) {
            d->view = new QGraphicsView(this);
            d->view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            d->view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            d->view->setFrameShape(QFrame::NoFrame);
            d->view->viewport()->setAutoFillBackground(false);
            layout()->addWidget(d->view);
        }

        d->view->setScene(widget->scene());
        d->adjustView();

        adjustSize();

        widget->installEventFilter(this);
    } else {
        delete d->view;
        d->view = 0;
    }
}

//BIC FIXME: should be const
QGraphicsWidget *Dialog::graphicsWidget()
{
    return d->graphicsWidgetPtr.data();
}

bool Dialog::eventFilter(QObject *watched, QEvent *event)
{
    if (d->resizeStartCorner == Dialog::NoCorner && watched == d->graphicsWidgetPtr.data() &&
        (event->type() == QEvent::GraphicsSceneResize || event->type() == QEvent::GraphicsSceneMove)) {
        d->adjustViewTimer->start(150);
    }

    return QWidget::eventFilter(watched, event);
}

void Dialog::hideEvent(QHideEvent * event)
{
    Q_UNUSED(event);
    emit dialogVisible(false);
}

void Dialog::showEvent(QShowEvent * event)
{
    Q_UNUSED(event);

    //check if the widget size is still synced with the view
    d->themeChanged();
    d->updateResizeCorners();

    QGraphicsWidget *graphicsWidget = d->graphicsWidgetPtr.data();
    if (graphicsWidget && d->view && graphicsWidget->size().toSize() != d->view->size()) {
        d->adjustView();
    }

    if (d->view) {
        d->view->setFocus();
    }

    if (graphicsWidget) {
        graphicsWidget->setFocus();
    }

    emit dialogVisible(true);
    WindowEffects::overrideShadow(winId(), true);
}

void Dialog::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    if (d->view) {
        d->view->setFocus();
    }

    QGraphicsWidget *graphicsWidget = d->graphicsWidgetPtr.data();
    if (graphicsWidget) {
        graphicsWidget->setFocus();
    }
}

void Dialog::moveEvent(QMoveEvent *event)
{
    Q_UNUSED(event)

    if (!d->moveTimer) {
        d->moveTimer = new QTimer(this);
        d->moveTimer->setSingleShot(true);
        connect(d->moveTimer, SIGNAL(timeout()), this, SLOT(themeChanged()));
    }

    d->moveTimer->start(200);
}

void Dialog::setResizeHandleCorners(ResizeCorners corners)
{
    if ((d->resizeCorners != corners) && (aspectRatioMode() != FixedSize)) {
        d->resizeCorners = corners;
        d->updateResizeCorners();
    }
}

Dialog::ResizeCorners Dialog::resizeCorners() const
{
    return d->resizeCorners;
}

void Dialog::animatedHide(Plasma::Direction direction)
{
    if (!KWindowSystem::compositingActive()) {
        hide();
        return;
    }

    Location location = Desktop;
    switch (direction) {
    case Down:
        location = BottomEdge;
        break;
    case Right:
        location = RightEdge;
        break;
    case Left:
        location = LeftEdge;
        break;
    case Up:
        location = TopEdge;
        break;
    default:
        break;
    }

    Plasma::WindowEffects::slideWindow(this, location);
    hide();
}

void Dialog::animatedShow(Plasma::Direction direction)
{
    if (!KWindowSystem::compositingActive()) {
        show();
        return;
    }

    //copied to not add new api
    Location location = Desktop;
    switch (direction) {
    case Up:
        location = BottomEdge;
        break;
    case Left:
        location = RightEdge;
        break;
    case Right:
        location = LeftEdge;
        break;
    case Down:
        location = TopEdge;
        break;
    default:
        break;
    }

    if (KWindowSystem::compositingActive()) {
        Plasma::WindowEffects::slideWindow(this, location);
    }

    show();
}

bool Dialog::inControlArea(const QPoint &point)
{
    foreach (const QRect &r, d->resizeAreas) {
        if (r.contains(point)) {
            return true;
        }
    }
    return false;
}

Plasma::AspectRatioMode Dialog::aspectRatioMode() const
{
    return d->aspectRatioMode;
}

void Dialog::setAspectRatioMode(Plasma::AspectRatioMode mode)
{
    if (mode == FixedSize) {
        setResizeHandleCorners(NoCorner);
    }

    d->aspectRatioMode = mode;
}

}
#include "dialog.moc"
