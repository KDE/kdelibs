/*  This file is part of the KDE libraries
    Copyright (C) 2006 Michaël Larouche <michael.larouche@kdemail.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KMESSAGETEST_H
#define KMESSAGETEST_H

#include <QtGui/QWidget>

class KMessage_Test : public QWidget
{
    Q_OBJECT
public:
    KMessage_Test(QWidget *parent = 0);

private slots:
    void showError();
    void showFatal();
    void showInformation();
    void showSorry();
    void showWarning();

};

#endif
// kate: space-indent on; indent-width 4; encoding utf-8; replace-tabs on;
