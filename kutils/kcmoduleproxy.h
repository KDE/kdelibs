/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>
    Copyright (C) 2004 Frans Englich <frans.englich@telia.com>

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

#ifndef KUTILS_KCMODULEPROXY_H
#define KUTILS_KCMODULEPROXY_H

#include <QtGui/QImage>
#include <QtCore/QStringList>

#include <kcmodule.h>
#include <kservice.h>
#include <kutils_export.h>

class KAboutData;
class KCModuleInfo;
class KComponentData;
class KCModuleProxyPrivate;

/**
 *
 * @brief Encapsulates a KCModule for embedding.
 *
 * KCModuleProxy is a wrapper for KCModule intended for cases where
 * modules are to be displayed. It ensures layout is consistent
 * and in general takes care of the details
 * needed for making a module available in an interface. A KCModuleProxy
 * can be treated as a QWidget, without worrying about the details specific
 * for modules such as library loading. KCModuleProxy is not a sub class of KCModule
 * but its API closely resembles KCModule's.\n
 * Usually, an instance is created by passing one of the constructors a KService::Ptr,
 * KCModuleInfo or simply the name of the module and then added to the layout as any
 * other widget. \n
 * When the user has changed the module, changed(bool) as well as changed(KCModuleProxy *)
 * is emitted. KCModuleProxy does not take care of prompting for saving - if the object is deleted while
 * changes is not saved the changes will be lost. changed() returns true if changes are unsaved. \n
 * \n
 * KCModuleProxy does not take care of authorization of KCModules. \n
 * KCModuleProxy implements lazy loading, meaning the library will not be loaded or
 * any other initialization done before its show() function is called. This means
 * modules will only be loaded when they are actually needed as well as it is possible to
 * load many KCModuleProxy without any speed penalty.
 *
 * KCModuleProxy should be used in all cases where modules are embedded in order to
 * promote code efficiency and usability consistency.
 *
 * @author Frans Englich <frans.englich@telia.com>
 * @author Matthias Kretz <kretz@kde.org>
 *
 */
class KUTILS_EXPORT KCModuleProxy : public QWidget
{
Q_DECLARE_PRIVATE(KCModuleProxy)
Q_OBJECT
public:
	/**
	 * Constructs a KCModuleProxy from a KCModuleInfo class.
	 *
	 * @param info The KCModuleInfo to construct the module from.
	 * @param parent the parent QWidget.
	 * @param args This is used in the implementation and is internal.
	 * Use the default.
	 */
	explicit KCModuleProxy( const KCModuleInfo& info, QWidget* parent = 0,
			const QStringList& args = QStringList() );

	/**
	 * Constructs a KCModuleProxy from a module's service name, which is
	 * equivalent to the desktop file for the kcm without the ".desktop" part.
	 * Otherwise equal to the one above.
	 *
	 * @param serviceName The module's service name to construct from.
	 * @param parent the parent QWidget.
	 * @param args This is used in the implementation and is internal.
	 * Use the default.
	 */
	explicit KCModuleProxy( const QString& serviceName, QWidget* parent = 0,
			const QStringList& args = QStringList() );

	/**
	 * Constructs a KCModuleProxy from KService. Otherwise equal to the one above.
	 *
	 * @param service The KService to construct from.
	 * @param parent the parent QWidget.
	 * @param args This is used in the implementation and is internal.
	 * Use the default.
	 */
	explicit KCModuleProxy( const KService::Ptr& service, QWidget* parent = 0,
			const QStringList& args = QStringList() );

	/**
	 * Default destructor
	 */
	~KCModuleProxy();

	/**
	 * Calling it will cause the contained module to
	 * run its load() routine.
	 */
	void load();

	/**
	 * Calling it will cause the contained module to
	 * run its save() routine.
	 *
	 * If the module was not modified, it will not be asked
	 * to save.
	 */
	void save();

	/**
	 * @return the module's quickHelp();
	 */
	QString quickHelp() const;

	/**
	 * @return the module's aboutData()
	 */
	const KAboutData * aboutData() const;

	/**
	 * @return what buttons the module
	 * needs
	 */
	KCModule::Buttons buttons() const;

	/**
	 * @return The module's custom root
	 * message, if it has one
	 * @deprecated
	 */
	QString rootOnlyMessage() const;
	//KDE4 remove. There's a limit for convenience functions,
	// this one's available via realModule()->

	/**
	 * @return If the module is a root module.
	 * @deprecated
	 */
	bool useRootOnlyMessage() const;
	//KDE4 remove. There's a limit for convenience functions,
	// this one's available via realModule()->

	/**
	 * Returns the embedded KCModule's KComponentData.
	 * @return The module's KComponentData.
	 * @deprecated
	 */
	KComponentData componentData() const;
	//KDE4 remove. There's a limit for convenience functions,
	// this one's available via realModule()

	/**
	 * @return true if the module is modified
	 * and needs to be saved.
	 */
	bool changed() const;

	/**
	 * Access to the actual module. However, if the module is
	 * running in root mode, see rootMode(), this function returns
	 * a NULL pointer, since the module is in another process. It may also
	 * return NULL if anything goes wrong.
	 *
	 * @return the encapsulated module.
	 */
	KCModule* realModule() const;

	/**
	 * @return a KCModuleInfo for the encapsulated
	 * module
	 */
	KCModuleInfo moduleInfo() const;

	/**
	 * Returns the DBUS Service name
	 */
	QString dbusService() const;
    /**
     * Returns the DBUS Path
     */
	QString dbusPath() const;
	/**
	 * Returns the recommended minimum size for the widget
	 */
	QSize minimumSizeHint() const;

public Q_SLOTS:

	/**
	 * Calling it will cause the contained module to
	 * load its default values.
	 */
	void defaults();

	/**
	 * Calling this, results in deleting the contained
	 * module, and unregistering from DCOP. A similar result is achieved
	 * by deleting the KCModuleProxy itself.
	 */
	void deleteClient();

Q_SIGNALS:

	/*
	 * This signal is emitted when the contained module is changed.
	 */
	void changed( bool state );

	/**
	 * This is emitted in the same situations as in the one above. Practical
	 * when several KCModuleProxys are loaded.
	 */
	void changed( KCModuleProxy* mod );

	/**
	 * When a module running with root privileges and exits, returns to normal mode, the
	 * childClosed() signal is emitted.
	 */
	void childClosed();

	/*
	 * This signal is relayed from the encapsulated module, and
	 * is equivalent to the module's own quickHelpChanged() signal.
	 */
	void quickHelpChanged();

protected:

	/**
	 * Reimplemented for internal purposes. Makes sure the encapsulated
	 * module is loaded before the show event is taken care of.
	 */
	void showEvent( QShowEvent * );

protected:
    KCModuleProxyPrivate *const d_ptr;

private:
    Q_PRIVATE_SLOT(d_func(), void _k_moduleChanged(bool))
    Q_PRIVATE_SLOT(d_func(), void _k_moduleDestroyed())
    Q_PRIVATE_SLOT(d_func(), void _k_ownerChanged(const QString &service, const QString &oldOwner, const QString &newOwner))
};

#endif // KUTILS_KCMODULEPROXY_H
// vim: ts=4
