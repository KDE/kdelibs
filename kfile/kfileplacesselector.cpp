/***************************************************************************
 *   Copyright (C) 2006 by Peter Penz (peter.penz@gmx.at)                  *
 *   Copyright (C) 2007 by Kevin Ottens (ervin@kde.org)                    *
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

#include "kfileplacesselector_p.h"

#include "kurlnavigator.h"

#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kfileplacesmodel.h>
#include <kmenu.h>
#include <kdebug.h>

#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <kicon.h>

KFilePlacesSelector::KFilePlacesSelector(KUrlNavigator* parent, KFilePlacesModel* placesModel) :
    KUrlButton(parent),
    m_placesModel(placesModel),
    m_selectedUrl()
{
    setFocusPolicy(Qt::NoFocus);

    m_placesMenu = new KMenu(this);

    updateMenu();

    connect(m_placesModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
            this, SLOT(updateMenu()));
    connect(m_placesModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
            this, SLOT(updateMenu()));
    connect(m_placesMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(activatePlace(QAction*)));

    setMenu(m_placesMenu);
}

KFilePlacesSelector::~KFilePlacesSelector()
{
}

void KFilePlacesSelector::updateMenu()
{
    m_placesMenu->clear();

    const int rowCount = m_placesModel->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        QModelIndex index = m_placesModel->index(i, 0);
        QAction* action = new QAction(m_placesModel->icon(index),
                                      m_placesModel->text(index),
                                      m_placesMenu);
        m_placesMenu->addAction(action);
        action->setData(i);
    }
    updateSelection(m_selectedUrl);
}

void KFilePlacesSelector::updateSelection(const KUrl& url)
{
    const QModelIndex index = m_placesModel->closestItem(url);
    if (index.isValid()) {
        m_selectedUrl = url;
        setIcon(m_placesModel->icon(index));
    }
    else {
        // No bookmark has been found which matches to the given Url. Show
        // a generic folder icon as pixmap for indication:
        setIcon(KIcon("folder"));
    }
}

KUrl KFilePlacesSelector::selectedPlaceUrl() const
{
    return m_selectedUrl;
}

QString KFilePlacesSelector::selectedPlaceText() const
{
    const QModelIndex index = m_placesModel->closestItem(m_selectedUrl);
    return index.isValid() ? m_placesModel->text(index) : QString();
}

QSize KFilePlacesSelector::sizeHint() const
{
    const int height = KUrlButton::sizeHint().height();
    return QSize(height, height);
}

void KFilePlacesSelector::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);

    const int buttonWidth  = width();
    const int buttonHeight = height();

    const QColor bgColor = backgroundColor();

    // draw button backround
    painter.setPen(Qt::NoPen);
    painter.setBrush(bgColor);
    painter.drawRect(0, 0, buttonWidth, buttonHeight);

    // draw icon
    const QPixmap pixmap = icon().pixmap();
    const int x = (buttonWidth -  pixmap.width()) / 2;
    const int y = (buttonHeight - pixmap.height()) / 2;
    painter.drawPixmap(x, y, pixmap);
}

void KFilePlacesSelector::activatePlace(QAction* action)
{
    Q_ASSERT(action != 0);
    const int selectedItem = action->data().toInt();
    QModelIndex index = m_placesModel->index(selectedItem, 0);
    if (index.isValid()) {
        setIcon(m_placesModel->icon(index));
        emit placeActivated(m_placesModel->url(index));
    }
}

#include "kfileplacesselector_p.moc"

