/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Espen Sand (espen@kde.org)
 * Copyright (C) 2008 Friedrich W. H. Kossebau <kossebau@kde.org>
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

#include <kdecore_export.h>
#include <klocale.h>
// Qt
#include <QtCore/QString>
#include <QtCore/QSharedDataPointer>

template <class T> class QList;
class QVariant;
class KAboutData;

/**
 * This class is used to store information about a person or developer.
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
 * KAboutData about("khello", "khello", ki18n("KHello"), "0.1",
 *                   ki18n("A KDE version of Hello, world!"),
 *                   KAboutData::License_LGPL,
 *                   ki18n("Copyright (C) 2003 Developer"));
 *
 * about.addAuthor(ki18n("Joe Developer"), ki18n("developer"), "joe@host.com", 0);
 * QList<KAboutPerson> people = about.authors();
 * about.addCredit(people[0].name(), people[0].task());
 * @endcode
 *
 * @note Instead of the more usual i18n calls, for translatable text the ki18n
 * calls are used to produce KLocalizedStrings, which can delay the translation
 * lookup. This is necessary because the translation catalogs are usually not
 * yet initialized at the point where KAboutData is constructed.
 *
 * @bc KDE4
 */
class KDECORE_EXPORT KAboutPerson
{
    friend class KAboutData;
public:
    /**
     * Convenience constructor
     *
     * @param name The name of the person.
     *
     * @param task The task of this person.
     *
     * @param emailAddress The email address of the person.
     *
     * @param webAddress Home page of the person.
     */
    explicit KAboutPerson( const KLocalizedString &name,
                           const KLocalizedString &task = KLocalizedString(),
                           const QByteArray &emailAddress = QByteArray(),
                           const QByteArray &webAddress = QByteArray() );

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
     *           constructed with an empty name)
     */
    QString name() const;

    /**
     * The person's task
     * @return the person's task (can be QString(), if it has been
     *           constructed with an empty task)
     */
    QString task() const;

    /**
     * The person's email address
     * @return the person's email address (can be QString(), if it has been
     *           constructed with an empty email)
     */
    QString emailAddress() const;

    /**
     * The home page or a relevant link
     * @return the persons home page (can be QString(), if it has been
     *           constructed with an empty home page)
     */
    QString webAddress() const;

private:
    /**
     * @internal Used by KAboutData to construct translator data.
     */
    explicit KAboutPerson( const QString &name, const QString &email );

    class Private;
    Private *const d;
};

class KAboutLicense;

// KDE5: refactor together with KComponentData.
// Like changing all property names which contain Program or App.

/**
 * This class is used to store information about a program. It can store
 * such values as version number, program name, home page, email address
 * for bug reporting, multiple authors and contributors
 * (using KAboutPerson), license and copyright information.
 *
 * Currently, the values set here are shown by the "About" box
 * (see KAboutDialog), used by the bug report dialog (see KBugReport),
 * and by the help shown on command line (see KCmdLineArgs).
 * They are also used for the icon and the name of the program's windows.
 *
 * @note Instead of the more usual i18n calls, for translatable text the ki18n
 * calls are used to produce KLocalizedStrings, which can delay the translation
 * lookup. This is necessary because the translation catalogs are usually not
 * yet initialized at the point where KAboutData is constructed.
 *
 * @short Holds information needed by the "About" box and other
 * classes.
 * @author Espen Sand (espen@kde.org), David Faure (faure@kde.org)
 */
class KDECORE_EXPORT KAboutData
{
  public:
  /**
   * Describes the license of the software.
   */
    enum LicenseKey // KDE5: move to KAboutLicense, cut License_ prefix
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
      License_QPL_V1_0 = 5,
      License_GPL_V3 = 6,
      License_LGPL_V3 = 7
    };

  /**
   * Format of the license name.
   */
    enum NameFormat // KDE5: move to KAboutLicense
    {
        ShortName,
        FullName
    };

  public:
    /**
     * Constructor.
     *
     * @param appName The program name used internally. Example: "kedit"
     *
     * @param catalogName The translation catalog name; if null or empty, the
     *        @p appName will be used. You may want the catalog name to
     *        differ from program name, for example, when you want to group
     *        translations of several smaller utilities under the same catalog.
     *
     * @param programName A displayable program name string. This string
     *        should be marked for translation. Example: ki18n("KEdit")
     *
     * @param version The program version string.
     *
     * @param shortDescription A short description of what the program does.
     *        This string should be marked for translation.
     *        Example: ki18n("A simple text editor.")
     *
     * @param licenseType The license identifier. Use setLicenseText or
              setLicenseTextFile if you use a license not predefined here.
     *
     * @param copyrightStatement A copyright statement, that can look like this:
     *        ki18n("Copyright (C) 1999-2000 Name"). The string specified here is
     *        taken verbatim; the author information from addAuthor is not used.
     *
     * @param otherText Some free form text, that can contain any kind of
     *        information. The text can contain newlines. This string
     *        should be marked for translation.
     *
     * @param homePageAddress The program homepage string.
     *        Start the address with "http://". "http://some.domain" is
     *        is correct, "some.domain" is not.
     * IMPORTANT: if you set a home page address, this will change the "organization domain"
     * of the application, which is used for automatic DBUS registration.
     * @see setOrganizationDomain
     *
     * @param bugsEmailAddress The bug report email address string.
     *        This defaults to the kde.org bug system.
     *
     */
    KAboutData( const QByteArray &appName,
                const QByteArray &catalogName,
                const KLocalizedString &programName,
                const QByteArray &version,
                const KLocalizedString &shortDescription = KLocalizedString(),
                enum LicenseKey licenseType = License_Unknown,
                const KLocalizedString &copyrightStatement = KLocalizedString(),
                const KLocalizedString &otherText = KLocalizedString(),
                const QByteArray &homePageAddress = QByteArray(),
                const QByteArray &bugsEmailAddress = "submit@bugs.kde.org"
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
     * Defines an author.
     *
     * You can call this function as many times as you need. Each entry is
     * appended to a list. The person in the first entry is assumed to be
     * the leader of the project.
     *
     * @param name The developer's name. It should be marked for translation
     *             like this: ki18n("Developer Name")
     *
     * @param task What the person is responsible for. This text can contain
     *             newlines. It should be marked for translation like this:
     *             ki18n("Task description..."). Can be left empty.
     *
     * @param emailAddress An Email address where the person can be reached.
     *                     Can be left empty.
     *
     * @param webAddress The person's homepage or a relevant link.
     *        Start the address with "http://". "http://some.domain" is
     *        correct, "some.domain" is not. Can be left empty.
     *
     */
    KAboutData &addAuthor( const KLocalizedString &name,
                           const KLocalizedString &task = KLocalizedString(),
                           const QByteArray &emailAddress = QByteArray(),
                           const QByteArray &webAddress = QByteArray() );

    /**
     * Defines a person that deserves credit.
     *
     * You can call this function as many times as you need. Each entry
     * is appended to a list.
     *
     * @param name The person's name. It should be marked for translation
     *             like this: ki18n("Contributor Name")
     *
     * @param task What the person has done to deserve the honor. The
     *        text can contain newlines. It should be marked for
     *        translation like this: ki18n("Task description...")
     *        Can be left empty.
     *
     * @param emailAddress An email address when the person can be reached.
     *        Can be left empty.
     *
     * @param webAddress The person's homepage or a relevant link.
     *        Start the address with "http://". "http://some.domain" is
     *        is correct, "some.domain" is not. Can be left empty.
     *
     */
    KAboutData &addCredit( const KLocalizedString &name,
                           const KLocalizedString &task = KLocalizedString(),
                           const QByteArray &emailAddress = QByteArray(),
                           const QByteArray &webAddress = QByteArray() );

    /**
     * @brief Sets the name(s) of the translator(s) of the GUI.
     *
     * Since this depends on the language, just use a dummy text marked for
     * translation.
     *
     * The canonical use is:
     *
     * \code
     * setTranslator(ki18nc("NAME OF TRANSLATORS", "Your names"),
     *               ki18nc("EMAIL OF TRANSLATORS", "Your emails"));
     * \endcode
     *
     * The translator can then translate this dummy text with his name
     * or with a list of names separated with ",".
     * If there is no translation or the application is used with the
     * default language, this function call is ignored.
     *
     * @param name the name(s) of the translator(s)
     * @param emailAddress the email address(es) of the translator(s)
     * @see KAboutTranslator
     */
    KAboutData &setTranslator( const KLocalizedString& name,
                               const KLocalizedString& emailAddress );

    /**
     * Defines a license text, which is marked for translation.
     *
     * Example:
     * \code
     * setLicenseText( ki18n("This is my license") );
     * \endcode
     *
     * @param license The license text.
     */
    KAboutData &setLicenseText( const KLocalizedString &license );

    /**
     * Adds a license text, which is marked for translation.
     *
     * If there is only one unknown license set, e.g. by using the default
     * parameter in the constructor, that one is replaced.
     *
     * Example:
     * \code
     * addLicenseText( ki18n("This is my license") );
     * \endcode
     *
     * @param license The license text.
     * @see setLicenseText, addLicense, addLicenseTextFile
     * @since 4.1
     */
    KAboutData &addLicenseText( const KLocalizedString &license );

    /**
     * Defines a license text by pointing to a file where it resides.
     * The file format has to be plain text in an encoding compatible to the locale.
     *
     * @param file Path to the file in the local filesystem containing the license text.
     */
    KAboutData &setLicenseTextFile( const QString &file );

    /**
     * Adds a license text by pointing to a file where it resides.
     * The file format has to be plain text in an encoding compatible to the locale.
     *
     * If there is only one unknown license set, e.g. by using the default
     * parameter in the constructor, that one is replaced.
     *
     * @param file Path to the file in the local filesystem containing the license text.
     * @see addLicenseText, addLicense, setLicenseTextFile
     * @since 4.1
     */
    KAboutData &addLicenseTextFile( const QString &file );

    /**
     * Defines the program name used internally.
     *
     * @param appName The application name. Example: "kate".
     */
    KAboutData &setAppName( const QByteArray &appName );

    /**
     * Defines the displayable program name string.
     *
     * @param programName The program name. This string should be
     *        marked for translation.
     *        Example: ki18n("Advanced Text Editor").
     */
    KAboutData &setProgramName( const KLocalizedString &programName );

    /**
     * Defines the program icon.
     *
     * Use this if you need to have an application icon
     * whose name is different than the application name.
     *
     * @param iconName name of the icon. Example: "accessories-text-editor"
     * @see programIconName()
     * @since 4.1
     */
    KAboutData &setProgramIconName( const QString &iconName );

    /**
     * Defines the program logo.
     *
     * Use this if you need to have an application logo
     * in AboutData other than the application icon.
     *
     * Because KAboutData is in kdecore it cannot use QImage directly,
     * so this is a QVariant that should contain a QImage.
     *
     * @param image logo image.
     * @see programLogo()
    */
    KAboutData &setProgramLogo(const QVariant& image);

    /**
     * Defines the program version string.
     *
     * @param version The program version.
     */
    KAboutData &setVersion( const QByteArray &version );

    /**
     * Defines a short description of what the program does.
     *
     * @param shortDescription The program description. This string should
     *        be marked for translation. Example: ki18n("An advanced text
     *        editor with syntax highlighting support.").
     */
    KAboutData &setShortDescription( const KLocalizedString &shortDescription );

    /**
     * Defines the translation catalog that the program uses.
     *
     * @param catalogName The translation catalog name.
     */
    KAboutData &setCatalogName( const QByteArray &catalogName );

    /**
     * Defines the license identifier.
     *
     * @param licenseKey The license identifier.
     * @see addLicenseText, setLicenseText, setLicenseTextFile
     */
    KAboutData &setLicense( LicenseKey licenseKey );

    /**
     * Adds a license identifier.
     *
     * If there is only one unknown license set, e.g. by using the default
     * parameter in the constructor, that one is replaced.
     *
     * @param licenseKey The license identifier.
     * @see setLicenseText, addLicenseText, addLicenseTextFile
     * @since 4.1
     */
    KAboutData &addLicense( LicenseKey licenseKey );

    /**
     * Defines the copyright statement to show when displaying the license.
     *
     * @param copyrightStatement A copyright statement, that can look like
     *        this: ki18n("Copyright (C) 1999-2000 Name"). The string specified here is
     *        taken verbatim; the author information from addAuthor is not used.
     */
    KAboutData &setCopyrightStatement( const KLocalizedString &copyrightStatement );

    /**
     * Defines the additional text to show in the about dialog.
     *
     * @param otherText Some free form text, that can contain any kind of
     *        information. The text can contain newlines. This string
     *        should be marked for translation.
     */
    KAboutData &setOtherText( const KLocalizedString &otherText );

    /**
     * Defines the program homepage.
     *
     * @param homepage The program homepage string.
     *        Start the address with "http://". "http://kate.kde.org"
     *        is correct but "kate.kde.org" is not.
     */
    KAboutData &setHomepage( const QByteArray &homepage );

    /**
     * Defines the address where bug reports should be sent.
     *
     * @param bugAddress The bug report email address string.
     *        This defaults to the kde.org bug system.
     */
    KAboutData &setBugAddress( const QByteArray &bugAddress );

    /**
     * Defines the Internet domain of the organization that wrote this application.
     * The domain is set to kde.org by default, or the domain of the homePageAddress constructor argument,
     * if set.
     *
     * Make sure to call setOrganizationDomain if your product is developed out of the
     * kde.org version-control system.
     *
     * Used by the automatic registration to DBus done by KApplication and KUniqueApplication.
     *
     * IMPORTANT: if the organization domain is set, the .desktop file that describes your
     * application should have an entry like X-DBUS-ServiceName=reversed_domain.kmyapp
     * For instance kwrite passes "http://www.kate-editor.org" as the homePageAddress so it needs
     * X-DBUS-ServiceName=org.kate-editor.kwrite in its kwrite.desktop file.
     *
     * @param domain the domain name, for instance kde.org, koffice.org, kdevelop.org, etc.
     */
    KAboutData &setOrganizationDomain( const QByteArray &domain );

    /**
     * Defines the product name which will be used in the KBugReport dialog.
     * By default it's the appName, but you can overwrite it here to provide
     * support for special components e.g. in the form 'product/component',
     * such as 'kontact/summary'.
     *
     * @param name The name of product
     */
    KAboutData &setProductName( const QByteArray &name );

    /**
     * Returns the application's internal name.
     * @return the internal program name.
     */
    QString appName() const;

    /**
     * Returns the application's product name, which will be used in KBugReport
     * dialog. By default it returns appName(), otherwise the one which is set
     * with setProductName()
     *
     * @return the product name.
     */
    QString productName() const;

    /**
     * Returns the translated program name.
     * @return the program name (translated).
     */
    QString programName() const;

    /**
     * Returns the domain name of the organization that wrote this application.
     *
     * Used by the automatic registration to DBus done by KApplication and KUniqueApplication.
     */
    QString organizationDomain() const;

    /**
     * @internal
     * Provided for use by KCrash
     */
    const char* internalProgramName() const;

    /**
     * @internal
     * Provided for use by KCrash
     */
    void translateInternalProgramName() const;

    /**
     * Returns the program's icon name.
     *
     * The default value is @p appName .
     * Use @p setProgramIconName if you need to have an icon
     * whose name is different from the internal application name.
     *
     * @return the program's icon name.
     * @see setProgramIconName()
     * @since 4.1
     */
    QString programIconName() const;

    /**
     * Returns the program logo image.
     *
     * Because KAboutData is in kdecore it cannot use QImage directly,
     * so this is a QVariant containing a QImage.
     *
     * @return the program logo data, or a null image if there is
     *         no custom application logo defined.
     */
    QVariant programLogo() const;

    /**
     * Returns the program's version.
     * @return the version string.
     */
    QString version() const;

    /**
     * @internal
     * Provided for use by KCrash
     */
    const char* internalVersion() const;

    /**
     * Returns a short, translated description.
     * @return the short description (translated). Can be
     *         QString() if not set.
     */
    QString shortDescription() const;

    /**
     * Returns the program's translation catalog name.
     * @return the catalog name.
     */
    QString catalogName() const;

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
     * Provided for use by KCrash
     */
    const char* internalBugAddress() const;

    /**
     * Returns a list of authors.
     * @return author information (list of persons).
     */
    QList<KAboutPerson> authors() const;

    /**
     * Returns a list of persons who contributed.
     * @return credit information (list of persons).
     */
    QList<KAboutPerson> credits() const;

    /**
     * Returns a list of translators.
     * @return translators information (list of persons)
     */
    QList<KAboutPerson> translators() const;

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
     *
     * @deprecated There could be multiple licenses, use licenses() instead.
     */
    QString license() const;

    /**
     * Returns the license name.
     *
     * @return The license name as a string.
     *
     * @deprecated There could be multiple licenses, use licenses() instead.
     */
    QString licenseName(NameFormat formatName) const;

    /**
     * Returns a list of licenses.
     *
     * @return licenses information (list of licenses)
     * @since 4.1
     */
    QList<KAboutLicense> licenses() const;

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
     * Setting both to parameters to KLocalizedString() will cause no message to be
     * displayed at all.  Call unsetCustomAuthorText() to revert to the default
     * message.
     */
    KAboutData &setCustomAuthorText(const KLocalizedString &plainText,
                                    const KLocalizedString &richText);

    /**
     * Clears any custom text displayed around the list of authors and falls
     * back to the default message telling users to send bug reports to
     * bugAddress().
     */
    KAboutData &unsetCustomAuthorText();

  private:

    class Private;
    Private *const d;
};


/**
 * This class is used to store information about a license.
 * The license can be one of some predefined, one given as text or one
 * that can be loaded from a file. This class is used in the KAboutData class.
 * Explicitly creating a KAboutLicense object is not possible.
 * If the license is wanted for a KDE component having KAboutData object,
 * use KAboutData::licenses() to get the licenses for that component.
 * If the license is for a non-code resource and given by a keyword
 * (e.g. in .desktop files), try using KAboutLicense::byKeyword().
 *
 * @note Instead of the more usual i18n calls, for translatable text the ki18n
 * calls are used to produce KLocalizedStrings, which can delay the translation
 * lookup. This is necessary because the translation catalogs are usually not
 * yet initialized at the point where KAboutData is constructed.
 */
class KDECORE_EXPORT KAboutLicense
{
    friend class KAboutData;
public:
    /**
     * Copy constructor.  Performs a deep copy.
     * @param other object to copy
     */
    KAboutLicense(const KAboutLicense& other);

    ~KAboutLicense();

    /**
     * Assignment operator.  Performs a deep copy.
     * @param other object to copy
     */
    KAboutLicense& operator=(const KAboutLicense& other);


    /**
     * Returns the full license text. If the licenseType argument of the
     * constructor has been used, any text defined by setLicenseText is ignored,
     * and the standard text for the chosen license will be returned.
     *
     * @return The license text.
     */
    QString text() const;

    /**
     * Returns the license name.
     *
     * @return The license name as a string.
     */
    QString name(KAboutData::NameFormat formatName) const;

    /**
     * Returns the license key.
     *
     * @return The license key as element of KAboutData::LicenseKey enum.
     * @since 4.1
     */
    KAboutData::LicenseKey key() const;

    /**
     * Fetch a known license by a keyword.
     *
     * Frequently the license data is provided by a terse keyword-like string,
     * e.g. by a field in a .desktop file. Using this method, an application
     * can get hold of a proper KAboutLicense object, providing that the
     * license is one of the several known to KDE, and use it to present
     * more human-readable information to the user.
     *
     * Keywords are matched by stripping all whitespace and lowercasing.
     * The known keywords correspond to the KAboutData::LicenseKey enumeration,
     * e.g. any of "LGPLV3", "LGPLv3", "LGPL v3" would match License_LGPL_V3.
     * If there is no match for the keyword, a valid license object is still
     * returned, with its name and text informing about a custom license,
     * and its key equal to KAboutData::License_Custom.
     *
     * @param keyword The license keyword.
     * @return The license object.
     *
     * @see KAboutData::LicenseKey
     * @since 4.1
     */
    static KAboutLicense byKeyword(const QString &keyword);

private:
    /**
     * @internal Used by KAboutData to construct a predefined license.
     */
    explicit KAboutLicense( enum KAboutData::LicenseKey licenseType, const KAboutData *aboutData );
    /**
     * @internal Used by KAboutData to construct license by given text
     */
    explicit KAboutLicense( const QString &pathToFile, const KAboutData *aboutData );
    /**
     * @internal Used by KAboutData to construct license by given text
     */
    explicit KAboutLicense( const KLocalizedString &licenseText, const KAboutData *aboutData );

    class Private;
    QSharedDataPointer<Private> d;
};

#endif

