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

class KAboutPersonPrivate;
class KAboutDataPrivate;

/**
 * This structure is used to store information about a person or developer.
 * It can store the person's name, a task, an email address and a
 * link to a home page. This class is intended for use in the
 * @ref KAboutData class, but it can be used elsewhere as well.
 * Normally you should at least define the person's name.
*/
class KAboutPerson
{
public:
    /**
     * Convenience constructor
     *
     * @param name The name of the person.
     *
     * @param task The task of this person. This string should be
     *              marked for translation, e.g.
     *              I18N_NOOP("Task description....")
     *
     * @param emailAddress The email address of the person.
     *
     * @param webAddress Home page of the person.
     */
    KAboutPerson( const char *name, const char *task,
                  const char *emailAddress, const char *webAddress )
    {
      mName = name;
      mTask = task;
      mEmailAddress = emailAddress;
      mWebAddress = webAddress;
    }
    /**
     * @internal - don't use. Required by QValueList
     */
    KAboutPerson() {}

    /**
     * The person's name
     * @return the person's name (can be QString::null, if it has been 
     *           constructed with a null name)
     */
    QString name() const;

    /**
     * The person's task
     * @return the person's task (can be QString::null, if it has been 
     *           constructed with a null task)
     */
    QString task() const;

    /**
     * The person's email address
     * @return the person's email address (can be QString::null, if it has been 
     *           constructed with a null email)
     */
    QString emailAddress() const;

    /**
     * The home page or a relevant link
     * @return the persons home page (can be QString::null, if it has been 
     *           constructed with a null home page)
     */
    QString webAddress() const;

private:
    const char *mName;
    const char *mTask;
    const char *mEmailAddress;
    const char *mWebAddress;

    KAboutPersonPrivate *d;
};

class KAboutTranslatorPrivate;
/**
 * This structure is used to store information about a translator.
 * It can store the translator's name and an email address.
 * This class is intended for use in the @ref KAboutData class, 
 * but it can be used elsewhere as well.
 * Normally you should at least define the translator's name.
 * 
 * It's not possible to use @ref KAboutPerson for this, because
 * @ref KAboutPerson stores internally only const char* pointers, but the
 * translator information is generated dynamically from the translation
 * of a dummy string.
*/
class KAboutTranslator
{
public:
    /**
     * Convenience constructor
     *
     * @param name The name of the person.
     *
     * @param emailAddress The email address of the person.
     */
    KAboutTranslator(const QString & name=QString::null,
                     const QString & emailAddress=QString::null);

    /** 
     * The translator's name
     * @return the translators's name (can be QString::null, if it has been 
     *           constructed with a null name)
     */
    QString name() const;

    /**
     * The translator's email
     * @return the translator's email address (can be QString::null, if it has been 
     *           constructed with a null email)
     */
    QString emailAddress() const;
    
private:
    QString mName;
    QString mEmail;
    KAboutTranslatorPrivate* d;
};


/**
 * This class is used to store information about a program. It can store
 * such values as version number, program name, home page, email address
 * for bug reporting, multiple authors and contributors
 * (using @ref KAboutPerson), license and copyright information.
 *
 * Currently, the values set here are shown by the "About" box
 * (see @ref KAboutDialog), used by the bug report dialog (see @ref KBugReport),
 * and by the help shown on command line (see @ref KCmdLineArgs).
 *
 * @short Holds information needed by the "About" box and other
 * classes.
 * @author Espen Sand (espen@kde.org), David Faure (faure@kde.org)
 * @version $Id$
 */
class KAboutData
{
  public:
  /**
   * Descibes the license of the software.
   */
    enum LicenseKey
    {
      License_Custom = -2,
      License_File = -1,
      License_Unknown = 0,
      License_GPL  = 1,
      License_GPL_V2 = 1,
      License_LGPL = 2,
      License_LGPL_V2 = 2,
      License_BSD  = 3,
      License_Artistic = 4,
      License_QPL = 5,
      License_QPL_V1_0 = 5
    };

  public:
    /**
     * Constructor.
     *
     * @param appName The program name used internally. Example: "kedit"
     *
     * @param programName A displayable program name string. This string
     *        should be marked for translation. Example: I18N_NOOP("KEdit")
     *
     * @param version The program version string.
     *
     * @param shortDescription A short description of what the program does.
     *        This string should be marked for translation.
     *        Example: I18N_NOOP("A simple text editor.")
     *
     * @param licenseType The license identifier. Use @ref setLicenseText if
     *        you use a license not predefined here.
     *
     * @param copyrightStatement A copyright statement, that can look like this:
     *        "(c) 1999-2000, Name". The string specified here is not modified
     *        in any manner. The author information from @ref addAuthor is not
     *        used.
     *
     * @param text Some free form text, that can contain any kind of
     *        information. The text can contain newlines. This string
     *        should be marked for translation.
     *
     * @param homePageAddress The program homepage string.
     *        Start the address with "http://". "http://some.domain" is
     *        is correct, "some.domain" is not.
     *
     * @param bugsEmailAddress The bug report email address string.
     *        This defaults to the kde.org bug system.
     *
     */
    KAboutData( const char *appName,
                const char *programName,
		const char *version,
		const char *shortDescription = 0,
		int licenseType = License_Unknown,
		const char *copyrightStatement = 0,
		const char *text = 0,
		const char *homePageAddress = 0,
		const char *bugsEmailAddress = "submit@bugs.kde.org"
		);

     ~KAboutData();

    /**
     * Defines an author. You can call this function as many times you
     * need. Each entry is appended to a list. The person in the first entry
     * is assumed to be the leader of the project.
     *
     * @param name The developer's name in UTF-8 encoding.
     *
     * @param task What the person is responsible for. This text can contain
     *             newlines. It should be marked for translation like this:
     *             I18N_NOOP("Task description..."). Can be 0.
     *
     * @param emailAddress An Email address where the person can be reached.
     *                     Can be 0.
     *
     * @param webAddress The person's homepage or a relevant link.
     *        Start the address with "http://". "http://some.domain" is
     *        is correct, "some.domain" is not. Can be 0.
     *
     */
    void addAuthor( const char *name,
		    const char *task=0,
		    const char *emailAddress=0,
		    const char *webAddress=0 );

    /**
     * Defines a person that deserves credit. You can call this function
     * as many times you need. Each entry is appended to a list.
     *
     * @param name The person's name in UTF-8 encoding.
     *
     * @param task What the person has done to deserve the honor. The
     *        text can contain newlines. It should be marked for 
     *        translation like this: I18N_NOOP("Task description...")
     *        Can be 0.
     *
     * @param emailAddress An Email address when the person can be reached.
     *        Can be 0.
     *
     * @param webAddress The person's homepage or a relevant link.
     *        Start the address with "http://". "http://some.domain" is
     *        is correct, "some.domain" is not. Can be 0.
     *
     */
    void addCredit( const char *name,
                    const char *task=0,
		    const char *emailAddress=0,
		    const char *webAddress=0 );

    /**
     * Sets the name of the translator of the gui. Since this depends
     * on the language, just use a dummy text marked for translation.
     * 
     * For example:
     * <pre>
     * setTranslator(I18N_NOOP("_: NAME OF TRANSLATORS\nYour names")
     * ,I18N_NOOP("_: EMAIL OF TRANSLATORS\nYour emails"));
     * </pre>
     *
     * The translator can then translate this dummy text with his name
     * or with a list of names separated with ",".
     * If there is no translation or the application is used with the
     * default language, this function call is ignored.
     *
     * Note: If you are using the default KDE automake environment,
     * there is no need to use this function, because the two
     * default strings above are added to the applications po file
     * automatically.
     *
     * @param name the name of the translator
     * @param emailAddress the email address of the translator
     * @see KAboutTranslator
     */
    void setTranslator(const char* name, const char* emailAddress);
    
    /**
     * Defines a licence text.
     *
     * The text will be translated if it got marked for
     * translations with the I18N_NOOP() macro.
     *
     * Example:
     * <pre>
     * setLicenseText( I18N_NOOP("This is my license"));
     * </pre>
     *
     * NOTE: No copy of the text is made.
     *
     * @param license The license text in utf8 encoding.
     */
    void setLicenseText( const char *license );

    /**
     * Defines a licence text. 
     *
     * @param file File containing the license text.
     */
    void setLicenseTextFile( const QString &file );

    /**
     * Returns the application's internal name.
     * @return the internal program name.
     */
    const char *appName() const;

    /**
     * Returns the translated program name.
     * @return the program name (translated).
     */
    QString programName() const;

    /**
     * Returns the program's version.
     * @return the version string.
     */
    QString version() const;

    /**
     * Returns a short, translated description.
     * @return the short description (translated). Can be 
     *         QString::null if not set.
     */
    QString shortDescription() const;

    /**
     * Returns the application homepage.
     * @return the application homepage URL. Can be QString::null if
     *         not set.
     */
    QString homepage() const;

    /**
     * Returns the email address for bugs.
     * @return the email address where to report bugs. 
     */
    QString bugAddress() const;

    /**
     * Returns a list of authors.
     * @return author information (list of persons).
     */
    const QValueList<KAboutPerson> authors() const;

    /**
     * Returns a list of persons who contributed.
     * @return credit information (list of persons).
     */
    const QValueList<KAboutPerson> credits() const;

    /**
     * Returns a list of translators.
     * @return translators information (list of persons)
     */
    const QValueList<KAboutTranslator> translators() const;

    /** 
     * Returns a message about the translation team.
     * @return a message about the translation team 
     */
    static QString aboutTranslationTeam();

    /**
     * Returns a translated, free form text.
     * @return the free form text (translated). Can be QString::null if not set.
     */
    QString otherText() const;

    /**
     * Returns the license. If the licenseType argument of the constructor has been
     * used, any text defined by @ref setLicenseText is ignored,
     * and the standard text for the chosen license will be returned.
     *
     * @return The license text.
     */
    QString license() const;

    /**
     * Returns the copyright statement.
     * @return the copyright statement. Can be QString::null if not set.
     */
    QString copyrightStatement() const { return( QString::fromLatin1(mCopyrightStatement )); }


  private:
    const char *mAppName;
    const char *mProgramName;
    const char *mVersion;
    const char *mShortDescription;
    int mLicenseKey;
    const char *mCopyrightStatement;
    const char *mOtherText;
    const char *mHomepageAddress;
    const char *mBugEmailAddress;
    QValueList<KAboutPerson> mAuthorList;
    QValueList<KAboutPerson> mCreditList;
    const char *mLicenseText;

    KAboutDataPrivate *d;
};

#endif

