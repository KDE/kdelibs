/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KCOMPONENTDATA_H
#define KCOMPONENTDATA_H

#include <kdecore_export.h>
#include <ksharedconfig.h>

class QByteArray;
class QString;
class KAboutData;
class KStandardDirs;
class KComponentDataPrivate;

/**
 * @short Per component data.
 *
 * This class holds a KAboutData object or only a component name, a KStandardDirs object and a
 * KSharedConfig object. Those objects normally are different per component but the same per
 * instance of one component.
 *
 * The application component data can always be accessed using KGlobal::mainComponent() (or the
 * convenience function KGlobal::dirs() and KGlobal::config()) while the
 * component data of the currently active component (mainly used for KParts) can be accessed using
 * KGlobal::activeComponent().
 *
 * @author Torben Weis
 * @author Matthias Kretz <kretz@kde.org>
 */
class KDECORE_EXPORT KComponentData // krazy:exclude=dpointer (implicitly shared)
{
public:
    /**
     * Creates an invalid KComponentData object.
     *
     * @see isValid()
     */
    KComponentData();

    /**
     * Copy constructor.
     *
     * It does not copy the data. The data is shared between the old and new objects.
     */
    KComponentData(const KComponentData&);

    /**
     * Assignment operator.
     *
     * It does not copy the data. The data is shared between the old and new objects.
     *
     * If the data of the left hand side object was only referenced
     * from this object and no referenced KSharedConfig object needs
     * it anymore, it is deleted
     */
    KComponentData &operator=(const KComponentData&);

    /**
     * Returns whether two KComponentData objects reference the same data.
     */
    bool operator==(const KComponentData&) const;

    /**
     * Returns whether two KComponentData objects do not reference the same data.
     */
    bool operator!=(const KComponentData &rhs) const { return !operator==(rhs); }

    enum MainComponentRegistration {
        RegisterAsMainComponent,
        SkipMainComponentRegistration
    };

    /**
     * Constructor.
     *
     * @param componentName the name of the component.
     * @param catalogName the name of the translation catalog;
     *                    if left empty @p componentName is used
     * @param registerAsMain whether to register the component as the main component
     *                       of the application. This has no effect, if the application
     *                       already has a main component.
     *                       @see KGlobal::mainComponent
     */
    explicit KComponentData(const QByteArray &componentName, const QByteArray &catalogName = QByteArray(),
                MainComponentRegistration registerAsMain = RegisterAsMainComponent);

    /**
     * Constructor.
     *
     * A copy of the aboutData object is made.
     *
     * @param aboutData data about this component
     * @param registerAsMain whether to register the component as the main component
     *                       of the application. This has no effect, if the application
     *                       already has a main component.
     *                       @see KGlobal::mainComponent
     *
     * @see KAboutData
     */
    explicit KComponentData(const KAboutData &aboutData, MainComponentRegistration registerAsMain = RegisterAsMainComponent);
    explicit KComponentData(const KAboutData *aboutData, MainComponentRegistration registerAsMain = RegisterAsMainComponent);

    /**
     * Destructor.
     */
    virtual ~KComponentData();

    /**
     * Returns whether this is a valid object.
     *
     * Don't call any functions on invalid objects, that will crash. Assignment (and of course
     * destruction) is the only valid operation you may do.
     */
    bool isValid() const;

    /**
     * Returns the application standard dirs object.
     * @return The KStandardDirs of the application.
     */
    KStandardDirs *dirs() const;

    /**
     * Returns the general config object ("appnamerc").
     * @return the KConfig object for the component.
     */
    const KSharedConfig::Ptr &config() const; //krazy:exclude=constref

    /**
     * Returns the about data of this component.
     *
     * @return The about data of the component. If none has been set in the
     *         constructor but a component name was set, a default constructed
     *         KAboutData object is returned.
     */
    const KAboutData *aboutData() const;

    /**
     * Returns the name of the component.
     *
     * @return The component name.
     */
    QString componentName() const;

    /**
     * Returns the name of the translation catalog.
     *
     * @return The catalog name.
     */
    QString catalogName() const;

protected:
    friend class KApplicationPrivate;

    /**
     * Set name of default config file.
     * @param name the name of the default config file
     */
    void setConfigName(const QString &name);

    /** Standard trick to add virtuals later. @internal */
    virtual void virtual_hook( int id, void* data );

private:
    // Ref-counted data
    KComponentDataPrivate* d;
    friend class KComponentDataPrivate;
};

#endif // KCOMPONENTDATA_H
