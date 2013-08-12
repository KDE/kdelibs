/*
   This file is part of the KDE libraries
   Copyright (c) 2012 Benjamin Port <benjamin.port@ben2367.fr>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kwindowconfig.h"

#include <QScreen>
#include <QWindow>

static const char* s_initialSizePropertyName = "_kconfig_initial_size";
static const char* s_initialScreenSizePropertyName = "_kconfig_initial_screen_size";

void KWindowConfig::saveWindowSize(const QWindow *window, KConfigGroup &config, KConfigGroup::WriteConfigFlags options)
{
    if (!window)
        return;
    const QRect desk = window->screen()->geometry();

    const QSize sizeToSave = window->size();
    const bool isMaximized = window->windowState() & Qt::WindowMaximized;

    const QString screenMaximizedString(QString::fromLatin1("Window-Maximized %1x%2").arg(desk.height()).arg(desk.width()));
    // Save size only if window is not maximized
    if (!isMaximized) {
        const QSize defaultSize(window->property(s_initialSizePropertyName).toSize());
        const QSize defaultScreenSize(window->property(s_initialScreenSizePropertyName).toSize());
	const bool sizeValid = defaultSize.isValid() && defaultScreenSize.isValid();
	if (!sizeValid || (sizeValid && (defaultSize != sizeToSave || defaultScreenSize != desk.size()))) {
	    const QString wString(QString::fromLatin1("Width %1").arg(desk.width()));
	    const QString hString(QString::fromLatin1("Height %1").arg(desk.height()));
	    config.writeEntry(wString, sizeToSave.width(), options);
	    config.writeEntry(hString, sizeToSave.height(), options);
        }
    }
    if ( (isMaximized == false) && !config.hasDefault(screenMaximizedString) )
        config.revertToDefault(screenMaximizedString);
    else
        config.writeEntry(screenMaximizedString, isMaximized, options);

}

void KWindowConfig::restoreWindowSize(QWindow* window, const KConfigGroup& config)
{
    if (!window)
        return;

    const QRect desk = window->screen()->geometry();

    const int width = config.readEntry(QString::fromLatin1("Width %1").arg(desk.width()), window->size().width());
    const int height = config.readEntry(QString::fromLatin1("Height %1").arg(desk.height()), window->size().height());
    const bool isMaximized = config.readEntry(QString::fromLatin1("Window-Maximized %1x%2").arg(desk.height()).arg(desk.width()), false);

    // Check default size
    const QSize defaultSize(window->property(s_initialSizePropertyName).toSize());
    const QSize defaultScreenSize(window->property(s_initialScreenSizePropertyName).toSize());
    if (!defaultSize.isValid() || !defaultScreenSize.isValid()) {
        window->setProperty(s_initialSizePropertyName, window->size());
        window->setProperty(s_initialScreenSizePropertyName, desk.size());
    }

    // If window is maximized set maximized state and in all case set the size
    window->resize(width, height);
    if (isMaximized) {
        window->setWindowState(Qt::WindowMaximized);
    }
}
