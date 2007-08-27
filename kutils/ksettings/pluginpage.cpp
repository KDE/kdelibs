/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

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

#define KDE3_SUPPORT
#include "ksettings/pluginpage.h"
#undef KDE3_SUPPORT
#include "kpluginselector.h"
#include <QtGui/QLayout>
#include <kdialog.h>
#include "ksettings/dispatcher.h"
#include <kcomponentdata.h>

namespace KSettings
{

class PluginPagePrivate
{
    public:
        PluginPagePrivate()
            : selwid( 0 )
        {
        }

        KPluginSelector * selwid;
        void _k_reparseConfiguration(const QByteArray &a);
};

PluginPage::PluginPage(const KComponentData &componentData, QWidget *parent, const QVariantList &args)
    : KCModule(componentData, parent, args),
    d_ptr(new PluginPagePrivate)
{
    Q_D(PluginPage);
    //d->q_ptr = this;
//    ( new QVBoxLayout( this, 0, KDialog::spacingHint() ) )->setAutoAdd( true );
    d->selwid = new KPluginSelector( this );
    connect( d->selwid, SIGNAL( changed( bool ) ), this, SIGNAL( changed( bool ) ) );
    connect(d->selwid, SIGNAL(configCommitted(const QByteArray &)), this,
            SLOT(_k_reparseConfiguration(const QByteArray &)));
}

void PluginPagePrivate::_k_reparseConfiguration(const QByteArray &a)
{
    Dispatcher::reparseConfiguration(a);
}

PluginPage::~PluginPage()
{
    delete d_ptr;
}

KPluginSelector * PluginPage::pluginSelector()
{
    return d_ptr->selwid;
}

void PluginPage::load()
{
    d_ptr->selwid->load();
}

void PluginPage::save()
{
    d_ptr->selwid->save();
}

void PluginPage::defaults()
{
    d_ptr->selwid->defaults();
}

} //namespace

#include "pluginpage.moc"
