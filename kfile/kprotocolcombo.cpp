/***************************************************************************
 *   Copyright (C) 2006 by Aaron J. Seigo (<aseigo@kde.org>)               *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/

#include "kprotocolcombo_p.h"

#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QStyleOption>

#include <kdebug.h>
#include <kprotocolinfo.h>
#include <kprotocolmanager.h>
#include <kurlnavigator.h>

KProtocolCombo::KProtocolCombo(const QString& protocol, KUrlNavigator* parent)
    : KUrlButton(parent),
      m_protocols()
{
    m_menu = new QMenu(this);
    connect(m_menu, SIGNAL(triggered(QAction*)), this, SLOT(setProtocol(QAction*)));
    setText(protocol);
    setMenu(m_menu);
}

void KProtocolCombo::setCustomProtocols(const QStringList &protocols)
{
    m_protocols = protocols;
    updateMenu();
}

QSize KProtocolCombo::sizeHint() const
{
    QSize size = KUrlButton::sizeHint();

    QFontMetrics fontMetrics(font());
    int width = fontMetrics.width(text());
    width += (3 * BorderWidth) + ArrowSize;

    return QSize(width, size.height());
}

void KProtocolCombo::setProtocol(const QString& protocol)
{
    setText(protocol);
}

QString KProtocolCombo::currentProtocol() const
{
    return text();
}

bool KProtocolCombo::event(QEvent* event)
{
    if ((event->type() == QEvent::Polish) && m_protocols.isEmpty()) {
        m_protocols = KProtocolInfo::protocols();
        qSort(m_protocols);

        QStringList::iterator it = m_protocols.begin();
        while (it != m_protocols.end()) {
            const KUrl url(*it + "://");
            if (!KProtocolManager::supportsListing(url)) {
                it = m_protocols.erase(it);
            } else {
                ++it;
            }
        }

        updateMenu();
    }

    return KUrlButton::event(event);
}

void KProtocolCombo::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    const int buttonWidth  = width();
    const int buttonHeight = height();

    drawHoverBackground(&painter);

    const QColor fgColor = foregroundColor();
    painter.setPen(fgColor);

    // draw arrow
    const int arrowX = buttonWidth - ArrowSize - BorderWidth;
    const int arrowY = (buttonHeight - ArrowSize) / 2;

    QStyleOption option;
    option.rect = QRect(arrowX, arrowY, ArrowSize, ArrowSize);
    option.palette = palette();
    option.palette.setColor(QPalette::Text, fgColor);
    option.palette.setColor(QPalette::WindowText, fgColor);
    option.palette.setColor(QPalette::ButtonText, fgColor);
    style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &option, &painter, this );

    // draw text
    const int textWidth = arrowX - (2 * BorderWidth);
    painter.drawText(QRect(BorderWidth, 0, textWidth, buttonHeight), Qt::AlignCenter, text());
}

void KProtocolCombo::setProtocol(QAction* action)
{
    const int index = action->data().toInt();
    Q_ASSERT((index >= 0) && (index < m_protocols.count()));
    const QString protocol = m_protocols[index];
    setText(protocol);
    emit activated(protocol);
}

void KProtocolCombo::updateMenu()
{
    m_menu->clear();

    int i = 0;
    foreach (const QString &protocol, m_protocols) {
        QAction *action = m_menu->addAction(protocol);
        action->setData(i++);
    }
}

#include "kprotocolcombo_p.moc"
