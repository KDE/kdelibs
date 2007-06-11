/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _NEPOMUK_ERRORS_H_
#define _NEPOMUK_ERRORS_H_

#include <QtCore/QString>

namespace Nepomuk {
    namespace Middleware {
	/**
	 * The Error namespace contains all predefined errors that may occurre
	 * during Nepomuk service communication. In addition to these errors
	 * services and clients implemented using the D-Bus communication may
	 * also throw the errors defined by D-Bus such as
	 * \code
	 * org.freedesktop.DBus.Error.InvalidArgs.
	 * \endcode
	 *
	 * Specific services may also add specific errors.
	 */
	namespace Error {
	    /**
	     * No error occurred. The process was successful.
	     */
	    const QString NoError = QLatin1String("org.semanticdesktop.nepomuk.error.NoError");

	    /**
	     * The service could not be found, i.e. an invalid service URL has been used.
	     */
	    const QString ServiceUnknown =
                QLatin1String("org.semanticdesktop.nepomuk.error.ServiceUnknown");

	    /**
	     * The called method is not known, i.e. the service does not implement this method.
	     */
	    const QString MethodUnknown =
                QLatin1String("org.semanticdesktop.nepomuk.error.MethodUnknown");

	    /**
	     * The method call failed for some unknown reason.
	     */
	    const QString Failed = QLatin1String("org.semanticdesktop.nepomuk.error.Failed");

	    /**
	     * No running middleware could be found. This means that either the middleware crashed
	     * or was never started.
	     */
	    const QString NoMiddleware =
                QLatin1String("org.semanticdesktop.nepomuk.error.NoMiddleware");

	    /**
	     * A certain component might not implement a complete service.
	     */
	    const QString MethodNotImplemented =
                QLatin1String("org.semanticdesktop.nepomuk.error.MethodNotImplemented");

	    /**
	     * The value of one of the parameters of the method call is invalid. The error message can
	     * be used to further describe the issue.
	     */
	    const QString InvalidParameter =
                QLatin1String("org.semanticdesktop.nepomuk.error.InvalidParameter");

	    /**
	     * An unknown error may be used in case no specific error code fits. The error message can
	     * be used to further describe the issue.
	     */
	    const QString UnknownError =
                QLatin1String("org.semanticdesktop.nepomuk.error.UnknownError");
	}
    }
}

#endif
