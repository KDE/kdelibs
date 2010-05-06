/* This file is part of the Nepomuk-KDE libraries
    Copyright (c) 2007 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_LITERAL_H_
#define _NEPOMUK_LITERAL_H_

#include <QtCore/QUrl>
#include <QtCore/QVariant>
#include <QtCore/QSharedData>

#include "nepomuk_export.h"

namespace Nepomuk {
    namespace Types {
        /**
         * \class Literal literal.h Nepomuk/Types/Literal
         *
         * \brief Defines a literal type based on XML Schema.
         *
         * Each valid Literal represents the literal type
         * as XML Schema URI and as QVariant type.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         */
        class NEPOMUK_EXPORT Literal
	{
	public:
	    /**
	     * Default constructor.
	     * Creates an empty Literal
	     */
	    Literal();
	    Literal( const Literal& );
	    Literal( const QUrl& dataTypeUri );
	    ~Literal();

	    Literal& operator=( const Literal& );

	    /**
	     * The XML Schema type URI.
	     */
	    QUrl dataTypeUri() const;

	    /**
	     * The type converted to a QVariant::Type.
	     * \return The QVariant::Type that corresponds to the XML Schema type or
	     * QVariant::Invalid if it could not be matched.
	     */
	    QVariant::Type dataType() const;

            /**
             * Is this a valid Literal, i.e. has it a valid URI.
             */
            bool isValid() const;

	private:
	    class Private;
	    QExplicitlySharedDataPointer<Private> d;
	};
    }
}


#ifndef DISABLE_NEPOMUK_LEGACY

namespace Nepomuk {
    /**
     * \brief Defines a literal type based on XML Schema.
     *
     * Each valid Literal represents the literal type
     * as XML Schema URI and as QVariant type.
     *
     * \deprecated in favor of Nepomuk::Types::Literal
     */
    class KDE_DEPRECATED NEPOMUK_EXPORT Literal
    {
    public:
        /**
         * Default constructor.
         * Creates an empty Literal
         */
        Literal();
        Literal( const Literal& );
        Literal( const QUrl& dataTypeUri );
        ~Literal();

        Literal& operator=( const Literal& );

        /**
         * The XML Schema type URI.
         */
        const QUrl& dataTypeUri() const;

        /**
         * The type converted to a QVariant::Type.
         * \return The QVariant::Type that corresponds to the XML Schema type or
         * QVariant::Invalid if it could not be matched.
         */
        QVariant::Type dataType() const;

    private:
        class Private;
        QSharedDataPointer<Private> d;
    };
}

#endif // DISABLE_NEPOMUK_LEGACY

#endif
