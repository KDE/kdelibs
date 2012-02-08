/*
 *   Copyright © 2008 Fredrik Höglund <fredrik@kde.org>
 *   Copyright © 2008 Marco Martin <notmart@gmail.com>
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

#include "scrollbar.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QGraphicsSceneContextMenuEvent>

#include <plasma/private/style_p.h>

namespace Plasma
{

class ScrollBarPrivate
{
public:
    Plasma::Style::Ptr style;
};

ScrollBar::ScrollBar(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new ScrollBarPrivate)
{
   QScrollBar *scrollbar = new QScrollBar();
   scrollbar->setWindowFlags(scrollbar->windowFlags()|Qt::BypassGraphicsProxyWidget);
   scrollbar->setAttribute(Qt::WA_NoSystemBackground);
   setWidget(scrollbar);
   scrollbar->setWindowIcon(QIcon());
   d->style = Plasma::Style::sharedStyle();
   scrollbar->setStyle(d->style.data());

   scrollbar->resize(scrollbar->sizeHint());
   connect(scrollbar, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)));
   connect(scrollbar, SIGNAL(sliderMoved(int)), this, SIGNAL(sliderMoved(int)));
}

ScrollBar::~ScrollBar()
{
    delete d;
    Plasma::Style::doneWithSharedStyle();
}

void ScrollBar::setRange(int min, int max)
{
   static_cast<QScrollBar*>(widget())->setRange(min, max);
}

void ScrollBar::setSingleStep(int val)
{
   static_cast<QScrollBar*>(widget())->setSingleStep(val);
}

int ScrollBar::singleStep()
{
   return static_cast<QScrollBar*>(widget())->singleStep();
}

void ScrollBar::setPageStep(int val)
{
   static_cast<QScrollBar*>(widget())->setPageStep(val);
}

int ScrollBar::pageStep()
{
   return static_cast<QScrollBar*>(widget())->pageStep();
}

void ScrollBar::setValue(int val)
{
   static_cast<QScrollBar*>(widget())->setValue(val);
}

int ScrollBar::value() const
{
   return static_cast<QScrollBar*>(widget())->value();
}

int ScrollBar::minimum() const
{
   return static_cast<QScrollBar*>(widget())->minimum();
}

int ScrollBar::maximum() const
{
   return static_cast<QScrollBar*>(widget())->maximum();
}

void ScrollBar::setMinimum(const int min) const
{
    static_cast<QScrollBar*>(widget())->setMinimum(min);
}

void ScrollBar::setMaximum(const int max) const
{
    static_cast<QScrollBar*>(widget())->setMaximum(max);
}

void ScrollBar::setStyleSheet(const QString &stylesheet)
{
   widget()->setStyleSheet(stylesheet);
}

QString ScrollBar::styleSheet()
{
   return widget()->styleSheet();
}

QScrollBar *ScrollBar::nativeWidget() const
{
   return static_cast<QScrollBar *>(widget());
}

Qt::Orientation ScrollBar::orientation() const
{
    return nativeWidget()->orientation();
}

void ScrollBar::setOrientation(Qt::Orientation orientation)
{
   QScrollBar *native = static_cast<QScrollBar *>(widget());
   native->setOrientation(orientation);
   resize(native->sizeHint());
}

void ScrollBar::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QContextMenuEvent contextMenuEvent(QContextMenuEvent::Reason(event->reason()),
                                       event->pos().toPoint(), event->screenPos(), event->modifiers());
    QApplication::sendEvent(nativeWidget(), &contextMenuEvent);
}

}



#include "moc_scrollbar.cpp"
