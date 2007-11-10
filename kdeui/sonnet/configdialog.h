/**
 * configdialog.h
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
#ifndef SONNET_CONFIGDIALOG_H
#define SONNET_CONFIGDIALOG_H

#include <kdialog.h>
#include <kconfig.h>

namespace Sonnet
{
    /// The sonnet ConfigDialog
    class KDEUI_EXPORT ConfigDialog : public KDialog
    {
        Q_OBJECT
    public:
        ConfigDialog(KConfig *config,
                     QWidget *parent);
        ~ConfigDialog();

    protected Q_SLOTS:
        virtual void slotOk();
        virtual void slotApply();

    private:
        void init(KConfig *config);
    private:
        class Private;
        Private *const d;
        Q_DISABLE_COPY(ConfigDialog)
    };
}

#endif
