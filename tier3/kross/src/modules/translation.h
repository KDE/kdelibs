/***************************************************************************
 * translation.h
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

#ifndef KROSS_TRANSLATION_H
#define KROSS_TRANSLATION_H

#include <QObject>
#include <QVariantList>

class QString;

class KLocalizedString;

namespace Kross {

    /**
     * The TranslationModule provides access to KDE translation and internationalization facilities.
     *
     * Example (in Python) :
     * \code
     * import Kross
     * t = Kross.module("kdetranslation")
     * print t.i18n("This string can be translated")
     * print t.i18ncp("Plural example", "%1 file not deleted %2", "%1 files not deleted %2", 3, [t.i18n("yesterday")])
     * \endcode
     */
    class TranslationModule: public QObject
    {
            Q_OBJECT

        public:
            explicit TranslationModule();
            virtual ~TranslationModule();

        public Q_SLOTS:
            /// Creates localized string from a given @p text. Substitute @p arguments (may be empty)
            QString i18n( const QString &text, const QVariantList &arguments = QVariantList() ) const;
            /// Creates localized string from a given @p text, with added context. Substitute @p arguments (may be empty)
            QString i18nc( const QString &context, const QString &text, const QVariantList &arguments = QVariantList() ) const;
            /// Creates localized string from a given @p plural and @p singular form dependent on @p number. Substitute @p arguments (may be empty)
            QString i18np( const QString &singular, const QString &plural, int number, const QVariantList &arguments = QVariantList() ) const;
            /// Creates localized string from a given @p plural and @p singular form dependent on @p number, with added context. Substitute @p arguments (may be empty)
            QString i18ncp( const QString &context, const QString &singular, const QString &plural, int number, const QVariantList &arguments = QVariantList() ) const;

        protected:
            KLocalizedString substituteArguments( const KLocalizedString &kls, const QVariantList &arguments, int max = 99 ) const;

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };
}

#endif

