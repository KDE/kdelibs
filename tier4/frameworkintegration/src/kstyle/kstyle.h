/**
 * KStyle for KDE5 (KDE Integration)
 * Copyright (C) 2013 Alejandro Fiestas Olivares <afiestas@kde.org>
 *
 * KStyle for KDE4
 * Copyright (C) 2004-2005 Maksim Orlovich <maksim@kde.org>
 * Copyright (C) 2005,2006 Sandro Giessl <giessl@kde.org>
 *
 * Based in part on the following software:
 *  KStyle for KDE3
 *      Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>
 *      Portions  (C) 1998-2000 TrollTech AS
 *  Keramik for KDE3,
 *      Copyright (C) 2002      Malte Starostik   <malte@kde.org>
 *                (C) 2002-2003 Maksim Orlovich  <maksim@kde.org>
 *      Portions  (C) 2001-2002 Karol Szwed     <gallium@kde.org>
 *                (C) 2001-2002 Fredrik Höglund <fredrik@kde.org>
 *                (C) 2000 Daniel M. Duley       <mosfet@kde.org>
 *                (C) 2000 Dirk Mueller         <mueller@kde.org>
 *                (C) 2001 Martijn Klingens    <klingens@kde.org>
 *                (C) 2003 Sandro Giessl      <sandro@giessl.com>
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
 
#ifndef KDE_KSTYLE_H
#define KDE_KSTYLE_H

#include <kstyle_export.h>
#include <QCommonStyle>
#include <QPalette>

class KStylePrivate;
/**
 * Provides integration with KDE Plasma Workspace settings for Qt styles.
 *
 * Derive your Qt style from KStyle to automatically inherit
 * various settings from the KDE Plasma Workspace, providing a
 * consistent user experience. For example, this will ensure a
 * consistent single-click or double-click activation setting,
 * and the use of standard themed icons.
 *
 * @author Maksim Orlovich (maksim\@kde.org)
 * @author Sandro Giessl (giessl\@kde.org)
 * @author Àlex Fiestas (afiestas\@kde.org)
 */

class KSTYLE_EXPORT KStyle: public QCommonStyle
{
    Q_OBJECT

public:
    KStyle();
    ~KStyle();

public:
//@{
    virtual int pixelMetric(PixelMetric m, const QStyleOption* opt = 0, const QWidget* widget = 0) const Q_DECL_OVERRIDE;
    virtual int styleHint(StyleHint hint, const QStyleOption* opt,
                            const QWidget* w, QStyleHintReturn* returnData) const Q_DECL_OVERRIDE;

    virtual void polish(QWidget *) Q_DECL_OVERRIDE;
    using  QCommonStyle::polish; //! needed to avoid warnings at compilation time

    virtual QPalette standardPalette() const Q_DECL_OVERRIDE;

    virtual QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *option = 0,
                                     const QWidget *widget = 0) const Q_DECL_OVERRIDE;
//@}
private:
    KStylePrivate * const d;
};

#endif //KDE_KSTYLE_H
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
