/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef KDEUI_KDIALOG_P_H
#define KDEUI_KDIALOG_P_H

#include "kdialog.h"
#include <QtCore/QPointer>
#include <QtCore/QSignalMapper>
#include <QtCore/QSize>
#include <QtCore/QHash>

class QBoxLayout;
class KPushButton;
class KUrlLabel;
class KSeparator;
class QDialogButtonBox;

class KDialogPrivate
{
    Q_DECLARE_PUBLIC(KDialog)
    protected:
        KDialogPrivate()
            : mDetailsVisible(false), mSettingDetails(false), mDeferredDelete(false),
            mDetailsWidget(0),
            mTopLayout(0), mMainWidget(0), mUrlHelp(0), mActionSeparator(0),
            mButtonOrientation(Qt::Horizontal),
            mButtonBox(0)
        {
        }

        virtual ~KDialogPrivate() {}

        KDialog *q_ptr;

        void setupLayout();
        void appendButton( KDialog::ButtonCode code , const KGuiItem &item );
        KPushButton *button( KDialog::ButtonCode code ) const;


        bool mDetailsVisible;
        bool mSettingDetails;
        bool mDeferredDelete;
        QWidget *mDetailsWidget;
        QSize mIncSize;
        QSize mMinSize;
        QString mDetailsButtonText;

        QBoxLayout *mTopLayout;
        QPointer<QWidget> mMainWidget;
        KUrlLabel *mUrlHelp;
        KSeparator *mActionSeparator;

        QString mAnchor;
        QString mHelpApp;
        QString mHelpLinkText;

        Qt::Orientation mButtonOrientation;
        KDialog::ButtonCode mEscapeButton;

        QDialogButtonBox *mButtonBox;
        QHash<int, KPushButton*> mButtonList;
        QSignalMapper mButtonSignalMapper;

    protected Q_SLOTS:
        void queuedLayoutUpdate();
        void helpLinkClicked();

    private:
        void init(KDialog *);
        bool dirty: 1;
};

#endif // KDEUI_KDIALOG_P_H
