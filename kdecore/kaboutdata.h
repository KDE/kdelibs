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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef KABOUTDATA_H
#define KABOUTDATA_H

#include <qstring.h>
#include <kdelibs_export.h>

class QImage;
template <class T> class QList;

/**
 * This structure is used to store information about a person or developer.
 * It can store the person's name, a task, an email address and a
 * link to a home page. This class is intended for use in the
 * KAboutData class, but it can be used elsewhere as well.
 * Normally you should at least define the person's name.
 * Creating a KAboutPerson object by yourself is relatively useless,
 * but the KAboutData methods KAboutData::authors() and KAboutData::credits()
 * return lists of KAboutPerson data objects which you can examine.
 *
 * Example usage within a main(), retrieving the list of people involved
 * with a program and re-using data from one of them:
 *
 * @code
 * KAboutData about("khello", I18N_NOOP("KHello"), "0.1",
 *                   I18N_NOOP("A KDE version of Hello, world!"),
 *                   KAboutData::License_LGPL,
 *                   I18N_NOOP("Copyright (c) 2003 Developer"));
 *
 * about.addAuthor("Joe Developer", I18N_NOOP("developer"), "joe@host.com", 0);
 * QList<KAboutPerson> people = about.authors();
 * about.addCredit(people[0].name(), people[0].task());
 * @endcode
 *
 * @bc KDE4
 */
class KDECORE_EXPORT KAboutPerson
{
public:
    /**
     * Convenience constructor
     *
     * @param name The name of the person.
     *
     * @param task The task of this person. This string should be
     *              marked for translation, e.g. use
     *              I18N_NOOP() on it.
     *
     * @param emailAddress The email address of the person.
     *
     * @param webAddress Home page of the person.
     */
    KAboutPerson( const char *name=0, const char *task=0,
                  const char *emailAddress=0, const char *webAddress=0 );

    /**
     * Copy constructor.  Performs a deep copy.
     * @param other object to copy
     */
    KAboutPerson(const KAboutPerson& other);

    ~KAboutPerson();

    /**
     * Assignment operator.  Performs a deep copy.
     * @param other object to copy
     */
    KAboutPerson& operator=(const KAboutPerson& other);


    /**
     * The person's name
     * @return the person's name (can be QString(), if it has been
     *           constructed with a NULL name)
     */
    QString name() const;

    /**
     * The person's task
     * @return the person's task (can be QString(), if it has been
     *           constructed with a NULL task)
     */
    QString task() const;

    /**
     * The person's email address
     * @return the person's email address (can be QString(), if it has been
     *           constructed with a NULL email)
     */
    QString emailAddress() const;

    /**
     * The home page or a relevant link
     * @return the persons home page (can be QString(), if it has been
     *           constructed with a NULL home page)
     */
    QString webAddress() const;

private:
    class Private;
    Private *const d;
};

/**
 * This structure is used to store information about a translator.
 * It can store the translator's name and an email address.
 * This class is intended for use in the KAboutData class,
 * but it can be used elsewhere as well.
 * Normally you should at least define the translator's name.
 *
 * It's not possible to use KAboutPerson for this, because
 * KAboutPerson stores internally only const char* pointers, but the
 * translator information is generated dynamically from the translation
 * of a dummy string.
*/
class KDECORE_EXPORT KAboutTranslator
{
public:
    /**
     * Convenience constructor
     *
     * @param name The name of the person.
     *
     * @param emailAddress The email address of the person.
     */
    KAboutTranslator(const QString & name=QString(),
                     const QString & emailAddress=QString());

    /**
     * Copy constructor.  Performs a deep copy.
     * @param other object to copy
     */
    KAboutTranslator(const KAboutTranslator& other);

    ~KAboutTranslator();

    /**
     * Assignment operator.  Performs a deep copy.
     * @param other object to copy
     */
    KAboutTranslator& operator=(const KAboutTranslator& other);

    /**
     * The translator's name
     * @return the translators's name (can be QString(), if it has been
     *           constructed with a null name)
     */
    QString name() const;

    /**
     * The translator's email
     * @return the translator's email address (can be QString(), if it has been
     *           constructed with a null email)
     */
    QString emailAddress() const;

private:
    class Private;
    Private *const d;
};


/**
 * This class is used to store information about a program. It can store
 * such values as version number, program name, home page, email address
 * for bug reporting, multiple authors and contributors
 * (using KAboutPerson), license and copyright information.
 *
 * Currently, the values set here are shown by the "About" box
 * (see KAboutDialog), used by the bug report dialog (see KBugReport),
 * and by the help shown on command line (see KCmdLineArgs).
 *
 * @short Holds information needed by the "About" box and other
 * classes.
 * @author Espen Sand (espen@kde.org), David Faure (faure@kde.org)
 */
class KDECORE_EXPORT KAboutData
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
     * @param licenseType The license identifier. Use setLicenseText if
     *        you use a license not predefined here.
     *
     * @param copyrightStatement A copyright statement, that can look like this:
     *        "(c) 1999-2000, Name". The string specified here is not modified
     *        in any manner. The author information from addAuthor is not
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
		enum LicenseKey licenseType = License_Unknown,
		const char *copyrightStatement = 0,
		const char *text = 0,
		const char *homePageAddress = 0,
		const char *bugsEmailAddress = "submit@bugs.kde.org"
		);

    /**
     * Copy constructor.  Performs a deep copy.
     * @param other object to copy
     */
     KAboutData(const KAboutData& other);

    /**
     * Assignment operator.  Performs a deep copy.
     * @param other object to copy
     */
     KAboutData& operator=(const KAboutData& other);

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
     *        correct, "some.domain" is not. Can be 0.
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
     * \code
     * setTranslator(I18N_NOOP2("NAME OF TRANSLATORS","Your names")
     * ,I18N_NOOP2("EMAIL OF TRANSLATORS","Your emails"));
     * \endcode
     *
     * The translator can then translate this dummy text with his name
     * or with a list of names separated with ",".
     * If there is no translation or the application is used with the
     * default language, this function call is ignored.
     *
     * @param name the name of the translator
     * @param emailAddress the email address of the translator
     * @see KAboutTranslator
     */
    void setTranslator(const char *name, const char* emailAddress);

    /**
     * Defines a license text.
     *
     * The text will be translated if it got marked for
     * translations with the I18N_NOOP() macro.
     *
     * Example:
     * \code
     * setLicenseText( I18N_NOOP("This is my license"));
     * \endcode
     *
     * NOTE: No copy of the text is made.
     *
     * @param license The license text in utf8 encoding.
     */
    void setLicenseText( const char *license );

    /**
     * Defines a license text.
     *
     * @param file File containing the license text.
     */
    void setLicenseTextFile( const QString &file );

    /**
     * Defines the program name used internally.
     *
     * @param appName The application name. Example: "kate".
     */
    void setAppName( const char *appName );

    /**
     * Defines the displayable program name string.
     *
     * @param programName The program name. This string should be
     *        marked for translation.
     *        Example: I18N_NOOP("Advanced Text Editor").
     */
    void setProgramName( const char* programName );

    /**
     * Defines the program logo.
     * Use this if you need to have application logo
     * in AboutData other than application icon.
     *
     * @param image logo image.
     * @see programLogo()
     * @since 3.4
    */
    void setProgramLogo(const QImage& image);

    /**
     * Defines the program version string.
     *
     * @param version The program version.
     */
    void setVersion( const char* version );

    /**
     * Defines a short description of what the program does.
     *
     * @param shortDescription The program description This string should be marked
     *        for translation. Example: I18N_NOOP("An advanced text editor
     *        with syntax highlithing support.").
     */
    void setShortDescription( const char *shortDescription );

    /**
     * Defines the license identifier.
     *
     * @param licenseKey The license identifier.
     */
    void setLicense( LicenseKey licenseKey);

    /**
     * Defines the copyright statement to show when displaying the license.
     *
     * @param copyrightStatement A copyright statement, that can look like
     *        this: "(c) 1999-2000, Name". The string specified here is not
     *        modified in any manner. The author information from addAuthor
     *        is not used.
     */
    void setCopyrightStatement( const char *copyrightStatement );

    /**
     * Defines the additional text to show in the about dialog.
     *
     * @param otherText Some free form text, that can contain any kind of
     *        information. The text can contain newlines. This string
     *        should be marked for translation.
     */
    void setOtherText( const char *otherText );

    /**
     * Defines the program homepage.
     *
     * @param homepage The program homepage string.
     *        Start the address with "http://". "http://kate.kde.org" is
     *        is correct, "kde.kde.org" is not.
     */
    void setHomepage( const char *homepage );

    /**
     * Defines the address where bug reports should be sent.
     *
     * @param bugAddress The bug report email address string.
     *        This defaults to the kde.org bug system.
     */
    void setBugAddress( const char *bugAddress );

    /**
     * Defines the product name wich will be used in the KBugReport dialog.
     * By default it's the appName, but you can overwrite it here to provide
     * support for special components e.g. 'product/component' like
     * 'kontact/summary'.
     *
     * @param name The name of product
     */
    void setProductName( const char *name );

    /**
     * Returns the application's internal name.
     * @return the internal program name.
     */
    const char *appName() const;

    /**
     * Returns the application's product name, which will be used in KBugReport
     * dialog. By default it returns appName(), otherwise the one which is set
     * with setProductName()
     *
     * @return the product name.
     */
    const char *productName() const;

    /**
     * Returns the translated program name.
     * @return the program name (translated).
     */
    QString programName() const;

    /**
     * @internal
     */
    const char* internalProgramName() const;

    /**
     * @internal
     */
    void translateInternalProgramName() const;

    /**
     * Returns the program logo image.
     * @return the program logo data or null image if there is
     * no custom application logo defined.
     * @since 3.4
     */
    QImage programLogo() const;

    /**
     * Returns the program's version.
     * @return the version string.
     */
    QString version() const;

    /**
     * @internal
     */
    const char* internalVersion() const;

    /**
     * Returns a short, translated description.
     * @return the short description (translated). Can be
     *         QString() if not set.
     */
    QString shortDescription() const;

    /**
     * Returns the application homepage.
     * @return the application homepage URL. Can be QString() if
     *         not set.
     */
    QString homepage() const;

    /**
     * Returns the email address for bugs.
     * @return the email address where to report bugs.
     */
    QString bugAddress() const;
    
    /**
     * @internal
     */
    const char* internalBugAddress() const;

    /**
     * Returns a list of authors.
     * @return author information (list of persons).
     */
    const QList<KAboutPerson> authors() const;

    /**
     * Returns a list of persons who contributed.
     * @return credit information (list of persons).
     */
    const QList<KAboutPerson> credits() const;

    /**
     * Returns a list of translators.
     * @return translators information (list of persons)
     */
    const QList<KAboutTranslator> translators() const;

    /**
     * Returns a message about the translation team.
     * @return a message about the translation team
     */
    static QString aboutTranslationTeam();

    /**
     * Returns a translated, free form text.
     * @return the free form text (translated). Can be QString() if not set.
     */
    QString otherText() const;

    /**
     * Returns the license. If the licenseType argument of the constructor has been
     * used, any text defined by setLicenseText is ignored,
     * and the standard text for the chosen license will be returned.
     *
     * @return The license text.
     */
    QString license() const;

    /**
     * Returns the copyright statement.
     * @return the copyright statement. Can be QString() if not set.
     */
    QString copyrightStatement() const;

    /**
     * Returns the plain text displayed around the list of authors instead
     * of the default message telling users to send bug reports to bugAddress().
     *
     * @return the plain text displayed around the list of authors instead
     *         of the default message.  Can be QString().
     */
    QString customAuthorPlainText() const;

    /**
     * Returns the rich text displayed around the list of authors instead
     * of the default message telling users to send bug reports to bugAddress().
     *
     * @return the rich text displayed around the list of authors instead
     *         of the default message.  Can be QString().
     */
    QString customAuthorRichText() const;

    /**
     * Returns whether custom text should be displayed around the list of
     * authors.
     *
     * @return whether custom text should be displayed around the list of
     *         authors.
     */
    bool customAuthorTextEnabled() const;

    /**
     * Sets the custom text displayed around the list of authors instead
     * of the default message telling users to send bug reports to bugAddress().
     *
     * @param plainText The plain text.
     * @param richText The rich text.
     *
     * Both parameters can be QString() to not display any message at
     * all.  Call unsetCustomAuthorText() to revert to the default mesage.
     */
    void setCustomAuthorText(const QString &plainText, const QString &richText);

    /**
     * Clears any custom text displayed around the list of authors and falls
     * back to the default message telling users to send bug reports to
     * bugAddress().
     */
    void unsetCustomAuthorText();

  private:

    class Private;
    Private *const d;
};

#endif

