/* This file is part of the KDE libraries
   Copyright (C) 1999 Sirtaj Singh Kanq <taj@kde.org>
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
#ifndef _KGLOBAL_H
#define _KGLOBAL_H

#include <kde4support_export.h>

#ifdef KDE4SUPPORT_NO_DEPRECATED_NOISE
#warning "This file is deprecated."
#endif

#include <QtCore/QAtomicPointer>
#include <sys/types.h>
#include <QtCore/QDebug>
// To simplify Qt5 porting in KDE code not yet ported to frameworks.
#include <QMimeData>
#include <klocale.h>

// TODO: Re-add for source compat: #include <kdemacros.h>

//
// WARNING!!
// This code uses undocumented Qt API
// Do not copy it to your application! Use only the functions that are here!
// Otherwise, it could break when a new version of Qt ships.
//

class KComponentData;
class KCharsets;
class KConfig;
class KStandardDirs;
class KSharedConfig;
template <typename T>
class QExplicitlySharedDataPointer;
typedef QExplicitlySharedDataPointer<KSharedConfig> KSharedConfigPtr;

/// @cond InternalDocs

/**
 * @internal
 */
typedef void (*KdeCleanUpFunction)();

/**
 * @internal
 *
 * Helper class for K_GLOBAL_STATIC to clean up the object on library unload or application
 * shutdown.
 */
class KCleanUpGlobalStatic
{
    public:
        KdeCleanUpFunction func;

        inline ~KCleanUpGlobalStatic() { func(); }
};

#ifdef Q_CC_MSVC
/**
 * @internal
 *
 * MSVC seems to give anonymous structs the same name which fails at link time. So instead we name
 * the struct and hope that by adding the line number to the name it's unique enough to never clash.
 */
# define K_GLOBAL_STATIC_STRUCT_NAME(NAME) _k_##NAME##__LINE__
#else
/**
 * @internal
 *
 * Make the struct of the K_GLOBAL_STATIC anonymous.
 */
# define K_GLOBAL_STATIC_STRUCT_NAME(NAME)
#endif

/// @endcond

/**
 * This macro makes it easy to use non-POD types as global statics.
 * The object is created on first use and creation is threadsafe.
 *
 * The object is destructed on library unload or application exit.
 * Be careful with calling other objects in the destructor of the class
 * as you have to be sure that they (or objects they depend on) are not already destructed.
 *
 * @param TYPE The type of the global static object. Do not add a *.
 * @param NAME The name of the function to get a pointer to the global static object.
 *
 * If you have code that might be called after the global object has been destroyed you can check
 * for that using the isDestroyed() function.
 *
 * If needed (If the destructor of the global object calls other functions that depend on other
 * global statics (e.g. KConfig::sync) your destructor has to be called before those global statics
 * are destroyed. A Qt post routine does that.) you can also install a post routine (qAddPostRoutine) to clean up the object
 * using the destroy() method. If you registered a post routine and the object is destroyed because
 * of a lib unload you have to call qRemovePostRoutine!
 *
 * Example:
 * @code
 * class A {
 * public:
 *     ~A();
 *     ...
 * };
 *
 * K_GLOBAL_STATIC(A, globalA)
 * // The above creates a new globally static variable named 'globalA' which you
 * // can use as a pointer to an instance of A.
 *
 * void doSomething()
 * {
 *     //  The first time you access globalA a new instance of A will be created automatically.
 *     A *a = globalA;
 *     ...
 * }
 *
 * void doSomethingElse()
 * {
 *     if (globalA.isDestroyed()) {
 *         return;
 *     }
 *     A *a = globalA;
 *     ...
 * }
 *
 * void installPostRoutine()
 * {
 *     // A post routine can be used to delete the object when QCoreApplication destructs,
 *     // not adding such a post routine will delete the object normally at program unload
 *     qAddPostRoutine(globalA.destroy);
 * }
 *
 * A::~A()
 * {
 *     // When you install a post routine you have to remove the post routine from the destructor of
 *     // the class used as global static!
 *     qRemovePostRoutine(globalA.destroy);
 * }
 * @endcode
 *
 * A common case for the need of deletion on lib unload/app shutdown are Singleton classes. Here's
 * an example how to do it:
 * @code
 * class MySingletonPrivate;
 * class EXPORT_MACRO MySingleton
 * {
 * friend class MySingletonPrivate;
 * public:
 *     static MySingleton *self();
 *     QString someFunction();
 *
 * private:
 *     MySingleton();
 *     ~MySingleton();
 * };
 * @endcode
 * in the .cpp file:
 * @code
 * // This class will be instantiated and referenced as a singleton in this example
 * class MySingletonPrivate
 * {
 * public:
 *     QString foo;
 *     MySingleton instance;
 * };
 *
 * K_GLOBAL_STATIC(MySingletonPrivate, mySingletonPrivate)
 *
 * MySingleton *MySingleton::self()
 * {
 *     // returns the singleton; automatically creates a new instance if that has not happened yet.
 *     return &mySingletonPrivate->instance;
 * }
 * QString MySingleton::someFunction()
 * {
 *     // Refencing the singleton directly is possible for your convenience
 *     return mySingletonPrivate->foo;
 * }
 * @endcode
 *
 * Instead of the above you can use also the following pattern (ignore the name of the namespace):
 * @code
 * namespace MySingleton
 * {
 *     EXPORT_MACRO QString someFunction();
 * }
 * @endcode
 * in the .cpp file:
 * @code
 * class MySingletonPrivate
 * {
 * public:
 *     QString foo;
 * };
 *
 * K_GLOBAL_STATIC(MySingletonPrivate, mySingletonPrivate)
 *
 * QString MySingleton::someFunction()
 * {
 *     return mySingletonPrivate->foo;
 * }
 * @endcode
 *
 * Now code that wants to call someFunction() doesn't have to do
 * @code
 * MySingleton::self()->someFunction();
 * @endcode
 * anymore but instead:
 * @code
 * MySingleton::someFunction();
 * @endcode
 *
 * @ingroup KDEMacros
 */
#define K_GLOBAL_STATIC(TYPE, NAME) K_GLOBAL_STATIC_WITH_ARGS(TYPE, NAME, ())

/**
 * @overload
 * This is the same as K_GLOBAL_STATIC, but can take arguments that are passed
 * to the object's constructor
 *
 * @param TYPE The type of the global static object. Do not add a *.
 * @param NAME The name of the function to get a pointer to the global static object.
 * @param ARGS the list of arguments, between brackets
 *
 * Example:
 * @code
 * class A
 * {
 * public:
 *     A(const char *s, int i);
 *     ...
 * };
 *
 * K_GLOBAL_STATIC_WITH_ARGS(A, globalA, ("foo", 0))
 * // The above creates a new globally static variable named 'globalA' which you
 * // can use as a pointer to an instance of A.
 *
 * void doSomething()
 * {
 *     //  The first time you access globalA a new instance of A will be created automatically.
 *     A *a = globalA;
 *     ...
 * }
 * @endcode
 *
 * @ingroup KDEMacros
 */

// In Qt5 QBasicAtomicPointer<T> no longer implicit casts to T*
// Instead it has load() and store() methods which do not exist in Qt4.
// In practice, we should be porting frameworks to the new Q_GLOBAL_STATIC
// which isn't in Qt5 yet, so duplicate for now.

#define K_GLOBAL_STATIC_WITH_ARGS(TYPE, NAME, ARGS)                            \
static QBasicAtomicPointer<TYPE > _k_static_##NAME = Q_BASIC_ATOMIC_INITIALIZER(0); \
static bool _k_static_##NAME##_destroyed;                                      \
static struct K_GLOBAL_STATIC_STRUCT_NAME(NAME)                                \
{                                                                              \
    inline bool isDestroyed() const                                            \
    {                                                                          \
        return _k_static_##NAME##_destroyed;                                   \
    }                                                                          \
    inline bool exists() const                                                 \
    {                                                                          \
        return _k_static_##NAME.load() != 0;                                          \
    }                                                                          \
    inline operator TYPE*()                                                    \
    {                                                                          \
        return operator->();                                                   \
    }                                                                          \
    inline TYPE *operator->()                                                  \
    {                                                                          \
        if (!_k_static_##NAME.load()) {                                               \
            if (isDestroyed()) {                                               \
                qFatal("Fatal Error: Accessed global static '%s *%s()' after destruction. " \
                       "Defined at %s:%d", #TYPE, #NAME, __FILE__, __LINE__);  \
            }                                                                  \
            TYPE *x = new TYPE ARGS;                                           \
            if (!_k_static_##NAME.testAndSetOrdered(0, x)                      \
                && _k_static_##NAME.load() != x ) {                                   \
                delete x;                                                      \
            } else {                                                           \
                static KCleanUpGlobalStatic cleanUpObject = { destroy };       \
            }                                                                  \
        }                                                                      \
        return _k_static_##NAME.load();                                               \
    }                                                                          \
    inline TYPE &operator*()                                                   \
    {                                                                          \
        return *operator->();                                                  \
    }                                                                          \
    static void destroy()                                                      \
    {                                                                          \
        _k_static_##NAME##_destroyed = true;                                   \
        TYPE *x = _k_static_##NAME.load();                                            \
        _k_static_##NAME.store(0);                                                  \
        delete x;                                                              \
    }                                                                          \
} NAME;


/**
 * Access to the KDE global objects.
 * KGlobal provides you with pointers of many central
 * objects that exist only once in the process. It is also
 * responsible for managing instances of KStaticDeleterBase.
 *
 * @see KStaticDeleterBase
 * @author Sirtaj Singh Kang (taj@kde.org)
 */
namespace KGlobal
{

  struct KDE4SUPPORT_DEPRECATED_EXPORT_NOISE LocaleWrapper : public KLocale
  {
    explicit LocaleWrapper(KLocale *locale)
      : KLocale(*locale)
    {

    }

    KDE4SUPPORT_DEPRECATED static void insertCatalog(const QString &)
    {
      qWarning() << "Your code needs to be ported in KF5.  See the Ki18n programmers guide.";
    }

    LocaleWrapper* operator->()
    {
      return this;
    }

    operator KLocale*()
    {
      return this;
    }
  };

    /**
     * Returns the global component data.  There is always at least
     * one instance of a component in one application (in most
     * cases the application itself).
     * @return the global component data
     * @deprecated since 5.0 use KComponentData::mainComponent() if you really need a KComponentData
     */
    KDE4SUPPORT_DEPRECATED_EXPORT const KComponentData &mainComponent(); //krazy:exclude=constref (don't mess up ref-counting)

    /**
     * @internal
     * Returns whether a main KComponentData is available.
     * @deprecated since 5.0, use KComponentData::hasMainComponent() if you really need a KComponentData
     */
    KDE4SUPPORT_DEPRECATED_EXPORT bool hasMainComponent();

    /**
     * Returns the application standard dirs object.
     * @return the global standard dir object
     */
    KDE4SUPPORT_DEPRECATED_EXPORT KStandardDirs *dirs();

    /**
     * Returns the general config object.
     * @return the global configuration object.
     * @deprecated since 5.0, use KSharedConfig::openConfig()
     */
    KDE4SUPPORT_DEPRECATED_EXPORT KSharedConfigPtr config();

    /**
     * Returns the global locale object.
     * @return the global locale object
     *
     * Note: in multi-threaded programs, you should call KLocale::global()
     * in the main thread (e.g. in main(), after creating the QCoreApplication
     * and setting the main component), to ensure that the initialization is
     * done in the main thread. However KApplication takes care of this, so this
     * is only needed when not using KApplication.
     *
     * @deprecated since 5.0, use KLocale::global()
     */
    KDE4SUPPORT_DEPRECATED_EXPORT LocaleWrapper locale();
    /**
     * @internal
     * Returns whether KGlobal has a valid KLocale object
     * @deprecated since 5.0, port to if (qApp) because KLocale::global() can be called, as soon as a qApp exists.
     */
    KDE4SUPPORT_DEPRECATED_EXPORT bool hasLocale();

    /**
     * The global charset manager.
     * @return the global charset manager
     * @deprecated since 5.0, use KCharsets::charsets()
     */
    KDE4SUPPORT_DEPRECATED_EXPORT KCharsets *charsets();

    /**
     * Returns the umask of the process.
     * @return the umask of the process
     */
    KDE4SUPPORT_DEPRECATED_EXPORT mode_t umask();

    /**
     * Creates a static QString.
     *
     * To be used inside functions(!) like:
     * @code
     * static const QString &myString = KGlobal::staticQString("myText");
     * @endcode
     *
     * @attention Do @b NOT use code such as:
     * @code
     * static QString myString = KGlobal::staticQString("myText");
     * @endcode
     * This creates a static object (instead of a static reference)
     * and as you know static objects are EVIL.
     * @param str the string to create
     * @return the static string
     * @deprecated since 5.0, use QLatin1Literal()
     */
    KDE4SUPPORT_DEPRECATED_EXPORT const QString& staticQString(const char *str); //krazy:exclude=constref (doesn't make sense otherwise)

    /**
     * Creates a static QString.
     *
     * To be used inside functions(!) like:
     * @code
     * static const QString &myString = KGlobal::staticQString(i18n("My Text"));
     * @endcode
     *
     * @attention Do @b NOT use code such as:
     * @code
     * static QString myString = KGlobal::staticQString(i18n("myText"));
     * @endcode
     * This creates a static object (instead of a static reference)
     * and as you know static objects are EVIL.
     * @param str the string to create
     * @return the static string
     * @deprecated since 5.0 don't make the string static
     */
    KDE4SUPPORT_DEPRECATED_EXPORT const QString& staticQString(const QString &str); //krazy:exclude=constref (doesn't make sense otherwise)

    /**
     * Tells KGlobal about one more operations that should be finished
     * before the application exits. The standard behavior is to exit on the
     * "last window closed" event, but some events should outlive the last window closed
     * (e.g. a file copy for a file manager, or 'compacting folders on exit' for a mail client),
     * or simply any application with a system tray icon.
     *
     * We have some use cases that we want to take care of (the format is "action refcount"):
     * - open window -> setAllowQuit(true) 1 ; close window 0 => EXIT
     * - job start 1; job end 0 [don't exit yet]; open window -> setAllowQuit(true) 1 ; close window 0 => EXIT
     * - job start 1; open window -> setAllowQuit(true) 2; close window 1; job end 0 => EXIT
     * - job start 1; open window -> setAllowQuit(true) 2; job end 1; close window 0 => EXIT
     * - open dialog 0; close dialog 0; => DO NOT EXIT
     * - job start 1; job end 0; create two main objects 2; delete both main objects 0 => EXIT
     * - open window -> setAllowQuit(true) 1; add systray icon 2; close window 1 => DO NOT EXIT
     * - open window -> setAllowQuit(true) 1; add systray icon 2; remove systray icon 1; close window 0 => EXIT
     * - unit test which opens and closes many windows: should call ref() to avoid subevent-loops quitting too early.
     *
     * Note that for this to happen you must call qApp->setQuitOnLastWindowClosed(false),
     * in main() for instance.
     */
    KDE4SUPPORT_DEPRECATED_EXPORT void ref();

    /**
     * Tells KGlobal that one operation such as those described in ref() just finished.
     * This call makes the QApplication quit if the counter is back to 0.
     */
    KDE4SUPPORT_DEPRECATED_EXPORT void deref();

    /**
     * If refcounting reaches 0 (or less), and @p allowQuit is true, the instance of the application
     * will automatically be exited. Otherwise, the application will not exit automatically.
     *
     * This is used by KMainWindow to allow quitting after the first mainwindow is created,
     * and is used by special applications like kfmclient, to allow quitting even though
     * no mainwindow was created.
     *
     * However, don't try to call setAllowQuit(false) in applications, it doesn't make sense.
     * If you find that the application quits too early when closing a window, then consider
     * _what_ is making your application still alive to the user (like a systray icon or a D-Bus object)
     * and use KGlobal::ref() + KGlobal::deref() in that object.
     *
     * @since 4.1.1
     */
    KDE4SUPPORT_DEPRECATED_EXPORT void setAllowQuit(bool allowQuit);

    /**
     * The component currently active (useful in a multi-component
     * application, such as a KParts application).
     * Don't use this - it's mainly for KAboutDialog and KBugReport.
     * @internal
     * @deprecated since 5.0 use KComponentData::activeComponent()
     */
    KDE4SUPPORT_DEPRECATED_EXPORT KComponentData activeComponent();

    /**
     * Set the active component for use by KAboutDialog and KBugReport.
     * To be used only by a multi-component (KParts) application.
     *
     * @see activeComponent()
     * @deprecated since 5.0 use KComponentData::setActiveComponent
     */
    KDE4SUPPORT_DEPRECATED_EXPORT void setActiveComponent(const KComponentData &d);

    /**
     * Returns a text for the window caption.
     *
     * This may be set by
     * "-caption", otherwise it will be equivalent to the name of the
     * executable.
     * @return the text for the window caption
     * @deprecated since 5.0. Don't use in window titles anymore, Qt takes care of it.
     * If you really need this, use QGuiApplication::applicationDisplayName(), and if that's empty, QCoreApplication::applicationName().
     */
    KDE4SUPPORT_DEPRECATED_EXPORT QString caption();

    /// @internal
    KDE4SUPPORT_DEPRECATED_EXPORT QObject* findDirectChild_helper(const QObject* parent, const QMetaObject& mo);

    /**
     * Returns the child of the given object that can be cast into type T, or 0 if there is no such object.
     * Unlike QObject::findChild, the search is NOT performed recursively.
     * @since 4.4
     * @deprecated since Qt 5, use QObject::findChild(FindDirectChildrenOnly)
     */
    template<typename T>
    KDE4SUPPORT_DEPRECATED inline T findDirectChild(const QObject* object) {
        return static_cast<T>(findDirectChild_helper(object, (static_cast<T>(0))->staticMetaObject));
    }
}

#endif // _KGLOBAL_H

