/***************************************************************************
 *   Copyright (C) 2008 by Carlo Segato <brandon.ml@gmail.com>             *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef KEMOTICONS_THEME_H
#define KEMOTICONS_THEME_H

#include "kemoticons_export.h"
#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtCore/QVariant>

class KEmoticonsThemePrivate
{
    public:
        KEmoticonsThemePrivate();
        QString themeName;
};

class KEmoticonsTheme : public QObject
{
    Q_OBJECT
    public:
        KEmoticonsTheme(QObject *parent, const QVariantList &args);
        virtual ~KEmoticonsTheme();
        virtual bool removeEmoticon(const QString &emo) = 0;
        virtual bool addEmoticon(const QString &emo, const QString &text, bool copy) = 0;
        virtual void save() = 0;
        
        QString parseEmoticons(const QString &text);
        
    protected:
        virtual void loadTheme() = 0;
        KEmoticonsThemePrivate * const d;
        
};

#endif /* KEMOTICONS_THEME_H */

// kate: space-indent on; indent-width 4; replace-tabs on;
