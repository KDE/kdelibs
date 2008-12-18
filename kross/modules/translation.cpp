/***************************************************************************
 * translation.cpp
 * This file is part of the KDE project
 * copyright (C)2008 by Dag Andersen <danders@get2net.dk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "translation.h"

#include <QString>
#include <QVariant>
#include <QVariantList>

#include <kdebug.h>
#include <klocale.h>
#include <klocalizedstring.h>
#include <kpluginloader.h>
#include <kpluginfactory.h>

extern "C"
{
    KDE_EXPORT QObject* krossmodule()
    {
        return new Kross::TranslationModule();
    }
}

using namespace Kross;


namespace Kross {

    /// \internal d-pointer class.
    class TranslationModule::Private
    {
    };
}

TranslationModule::TranslationModule()
    : QObject()
    , d( new Private() )
{
}

TranslationModule::~TranslationModule()
{
    delete d;
}

KLocalizedString TranslationModule::substituteArguments( const KLocalizedString &kls, const QVariantList &arguments, int max ) const
{
    KLocalizedString ls = kls;
    int cnt = qMin( arguments.count(), max ); // QString supports max 99
    for ( int i = 0; i < cnt; ++i ) {
        QVariant arg = arguments[i];
        switch ( arg.type() ) {
            case QVariant::Int: ls = ls.subs(arg.toInt()); break;
            case QVariant::UInt: ls = ls.subs(arg.toUInt()); break;
            case QVariant::LongLong: ls = ls.subs(arg.toLongLong()); break;
            case QVariant::ULongLong: ls = ls.subs(arg.toULongLong()); break;
            case QVariant::Double: ls = ls.subs(arg.toDouble()); break;
            default: ls = ls.subs(arg.toString()); break;
        }
    }
    return ls;
}

QString TranslationModule::i18n( const QString &text, const QVariantList &arguments ) const
{
    KLocalizedString ls = ki18n(text.toUtf8());
    return substituteArguments( ls, arguments ).toString();
}

QString TranslationModule::i18nc( const QString &context, const QString &text, const QVariantList &arguments ) const
{
    KLocalizedString ls = ki18nc(context.toUtf8(), text.toUtf8());
    return substituteArguments( ls, arguments ).toString();
}

QString TranslationModule::i18np( const QString &singular, const QString &plural, int number, const QVariantList &arguments ) const
{
    KLocalizedString ls = ki18np(singular.toUtf8(), plural.toUtf8()).subs(number);
    return substituteArguments( ls, arguments, 98 ).toString();
}

QString TranslationModule::i18ncp( const QString &context, const QString &singular, const QString &plural,  int number, const QVariantList &arguments ) const
{
    KLocalizedString ls = ki18ncp(context.toUtf8(), singular.toUtf8(), plural.toUtf8()).subs( number );
    return substituteArguments( ls, arguments, 98 ).toString();
}

#include "translation.moc"
