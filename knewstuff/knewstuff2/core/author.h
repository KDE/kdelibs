/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2006 Josef Spillner <spillner@kde.org>

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
#ifndef KNEWSTUFF2_AUTHOR_H
#define KNEWSTUFF2_AUTHOR_H

#include <qstring.h>

namespace KNS {

/**
 * @short KNewStuff author information.
 *
 * This class provides accessor methods to the author data
 * as used by KNewStuff.
 * It should probably not be used directly by the application.
 *
 * @author Josef Spillner (spillner@kde.org)
 */
class Author
{
  public:
    /**
     * Constructor.
     */
    Author();

    /**
     * Destructor.
     */
    ~Author();

    /**
     * Sets the full name of the author.
     */
    void setName(const QString& name);

    /**
     * Retrieve the author's name.
     *
     * @return author name
     */
    QString name() const;

    /**
     * Sets the email address of the author.
     */
    void setEmail(const QString& email);

    /**
     * Retrieve the author's email address.
     *
     * @return author email address
     */
    QString email() const;

    /**
     * Sets the jabber address of the author.
     */
    void setJabber(const QString& jabber);

    /**
     * Retrieve the author's jabber address.
     *
     * @return author jabber address
     */
    QString jabber() const;

    /**
     * Sets the homepage of the author.
     */
    void setHomepage(const QString& homepage);

    /**
     * Retrieve the author's homepage.
     *
     * @return author homepage
     */
    QString homepage() const;

  private:
    QString mName;
    QString mEmail;
    QString mJabber;
    QString mHomepage;

    class AuthorPrivate *d;
};

}

#endif
