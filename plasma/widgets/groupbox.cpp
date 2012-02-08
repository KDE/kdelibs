/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include "groupbox.h"

#include <QGroupBox>
#include <QIcon>
#include <QPainter>

#include <kmimetype.h>

#include "svg.h"
#include "private/themedwidgetinterface_p.h"
#include "theme.h"

namespace Plasma
{

class GroupBoxPrivate : public ThemedWidgetInterface<GroupBox>
{
public:
    GroupBoxPrivate(GroupBox *groupBox)
      :ThemedWidgetInterface<GroupBox>(groupBox)
    {
    }

    ~GroupBoxPrivate()
    {
    }
};

GroupBox::GroupBox(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new GroupBoxPrivate(this))
{
    QGroupBox *native = new QGroupBox;
    d->setWidget(native);
    native->setWindowIcon(QIcon());
    native->setAttribute(Qt::WA_NoSystemBackground);
    d->initTheming();
}

GroupBox::~GroupBox()
{
    delete d;
}

void GroupBox::setText(const QString &text)
{
    static_cast<QGroupBox*>(widget())->setTitle(text);
}

QString GroupBox::text() const
{
    return static_cast<QGroupBox*>(widget())->title();
}

void GroupBox::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString GroupBox::styleSheet()
{
    return widget()->styleSheet();
}

QGroupBox *GroupBox::nativeWidget() const
{
    return static_cast<QGroupBox*>(widget());
}

void GroupBox::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    QGraphicsProxyWidget::resizeEvent(event);
}

void GroupBox::changeEvent(QEvent *event)
{
    d->changeEvent(event);
    QGraphicsProxyWidget::changeEvent(event);
}

} // namespace Plasma




#include "moc_groupbox.cpp"
