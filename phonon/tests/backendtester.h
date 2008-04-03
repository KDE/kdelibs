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

#ifndef BACKENDTESTER_H
#define BACKENDTESTER_H

#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtGui/QMainWindow>
#include "ui_backendtester.h"

class MainWindow : public QMainWindow, private Ui::BackendTester
{
    Q_OBJECT
    public:
        MainWindow();
        ~MainWindow();

    private:
        QString m_localAudioFile1;
        QString m_localAudioFile2;
        QString m_localVideoFile1;
        QString m_localVideoFile2;
        QUrl m_remoteAudioFile1;
        QUrl m_remoteAudioFile2;
};

#endif // BACKENDTESTER_H
