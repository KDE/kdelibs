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
#include <QTimeLine>
#include <QIcon>

#include <kglobalsettings.h>

#include "kdeuiwidgetsproxystyle_p.h"

class KLineEditButton : public QWidget
{
    Q_OBJECT

public:
    KLineEditButton(QWidget *parent)
        : QWidget(parent)
    {
        m_timeline = new QTimeLine(200, this);
        m_timeline->setFrameRange(0, 255);
        m_timeline->setCurveShape(QTimeLine::EaseInOutCurve);
        m_timeline->setDirection(QTimeLine::Backward);
        connect(m_timeline, SIGNAL(finished()), this, SLOT(animationFinished()));
        connect(m_timeline, SIGNAL(frameChanged(int)), this, SLOT(update()));
    }

    QSize sizeHint () const
    {
        return m_pixmap.size();
    }

    void animateVisible(bool visible)
    {
        if (visible) {
            if (m_timeline->direction() == QTimeLine::Forward) {
                return;
            }

            m_timeline->setDirection(QTimeLine::Forward);
            m_timeline->setDuration(150);
            show();
        } else {
            if (m_timeline->direction() == QTimeLine::Backward) {
                return;
            }

            m_timeline->setDirection(QTimeLine::Backward);
            m_timeline->setDuration(250);
        }

        if (KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects) {
            if (m_timeline->state() != QTimeLine::Running)
                m_timeline->start();
        } else {
            setVisible(m_timeline->direction() == QTimeLine::Forward);
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
        // it wont be painted, resulting (m->timeLine->currentTime() == 0) true,
        // and therefore a bad painting. This is needed for the case that we
        // come from a non animated widget and want it animated. (ereslibre)
        if (animationsEnabled && m_timeline->direction() == QTimeLine::Forward) {
            m_timeline->setCurrentTime(150);
        }
    }

protected:
    void paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event)

        if (KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects) {
            if (m_pixmap.isNull() || m_timeline->currentTime() == 0) {
                return;
            }

            int opacity(m_timeline->currentFrame());
            if (opacity > 0 && opacity < 255) {
                // fade pixmap
                QPixmap pm(m_pixmap);
                QColor color(Qt::black);
                color.setAlpha(opacity);
                QPainter p(&pm);
                p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                p.fillRect(pm.rect(), color);
                p.end();

                // draw pixmap on widget
                QPainter pp(this);
                pp.drawPixmap((width() - pm.width()) / 2,
                             (height() - pm.height()) / 2,
                             pm);
            } else if (opacity > 0) {
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

protected slots:
    void animationFinished()
    {
        if (m_timeline->direction() == QTimeLine::Forward) {
            update();
        } else {
            hide();
        }
    }

private:
    QTimeLine *m_timeline;
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

  QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const;

  int m_overlap;
  QWeakPointer<QStyle> m_subStyle;
  bool m_sentinel;
};

#endif

