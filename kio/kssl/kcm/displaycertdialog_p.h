/*  This file is part of the KDE project
    Copyright (C) 2010 Andreas Hartmetz <ahartmetz@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef DISPLAYCERTDIALOG_P_H
#define DISPLAYCERTDIALOG_P_H

#include "ui_displaycert.h"
#include <QDialog>
#include <QtNetwork/QSslCertificate>

class DisplayCertDialog : public QDialog
{
    Q_OBJECT
public:
    DisplayCertDialog(QWidget *parent);
    void setCertificates(const QList<QSslCertificate> &certs);
private:
    void showCertificate(int index);
private Q_SLOTS:
    void nextClicked();
    void previousClicked();

private:
    Ui::DisplayCert m_ui;
    QList<QSslCertificate> m_certs;
    int m_index;
    QPushButton *m_previousButton;
    QPushButton *m_nextButton;
};
#endif // DISPLAYCERTDIALOG_P_H
