/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef _NEPOMUK_MESSAGE_H_
#define _NEPOMUK_MESSAGE_H_

#include <knepomuk/knepomuk_export.h>

#include <QtCore/QSharedData>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QList>


namespace Nepomuk {
  namespace Backbone {

    /**
     * \brief A message encapsulates the parameters of a method call through different
     * communication backends in the Nepomuk backbone system.
     *
     * Message uses a shared data approach. Thus, copying a Message object is fast and
     * uses little memory.
     *
     * \see Result
     */
    class KNEPOMUK_EXPORT Message
      {
      public:
	/**
	 * Default constructor. Creates an invalid message.
	 */
	Message();

	/**
	 * Default copy constructor
	 */
	Message( const Message& );

	/**
	 * Create a message.
	 *
	 * \param service The Nepomuk service URI
	 * \param method The method to be called
	 */
	Message( const QString& service, const QString& method );

	~Message();
     
	/**
	 * This method is mostly useful to check if a method call
	 * was successful and thus, the reply is valid.
	 *
	 * \returns True if the message is valid.
	 */
	bool isValid() const;

	int numArgs() const;
	QList<QVariant> arguments() const;

	void addArgument( const QVariant& );
	void setArguments( const QList<QVariant>& args );

	void setService( const QString& );
	void setMethod( const QString& );

	const QString& service() const;
	const QString& method() const;

	const QVariant& operator[]( int i ) const;
	QVariant& operator[]( int i );

	Message& operator=( const Message& );

      private:
	class Private;
	QSharedDataPointer<Private> d;
      };
  }
}

#endif
