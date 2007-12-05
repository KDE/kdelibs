/*  This file is part of the KDE libraries    Copyright (C) 2007 Chusslove Illich <caslav.ilic@gmx.net>

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

#include <ktranscript_p.h>

#include <config.h>

#include <kdecore_export.h>
#include <kglobal.h>

//#include <unistd.h>

#include <kjs/value.h>
#include <kjs/object.h>
#include <kjs/lookup.h>
#include <kjs/function.h>
#include <kjs/interpreter.h>
#include <kjs/string_object.h>
#include <kjs/error_object.h>

#include <QVariant>
#include <QStringList>
#include <QList>
#include <QHash>
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QRegExp>

using namespace KJS;

class KTranscriptImp;
class Scriptface;

// Transcript implementation (used as singleton).
class KTranscriptImp : public KTranscript
{
    public:

    KTranscriptImp ();
    ~KTranscriptImp ();

    QString eval (const QList<QVariant> &argv,
                  const QString &lang,
                  const QString &lscr,
                  const QString &msgctxt,
                  const QHash<QString, QString> &dynctxt,
                  const QString &msgid,
                  const QStringList &subs,
                  const QList<QVariant> &vals,
                  const QString &final,
                  QList<QStringList> &mods,
                  QString &error,
                  bool &fallback);

    QStringList postCalls (const QString &lang);

    // Lexical path of the module for the executing code.
    QString currentModulePath;

    private:

    void loadModules (const QList<QStringList> &mods, QString &error);
    void setupInterpreter (const QString &lang);

    QHash<QString, Interpreter*> m_jsi;
    QHash<QString, Scriptface*> m_sface;
};

// Script-side transcript interface.
class Scriptface : public JSObject
{
    public:
    Scriptface (ExecState *exec);
    ~Scriptface ();

    // Interface functions.
    JSValue *loadf (ExecState *exec, const List &fnames);
    JSValue *setcallf (ExecState *exec, JSValue *name,
                       JSValue *func, JSValue *fval);
    JSValue *callForallf (ExecState *exec, JSValue *name,
                          JSValue *func, JSValue *fval);
    JSValue *fallbackf (ExecState *exec);
    JSValue *nsubsf (ExecState *exec);
    JSValue *subsf (ExecState *exec, JSValue *index);
    JSValue *valsf (ExecState *exec, JSValue *index);
    JSValue *msgctxtf (ExecState *exec);
    JSValue *dynctxtf (ExecState *exec, JSValue *key);
    JSValue *msgidf (ExecState *exec);
    JSValue *msgkeyf (ExecState *exec);
    JSValue *msgstrff (ExecState *exec);
    JSValue *dbgputsf (ExecState *exec, JSValue *str);
    JSValue *lscrf (ExecState *exec);
    JSValue *normKeyf (ExecState *exec, JSValue *phrase);
    JSValue *loadPropsf (ExecState *exec, const List &fnames);
    JSValue *getPropf (ExecState *exec, JSValue *phrase, JSValue *prop);
    JSValue *toUpperFirstf (ExecState *exec, JSValue *str, JSValue *nalt);

    enum {
        Load,
        Setcall,
        CallForall,
        Fallback,
        Nsubs,
        Subs,
        Vals,
        Msgctxt,
        Dynctxt,
        Msgid,
        Msgkey,
        Msgstrf,
        Dbgputs,
        Lscr,
        normKey,
        loadProps,
        getProp,
        toUpperFirst
    };

    // Virtual implementations.
    bool getOwnPropertySlot (ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
    JSValue *getValueProperty (ExecState *exec, int token) const;
    void put (ExecState *exec, const Identifier &propertyName, JSValue *value, int attr);
    void putValueProperty (ExecState *exec, int token, JSValue *value, int attr);
    const ClassInfo* classInfo() const { return &info; }

    static const ClassInfo info;

    // Current message data.
    const QString *msgctxt;
    const QHash<QString, QString> *dynctxt;
    const QString *msgid;
    const QStringList *subs;
    const QList<QVariant> *vals;
    const QString *final;
    const QString *lscr;

    // Fallback request handle.
    bool *fallback;

    // Function register.
    QHash<QString, JSObject*> funcs;
    QHash<QString, JSValue*> fvals;
    QHash<QString, QString> fpaths;

    // Ordering of those functions which execute for all messages.
    QList<QString> nameForalls;

    // Property values per phrase (used by *Prop interface calls).
    QHash<QString, QHash<QString, QString> > phraseProps;
};

// ----------------------------------------------------------------------
// Custom debug output (kdebug not available)
#define DBGP "KTranscript: "
void dbgout (const QString &str) {
    #ifndef NDEBUG
    fprintf(stderr, DBGP"%s\n", str.toLocal8Bit().data());
    #else
    Q_UNUSED(str);
    #endif
}
template <typename T1>
void dbgout (const QString &str, const T1 &a1) {
    #ifndef NDEBUG
    fprintf(stderr, DBGP"%s\n", str.arg(a1).toLocal8Bit().data());
    #else
    Q_UNUSED(str); Q_UNUSED(a1);
    #endif
}
template <typename T1, typename T2>
void dbgout (const QString &str, const T1 &a1, const T2 &a2) {
    #ifndef NDEBUG
    fprintf(stderr, DBGP"%s\n", str.arg(a1).arg(a2).toLocal8Bit().data());
    #else
    Q_UNUSED(str); Q_UNUSED(a1); Q_UNUSED(a2);
    #endif
}
template <typename T1, typename T2, typename T3>
void dbgout (const QString &str, const T1 &a1, const T2 &a2, const T2 &a3) {
    #ifndef NDEBUG
    fprintf(stderr, DBGP"%s\n", str.arg(a1).arg(a2).arg(a3).toLocal8Bit().data());
    #else
    Q_UNUSED(str); Q_UNUSED(a1); Q_UNUSED(a2); Q_UNUSED(a3);
    #endif
}

// ----------------------------------------------------------------------
// Conversions between QString and KJS UString.
// Taken from kate.
UString::UString(const QString &d)
{
    unsigned int len = d.length();
    UChar *dat = static_cast<UChar*>(fastMalloc(sizeof(UChar) * len));
    memcpy(dat, d.unicode(), len * sizeof(UChar));
    m_rep = UString::Rep::create(dat, len);
}
QString UString::qstring() const
{
    return QString((QChar*) data(), size());
}

// ----------------------------------------------------------------------
// Produces a string out of a KJS exception.
QString expt2str (ExecState *exec)
{
    JSValue *expt = exec->exception();
    if (   expt->isObject()
        && expt->getObject()->hasProperty(exec, "message"))
    {
        JSValue *msg = expt->getObject()->get(exec, "message");
        return QString("Error: %1").arg(msg->getString().qstring());
    }
    else
    {
        QString strexpt = exec->exception()->toString(exec).qstring();
        return QString("Caught exception: %1").arg(strexpt);
    }
}

// ----------------------------------------------------------------------
// Count number of lines in the string,
// up to and excluding the requested position.
int countLines (const QString &s, int p)
{
    int n = 1;
    int len = s.length();
    for (int i = 0; i < p && i < len; ++i) {
        if (s[i] == '\n') {
            ++n;
        }
    }
    return n;
}

// ----------------------------------------------------------------------
// Normalize string key for hash lookups,
QString normKeystr (const QString &raw)
{
    // NOTE: Regexes should not be used here for performance reasons.
    // This function may potentially be called thousands of times
    // on application startup.

    QString key = raw;

    // Strip all whitespace.
    int len = key.length();
    QString nkey;
    for (int i = 0; i < len; ++i) {
        QChar c = key[i];
        if (!c.isSpace()) {
            nkey.append(c);
        }
    }
    key = nkey;

    // Convert to lower case.
    key = key.toLower();

    return key;
}

// ----------------------------------------------------------------------
// Trim multiline string in a "smart" way:
// Remove leading and trailing whitespace up to and including first
// newline from that side, if there is one; otherwise, don't touch.
QString trimSmart (const QString &raw)
{
    // NOTE: This could be done by a single regex, but is not due to
    // performance reasons.
    // This function may potentially be called thousands of times
    // on application startup.

    int len = raw.length();

    int is = 0;
    while (is < len && raw[is].isSpace() && raw[is] != '\n') {
        ++is;
    }
    if (is >= len || raw[is] != '\n') {
        is = -1;
    }

    int ie = len - 1;
    while (ie >= 0 && raw[ie].isSpace() && raw[ie] != '\n') {
        --ie;
    }
    if (ie < 0 || raw[ie] != '\n') {
        ie = len;
    }

    return raw.mid(is + 1, ie - is - 1);
}

// ----------------------------------------------------------------------
// Produce a JavaScript object out of Qt variant.
JSValue *variantToJsValue (const QVariant &val)
{
    QVariant::Type vtype = val.type();
    if (vtype == QVariant::String)
        return jsString(val.toString());
    else if (   vtype == QVariant::Double \
             || vtype == QVariant::Int || vtype == QVariant::UInt \
             || vtype == QVariant::LongLong || vtype == QVariant::ULongLong)
        return jsNumber(val.toDouble());
    else
        return jsUndefined();
}

// ----------------------------------------------------------------------
// Dynamic loading.
K_GLOBAL_STATIC(KTranscriptImp, globalKTI)
extern "C"
{
    KDE_EXPORT KTranscript *load_transcript ()
    {
        return globalKTI;
    }
}

// ----------------------------------------------------------------------
// KTranscript definitions.

KTranscriptImp::KTranscriptImp ()
{}

KTranscriptImp::~KTranscriptImp ()
{
    // FIXME: vallgrind shows an afwul lot of "invalid read" in WTF:: stuff
    // when deref is called... Are we leaking somewhere?
    //foreach (Interpreter *jsi, m_jsi.values())
    //    jsi->deref();
}

QString KTranscriptImp::eval (const QList<QVariant> &argv,
                              const QString &lang,
                              const QString &lscr,
                              const QString &msgctxt,
                              const QHash<QString, QString> &dynctxt,
                              const QString &msgid,
                              const QStringList &subs,
                              const QList<QVariant> &vals,
                              const QString &final,
                              QList<QStringList> &mods,
                              QString &error,
                              bool &fallback)
{
    //error = "debug"; return QString();

    error.clear(); // empty error message means successful evaluation
    fallback = false; // fallback not requested

    #if 0
    // FIXME: Maybe not needed, as KJS has no native outside access?
    // Unportable (needs unistd.h)?

    // If effective user id is root and real user id is not root.
    if (geteuid() == 0 && getuid() != 0)
    {
        // Since scripts are user input, and the program is running with
        // root permissions while real user is not root, do not invoke
        // scripting at all, to prevent exploits.
        error = "Security block: trying to execute a script in suid environment.";
        return QString();
    }
    #endif

    // Load any new modules and clear the list.
    if (!mods.isEmpty())
    {
        loadModules(mods, error);
        mods.clear();
        if (!error.isEmpty())
            return QString();
    }

    // Add interpreters for new languages.
    // (though it should never happen here, but earlier when loading modules;
    // this also means there are no calls set, so the unregistered call error
    // below will be reported).
    if (!m_jsi.contains(lang))
        setupInterpreter(lang);

    // Shortcuts.
    Interpreter *jsi = m_jsi[lang];
    Scriptface *sface = m_sface[lang];
    ExecState *exec = jsi->globalExec();
    JSObject *gobj = jsi->globalObject();

    // Link current message data for script-side interface.
    sface->msgctxt = &msgctxt;
    sface->dynctxt = &dynctxt;
    sface->msgid = &msgid;
    sface->subs = &subs;
    sface->vals = &vals;
    sface->final = &final;
    sface->fallback = &fallback;
    sface->lscr = &lscr;

    // Find corresponding JS function.
    int argc = argv.size();
    if (argc < 1)
    {
        //error = "At least the call name must be supplied.";
        // Empty interpolation is OK, possibly used just to initialize
        // at a given point (e.g. for Ts.setForall() to start having effect).
        return QString();
    }
    QString funcName = argv[0].toString();
    if (!sface->funcs.contains(funcName))
    {
        error = QString("Unregistered call to '%1'.").arg(funcName);
        return QString();
    }
    JSObject *func = sface->funcs[funcName];
    JSValue *fval = sface->fvals[funcName];

    // Recover module path from the time of definition of this call,
    // for possible load calls.
    currentModulePath = sface->fpaths[funcName];

    // Execute function.
    List arglist;
    for (int i = 1; i < argc; ++i)
        arglist.append(variantToJsValue(argv[i]));
    JSValue *val;
    if (fval->isObject())
        val = func->callAsFunction(exec, fval->getObject(), arglist);
    else // no object associated to this function, use global
        val = func->callAsFunction(exec, gobj, arglist);

    if (fallback)
    // Fallback to ordinary translation requested.
    {
        // Possibly clear exception state.
        if (exec->hadException())
            exec->clearException();

        return QString();
    }
    else if (!exec->hadException())
    // Evaluation successful.
    {
        if (val->isString())
        // Good to go.
        {
            return val->getString().qstring();
        }
        else
        // Accept only strings.
        {
            QString strval = val->toString(exec).qstring();
            error = QString("Non-string return value: %1").arg(strval);
            return QString();
        }
    }
    else
    // Exception raised.
    {
        error = expt2str(exec);

        exec->clearException();

        return QString();
    }
}

QStringList KTranscriptImp::postCalls (const QString &lang)
{
    // Return no calls if scripting was not already set up for this language.
    // NOTE: This shouldn't happen, as postCalls cannot be called in such case.
    if (!m_sface.contains(lang))
        return QStringList();

    // Shortcuts.
    Scriptface *sface = m_sface[lang];

    return sface->nameForalls;
}

void KTranscriptImp::loadModules (const QList<QStringList> &mods,
                                  QString &error)
{
    QList<QString> modErrors;

    foreach (const QStringList &mod, mods)
    {
        QString mpath = mod[0];
        QString mlang = mod[1];

        // Add interpreters for new languages.
        if (!m_jsi.contains(mlang))
            setupInterpreter(mlang);

        // Setup current module path for loading submodules.
        // (sort of closure over invocations of loadf)
        int posls = mpath.lastIndexOf('/');
        if (posls < 1)
        {
            modErrors.append(QString("Funny module path '%1', skipping.")
                                    .arg(mpath));
            continue;
        }
        currentModulePath = mpath.left(posls);
        QString fname = mpath.mid(posls + 1);
        // Scriptface::loadf() wants no extension on the filename
        fname = fname.left(fname.lastIndexOf('.'));

        // Load the module.
        ExecState *exec = m_jsi[mlang]->globalExec();
        List alist;
        alist.append(jsString(fname));

        m_sface[mlang]->loadf(exec, alist);

        // Handle any exception.
        if (exec->hadException())
        {
            modErrors.append(expt2str(exec));
            exec->clearException();
        }
    }

    // Unset module path.
    currentModulePath.clear();

    foreach (const QString &merr, modErrors)
        error.append(merr + '\n');
}

#define SFNAME "Ts"
void KTranscriptImp::setupInterpreter (const QString &lang)
{
    // Create new interpreter.
    Interpreter *jsi = new Interpreter;
    jsi->initGlobalObject();
    jsi->ref();

    // Add scripting interface into the interpreter.
    Scriptface *sface = new Scriptface(jsi->globalExec());
    jsi->globalObject()->put(jsi->globalExec(), SFNAME, sface,
                             DontDelete|ReadOnly);

    // Store interpreter and scriptface.
    m_jsi[lang] = jsi;
    m_sface[lang] = sface;

    //dbgout("=====> Created interpreter for '%1'", lang);
}

// ----------------------------------------------------------------------
// Scriptface internal mechanics.
#include "ktranscript.lut.h"

/* Source for ScriptfaceProtoTable.
@begin ScriptfaceProtoTable 2
    load            Scriptface::Load            DontDelete|ReadOnly|Function 0
    setcall         Scriptface::Setcall         DontDelete|ReadOnly|Function 3
    callForall      Scriptface::CallForall      DontDelete|ReadOnly|Function 3
    fallback        Scriptface::Fallback        DontDelete|ReadOnly|Function 0
    nsubs           Scriptface::Nsubs           DontDelete|ReadOnly|Function 0
    subs            Scriptface::Subs            DontDelete|ReadOnly|Function 1
    vals            Scriptface::Vals            DontDelete|ReadOnly|Function 1
    msgctxt         Scriptface::Msgctxt         DontDelete|ReadOnly|Function 0
    dynctxt         Scriptface::Dynctxt         DontDelete|ReadOnly|Function 1
    msgid           Scriptface::Msgid           DontDelete|ReadOnly|Function 0
    msgkey          Scriptface::Msgkey          DontDelete|ReadOnly|Function 0
    msgstrf         Scriptface::Msgstrf         DontDelete|ReadOnly|Function 0
    dbgputs         Scriptface::Dbgputs         DontDelete|ReadOnly|Function 1
    lscr            Scriptface::Lscr            DontDelete|ReadOnly|Function 0
    normKey         Scriptface::normKey         DontDelete|ReadOnly|Function 1
    loadProps       Scriptface::loadProps       DontDelete|ReadOnly|Function 0
    getProp         Scriptface::getProp         DontDelete|ReadOnly|Function 2
    toUpperFirst    Scriptface::toUpperFirst    DontDelete|ReadOnly|Function 2
@end
*/
/* Source for ScriptfaceTable.
@begin ScriptfaceTable 0
@end
*/

KJS_DEFINE_PROTOTYPE(ScriptfaceProto)
KJS_IMPLEMENT_PROTOFUNC(ScriptfaceProtoFunc)
KJS_IMPLEMENT_PROTOTYPE("Scriptface", ScriptfaceProto, ScriptfaceProtoFunc)

const ClassInfo Scriptface::info = {"Scriptface", 0, &ScriptfaceTable, 0};

Scriptface::Scriptface (ExecState *exec)
: JSObject(ScriptfaceProto::self(exec)), fallback(NULL)
{}

Scriptface::~Scriptface ()
{}

bool Scriptface::getOwnPropertySlot (ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<Scriptface, JSObject>(exec, &ScriptfaceTable, this, propertyName, slot);
}

JSValue *Scriptface::getValueProperty (ExecState * /*exec*/, int token) const
{
    switch (token) {
        default:
            dbgout("Scriptface::getValueProperty: Unknown property id %1", token);
    }
    return jsUndefined();
}

void Scriptface::put (ExecState *exec, const Identifier &propertyName, JSValue *value, int attr)
{
    lookupPut<Scriptface, JSObject>(exec, propertyName, value, attr, &ScriptfaceTable, this);
}

void Scriptface::putValueProperty (ExecState * /*exec*/, int token, JSValue * /*value*/, int /*attr*/)
{
    switch(token) {
        default:
            dbgout("Scriptface::putValueProperty: Unknown property id %1", token);
    }
}

#define CALLARG(i) (args.size() > i ? args[i] : jsNull())
JSValue *ScriptfaceProtoFunc::callAsFunction (ExecState *exec, JSObject *thisObj, const List &args)
{
    if (!thisObj->inherits(&Scriptface::info)) {
        return throwError(exec, TypeError);
    }
    Scriptface *obj = static_cast<Scriptface*>(thisObj);
    switch (id) {
        case Scriptface::Load:
            return obj->loadf(exec, args);
        case Scriptface::Setcall:
            return obj->setcallf(exec, CALLARG(0), CALLARG(1), CALLARG(2));
        case Scriptface::CallForall:
            return obj->callForallf(exec, CALLARG(0), CALLARG(1), CALLARG(2));
        case Scriptface::Fallback:
            return obj->fallbackf(exec);
        case Scriptface::Nsubs:
            return obj->nsubsf(exec);
        case Scriptface::Subs:
            return obj->subsf(exec, CALLARG(0));
        case Scriptface::Vals:
            return obj->valsf(exec, CALLARG(0));
        case Scriptface::Msgctxt:
            return obj->msgctxtf(exec);
        case Scriptface::Dynctxt:
            return obj->dynctxtf(exec, CALLARG(0));
        case Scriptface::Msgid:
            return obj->msgidf(exec);
        case Scriptface::Msgkey:
            return obj->msgkeyf(exec);
        case Scriptface::Msgstrf:
            return obj->msgstrff(exec);
        case Scriptface::Dbgputs:
            return obj->dbgputsf(exec, CALLARG(0));
        case Scriptface::Lscr:
            return obj->lscrf(exec);
        case Scriptface::normKey:
            return obj->normKeyf(exec, CALLARG(0));
        case Scriptface::loadProps:
            return obj->loadPropsf(exec, args);
        case Scriptface::getProp:
            return obj->getPropf(exec, CALLARG(0), CALLARG(1));
        case Scriptface::toUpperFirst:
            return obj->toUpperFirstf(exec, CALLARG(0), CALLARG(1));
        default:
            return jsUndefined();
    }
}

// ----------------------------------------------------------------------
// Scriptface interface functions.
#define SPREF SFNAME"."

JSValue *Scriptface::loadf (ExecState *exec, const List &fnames)
{
    if (globalKTI->currentModulePath.isEmpty())
        return throwError(exec, GeneralError,
                          SPREF"load: no current module path, aiiie...");

    for (int i = 0; i < fnames.size(); ++i)
        if (!fnames[i]->isString())
            return throwError(exec, TypeError,
                              SPREF"load: expected string as file name");

    for (int i = 0; i < fnames.size(); ++i)
    {
        QString qfname = fnames[i]->getString().qstring();
        QString qfpath = globalKTI->currentModulePath + '/' + qfname + ".js";

        QFile file(qfpath);
        if (!file.open(QIODevice::ReadOnly))
            return throwError(exec, GeneralError,
                              QString(SPREF"load: cannot read file '%1'")\
                                     .arg(qfpath));

        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        QString source = stream.readAll();
        file.close();

        Interpreter *jsi = exec->lexicalInterpreter(); // shortcut
        // FIXME: Hmm... use dynamic or lexical interpreter?

        Completion comp = jsi->evaluate(qfpath, 0, source);

        if (comp.complType() == Throw)
        {
            JSValue *exval = comp.value();
            ExecState *exec = jsi->globalExec();
            QString msg = exval->toString(exec).qstring();

            QString line;
            if (exval->type() == ObjectType)
            {
                JSValue *lval = exval->getObject()->get(exec, "line");
                if (lval->type() == NumberType)
                    line = QString::number(lval->toInt32(exec));
            }

            return throwError(exec, TypeError,
                              QString("at %1:%2: %3")
                                     .arg(qfpath, line, msg));
        }
        dbgout("Loaded module: %1", qfpath);
    }

    return jsUndefined();
}

JSValue *Scriptface::setcallf (ExecState *exec, JSValue *name,
                               JSValue *func, JSValue *fval)
{
    if (!name->isString())
        return throwError(exec, TypeError,
                          SPREF"setcall: expected string as first argument");
    if (   !func->isObject()
        || !func->getObject()->implementsCall())
        return throwError(exec, TypeError,
                          SPREF"setcall: expected function as second argument");
    if (!(fval->isObject() || fval->isNull()))
        return throwError(exec, TypeError,
                          SPREF"setcall: expected object or null as third argument");

    QString qname = name->toString(exec).qstring();
    funcs[qname] = func->getObject();
    fvals[qname] = fval;

    // Register values to keep GC from collecting them. Is this needed?
    put(exec, Identifier(QString("#:f<%1>").arg(qname)), func, Internal);
    put(exec, Identifier(QString("#:o<%1>").arg(qname)), fval, Internal);

    // Set current module path as module path for this call,
    // in case it contains load subcalls.
    fpaths[qname] = globalKTI->currentModulePath;

    return jsUndefined();
}

JSValue *Scriptface::callForallf (ExecState *exec, JSValue *name,
                                  JSValue *func, JSValue *fval)
{
    if (!name->isString())
        return throwError(exec, TypeError,
                          SPREF"callForall: expected string as first argument");
    if (   !func->isObject()
        || !func->getObject()->implementsCall())
        return throwError(exec, TypeError,
                          SPREF"callForall: expected function as second argument");
    if (!(fval->isObject() || fval->isNull()))
        return throwError(exec, TypeError,
                          SPREF"callForall: expected object or null as third argument");

    QString qname = name->toString(exec).qstring();
    funcs[qname] = func->getObject();
    fvals[qname] = fval;

    // Register values to keep GC from collecting them. Is this needed?
    put(exec, Identifier(QString("#:fall<%1>").arg(qname)), func, Internal);
    put(exec, Identifier(QString("#:oall<%1>").arg(qname)), fval, Internal);

    // Set current module path as module path for this call,
    // in case it contains load subcalls.
    fpaths[qname] = globalKTI->currentModulePath;

    // Put in the queue order for execution on all messages.
    nameForalls.append(qname);

    return jsUndefined();
}

JSValue *Scriptface::fallbackf (ExecState *exec)
{
    Q_UNUSED(exec);
    if (fallback != NULL)
        *fallback = true;
    return jsUndefined();
}

JSValue *Scriptface::nsubsf (ExecState *exec)
{
    Q_UNUSED(exec);
    return jsNumber(subs->size());
}

JSValue *Scriptface::subsf (ExecState *exec, JSValue *index)
{
    if (!index->isNumber())
        return throwError(exec, TypeError,
                          SPREF"subs: expected number as first argument");

    int i = qRound(index->getNumber());
    if (i < 0 || i >= subs->size())
        return throwError(exec, RangeError,
                          SPREF"subs: index out of range");

    return jsString(subs->at(i));
}

JSValue *Scriptface::valsf (ExecState *exec, JSValue *index)
{
    if (!index->isNumber())
        return throwError(exec, TypeError,
                          SPREF"vals: expected number as first argument");

    int i = qRound(index->getNumber());
    if (i < 0 || i >= vals->size())
        return throwError(exec, RangeError,
                          SPREF"vals: index out of range");

    return variantToJsValue(vals->at(i));
}

JSValue *Scriptface::msgctxtf (ExecState *exec)
{
    Q_UNUSED(exec);
    return jsString(*msgctxt);
}

JSValue *Scriptface::dynctxtf (ExecState *exec, JSValue *key)
{
    if (!key->isString())
        return throwError(exec, TypeError,
                          SPREF"dynctxt: expected string as first argument");

    QString qkey = key->getString().qstring();
    if (dynctxt->contains(qkey)) {
        return jsString(dynctxt->value(qkey));
    }
    return jsUndefined();
}

JSValue *Scriptface::msgidf (ExecState *exec)
{
    Q_UNUSED(exec);
    return jsString(*msgid);
}

JSValue *Scriptface::msgkeyf (ExecState *exec)
{
    Q_UNUSED(exec);
    return jsString(*msgctxt + '|' + *msgid);
}

JSValue *Scriptface::msgstrff (ExecState *exec)
{
    Q_UNUSED(exec);
    return jsString(*final);
}

JSValue *Scriptface::dbgputsf (ExecState *exec, JSValue *str)
{
    if (!str->isString())
        return throwError(exec, TypeError,
                          SPREF"dbgputs: expected string as first argument");

    QString qstr = str->getString().qstring();

    dbgout("(JS) " + qstr);

    return jsUndefined();
}

JSValue *Scriptface::lscrf (ExecState *exec)
{
    Q_UNUSED(exec);
    return jsString(*lscr);
}

JSValue *Scriptface::normKeyf (ExecState *exec, JSValue *phrase)
{
    if (!phrase->isString()) {
        return throwError(exec, TypeError,
                          SPREF"normKey: expected string as argument");
    }

    QString nqphrase = normKeystr(phrase->toString(exec).qstring());
    return jsString(nqphrase);
}

JSValue *Scriptface::loadPropsf (ExecState *exec, const List &fnames)
{
    if (globalKTI->currentModulePath.isEmpty()) {
        return throwError(exec, GeneralError,
                          SPREF"loadProps: no current module path, aiiie...");
    }

    for (int i = 0; i < fnames.size(); ++i) {
        if (!fnames[i]->isString()) {
            return throwError(exec, TypeError,
                              SPREF"loadProps: expected string as file name");
        }
    }

    for (int i = 0; i < fnames.size(); ++i)
    {
        QString qfname = fnames[i]->getString().qstring();
        QString qfpath = globalKTI->currentModulePath + '/' + qfname + ".pmap";

        QFile file(qfpath);
        if (!file.open(QIODevice::ReadOnly)) {
            return throwError(exec, GeneralError,
                              QString(SPREF"loadProps: cannot read file '%1'")
                                     .arg(qfpath));
        }

        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        QString s = stream.readAll();
        file.close();

        // Parse the map.
        // Should care about performance: possibly executed on each KDE
        // app startup and reading houndreds of thousands of characters.
        enum {s_nextEntry, s_nextKey, s_nextValue};
        QStringList ekeys; // holds keys for current entry
        QHash<QString, QString> props; // holds properties for current entry
        int slen = s.length();
        int state = s_nextEntry;
        QString pkey;
        QChar prop_sep, key_sep;
        while (1) {
            int i_checkpoint = i;

            if (state == s_nextEntry) {
                while (s[i].isSpace()) {
                    ++i;
                    if (i >= slen) goto END_PROP_PARSE;
                }
                if (i + 1 >= slen) {
                    return throwError(exec, SyntaxError,
                        QString(SPREF"loadProps: unexpected end "
                                "of file in %1").arg(qfpath));
                }
                if (s[i] != '#') {
                    // Separator characters for this entry.
                    key_sep = s[i];
                    prop_sep = s[i + 1];
                    if (key_sep.isLetter() || prop_sep.isLetter()) {
                        return throwError(exec, SyntaxError,
                            QString(SPREF"loadProps: separator characters "
                                    "must not be letters at %1:%2")
                                   .arg(qfpath).arg(countLines(s, i)));
                    }

                    // Reset all data for current entry.
                    ekeys.clear();
                    props.clear();
                    pkey.clear();

                    i += 2;
                    state = s_nextKey;
                }
                else {
                    // This is a comment, skip to EOL, don't change state.
                    while (s[i] != '\n') {
                        ++i;
                        if (i >= slen) goto END_PROP_PARSE;
                    }
                }
            }
            else if (state == s_nextKey) {
                int ip = i;
                // Proceed up to next key or property separator.
                while (s[i] != key_sep && s[i] != prop_sep) {
                    ++i;
                    if (i >= slen) goto END_PROP_PARSE;
                }
                if (s[i] == key_sep) {
                    // This is a property key,
                    // record for when the value gets parsed.
                    pkey = normKeystr(s.mid(ip, i - ip));

                    i += 1;
                    state = s_nextValue;
                }
                else { // if (s[i] == prop_sep) {
                    // This is an entry key, or end of entry.
                    QString ekey = normKeystr(s.mid(ip, i - ip));
                    if (!ekey.isEmpty()) {
                        // An entry key.
                        ekeys.append(ekey);

                        i += 1;
                        state = s_nextKey;
                    }
                    else {
                        // End of entry.
                        if (ekeys.size() < 1) {
                            return throwError(exec, SyntaxError,
                                QString(SPREF"loadProps: no entry key "
                                        "for entry ending at %1:%2")
                                       .arg(qfpath).arg(countLines(s, i)));
                        }

                        // Put collected properties into global store,
                        // filed once under each entry key.
                        // TODO: Wastes memory, perhaps it would be better
                        // to store pointers to single property hash.
                        foreach (const QString &ekey, ekeys) {
                            phraseProps[ekey] = props;
                        }

                        i += 1;
                        state = s_nextEntry;
                    }
                }
            }
            else if (state == s_nextValue) {
                int ip = i;
                // Proceed up to next property separator.
                while (s[i] != prop_sep) {
                    ++i;
                    if (i >= slen) goto END_PROP_PARSE;
                    if (s[i] == key_sep) {
                        return throwError(exec, SyntaxError,
                            QString(SPREF"loadProps: property separator "
                                    "inside property value at %1:%2")
                                   .arg(qfpath).arg(countLines(s, i)));
                    }
                }
                // Extract the property value and store the property.
                QString pval = trimSmart(s.mid(ip, i - ip));
                props[pkey] = pval;

                i += 1;
                state = s_nextKey;
            }
            else {
                return throwError(exec, GeneralError,
                    QString(SPREF"loadProps: internal error 10 at %1:%2")
                           .arg(qfpath).arg(countLines(s, i)));
            }

            // To avoid infinite looping and stepping out.
            if (i == i_checkpoint || i >= slen) {
                return throwError(exec, GeneralError,
                    QString(SPREF"loadProps: internal error 20 at %1:%2")
                           .arg(qfpath).arg(countLines(s, i)));
            }
        }

        END_PROP_PARSE:
        if (state != s_nextEntry) {
            return throwError(exec, SyntaxError,
                              QString(SPREF"loadProps: unexpected end of "
                                      "file in %1").arg(qfpath));
        }

        dbgout("Loaded property map: %1", qfpath);
    }

    return jsUndefined();
}

JSValue *Scriptface::getPropf (ExecState *exec, JSValue *phrase, JSValue *prop)
{
    if (!phrase->isString()) {
        return throwError(exec, TypeError,
                          SPREF"getProp: expected string as first argument");
    }
    if (!prop->isString()) {
        return throwError(exec, TypeError,
                          SPREF"getProp: expected string as second argument");
    }

    QString qphrase = normKeystr(phrase->toString(exec).qstring());
    QString qprop = prop->toString(exec).qstring();

    if (phraseProps.contains(qphrase)) {
        if (phraseProps[qphrase].contains(qprop)) {
            return jsString(phraseProps[qphrase][qprop]);
        }
    }
    return jsUndefined();
}

JSValue *Scriptface::toUpperFirstf (ExecState *exec,
                                    JSValue *str, JSValue *nalt)
{
    static QString head("~@");
    static int hlen = head.length();

    if (!str->isString()) {
        return throwError(exec, TypeError,
                          SPREF"toUpperFirst: expected string as first argument");
    }
    if (!(nalt->isNumber() || nalt->isNull())) {
        return throwError(exec, TypeError,
                          SPREF"toUpperFirst: expected number as second argument");
    }

    QString qstr = str->toString(exec).qstring();
    int qnalt = nalt->isNull() ? 0 : nalt->toInteger(exec);

    // If the first letter is found within an alternatives directive,
    // upcase the first letter in each of the alternatives in that directive.

    QString qstruc = qstr;
    int len = qstr.length();
    QChar altSep;
    int remainingAlts = 0;
    bool checkCase = true;
    int numUpcased = 0;
    int i = 0;
    while (i < len) {
        QChar c = qstr[i];

        if (qnalt && !remainingAlts && qstr.mid(i, hlen) == head) {
            // An alternatives directive is just starting.
            i += 2;
            if (i >= len) break; // malformed directive, bail out
            // Record alternatives separator, set number of remaining
            // alternatives, reactivate case checking.
            altSep = qstruc[i];
            remainingAlts = qnalt;
            checkCase = true;
        }
        else if (remainingAlts && c == altSep) {
            // Alternative separator found, reduce number of remaining
            // alternatives and reactivate case checking.
            --remainingAlts;
            checkCase = true;
        }
        else if (checkCase && c.isLetter()) {
            // Case check is active and the character is a letter; upcase.
            qstruc[i] = c.toUpper();
            ++numUpcased;
            // No more case checks until next alternatives separator.
            checkCase = false;
        }

        // If any letter has been upcased, and there are no more alternatives
        // to be processed, we're done.
        if (numUpcased > 0 && remainingAlts == 0) {
            break;
        }

        // Go to next character.
        ++i;
    }

    return jsString(qstruc);
}

