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

#include "kdelibs_export.h"
#include <kconfig.h>

class QByteArray;
class QString;
class KAboutData;
class KStandardDirs;
class KComponentDataPrivate;

/**
 * \short Per component data.
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
 * \author Torben Weis
 * \author Matthias Kretz <kretz@kde.org>
 */
class KDECORE_EXPORT KComponentData
{
    friend class KStandardDirs;
    KSharedConfig::Ptr &privateConfig() const;

public:
    /**
     * Creates an invalid KComponentData object.
     *
     * \see isValid()
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
     * If the data of the left hand side object was only referenced from this object it is deleted
     * if no referenced KSharedConfig object doesn't need it anymore.
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

    /**
     * Constructor.
     *
     * \param componentName the name of the component.
     */
    explicit KComponentData(const QByteArray &componentName);

    /**
     *  Constructor.
     *
     *  When instanciating a KComponentData that is not your KApplication,
     *  make sure that the KAboutData and the KComponentData have the same life time.
     *  You have to destroy both, since KComponentData does not own the about data.
     *  Do not create a KAboutData on the stack in this case!
     *  Building a KAboutData on the stack is only ok for usage with
     *  KCmdLineArgs and KApplication (not destroyed until the app exits).
     *
     *  \param aboutData data about this component
     *
     *  \see KAboutData
     */
    explicit KComponentData(const KAboutData *aboutData);

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
    const KSharedConfig::Ptr &config() const;

    /**
     * Returns the about data of this component.
     *
     * \return The about data of the component. If none has been set in the constructor but a
     * component name was set a default constructed KAboutData object is returned.
     */
    const KAboutData *aboutData() const;

    /**
     * Returns the name of the component.
     *
     * \return The component name.
     */
    QByteArray componentName() const;

protected:
    friend class KApplication;
    friend class KSharedConfigPtr;
    friend class KConfigBackEnd;

    /**
     * Set name of default config file.
     * @param name the name of the default config file
     */
    void setConfigName(const QString &name);

    void _checkConfig();

    /** Standard trick to add virtuals later. @internal */
    virtual void virtual_hook( int id, void* data );

    // Ref-counted data
    KComponentDataPrivate* d; // krazy:exclude=dpointer
};

#endif // KCOMPONENTDATA_H
