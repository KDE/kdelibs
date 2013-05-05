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

#ifndef KCMSSL_H
#define KCMSSL_H

#include <kcmodule.h>

class QTabWidget;
class CaCertificatesPage;

class KcmSsl : public KCModule
{
    Q_OBJECT
public:
    KcmSsl(QWidget *parent, const QVariantList &);

    virtual void load();
    virtual void save();
    virtual void defaults();

private Q_SLOTS:
    void pageChanged(bool isChanged);

private:
    QTabWidget *m_tabs;
    CaCertificatesPage *m_caCertificatesPage;
};

#endif
