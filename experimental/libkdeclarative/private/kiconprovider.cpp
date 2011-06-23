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

#include "kiconprovider_p.h"

#include <QPixmap>
#include <QSize>
#include <kicon.h>
#include <kiconloader.h>
#include <kiconeffect.h>

KIconProvider::KIconProvider()
  : QDeclarativeImageProvider(QDeclarativeImageProvider::Pixmap)
{
}

QPixmap KIconProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    // We need to handle QIcon::state
    const QStringList source = id.split('/');

    QPixmap pixmap;
    if (requestedSize.isValid()) {
        pixmap = KIcon(source.at(0)).pixmap(requestedSize);
    } else {
        pixmap = KIcon(source.at(0)).pixmap(IconSize(KIconLoader::Desktop));
    }

    if (source.size() == 2) {
        KIconEffect *effect = KIconLoader::global()->iconEffect();
        const QString state(source.at(1));
        int finalState = KIconLoader::DefaultState;

        if (state == "active") {
            finalState = KIconLoader::ActiveState;
        } else if (state == "disabled") {
            finalState = KIconLoader::DisabledState;
        } else if (state == "last") {
            finalState = KIconLoader::LastState;
        }

        // apply the effect for state
        pixmap = effect->apply(pixmap, KIconLoader::Desktop, finalState);
    }

    if (!pixmap.isNull() && size) {
        *size = pixmap.size();
    }

    return pixmap;
}

