/* This file is part of the KDE project
   Copyright (C) 2010 Maksim Orlovich <maksim@kde.org>
   Copyright (C) 2002 Koos Vriezen <koos.vriezen@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef kparts_scriptableextension_h
#define kparts_scriptableextension_h

#include <QtGlobal>
#include <QObject>
#include <QVariant>
#include <kparts/part.h>

namespace KParts {

class  ScriptableExtension;
struct ScriptableExtensionPrivate;
class  LiveConnectExtension;

/**
 * An extension class that permits KParts to be scripted (such as when embedded
 * inside a KHTMLPart) and to access the host's scriptable objects as well.
 *
 * See @ref ScriptValueTypes for how values are passed to/from various methods here.
 *
 * @since 4.5
 * \nosubgrouping
 */
class KPARTS_EXPORT ScriptableExtension: public QObject
{
    Q_OBJECT
public:
    /** @defgroup ScriptValueTypes Script Value Types
     * @{
     * Values are passed to and from scriptable methods or properties as QVariants.
     * Valid values may be bools, strings, and numbers (doubles), as well as the
     * following custom types:
     * \li @ref Null
     * \li @ref Undefined
     * \li @ref Exception
     * \li @ref Object
     * \li @ref FunctionRef
     */

    /// Corresponds to 'null' in JavaScript
    struct Null {};

    /// Corresponds to 'undefined' in JavaScript
    struct Undefined {};

    /// Returned from operations to denote a failure. May not be passed in as
    /// a parameter, only returned
    struct Exception {
        /// Error message returned from the callee. This should be assumed to be
        /// low-level (in particular, it might not be translated) and should
        /// only be displayed in low-level debugging tools and the like.
        QString message;

        Exception() {}
        Exception(const QString& msg): message(msg) {}
    };

    /// Objects are abstracted away as a pair of the ScriptableExtension
    /// the performs operations on it, and an implementation-specific Id,
    /// which gets passed to the extension's methods.
    ///
    /// Objects are reference-counted, with the following protocol:
    /// 1) Return values from methods, rootObject(), enclosingObject(),
    ///    and get() are already acquired by the producer, so the consumer
    ///    should release them when done.
    /// 2) During a call, the caller guarantees that all the arguments
    ///    will be live for the calls duration, but the callee must
    ///    acquire them if it stores it for longer than that.
    ///
    /// @see acquire, acquireValue, release, releaseValue
    struct Object {
        ScriptableExtension* owner;
        quint64              objId;

        Object(): owner(0), objId(0) {}
        Object(ScriptableExtension* o, quint64 id): owner(o), objId(id) {}
        bool operator==(const Object& other) const { return owner == other.owner && objId == other.objId; }
    };

    /// Function references are a pair of an object and a field in it.
    /// Essentially, if you have a base.field(something) call, the
    /// 'base' needs to be passed as the 'this' to the function, and
    /// these references can be used to resolve that.
    struct FunctionRef {
        Object   base;
        QString  field;

        FunctionRef() {}
        FunctionRef(const Object& b, const QString&f): base(b), field(f) {}
        bool operator==(const FunctionRef& other) const { return base == other.base && field == other.field; }
    };

    //@}


    ///@name lifetime
    //@{
protected:
    ScriptableExtension(QObject* parent);
public:
    virtual ~ScriptableExtension();

    /**
    * Queries @p obj for a child object which inherits from this
    * ScriptableExtension class. Convenience method.
    */
    static ScriptableExtension* childObject(QObject* obj);

    /**
    * This returns a bridge object that permits KParts implementing the older
    * LiveConnectExtension to be used via the ScriptableExtension API.
    * The bridge's parent will be the @p parentObj.
    */
    static ScriptableExtension* adapterFromLiveConnect(QObject* parentObj,
                                                       LiveConnectExtension* oldApi);

    //@}


    ///@name Object Hierarchy
    //@{

    /**
     * Reports the hosting ScriptableExtension to a child. It's the responsibility
     * of a parent part to call this method on all of its kids' ScriptableExtensions
     * as soon as possible.
     */
    void setHost(ScriptableExtension* host);

    /**
     * Returns any registered parent scripting context. May be 0 if setHost
     * was not called (or not call yet).
     */
    ScriptableExtension* host() const;

    /**
     * Return the root scriptable object of this KPart.
     * For example for an HTML part, it would represent a Window object.
     * May be undefined or null
     */
    virtual QVariant rootObject();

    /**
     * Returns an object that represents the host()'s view of us.
     * For example, if the host is an HTML part, it would return
     * a DOM node of an &lt;object&gt; handled by this part.
     * May be undefined or null
     *
     * Implemented in terms of objectForKid
     *
     */
    QVariant enclosingObject();
    //@}

    ///@name Object Operations
    /// All these methods share the following conventions:
    /// \li Values are passed and returned encoded as defined in
    ///   @ref ScriptValueTypes
    /// \li All methods may return an exception if unsupported
    /// \li All callers \b must provide an accurate callerPrincipal
    ///   argument describing which ScriptableExtension (and hence which KPart)
    ///   they're acting as. This is used to implement <b>security checks</b>. This
    ///   is \b not the same as the owner of an object. For example, if a plugin is
    ///   calling an operation on a KHTMLPart object,
    ///   then the 'this' parameter would be the object owner, a ScriptableExtension
    ///   provided by the KHTMLPart, while the callerPrincipal would be the
    ///   ScriptableExtension of the \em plugin. The extension is expected
    ///   to do appropriate cross-site scripting checks on this argument
    ///   if it is acting as a host.
    //@{

    typedef QList<QVariant> ArgList;

    /**
      Try to use the object @p objId associated with 'this' as a function.
    */
    virtual QVariant callAsFunction(ScriptableExtension* callerPrincipal, quint64 objId, const ArgList& args);

    /**
     Try to use a function reference to field @p f of object @objId as a function
     */
    virtual QVariant callFunctionReference(ScriptableExtension* callerPrincipal, quint64 objId,
                                           const QString& f, const ArgList& args);

    /**
      Try to use the object @p objId associated with 'this' as a constructor
      (corresponding to ECMAScript's new foo(bar, baz, glarch) expression).
    */
    virtual QVariant callAsConstructor(ScriptableExtension* callerPrincipal, quint64 objId, const ArgList& args);

    /**
     Returns true if the object @p objId associated with 'this' has the property
     @p propName.
    */
    virtual bool hasProperty(ScriptableExtension* callerPrincipal, quint64 objId, const QString& propName);

    /**
     Tries to get field @p propName from object @p objId associated with 'this'.
    */
    virtual QVariant get(ScriptableExtension* callerPrincipal, quint64 objId, const QString& propName);

    /**
     Tries to set the field @p propName from object @p objId associated with 'this'
     to @p value. Returns true on success
    */
    virtual bool put(ScriptableExtension* callerPrincipal, quint64 objId, const QString& propName, const QVariant& value);

    /**
     Tries to remove the field d @p propName from object @p objId associated with 'this'.
     Returns true on success
    */
    virtual bool removeProperty(ScriptableExtension* callerPrincipal, quint64 objId, const QString& propName);

    /**
     Tries to enumerate all fields of object @p objId associated with this to
     @p result. Returns true on success
    */
    virtual bool enumerateProperties(ScriptableExtension* callerPrincipal, quint64 objId, QStringList* result);

    /**
     Tries to raise an exception with given message in this extension's scripting
     context. Returns true on success
    */
    virtual bool setException(ScriptableExtension* callerPrincipal, const QString& message);


    enum ScriptLanguage {
        ECMAScript, /// < also known as JavaScript
        EnumLimit = 0xFFFF
    };
    
    /**
     Tries to evaluate a script @p code with the given object as its context.
     The parameter @p language specifies the language to execute it as.
     Use isScriptLanguageSupported to check for support.
    */
    virtual QVariant evaluateScript(ScriptableExtension* callerPrincipal,
                                    quint64 contextObjectId,
                                    const QString& code,
                                    ScriptLanguage language = ECMAScript);

    /** returns true if this extension can execute scripts in the given
        language */
    virtual bool isScriptLanguageSupported(ScriptLanguage lang) const;

    /**
      increases reference count of object @p objId
    */
    virtual void acquire(quint64 objid);

    /**
      Helper that calls @ref acquire on any object or function reference base
      stored in @p v.

      @return a copy of the passed in value
    */
    static QVariant acquireValue(const QVariant& v);

    /**
      decreases reference count of object @p objId
    */
    virtual void release(quint64 objid);

    /**
      Helper that calls @ref release on any object or function reference base
      stored in @p v.

      @return a copy of the passed in value
    */
    static QVariant releaseValue(const QVariant& v);

    //@}
private:
    /**
     *  If this extension is a host that provides an object corresponding
     *  to each kid, override this method to provide it.
     *  @see enclosingObject
     */
    virtual QVariant encloserForKid(KParts::ScriptableExtension* kid);

    ScriptableExtensionPrivate* const d;
};

KPARTS_EXPORT unsigned int qHash(const KParts::ScriptableExtension::Object& o);

KPARTS_EXPORT unsigned int qHash(const KParts::ScriptableExtension::FunctionRef& f);

} // namespace KParts

Q_DECLARE_METATYPE(KParts::ScriptableExtension::Null)
Q_DECLARE_METATYPE(KParts::ScriptableExtension::Undefined)
Q_DECLARE_METATYPE(KParts::ScriptableExtension::Exception)
Q_DECLARE_METATYPE(KParts::ScriptableExtension::Object)
Q_DECLARE_METATYPE(KParts::ScriptableExtension::FunctionRef)

#endif

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
