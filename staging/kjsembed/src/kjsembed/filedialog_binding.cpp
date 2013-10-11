/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2005, 2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2005, 2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2005, 2006 Erik L. Bunce <kde@bunce.us>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "filedialog_binding.h"

#include <QtCore/QStringList>
#include <QtCore/QProcess>
#include <QFileDialog>
#include <QtCore/QDebug>

#include <kjs/object.h>

#include "static_binding.h"

using namespace KJSEmbed;

KJS::JSValue *callGetExistingDirectory( KJS::ExecState *exec, KJS::JSObject * /*self*/, const KJS::List &args )
{
    QWidget *parent = KJSEmbed::extractObject<QWidget>(exec, args, 0, 0);
    QString caption = KJSEmbed::extractVariant<QString>(exec, args, 1, QString());
    QString dir = KJSEmbed::extractVariant<QString>(exec, args, 2, QString());
    QFileDialog::Options options = (QFileDialog::Options)KJSEmbed::extractVariant<uint>(exec, args, 3, QFileDialog::ShowDirsOnly);

    return KJS::jsString( QFileDialog::getExistingDirectory(parent, caption, dir, options) );
}

KJS::JSValue *callGetOpenFileName( KJS::ExecState *exec, KJS::JSObject * /*self*/, const KJS::List &args )
{
    QWidget *parent = KJSEmbed::extractObject<QWidget>(exec, args, 0, 0);
    QString caption = KJSEmbed::extractVariant<QString>(exec, args, 1, "");
    QString dir = KJSEmbed::extractVariant<QString>(exec, args, 2, "");
    QString filter = KJSEmbed::extractVariant<QString>(exec, args, 3, "");
//    QString *selectedFilter = KJSEmbed::extractVariant<QString>(exec, args, 4, 0);
    QFileDialog::Options options = (QFileDialog::Options)KJSEmbed::extractVariant<uint>(exec, args, 4, 0);

    return KJS::jsString( QFileDialog::getOpenFileName(parent, caption, dir, filter, 0, options) );
}

KJS::JSValue *callGetOpenFileNames( KJS::ExecState *exec, KJS::JSObject * /*self*/, const KJS::List &args )
{
    QWidget *parent = KJSEmbed::extractObject<QWidget>(exec, args, 0, 0);
    QString caption = KJSEmbed::extractVariant<QString>(exec, args, 1, QString());
    QString dir = KJSEmbed::extractVariant<QString>(exec, args, 2, QString());
    QString filter = KJSEmbed::extractVariant<QString>(exec, args, 3, QString());
//    QString *selectedFilter = KJSEmbed::extractVariant<QString>(exec, args, 4, 0);
    QFileDialog::Options options = (QFileDialog::Options)KJSEmbed::extractVariant<uint>(exec, args, 4, 0);

    QStringList fileNames =  QFileDialog::getOpenFileNames(parent, caption, dir, filter, 0, options);

    return convertToValue(exec, fileNames);
}

KJS::JSValue *callGetSaveFileName( KJS::ExecState *exec, KJS::JSObject * /*self*/, const KJS::List &args )
{
    QWidget *parent = KJSEmbed::extractObject<QWidget>(exec, args, 0, 0);
    QString caption = KJSEmbed::extractVariant<QString>(exec, args, 1, QString());
    QString dir = KJSEmbed::extractVariant<QString>(exec, args, 2, QString());
    QString filter = KJSEmbed::extractVariant<QString>(exec, args, 3, QString());
//    QString *selectedFilter = KJSEmbed::extractVariant<QString>(exec, args, 4, 0);
    QFileDialog::Options options = (QFileDialog::Options)KJSEmbed::extractVariant<uint>(exec, args, 4, 0);

    return KJS::jsString( QFileDialog::getSaveFileName(parent, caption, dir, filter, 0, options) );
}
const Method FileDialog::FileDialogMethods[] =
{
    {"getExistingDirectory", 1, KJS::DontDelete|KJS::ReadOnly, &callGetExistingDirectory },
    {"getOpenFileName", 1, KJS::DontDelete|KJS::ReadOnly, &callGetOpenFileName },
    {"getOpenFileNames", 1, KJS::DontDelete|KJS::ReadOnly, &callGetOpenFileNames },
    {"getSaveFileName", 0, KJS::DontDelete|KJS::ReadOnly, &callGetSaveFileName },
    {0, 0, 0, 0 }
};
//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
