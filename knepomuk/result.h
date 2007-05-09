/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#ifndef _NEPOMUK_RESULT_H_
#define _NEPOMUK_RESULT_H_

#include <knepomuk/knepomuk_export.h>

#include <QtCore/QSharedData>
#include <QtCore/QString>
#include <QtCore/QVariant>


namespace Nepomuk {
    namespace Backbone {

	/**
	 * \brief A Result encapsulates the return value of a method call through different
	 * communication backends in the Nepomuk backbone system.
	 *
	 * Result uses a shared data approach. Thus, copying a Result object is fast and
	 * uses little memory.
	 *
	 * \see Message
	 */
	class KNEPOMUK_EXPORT Result
	    {
	    public:
		/**
		 * Default constructor. Creates an empty result
		 * with a status of 0 (success).
		 */
		Result();

		/**
		 * Default copy constructor
		 */
		Result( const Result& );

                /**
                 * \deprecated use Result::createSimpleResult
                 */
		KDE_DEPRECATED explicit Result( int status, const QVariant& value = QVariant() );

		~Result();

		void setValue( const QVariant& );
		void setStatus( int );

		/**
		 * The return value of the method call
		 *
		 * \return A reference to the return value of the called method. 
		 *         If the method had a void return value an invalid QVariant
		 *         object is returned.
		 */
		const QVariant& value() const;

		/**
		 * Error code representing the success of the method call.
		 * \li 0 - success
		 */
		int status() const;

		/**
		 * Contains the name of the error that occurred if status() is not 0.
		 * This may also be a DBus error or a specific service or component error.
		 * 
		 * For default error codes see the Error namespace.
		 */
		QString errorName() const;

		/**
		 * A service may set an error message in addition to the errorName providing
		 * further details on the error.
		 */
		QString errorMessage() const;

		static Result createSimpleResult( const QVariant& value );
		static Result createErrorResult( int status, const QString& name, const QString& message = QString() );

		Result& operator=( const Result& );

	    private:
		class Private;
		QSharedDataPointer<Private> d;
	    };
    }
}

#endif
