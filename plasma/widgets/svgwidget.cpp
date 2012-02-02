/*
 *   Copyright 2008 by Davide Bettio <davide.bettio@kdemail.net>
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

#include "svgwidget.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>

#include "kdebug.h"
#include "svg.h"

namespace Plasma
{

class SvgWidgetPrivate
{
public:
    SvgWidgetPrivate(SvgWidget *widget, Svg *s, const QString &element)
        : q(widget), svg(s), elementID(element)
    {
    }

    void svgChanged()
    {
        q->update();
    }

    SvgWidget *q;
    Svg *svg;
    QString elementID;
};

SvgWidget::SvgWidget(QGraphicsItem *parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags),
      d(new SvgWidgetPrivate(this, 0, QString()))
{
}

SvgWidget::SvgWidget(Svg *svg, const QString &elementID, QGraphicsItem *parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags),
      d(new SvgWidgetPrivate(this, svg, elementID))
{
}

SvgWidget::~SvgWidget()
{
    delete d;
}

void SvgWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // we check for receivers so that SvgWidgets that aren't being used for events remain "click
    // transparent"
    if (receivers(SIGNAL(clicked(Qt::MouseButton)))) {
        event->accept();
    } else {
        QGraphicsWidget::mousePressEvent(event);
    }
}

void SvgWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (boundingRect().contains(event->pos())) {
        emit clicked(event->button());
    }
}

void SvgWidget::setSvg(Svg *svg)
{
    if (d->svg) {
        disconnect(d->svg);
    }

    d->svg = svg;

    if (svg) {
        connect(svg, SIGNAL(repaintNeeded()), this, SLOT(svgChanged()));
    }
    update();
}

Svg *SvgWidget::svg() const
{
    return d->svg;
}

void SvgWidget::setElementID(const QString &elementID)
{
    if (d->svg) {
        d->svg->setContainsMultipleImages(!elementID.isNull());
    }
    d->elementID = elementID;
    update();
}

QString SvgWidget::elementID() const
{
    return d->elementID;
}

void SvgWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (d->svg){
        d->svg->paint(painter, boundingRect(), d->elementID);
    }
}

QSizeF SvgWidget::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    if (d->svg && which == Qt::PreferredSize) {
        if (d->elementID.isNull()) {
            return d->svg->size();
        } else {
            return d->svg->elementSize(d->elementID);
        }
    } else {
        return QGraphicsWidget::sizeHint(which, constraint);
    }
}

} // Plasma namespace

