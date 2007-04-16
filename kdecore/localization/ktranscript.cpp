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

#include <ktranscript.h>

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

#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
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

    const QString eval (const QStringList &argv,
                        const QString &lang,
                        const QString &msgctxt,
                        const QString &msgid,
                        const QStringList &subs,
                        const QString &final,
                        const QList<QStringList> &mods,
                        QString &error,
                        bool &fallback);

    QString currentModulePath;

    private:

    void loadModules (const QList<QStringList> &mods, QString &error);
    void setupInterpreter (const QString &lang);

    QMap<QString, Interpreter*> m_jsi;
    QMap<QString, Scriptface*> m_sface;
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
    JSValue *fallbackf (ExecState *exec);
    JSValue *nsubsf (ExecState *exec);
    JSValue *subsf (ExecState *exec, JSValue *index);
    JSValue *msgctxtf (ExecState *exec);
    JSValue *msgidf (ExecState *exec);
    JSValue *msgkeyf (ExecState *exec);
    JSValue *msgstrff (ExecState *exec);
    JSValue *dbgputsf (ExecState *exec, JSValue *str);

    enum {
        Load,
        Setcall,
        Fallback,
        Nsubs,
        Subs,
        Msgctxt,
        Msgid,
        Msgkey,
        Msgstrf,
        Dbgputs
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
    const QString *msgid;
    const QStringList *subs;
    const QString *final;

    // Fallback request handle.
    bool *fallback;

    // Function register.
    QMap<QString, JSObject*> funcs;
    QMap<QString, JSValue*> fvals;
    QMap<QString, QString> fpaths;
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
// Dynamic loading.
K_GLOBAL_STATIC(KTranscriptImp, globalKTI)
extern "C"
{
    KDECORE_EXPORT KTranscript *load_transcript ()
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

const QString KTranscriptImp::eval (const QStringList &argv,
                                    const QString &lang,
                                    const QString &msgctxt,
                                    const QString &msgid,
                                    const QStringList &subs,
                                    const QString &final,
                                    const QList<QStringList> &mods,
                                    QString &error,
                                    bool &fallback)
{
    //error = "debug"; return QString();

    error.clear(); // empty error message means successfull evaluation
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

    // Load any new modules.
    if (!mods.isEmpty())
    {
        loadModules(mods, error);
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
    sface->msgid = &msgid;
    sface->subs = &subs;
    sface->final = &final;
    sface->fallback = &fallback;

    // Find corresponding JS function.
    int argc = argv.size();
    if (argc < 1)
    {
        error = "At least the call name must be supplied.";
        return QString();
    }
    if (!sface->funcs.contains(argv[0]))
    {
        error = QString("Unregistered call to '%1'.").arg(argv[0]);
        return QString();
    }
    JSObject *func = sface->funcs[argv[0]];
    JSValue *fval = sface->fvals[argv[0]];

    // Recover module path from the time of definition of this call,
    // for possible load calls.
    currentModulePath = sface->fpaths[argv[0]];

    // Execute function.
    List arglist;
    for (int i = 1; i < argc; ++i)
        arglist.append(String(argv[i]));
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
    // Evaluation successfull.
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
        alist.append(String(fname));

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
    load        Scriptface::Load        DontDelete|ReadOnly|Function 0
    setcall     Scriptface::Setcall     DontDelete|ReadOnly|Function 3
    fallback    Scriptface::Fallback    DontDelete|ReadOnly|Function 0
    nsubs       Scriptface::Nsubs       DontDelete|ReadOnly|Function 0
    subs        Scriptface::Subs        DontDelete|ReadOnly|Function 1
    msgctxt     Scriptface::Msgctxt     DontDelete|ReadOnly|Function 0
    msgid       Scriptface::Msgid       DontDelete|ReadOnly|Function 0
    msgkey      Scriptface::Msgkey      DontDelete|ReadOnly|Function 0
    msgstrf     Scriptface::Msgstrf     DontDelete|ReadOnly|Function 0
    dbgputs     Scriptface::Dbgputs     DontDelete|ReadOnly|Function 1
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

JSValue *Scriptface::getValueProperty (ExecState */*exec*/, int token) const
{
    switch (token) {
        default:
            dbgout("Scriptface::getValueProperty: Unknown property id %1", token);
    }
    return Undefined();
}

void Scriptface::put (ExecState *exec, const Identifier &propertyName, JSValue *value, int attr)
{
    lookupPut<Scriptface, JSObject>(exec, propertyName, value, attr, &ScriptfaceTable, this);
}

void Scriptface::putValueProperty (ExecState */*exec*/, int token, JSValue */*value*/, int /*attr*/)
{
    switch(token) {
        default:
            dbgout("Scriptface::putValueProperty: Unknown property id %1", token);
    }
}

#define CALLARG(i) (args.size() > i ? args[i] : Null())
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
        case Scriptface::Fallback:
            return obj->fallbackf(exec);
        case Scriptface::Nsubs:
            return obj->nsubsf(exec);
        case Scriptface::Subs:
            return obj->subsf(exec, CALLARG(0));
        case Scriptface::Msgctxt:
            return obj->msgctxtf(exec);
        case Scriptface::Msgid:
            return obj->msgidf(exec);
        case Scriptface::Msgkey:
            return obj->msgkeyf(exec);
        case Scriptface::Msgstrf:
            return obj->msgstrff(exec);
        case Scriptface::Dbgputs:
            return obj->dbgputsf(exec, CALLARG(0));
        default:
            return Undefined();
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
                              QString("Cannot read file '%1'").arg(qfpath));

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

    return Undefined();
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

    return Undefined();
}

JSValue *Scriptface::fallbackf (ExecState *exec)
{
    Q_UNUSED(exec);
    if (fallback != NULL)
        *fallback = true;
    return Undefined();
}

JSValue *Scriptface::nsubsf (ExecState *exec)
{
    Q_UNUSED(exec);
    return Number(subs->size());
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

    return String(subs->at(i));
}

JSValue *Scriptface::msgctxtf (ExecState *exec)
{
    Q_UNUSED(exec);
    return String(*msgctxt);
}

JSValue *Scriptface::msgidf (ExecState *exec)
{
    Q_UNUSED(exec);
    return String(*msgid);
}

JSValue *Scriptface::msgkeyf (ExecState *exec)
{
    Q_UNUSED(exec);
    return String(*msgctxt + '|' + *msgid);
}

JSValue *Scriptface::msgstrff (ExecState *exec)
{
    Q_UNUSED(exec);
    return String(*final);
}

JSValue *Scriptface::dbgputsf (ExecState *exec, JSValue *str)
{
    if (!str->isString())
        return throwError(exec, TypeError,
                          SPREF"dbgputs: expected string as first argument");

    QString qstr = str->getString().qstring();

    dbgout("(JS) " + qstr);

    return Undefined();
}
