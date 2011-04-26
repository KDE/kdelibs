/***************************************************************************
 *   Copyright 2011 Artur Duque de Souza <asouza@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/


#include <QPixmap>
#include <QSize>
#include <KIcon>
#include <KIconLoader>

#include "kiconprovider_p.h"


KIconProvider::KIconProvider()
  : QDeclarativeImageProvider(QDeclarativeImageProvider::Pixmap)
{
}

QPixmap KIconProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    // We need to handle QIcon::mode and QIcon::state
    // Maybe we should use the id to get mode and state

    QPixmap pixmap;
    if (requestedSize.isValid()) {
        pixmap = KIcon(id).pixmap(requestedSize);
    } else {
        pixmap = KIcon(id).pixmap(IconSize(KIconLoader::Desktop));
    }

    if (!pixmap.isNull() && size) {
        *size = pixmap.size();
    }

    return pixmap;
}

