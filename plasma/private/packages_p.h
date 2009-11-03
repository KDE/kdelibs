/******************************************************************************
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                        *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#ifndef LIBS_PLASMA_PACKAGES_P_H
#define LIBS_PLASMA_PACKAGES_P_H

#include "plasma/packagestructure.h"
#include "plasma/wallpaper.h"
#include "plasma/plasma.h"

namespace Plasma
{

class Wallpaper;

class PlasmoidPackage : public PackageStructure
{
    Q_OBJECT
public:
    explicit PlasmoidPackage(QObject *parent = 0);
    void createNewWidgetBrowser(QWidget *parent = 0);

protected:
    void pathChanged();

protected Q_SLOTS:
    void widgetBrowserFinished();
};

class ThemePackage : public PackageStructure
{
    Q_OBJECT
public:
    explicit ThemePackage(QObject *parent = 0);
};

class WallpaperPackage : public PackageStructure
{
    Q_OBJECT

public:
    explicit WallpaperPackage(Wallpaper *paper = 0, QObject *parent = 0);

protected:
    void pathChanged();

private:
    QSize resSize(const QString &str) const;
    void findBestPaper();
    float distance(const QSize& size, const QSize& desired,
                   Plasma::Wallpaper::ResizeMethod method) const;

private Q_SLOTS:
    void paperDestroyed();
    void renderHintsChanged();

private:
    Wallpaper *m_paper;
    bool m_fullPackage;
};

PackageStructure::Ptr defaultPackageStructure(ComponentType type);

} // namespace Plasma

#endif // LIBS_PLASMA_PACKAGES_P_H
