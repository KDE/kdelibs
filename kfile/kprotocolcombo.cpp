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

#include <kdebug.h>
#include <kprotocolinfo.h>
#include <kprotocolmanager.h>
#include <kurlnavigator.h>

//const static int customProtocolIndex = 0;

KProtocolCombo::KProtocolCombo(const QString& protocol, KUrlNavigator* parent)
    : KUrlButton(parent),
      m_protocols(KProtocolInfo::protocols())
{
    qSort(m_protocols);
    QStringList::iterator it = m_protocols.begin();
    QStringList::iterator itEnd = m_protocols.end();
    QMenu* menu = new QMenu(this);
    while (it != itEnd)
    {
        //kDebug() << "info for " << *it << " "
        //          << KProtocolInfo::protocolClass(*it) << endl;
        //TODO: wow this is ugly. or .. is it? ;) we need a way to determine
        //      if a protocol is appropriate for use in a file manager. hum!
        //if (KProtocolInfo::capabilities(*it).findIndex("filemanager") == -1)

        // DF: why not just supportsListing?

        if (KProtocolInfo::protocolClass(*it) == ":" /* ||
            !KProtocolManager::supportsWriting(*it)*/)
        {
        //kDebug() << "!!! removing " << *it << endl;
            QStringList::iterator tempIt = it;
            ++tempIt;
            m_protocols.erase(it);
            it = tempIt;
        }
        else
        {
            ++it;
        }
    }

//    setEditable(true);
//    menu->insertItem("", customProtocolIndex);
//    menu->insertStringList(m_protocols);
    int i = 0;
    for (QStringList::const_iterator it = m_protocols.constBegin();
         it != m_protocols.constEnd();
         ++it, ++i)
    {
        QAction* action = menu->addAction(*it);
        action->setData(i);
    }

    connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(setProtocol(QAction*)));
    setText(protocol);
    setMenu(menu);
}

QSize KProtocolCombo::sizeHint() const
{
    QSize size = KUrlButton::sizeHint();

    QFontMetrics fontMetrics(font());
    int width = fontMetrics.width(text());
    width += (3 * BorderWidth) + (2 * ArrowHeight);

    return QSize(width, size.height());
}

void KProtocolCombo::setProtocol(const QString& protocol)
{
    setText(protocol);
//    if (KProtocolInfo::isKnownProtocol(protocol))
//     int index = m_protocols.findIndex(protocol);
//     if (index == -1)
//     {
//         changeItem(protocol, customProtocolIndex);
//         setCurrentItem(customProtocolIndex);
//     }
//     else
//     {
//         setCurrentItem(index + 1);
//     }
}

QString KProtocolCombo::currentProtocol() const
{
    return text();
}

void KProtocolCombo::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setClipRect(event->rect());
    const int buttonWidth  = width();
    const int buttonHeight = height();

    const QColor bgColor = backgroundColor();
    QColor fgColor = foregroundColor();

    const bool isHighlighted = isDisplayHintEnabled(EnteredHint);
    const bool isActive = urlNavigator()->isActive();

    // draw button background
    painter.setPen(Qt::NoPen);
    painter.setBrush(bgColor);
    painter.drawRect(0, 0, buttonWidth, buttonHeight);

    if ((!isDisplayHintEnabled(ActivatedHint) || !isActive) && !isHighlighted) {
        fgColor.setAlpha(fgColor.alpha() / 2);
    }
    painter.setPen(fgColor);

    // draw arrow
    const int startY = (buttonHeight / 2) - (ArrowHeight / 2);
    const int startLeftX = buttonWidth - (2 * ArrowHeight) - BorderWidth;
    const int startRightX = startLeftX + (2 * ArrowHeight) - 1;
    for (int i = 0; i < ArrowHeight; ++i) {
        const int leftX = startLeftX + i;
        const int rightX = startRightX - i;
        const int endY = startY + i + 1;
        painter.drawLine(leftX, startY, leftX, endY);
        if (leftX != rightX) {
            // alpha blending is used, hence assure that a line is
            // never drawn twice
            painter.drawLine(rightX, startY, rightX, endY);
        }
    }

    const int textWidth = startLeftX - (2 * BorderWidth);
    painter.drawText(QRect(BorderWidth, 0, textWidth, buttonHeight), Qt::AlignCenter, text());
}

void KProtocolCombo::setProtocol(QAction* action)
{
    const int index = action->data().toInt();
    Q_ASSERT((index > 0) && (index < m_protocols.count()));
    const QString protocol = m_protocols[index];
    setText(protocol);
    emit activated(protocol);
}

#include "kprotocolcombo_p.moc"
