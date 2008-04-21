/*  This file is part of the KDE project
    Copyright (C) 2006-2008 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef DEVICEPREFERENCE_H_STUPID_UIC
#define DEVICEPREFERENCE_H_STUPID_UIC

#include "ui_devicepreference.h"
#include <phonon/objectdescriptionmodel.h>
#include <phonon/objectdescription.h>
#include <QtGui/QStandardItem>
#include <QtCore/QMap>
#include <QtCore/QList>
class QLabel;

class DevicePreference : public QWidget, private Ui::DevicePreference
{
    Q_OBJECT
    public:
        DevicePreference(QWidget *parent = 0);
        void load();
        void save();
        void defaults();

    Q_SIGNALS:
        void changed();

    protected:
        void changeEvent(QEvent *);

    private Q_SLOTS:
        void on_preferButton_clicked();
        void on_deferButton_clicked();
        void on_removeButton_clicked();
        void on_showCheckBox_toggled();
        void on_applyPreferencesButton_clicked();
        void updateButtonsEnabled();
        void updateDeviceList();
        void updateAudioOutputDevices();
        void updateAudioCaptureDevices();

    private:
        QList<Phonon::AudioOutputDevice> availableAudioOutputDevices() const;
        QList<Phonon::AudioCaptureDevice> availableAudioCaptureDevices() const;
        QMap<int, Phonon::AudioOutputDeviceModel *> m_outputModel;
        QMap<int, Phonon::AudioCaptureDeviceModel *> m_captureModel;
        QStandardItemModel m_categoryModel;
        QStandardItemModel m_headerModel;
        bool m_noCategoryChangeEventQueued;
};

#endif // DEVICEPREFERENCE_H_STUPID_UIC
