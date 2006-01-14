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
#ifndef KSPELL_CONFIGDIALOG_H
#define KSPELL_CONFIGDIALOG_H

#include <kdialogbase.h>

namespace KSpell2
{
    class Broker;
    class ConfigDialog : public KDialogBase
    {
        Q_OBJECT
    public:
        ConfigDialog( Broker *broker,
                      QWidget *parent );
        ~ConfigDialog();

    protected slots:
        virtual void slotOk();
        virtual void slotApply();

    private:
        void init( Broker *broker );
    private:
        class Private;
        Private* const d;
    };
}

#endif
