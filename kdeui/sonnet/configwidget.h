/**
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#ifndef SONNET_CONFIGWIDGET_H
#define SONNET_CONFIGWIDGET_H

#include <QtGui/QWidget>
#include <kdeui_export.h>

class KConfig;

namespace Sonnet
{
    /// The sonnet ConfigWidget
    class KDEUI_EXPORT ConfigWidget : public QWidget
    {
        Q_OBJECT
    public:
        ConfigWidget(KConfig *config, QWidget *parent);
        ~ConfigWidget();

        bool backgroundCheckingButtonShown() const;

    public Q_SLOTS:
        void save();
        void setBackgroundCheckingButtonShown( bool );
        void slotDefault();
    protected Q_SLOTS:
        void slotChanged();

    private:
        void init(KConfig *config);
        void setFromGui();
        void setCorrectLanguage( const QStringList& langs );

    private:
        class Private;
        Private* const d;
    };
}

#endif
