/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#ifndef PLASMA_FOCUSINDICATOR_P
#define PLASMA_FOCUSINDICATOR_P

#include <QGraphicsWidget>

#include <plasma/animations/animation.h>

namespace Plasma
{
    class FrameSvg;
    class FadeAnimation;

class FocusIndicator : public QGraphicsWidget
{
    Q_OBJECT
public:
    FocusIndicator(QGraphicsWidget *parent, const QString &widget);
    FocusIndicator(QGraphicsWidget *parent, FrameSvg *svg);
    ~FocusIndicator();

    void setCustomGeometry(const QRectF &geometry);
    void setCustomPrefix(const QString &prefix);

    void animateVisibility(const bool visible);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setFrameSvg(FrameSvg *svg);
    FrameSvg *frameSvg() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void resizeEvent(QGraphicsSceneResizeEvent *event);

private Q_SLOTS:
    void syncGeometry();

private:
    void init(QGraphicsWidget *parent); 
    QGraphicsWidget *m_parent;
    Plasma::FrameSvg *m_background;
    Animation *m_fade;
    Animation *m_hoverAnimation;
    QRectF m_customGeometry;
    QString m_prefix;
    QString m_customPrefix;
    QString m_testPrefix;
    bool m_isUnderMouse;
};
}
#endif
