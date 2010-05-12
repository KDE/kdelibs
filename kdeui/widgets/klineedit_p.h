/* This file is part of the KDE libraries

   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KLINEEDIT_P_H
#define KLINEEDIT_P_H

#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QPropertyAnimation>
#include <QIcon>

#include <kglobalsettings.h>

#include "kdeuiwidgetsproxystyle_p.h"

class KLineEditButton : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( int opacity READ opacity WRITE setOpacity )

public:
    KLineEditButton(QWidget *parent)
        : QWidget(parent),
          m_opacity(0)
    {
        m_animation = new QPropertyAnimation(this, "opacity", this);
        m_animation->setStartValue(0);
        m_animation->setEndValue(255);
        m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    }

    QSize sizeHint () const
    {
        return m_pixmap.size();
    }

    void animateVisible(bool visible)
    {
        if (visible) {
            if (m_animation->direction() == QPropertyAnimation::Forward) {
                return;
            }

            m_animation->setDirection(QPropertyAnimation::Forward);
            m_animation->setDuration(150);
            show();
        } else {
            if (m_animation->direction() == QPropertyAnimation::Backward) {
                return;
            }

            m_animation->setDirection(QPropertyAnimation::Backward);
            m_animation->setDuration(250);
        }

        if (KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects) {
            if (m_animation->state() != QPropertyAnimation::Running)
                m_animation->start();
        } else {
            setVisible(m_animation->direction() == QPropertyAnimation::Forward);
        }
    }

    void setPixmap(const QPixmap& p)
    {
        m_pixmap = p;
        m_icon = QIcon(p);
    }

    QPixmap pixmap() const
    {
        return m_pixmap;
    }

    void setAnimationsEnabled(bool animationsEnabled)
    {
        // We need to set the current time in the case that we had the clear
        // button shown, for it being painted on the paintEvent(). Otherwise
        // it wont be painted, resulting (m_opacity == 0) true,
        // and therefore a bad painting. This is needed for the case that we
        // come from a non animated widget and want it animated. (ereslibre)
        if (animationsEnabled && m_animation->direction() == QPropertyAnimation::Forward) {
            m_animation->setCurrentTime(150);
        }
    }

    int opacity() const
    {
        return m_opacity;
    }

    void setOpacity( int value )
    {
        m_opacity = value;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event)

        // check pixmap
        if (m_pixmap.isNull()) {
            return;
        }

        if (KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects) {

            if (m_opacity == 0) {
                if (m_animation->direction() == QPropertyAnimation::Backward)
                    hide();
                return;
            }

            if (m_opacity < 255) {
                // fade pixmap
                QPixmap pm(m_pixmap);
                QColor color(Qt::black);
                color.setAlpha(m_opacity);
                QPainter p(&pm);
                p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                p.fillRect(pm.rect(), color);
                p.end();

                // draw pixmap on widget
                QPainter pp(this);
                pp.drawPixmap((width() - pm.width()) / 2,
                             (height() - pm.height()) / 2,
                             pm);
            } else {
                QPainter p(this);
                p.drawPixmap((width() - m_pixmap.width()) / 2,
                             (height() - m_pixmap.height()) / 2,
                             m_pixmap);
            }
        } else {
            QPainter p(this);
            p.drawPixmap((width() - m_pixmap.width()) / 2,
                        (height() - m_pixmap.height()) / 2,
                        m_pixmap);
        }
    }

protected:
    virtual bool event( QEvent* event )
    {
        if (event->type() == QEvent::EnabledChange) {
            m_pixmap = m_icon.pixmap(m_pixmap.size(), isEnabled() ? QIcon::Normal : QIcon::Disabled);
        }
        return QWidget::event( event );
    }

private:
    QPropertyAnimation* m_animation;
    int m_opacity;
    QPixmap m_pixmap;
    QIcon m_icon;
};

class KLineEditStyle : public KdeUiProxyStyle
{
    Q_OBJECT
public:
  KLineEditStyle(KLineEdit *parent)
    : KdeUiProxyStyle(parent),
      m_overlap(0),
      m_sentinel(false)
  {
  }

  QStyle *style() const;
  QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const;

  int m_overlap;
  QWeakPointer<QStyle> m_subStyle;
  QString m_lastStyleClass;
  bool m_sentinel;
};

#endif

