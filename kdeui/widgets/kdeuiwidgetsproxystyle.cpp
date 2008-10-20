/*  This file is part of the KDE libraries

    Copyright (C) 2007 Fredrik Höglund <fredrik@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License (LGPL) as published by the Free Software Foundation;
    either version 2 of the License, or (at your option) any later
    version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdeuiwidgetsproxystyle_p.h"

#include <QtGui/QApplication>
#include <QtGui/QWidget>
#include <QtGui/QWindowsStyle>

KdeUiProxyStyle::KdeUiProxyStyle(QWidget *parent)
    : QStyle(), parent(parent)
{
}

KdeUiProxyStyle::~KdeUiProxyStyle()
{
}

QStyle *KdeUiProxyStyle::style() const
{
    QStyle* baseStyle;
    if (parent && parent->parentWidget()) {
        baseStyle = parent->parentWidget()->style();
    } else {
        baseStyle = QApplication::style();
    }
    return baseStyle;
}

void KdeUiProxyStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                        QPainter *painter, const QWidget *widget) const
{
    style()->drawComplexControl(control, option, painter, widget);
}

void KdeUiProxyStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
                                 const QWidget *widget) const
{
    style()->drawControl(element, option, painter, widget);
}

void KdeUiProxyStyle::drawItemPixmap(QPainter *painter, const QRect &rectangle, int alignment,
                                    const QPixmap &pixmap) const
{
    style()->drawItemPixmap(painter, rectangle, alignment, pixmap);
}

void KdeUiProxyStyle::drawItemText(QPainter *painter, const QRect &rectangle, int alignment, const QPalette &palette,
                                  bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    style()->drawItemText(painter, rectangle, alignment, palette, enabled, text, textRole);
}

void KdeUiProxyStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
                                   const QWidget *widget) const
{
    style()->drawPrimitive(element, option, painter, widget);
}

QPixmap KdeUiProxyStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap,
                                            const QStyleOption *option) const
{
    return style()->generatedIconPixmap(iconMode, pixmap, option);
}

QStyle::SubControl KdeUiProxyStyle::hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                                         const QPoint &position, const QWidget *widget) const
{
    return style()->hitTestComplexControl(control, option, position, widget);
}

QRect KdeUiProxyStyle::itemPixmapRect(const QRect &rectangle, int alignment, const QPixmap &pixmap) const
{
    return style()->itemPixmapRect(rectangle, alignment, pixmap);
}

QRect KdeUiProxyStyle::itemTextRect(const QFontMetrics &metrics, const QRect &rectangle, int alignment,
                                   bool enabled, const QString &text) const
{
    return style()->itemTextRect(metrics, rectangle, alignment, enabled, text);
}

int KdeUiProxyStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    return style()->pixelMetric(metric, option, widget);
}

void KdeUiProxyStyle::polish(QWidget *widget)
{
    style()->polish(widget);
}

void KdeUiProxyStyle::polish(QApplication *application)
{
    style()->polish(application);
}

void KdeUiProxyStyle::polish(QPalette &palette)
{
    style()->polish(palette);
}

QSize KdeUiProxyStyle::sizeFromContents(ContentsType type, const QStyleOption *option,
                                       const QSize &contentsSize, const QWidget *widget) const
{
    return style()->sizeFromContents(type, option, contentsSize, widget);
}

QIcon KdeUiProxyStyle::standardIcon(StandardPixmap standardIcon, const QStyleOption *option,
                                   const QWidget *widget) const
{
    return style()->standardIcon(standardIcon, option, widget);
}

QPixmap KdeUiProxyStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option,
                                       const QWidget *widget) const
{
    return style()->standardPixmap(standardPixmap, option, widget);
}

QPalette KdeUiProxyStyle::standardPalette() const
{
    return style()->standardPalette();
}

int KdeUiProxyStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget,
                              QStyleHintReturn *returnData) const
{
    return style()->styleHint(hint, option, widget, returnData);
}

QRect KdeUiProxyStyle::subControlRect(ComplexControl control, const QStyleOptionComplex *option,
                                     SubControl subControl, const QWidget *widget) const
{
    return style()->subControlRect(control, option, subControl, widget);
}

QRect KdeUiProxyStyle::subElementRect(SubElement element, const QStyleOption *option,
                                     const QWidget *widget) const
{
    return style()->subElementRect(element, option, widget);
}

void KdeUiProxyStyle::unpolish(QWidget *widget)
{
    style()->unpolish(widget);
}

void KdeUiProxyStyle::unpolish(QApplication *application)
{
    style()->unpolish(application);
}

