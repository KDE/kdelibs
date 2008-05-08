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

#ifndef PIDGIN_EMOTICONS_H
#define PIDGIN_EMOTICONS_H

#include <kemoticonsprovider.h>

class PidginEmoticons : public KEmoticonsProvider
{
    Q_OBJECT
public:
    PidginEmoticons(QObject *parent, const QVariantList &args);
    ~PidginEmoticons();

    bool loadTheme(const QString &path);

    bool removeEmoticon(const QString &emo);
    bool addEmoticon(const QString &emo, const QString &text, bool copy=false);
    void save();

    void createNew();

private:
    QStringList m_text;
};

#endif /* PIDGIN_EMOTICONS_H */

// kate: space-indent on; indent-width 4; replace-tabs on;
