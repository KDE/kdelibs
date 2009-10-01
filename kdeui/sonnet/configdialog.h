/*
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

        /**
         * Sets the language/dictionary that will be selected by default
         * in this config dialog.
         * This overrides the setting in the config file.
         *
         * @param language the language which will be selected by default.
         * @since 4.1
         */
        void setLanguage( const QString &language );

    protected Q_SLOTS:
        virtual void slotOk();
        virtual void slotApply();

    Q_SIGNALS:

        /**
         * This is emitted when the user closed the dialog (and did not
         * cancel it).
         *
         * @param language the language which the user has selected
         * @since 4.1
         */
        void languageChanged( const QString &language );

    private:
        void init(KConfig *config);
    private:
        class Private;
        friend class Private;
        Private *const d;
        Q_DISABLE_COPY(ConfigDialog)
        Q_PRIVATE_SLOT(d, void slotConfigChanged())
    };
}

#endif
