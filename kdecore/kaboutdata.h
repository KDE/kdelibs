/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Espen Sand (espen@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include <qvaluelist.h>
#include <qstring.h>

#ifndef _KABOUTDATA_H_
#define _KABOUTDATA_H_

/**
 * This structure is used to store information about a person or developer.
 * It can store the person's name, a task, an email address and a
 * link to a homepage. This class is intended for use in the
 * @ref KAboutProgram class, but it can be used elsewhere as well.
 * Normally you should at least define the person's name.
*/
struct KAboutPerson
{
    /**
     * Convenience constructor
     */
    KAboutPerson( const QString &_name, const QString &_task,
                  const QString &_emailAddress, const QString &_webAddress )
    {
      name = _name;
      task = _task;
      emailAddress = _emailAddress;
      webAddress = _webAddress;
    }
    /**
     * @internal - don't use. Required by QValueList
     */
    KAboutPerson() {}

    /**
     * The person's name
     */
    QString name;
    /**
     * The person's task
     */
    QString task;
    /**
     * The person's email address
     */
    QString emailAddress;
    /**
     * The homepage or a relevant link
     */
    QString webAddress;
};

/**
 * This class is used to store information about a program. It can store
 * such values a version number, program name, homepage, email address
 * for bug reporting, multiple authors and contributors
 * (using @ref KAboutPerson), license and copyright information.
 *
 * Currently, the values set here are shown by the "About" box
 * (@see KAboutDialog), used by the bug report dialog (@see KBugReport),
 * and by the help shown on command line (@see KCmdLineArguments).
 *
 * @short Class holding information needed by the "About" box and other
 * classes.
 * @author Espen Sand (espen@kde.org), David Faure (faure@kde.org)
 * @version $Id$
 */
class KAboutData
{
  public:
    enum LicenseKey
    {
      GPL  = 1,
      LGPL = 2,
      BSD  = 3,
      Artistic = 4
    };

  public:
    /**
     * Constructor.
     *
     * @param programName The program name string. Example : i18n("KEdit")
     *
     * @param version The program version string.
     *
     * @param shortDescription A short description of what the program does
     *
     * @param licenseType The license identifier. Use @ref setLicenseText if
     *        you use a license not predefined here.
     *
     * @param copyrightStatement A copyright statement, that can look like this:
     *        "(c) 1999-2000, Name". The string specified here is not modified
     *        in any manner. The author information from @ref addAuthor is not
     *        used.
     *
     * @param webAddress The program homepage string.
     *        Start the address with "http://". "http://some.domain" is
     *        is correct, "some.domain" is not.
     *
     * @param text Some free form text, that can contain any kind of
     *        information. The text can contain newlines.
     *
     * @param bugsEmailAddress The bug report email address string.
     *
     */
    KAboutData( const QString &programName,
		const QString &version,
		const QString &shortDescription = QString::null,
		int licenseType = 0,
		const QString &copyrightStatement = QString::null,
		const QString &text = QString::null,
		const QString &homePageAddress = QString::null,
		const QString &bugsEmailAddress = "submit@bugs.kde.org"
		);

    /**
     * Defines an author. You can call this function as many times you
     * need. Each entry is appended to a list. The person in the first entry
     * is assumed to be the leader of the project.
     *
     * @param name The developer's name.
     *
     * @param task What the person is resposible for. This text can contain
     *             newlines.
     *
     * @param emailAddress An Email address when the person can be reached.
     *
     * @param webAddress The person's homepage or a relevant link.
     *        Start the address with "http://". "http://some.domain" is
     *        is correct, "some.domain" is not.
     *
     */
    void addAuthor( const QString &name,
		    const QString &task=QString::null,
		    const QString &emailAddress=QString::null,
		    const QString &webAddress=QString::null );

    /**
     * Defines a person that deserves credit. You can call this function
     * as many times you need. Each entry is appended to a list.
     *
     * @param name The person's name.
     *
     * @param task What the person has done to deserve the hounor. The
     *        text can contain newlines.
     *
     * @param emailAddress An Email address when the person can be reached.
     *
     * @param webAddress The person's homepage or a relevant link.
     *        Start the address with "http://". "http://some.domain" is
     *        is correct, "some.domain" is not.
     *
     */
    void addCredit( const QString &name,
                    const QString &task=QString::null,
		    const QString &emailAddress=QString::null,
		    const QString &webAddress=QString::null );

    /**
     * Defines a licence text. Normally you should set a
     * value in the constructor which provides a set of standard licenses.
     * If you do so, any text defined with this function is ignored.
     *
     * @param license The license text.
     */
    void setLicenseText( const QString &license );

    /**
     * @return the program name (usually translated).
     */
    QString programName() const { return mProgramName; }

    /**
     * @return the version string.
     */
    QString version() const { return mVersion; }

    /**
     * @return the short description.
     */
    QString shortDescription() const { return mShortDescription; }

    /**
     * @return the application homepage URL.
     */
    QString homepage() const { return mHomepageAddress; }

    /**
     * @return the email address where to report bugs.
     */
    QString bugAddress() const { return mBugEmailAddress; }

    /**
     * @return author information (list of persons).
     */
    const QValueList<KAboutPerson> authors() const;

    /**
     * @return credit information (list of persons).
     */
    const QValueList<KAboutPerson> credits() const;

    /**
     * @return the free form text.
     */
    QString otherText() const { return mOtherText; }

    /**
     * Returns the license. If @ref setLicenseType has been
     * used, any text defined by @ref setLicenseText is ignored,
     * and the standard text for the chosen license will be returned.
     *
     * @return The license text.
     */
    QString license() const;

    /**
     * @return the copyright statement.
     */
    QString copyrightStatement() const { return( mCopyrightStatement ); }

  private:
    QString mProgramName;
    QString mVersion;
    QString mShortDescription;
    int mLicenseKey;
    QString mCopyrightStatement;
    QString mOtherText;
    QString mHomepageAddress;
    QString mBugEmailAddress;
    QValueList<KAboutPerson> mAuthorList;
    QValueList<KAboutPerson> mCreditList;
    QString mLicenseText;
};


#endif

