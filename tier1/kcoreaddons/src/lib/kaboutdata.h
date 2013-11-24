/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Espen Sand (espen@kde.org)
 * Copyright (C) 2008 Friedrich W. H. Kossebau <kossebau@kde.org>
 * Copyright (C) 2010 Teo Mrnjavac <teo@kde.org>
 * Copyright (C) 2013 David Faure <faure+bluesystems@kde.org>
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

#include <kcoreaddons_export.h>
#include <QtCore/QString>
#include <QtCore/QSharedDataPointer>

template <class T> class QList;
class QVariant;
class KAboutData;
namespace KCrash { void defaultCrashHandler(int sig); }

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
 * KAboutData about("khello", "khello", i18n("KHello"), "0.1",
 *                   i18n("A KDE version of Hello, world!"),
 *                   KAboutData::License_LGPL,
 *                   i18n("Copyright (C) 2003 Developer"));
 *
 * about.addAuthor(i18n("Joe Developer"), i18n("developer"), "joe@host.com", 0);
 * QList<KAboutPerson> people = about.authors();
 * about.addCredit(people[0].name(), people[0].task());
 * @endcode
 */
class KCOREADDONS_EXPORT KAboutPerson
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
     *
     * @param ocsUsername Open Collaboration Services username of the person.
     */
    explicit KAboutPerson(const QString &name,
                          const QString &task = QString(),
                          const QString &emailAddress = QString(),
                          const QString &webAddress = QString(),
                          const QString &ocsUsername = QString());

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

    /**
     * The person's Open Collaboration Services username
     * @return the persons OCS username (can be QString(), if it has been
     *           constructed with an empty username)
     */
    QString ocsUsername() const;

private:
    /**
     * @internal Used by KAboutData to construct translator data.
     */
    explicit KAboutPerson(const QString &name, const QString &email, bool disambiguation);

    class Private;
    Private *const d;
};

class KAboutLicense;

/**
 * This class is used to store information about a program or plugin.
 * It can store such values as version number, program name, home page, email address
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
class KCOREADDONS_EXPORT KAboutData
{
public:

    /**
     * Returns the KAboutData for the application.
     * This contains information such as authors, license, etc.
     * provided that the main() called setApplicationData.
     */
    static KAboutData applicationData();
    /**
     * Sets the application data for this application.
     */
    static void setApplicationData(const KAboutData &aboutData);

    /**
     * Register the KAboutData information for a plugin.
     * Call this from the constructor of the plugin.
     * This will register the plugin's @p aboutData under the component name
     * that was set in @p aboutData.
     */
    static void registerPluginData(const KAboutData &aboutData);
    /**
     * Return the KAboutData for the given plugin identified by @p componentName.
     */
    static KAboutData *pluginData(const QString &componentName);

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
     * @param componentName The program name or plugin name used internally.
     * Example: "kwrite".
     *
     * @param catalogName The translation catalog name; if null or empty, the
     *        @p componentName will be used. You may want the catalog name to
     *        differ from component name, for example, when you want to group
     *        translations of several smaller utilities under the same catalog.
     *
     * @param displayName A displayable name for the program or plugin. This string
     *        should be translated. Example: i18n("KWrite")
     *
     * @param version The component version string.
     *
     * @param shortDescription A short description of what the component does.
     *        This string should be translated.
     *        Example: i18n("A simple text editor.")
     *
     * @param licenseType The license identifier. Use setLicenseText or
              setLicenseTextFile if you use a license not predefined here.
     *
     * @param copyrightStatement A copyright statement, that can look like this:
     *        i18n("Copyright (C) 1999-2000 Name"). The string specified here is
     *        taken verbatim; the author information from addAuthor is not used.
     *
     * @param otherText Some free form text, that can contain any kind of
     *        information. The text can contain newlines. This string
     *        should be translated.
     *
     * @param homePageAddress The component's homepage string.
     *        Start the address with "http://". "http://some.domain" is
     *        is correct, "some.domain" is not.
     * IMPORTANT: if you set a home page address, this will change the "organization domain"
     * of the application, which is used for automatic D-Bus registration.
     * @see setOrganizationDomain
     *
     * @param bugsEmailAddress The bug report email address string.
     *        This defaults to the kde.org bug system.
     *
     */
    KAboutData( const QString &componentName,
                const QString &catalogName,
                const QString &displayName,
                const QString &version,
                const QString &shortDescription = QString(),
                enum LicenseKey licenseType = License_Unknown,
                const QString &copyrightStatement = QString(),
                const QString &otherText = QString(),
                const QString &homePageAddress = QString(),
                const QString &bugsEmailAddress = QLatin1String("submit@bugs.kde.org")
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
     * @param name The developer's name. It should be translated.
     *
     * @param task What the person is responsible for. This text can contain
     *             newlines. It should be translated.
     *             Can be left empty.
     *
     * @param emailAddress An Email address where the person can be reached.
     *                     Can be left empty.
     *
     * @param webAddress The person's homepage or a relevant link.
     *        Start the address with "http://". "http://some.domain" is
     *        correct, "some.domain" is not. Can be left empty.
     *
     * @param ocsUsername The person's Open Collaboration Services username.
     *        The provider can be optionally specified with @see setOcsProvider.
     *
     */
    KAboutData &addAuthor( const QString &name,
                           const QString &task = QString(),
                           const QString &emailAddress = QString(),
                           const QString &webAddress = QString(),
                           const QString &ocsUsername = QString());


    /**
     * Defines a person that deserves credit.
     *
     * You can call this function as many times as you need. Each entry
     * is appended to a list.
     *
     * @param name The person's name. It should be translated.
     *
     * @param task What the person has done to deserve the honor. The
     *        text can contain newlines. It should be translated.
     *        Can be left empty.
     *
     * @param emailAddress An email address when the person can be reached.
     *        Can be left empty.
     *
     * @param webAddress The person's homepage or a relevant link.
     *        Start the address with "http://". "http://some.domain" is
     *        is correct, "some.domain" is not. Can be left empty.
     *
     * @param ocsUsername The person's Open Collaboration Services username.
     *        The provider can be optionally specified with @see setOcsProvider.
     *
     */
    KAboutData &addCredit(const QString &name,
                          const QString &task = QString(),
                          const QString &emailAddress = QString(),
                          const QString &webAddress = QString(),
                          const QString &ocsUserName = QString());

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
    KAboutData &setTranslator( const QString& name,
                               const QString& emailAddress );

    /**
     * Defines a license text, which is translated.
     *
     * Example:
     * \code
     * setLicenseText( i18n("This is my license") );
     * \endcode
     *
     * @param license The license text.
     */
    KAboutData &setLicenseText( const QString &license );

    /**
     * Adds a license text, which is translated.
     *
     * If there is only one unknown license set, e.g. by using the default
     * parameter in the constructor, that one is replaced.
     *
     * Example:
     * \code
     * addLicenseText( i18n("This is my license") );
     * \endcode
     *
     * @param license The license text.
     * @see setLicenseText, addLicense, addLicenseTextFile
     * @since 4.1
     */
    KAboutData &addLicenseText( const QString &license );

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
     * Defines the component name used internally.
     *
     * @param componentName The application or plugin name. Example: "kate".
     */
    KAboutData &setComponentName(const QString &componentName);

    /**
     * Defines the displayable component name string.
     *
     * @param displayName The display name. This string should be
     *        translated.
     *        Example: i18n("Advanced Text Editor").
     */
    KAboutData &setDisplayName( const QString &displayName );

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
     * Because KAboutData is a core class it cannot use QImage directly,
     * so this is a QVariant that should contain a QImage.
     *
     * @param image logo image.
     * @see programLogo()
    */
    KAboutData &setProgramLogo(const QVariant& image);

    /**
     * Specifies an Open Collaboration Services provider by URL.
     * A provider file must be available for the chosen provider.
     *
     * Use this if you need to override the default provider.
     *
     * If this method is not used, all the KAboutPerson OCS usernames
     * will be used with the openDesktop.org entry from the default
     * provider file.
     *
     * @param providerUrl The provider URL as defined in the provider file.
     */
    KAboutData &setOcsProvider( const QString &providerUrl );

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
     *        be translated. Example: i18n("An advanced text
     *        editor with syntax highlighting support.").
     */
    KAboutData &setShortDescription( const QString &shortDescription );

    /**
     * Defines the translation catalog that the program uses.
     *
     * @param catalogName The translation catalog name.
     */
    KAboutData &setCatalogName(const QString &catalogName);

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
     *        this: i18n("Copyright (C) 1999-2000 Name"). The string specified here is
     *        taken verbatim; the author information from addAuthor is not used.
     */
    KAboutData &setCopyrightStatement( const QString &copyrightStatement );

    /**
     * Defines the additional text to show in the about dialog.
     *
     * @param otherText Some free form text, that can contain any kind of
     *        information. The text can contain newlines. This string
     *        should be translated.
     */
    KAboutData &setOtherText( const QString &otherText );

    /**
     * Defines the program homepage.
     *
     * @param homepage The program homepage string.
     *        Start the address with "http://". "http://kate.kde.org"
     *        is correct but "kate.kde.org" is not.
     */
    KAboutData &setHomepage(const QString &homepage);

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
     * Used by the automatic registration to D-Bus done by KApplication and KUniqueApplication.
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
     * By default it's the componentName, but you can overwrite it here to provide
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
    QString componentName() const;

    /**
     * Returns the application's product name, which will be used in KBugReport
     * dialog. By default it returns componentName(), otherwise the one which is set
     * with setProductName()
     *
     * @return the product name.
     */
    QString productName() const;

    /**
     * Returns the translated program name.
     * @return the program name (translated).
     */
    QString displayName() const;

    /**
     * Returns the domain name of the organization that wrote this application.
     *
     * Used by the automatic registration to D-Bus done by KApplication and KUniqueApplication.
     */
    QString organizationDomain() const;

    /**
     * @internal
     * Provided for use by KCrash
     */
    const char* internalProgramName() const;

    /**
     * Returns the program's icon name.
     *
     * The default value is componentName().
     * Use setProgramIconName() if you need to have an icon
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
     * Because KAboutData is a core class it cannot use QImage directly,
     * so this is a QVariant containing a QImage.
     *
     * @return the program logo data, or a null image if there is
     *         no custom application logo defined.
     */
    QVariant programLogo() const;

    /**
     * Returns the chosen Open Collaboration Services provider URL.
     * @return the provider URL.
     */
    QString ocsProviderUrl() const;

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
     * Setting both to parameters to QString() will cause no message to be
     * displayed at all.  Call unsetCustomAuthorText() to revert to the default
     * message.
     */
    KAboutData &setCustomAuthorText(const QString &plainText,
                                    const QString &richText);

    /**
     * Clears any custom text displayed around the list of authors and falls
     * back to the default message telling users to send bug reports to
     * bugAddress().
     */
    KAboutData &unsetCustomAuthorText();

  private:

    friend void KCrash::defaultCrashHandler(int sig);
    static const KAboutData* applicationDataPointer();


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
 */
class KCOREADDONS_EXPORT KAboutLicense
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
     * @internal Used by KAboutData to construct a KAboutLicense
     */
    explicit KAboutLicense(const KAboutData *aboutData);
    /**
     * @internal Used by KAboutData to construct license by given text
     */
    void setLicenseFromPath(const QString &pathToFile);
    //explicit KAboutLicense( const QString &pathToFile, const KAboutData *aboutData );
    /**
     * @internal Used by KAboutData to construct license by given text
     */
    void setLicenseFromText(const QString &licenseText);
    //explicit KAboutLicense( const QString &licenseText, const KAboutData *aboutData );

    class Private;
    QSharedDataPointer<Private> d;
};

#endif

