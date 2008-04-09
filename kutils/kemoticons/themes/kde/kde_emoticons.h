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

#ifndef KDE_EMOTICONS_H
#define KDE_EMOTICONS_H

#include <kemoticonstheme.h>

class KdeEmoticons : public KEmoticonsTheme
{
    Q_OBJECT
    public:
        KdeEmoticons(QObject *parent, const QVariantList &args);
        ~KdeEmoticons();
        
        void loadTheme(const QString &emo);
        
        bool removeEmoticon(const QString &emo);
        bool addEmoticon(const QString &emo, const QString &text, bool copy);
        void save();
        
};

#endif /* KDE_EMOTICONS_H */

// kate: space-indent on; indent-width 4; replace-tabs on;
