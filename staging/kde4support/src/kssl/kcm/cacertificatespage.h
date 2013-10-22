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

#ifndef CACERTIFICATESPAGE_H
#define CACERTIFICATESPAGE_H

#include "ui_cacertificates.h"

#include <QWidget>
class QTreeWidgetItem;
class KSslCaCertificate;

class CaCertificatesPage : public QWidget
{
    Q_OBJECT
public:
    CaCertificatesPage(QWidget *parent);

    void load();
    void save();
    void defaults();

private Q_SLOTS:
    void itemSelectionChanged();

    void displaySelectionClicked();
    void disableSelectionClicked();
    void enableSelectionClicked();
    void removeSelectionClicked();
    void addCertificateClicked();
    void itemChanged(QTreeWidgetItem *item, int column);

Q_SIGNALS:
    void changed(bool state);

protected:
    void showEvent(QShowEvent *event);
    
private:
    bool addCertificateItem(const KSslCaCertificate &caCert);
    void enableDisableSelectionClicked(bool isEnable);
    
    Ui::CaCertificatesPage m_ui;
    QTreeWidgetItem *m_systemCertificatesParent;
    QTreeWidgetItem *m_userCertificatesParent;
    QSet<QByteArray> m_knownCertificates;
    bool m_firstShowEvent;
    bool m_blockItemChanged;
};

#endif
