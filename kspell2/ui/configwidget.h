/**
 * configwidget.h
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 */
#ifndef KSPELL_CONFIGWIDGET_H
#define KSPELL_CONFIGWIDGET_H

#include <qwidget.h>

namespace KSpell2
{
    class Broker;
    class ConfigWidget : public QWidget
    {
        Q_OBJECT
    public:
        ConfigWidget( Broker *broker, QWidget *parent, const char *name =0 );
        ~ConfigWidget();

    public slots:
        void save();

    protected slots:
        void slotChanged();
    private:
        void init( Broker *broker );
        void setFromGUI();
        void setCorrectLanguage( const QStringList& langs );

    private:
        class Private;
        Private *d;
    };
}

#endif
