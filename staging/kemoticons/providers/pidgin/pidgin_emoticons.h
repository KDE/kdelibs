/**********************************************************************************
 *   Copyright (C) 2008 by Carlo Segato <brandon.ml@gmail.com>                    *
 *                                                                                *
 *   This library is free software; you can redistribute it and/or                *
 *   modify it under the terms of the GNU Lesser General Public                   *
 *   License as published by the Free Software Foundation; either                 *
 *   version 2.1 of the License, or (at your option) any later version.           *
 *                                                                                *
 *   This library is distributed in the hope that it will be useful,              *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of               *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU            *
 *   Lesser General Public License for more details.                              *
 *                                                                                *
 *   You should have received a copy of the GNU Lesser General Public             *
 *   License along with this library.  If not, see <http://www.gnu.org/licenses/>.*
 *                                                                                *
 **********************************************************************************/

#ifndef PIDGIN_EMOTICONS_H
#define PIDGIN_EMOTICONS_H

#include <kemoticonsprovider.h>

class PidginEmoticons : public KEmoticonsProvider
{
    Q_OBJECT
public:
    PidginEmoticons(QObject *parent, const QVariantList &args);

    bool loadTheme(const QString &path);

    bool removeEmoticon(const QString &emo);
    bool addEmoticon(const QString &emo, const QString &text, AddEmoticonOption option = DoNotCopy);
    void save();

    void createNew();

private:
    QStringList m_text;
};

#endif /* PIDGIN_EMOTICONS_H */

// kate: space-indent on; indent-width 4; replace-tabs on;
