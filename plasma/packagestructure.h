/******************************************************************************
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                        *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#ifndef PLASMA_PACKAGESTRUCTURE_H
#define PLASMA_PACKAGESTRUCTURE_H

#include <QtCore/QStringList>
#include <QtCore/QSharedData>

#include <kgenericfactory.h>
#include <klocale.h>
#include <kplugininfo.h>
#include <ksharedptr.h>

#include <plasma/version.h>

class KConfigBase;

namespace Plasma
{

class PackageStructurePrivate;

/**
 * @class PackageStructure plasma/packagestructure.h <Plasma/PackageStructure>
 *
 * @short A description of the expected file structure of a given package type
 *
 * PackageStructure defines what is in a package. This information is used
 * to create packages and provides a way to programatically refer to contents.
 *
 * An example usage of this class might be:
 *
 @code
    PackageStructure structure;

    structure.addDirectoryDefinition("images", "pics/", i18n("Images"));
    QStringList mimeTypes;
    mimeTypes << "image/svg" << "image/png" << "image/jpeg";
    structure.setMimeTypes("images", mimeTypes);

    structure.addDirectoryDefinition("scripts", "code/", i18n("Executable Scripts"));
    mimeTypes.clear();
    mimeTypes << "text/\*";
    structure.setMimeTypes("scripts", mimeTypes);

    structure.addFileDefinition("mainscript", "code/main.js", i18n("Main Script File"));
    structure.setRequired("mainscript", true);
 @endcode
 * One may also choose to create a subclass of PackageStructure and include the setup
 * in the constructor.
 *
 * Either way, PackageStructure creates a sort of "contract" between the packager and
 * the application which is also self-documenting.
 **/
class PLASMA_EXPORT PackageStructure : public QObject, public QSharedData
{
    Q_OBJECT

public:
    typedef KSharedPtr<PackageStructure> Ptr;

    /**
     * Default constructor for a package structure definition
     *
     * @arg type the type of package. This is often application specific.
     **/
    explicit PackageStructure(QObject *parent = 0,
                              const QString &type = i18nc("A non-functional package", "Invalid"));

    /**
     * Destructor
     **/
    virtual ~PackageStructure();

    /**
     * Assignment operator
     **/
    PackageStructure &operator=(const PackageStructure &rhs);

    /**
     * Loads a package format by name.
     *
     * @arg format If not empty, attempts to locate the given format, either
     *             from built-ins or via plugins.
     * @return a package that matches the format, if available. The caller
     *         is responsible for deleting the object.
     */
    static PackageStructure::Ptr load(const QString &packageFormat);

    /**
     * Type of package this structure describes
     **/
    QString type() const;

    /**
     * The directories defined for this package
     **/
    QList<const char*> directories() const;

    /**
     * The required directories defined for this package
     **/
    QList<const char*> requiredDirectories() const;

    /**
     * The individual files, by key, that are defined for this package
     **/
    QList<const char*> files() const;

    /**
     * The individual required files, by key, that are defined for this package
     **/
    QList<const char*> requiredFiles() const;

    /**
     * Adds a directory to the structure of the package. It is added as
     * a not-required element with no associated mimeTypes.
     *
     * Starting in 4.6, if an entry with the given key
     * already exists, the path is added to it as a search alternative.
     *
     * @param key  used as an internal label for this directory
     * @param path the path within the package for this directory
     * @param name the user visible (translated) name for the directory
     **/
    void addDirectoryDefinition(const char *key, const QString &path, const QString &name);

    /**
     * Adds a file to the structure of the package. It is added as
     * a not-required element with no associated mimeTypes.
     *
     * Starting in 4.6, if an entry with the given key
     * already exists, the path is added to it as a search alternative.
     *
     * @param key  used as an internal label for this file
     * @param path the path within the package for this file
     * @param name the user visible (translated) name for the file
     **/
    void addFileDefinition(const char *key, const QString &path, const QString &name);

    /**
     * Removes a definition from the structure of the package.
     * @since 4.6
     * @param key the internal label of the file or directory to remove
     */
    void removeDefinition(const char *key);

    /**
     * @return path relative to the package root for the given entry
     * @deprecatd use searchPaths instead
     **/
    QString path(const char *key) const;

    /**
     * @return a list of paths relative to the package root for the given entry.
     *         They are orted by importance: when searching for a file the paths
     *         will be searched in order
     * @since 4.6
     **/
    QStringList searchPath(const char *key) const;

    /**
     * Get the list of files of a given type.
     *
     * @arg key the type of file to look for
     * @return list of files by name
     * @since 4.3
     */
    QStringList entryList(const char *key);

    /**
     * @return user visible name for the given entry
     **/
    QString name(const char *key) const;

    /**
     * Sets whether or not a given part of the structure is required or not.
     * The path must already have been added using addDirectoryDefinition
     * or addFileDefinition.
     *
     * @param key the entry within the package
     * @param required true if this entry is required, false if not
     */
    void setRequired(const char *key, bool required);

    /**
     * @return true if the item at path exists and is required
     **/
    bool isRequired(const char *key) const;

    /**
     * Defines the default mimeTypes for any definitions that do not have
     * associated mimeTypes. Handy for packages with only one or predominantly
     * one file type.
     *
     * @param mimeTypes a list of mimeTypes
     **/
    void setDefaultMimeTypes(QStringList mimeTypes);

    /**
     * Define mimeTypes for a given part of the structure
     * The path must already have been added using addDirectoryDefinition
     * or addFileDefinition.
     *
     * @param key the entry within the package
     * @param mimeTypes a list of mimeTypes
     **/
    void setMimeTypes(const char *key, QStringList mimeTypes);

    /**
     * @return the mimeTypes associated with the path, if any
     **/
    QStringList mimeTypes(const char *key) const;

    /**
     * Sets the path to the package. Useful for package formats
     * which do not have well defined contents prior to installation.
     */
    void setPath(const QString &path);

    /**
     * @return the path to the package, or QString() if none
     */
    QString path() const;

    /**
     * Read a package structure from a config file.
     */
    void read(const KConfigBase *config);

    /**
     * Write this package structure to a config file.
     */
    void write(KConfigBase *config) const;

    /**
     * Installs a package matching this package structure. By default installs a
     * native Plasma::Package.
     *
     * @param archivePath path to the package archive file
     * @param packageRoot path to the directory where the package should be
     *                    installed to
     * @return true on successful installation, false otherwise
     **/
    virtual bool installPackage(const QString &archivePath, const QString &packageRoot);

    /**
     * Uninstalls a package matching this package structure.
     *
     * @arg packageName the name of the package to remove
     * @arg packageRoot path to the directory where the package should be installed to
     * @return true on successful removal of the package, false otherwise
     */
    virtual bool uninstallPackage(const QString &packageName, const QString &packageRoot);

    /**
     * When called, the package plugin should display a window to the user
     * that they can use to browser, select and then install widgets supported by
     * this package plugin with.
     *
     * The user interface may be an in-process dialog or an out-of-process application.
     *
     * When the process is complete, the newWidgetBrowserFinished() signal must be
     * emitted.
     *
     * @param parent the parent widget to use for the widget
     */
    virtual void createNewWidgetBrowser(QWidget *parent = 0);

    /**
     * @return the prefix paths inserted between the base path and content entries, in order of priority.
     *         When searching for a file, all paths will be tried in order.
     * @since 4.6
     */
    QStringList contentsPrefixPaths() const;

    /**
     * @return preferred package root. This defaults to plasma/plasmoids/
     */
    QString defaultPackageRoot() const;

    /**
     * @return service prefix used in desktop files. This defaults to plasma-applet-
     */
    QString servicePrefix() const;

    /**
     * Sets service prefix.
     */
    void setServicePrefix(const QString &servicePrefix);

    /**
      * @return the package metadata object.
      */
    virtual KPluginInfo metadata() const;

    /**
     * @return true if paths/symlinks outside the package itself should be followed.
     * By default this is set to false for security reasons.
     */
    bool allowExternalPaths() const;

Q_SIGNALS:
    /**
     * Emitted when the new widget browser process completes.
     */
    void newWidgetBrowserFinished();

protected:
    /**
     * Sets whether or not external paths/symlinks can be followed by a package
     * @arg allow true if paths/symlinks outside of the package should be followed,
     *             false if they should be rejected.
     */
    void setAllowExternalPaths(bool allow);

    /**
     * Sets the prefixes that all the contents in this package should
     * appear under. This defaults to "contents/" and is added automatically
     * between the base path and the entries as defined by the package
     * structure. Multiple entries can be added.
     * In this case each file request will be searched in all prefixes in order,
     * and the first found will be returned.
     *
     * @arg prefix paths the directory prefix to use
     * @since 4.6
     */
    void setContentsPrefixPaths(const QStringList &prefixPaths);

    /**
     * Sets preferred package root.
     */
    void setDefaultPackageRoot(const QString &packageRoot);

    /**
     * Called whenever the path changes so that subclasses may take
     * package specific actions.
     */
    virtual void pathChanged();

private:
    PackageStructurePrivate * const d;
};

/**
 * Register an applet when it is contained in a loadable module
 */
#define K_EXPORT_PLASMA_PACKAGESTRUCTURE(libname, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_packagestructure_" #libname)) \
K_EXPORT_PLUGIN_VERSION(PLASMA_VERSION)

} // Plasma namespace
#endif
