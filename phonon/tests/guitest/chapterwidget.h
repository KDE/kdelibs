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

#ifndef CHAPTERWIDGET_H
#define CHAPTERWIDGET_H

#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>
#include <Phonon/MediaController>

using Phonon::MediaController;

class ChapterWidget : public QWidget
{
    Q_OBJECT
    public:
        ChapterWidget(QWidget *parent = 0);
        ~ChapterWidget();
        void setInterface(MediaController *i);

    private slots:
        void availableChaptersChanged(int);
    private:
        MediaController *m_iface;
        QSpinBox *m_currentChapter;
        QLabel *m_availableChapters;
};

#endif // CHAPTERWIDGET_H
