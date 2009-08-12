/***************************************************************************
 * action.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "action.h"
#include "actioncollection.h"
#include "interpreter.h"
#include "script.h"
#include "manager.h"
#include "wrapperinterface.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include <klocale.h>
#include <kicon.h>
#include <kmimetype.h>

using namespace Kross;

namespace Kross {

    /// \internal d-pointer class.
    class Action::Private
    {
        public:

            /**
            * The \a Script instance the \a Action uses if initialized. It will
            * be NULL as long as we didn't initialized it what will be done on
            * demand.
            */
            Script* script;

            /**
            * The version number this \a Action has. Those version number got
            * used internaly to deal with different releases of scripts.
            */
            int version;

            /**
            * The optional description to provide some more details about the
            * Action to the user.
            */
            QString description;

            /**
            * The name of the icon.
            */
            QString iconname;

            /**
            * The scripting code.
            */
            QByteArray code;

            /**
            * The name of the interpreter. This could be something
            * like for example "python" for the python binding.
            */
            QString interpretername;

            /**
            * The name of the scriptfile that should be executed. Those
            * scriptfile will be readed and the content will be used to
            * set the scripting code and, if not defined, the used
            * interpreter.
            */
            QString scriptfile;

            /**
            * The path list where the \a Script may be located.
            * \todo after BIC break: don't keep it all the time,
            * as it is now passed to [to|from]DomElement
            */
            QStringList searchpath;

            /**
            * Map of options that overwritte the \a InterpreterInfo::Option::Map
            * standard options.
            */
            QMap< QString, QVariant > options;

            Private() : script(0), version(0) {}
    };

}

enum InitOptions{Enable=1};
void static init(Action* th, const QString& name, int options=0)
{
    th->setEnabled(options&Enable);
    th->setObjectName(name);
    #ifdef KROSS_ACTION_DEBUG
        krossdebug( QString("Action::Action(QObject*,QString,QDir) Ctor name='%1'").arg(th->objectName()) );
    #endif
    QObject::connect(th, SIGNAL(triggered(bool)), th, SLOT(slotTriggered()));
}

Action::Action(QObject* parent, const QString& name, const QDir& packagepath)
    : QAction(parent)
    , QScriptable()
    , ChildrenInterface()
    , ErrorInterface()
    , d( new Private() )
{
    init(this,name);
    d->searchpath=QStringList(packagepath.absolutePath());
}

Action::Action(QObject* parent, const QUrl& url)
    : QAction(parent)
    , ChildrenInterface()
    , ErrorInterface()
    , d( new Private() )
{
    init(this,url.path(),Enable);
    QFileInfo fi( url.toLocalFile() );
    setText( fi.fileName() );
    setIconName( KMimeType::iconNameForUrl(url) );
    setFile( url.toLocalFile() );
}

Action::~Action()
{
    #ifdef KROSS_ACTION_DEBUG
        krossdebug( QString("Action::~Action() Dtor name='%1'").arg(objectName()) );
    #endif
    finalize();
    ActionCollection *coll = qobject_cast<ActionCollection*>(parent());
    if ( coll ) {
        coll->removeAction(this);
    }
    delete d;
}


void Action::fromDomElement(const QDomElement& element)
{
    fromDomElement(element, d->searchpath);
}

void Action::fromDomElement(const QDomElement& element, const QStringList& searchPath)
{
    if( element.isNull() )
        return;

    QString file = element.attribute("file");
    if( ! file.isEmpty() ) {
        if( QFileInfo(file).exists() ) {
            setFile(file);
        }
        else {
            foreach (const QString& packagepath, searchPath) {
                QFileInfo fi(QDir(packagepath), file);
                if( fi.exists() ) {
                    setFile( fi.absoluteFilePath() );
                    break;
                }
            }
        }
    }

    d->version = QVariant( element.attribute("version",QString(d->version)) ).toInt();

    setText( i18n( element.attribute("text").toUtf8() ) );
    setDescription( i18n( element.attribute("comment").toUtf8() ) );
    setEnabled( true );
    setInterpreter( element.attribute("interpreter") );
    setEnabled( QVariant(element.attribute("enabled","true")).toBool() && isEnabled() );

    QString icon = element.attribute("icon");
    if( icon.isEmpty() && ! d->scriptfile.isNull() )
        icon = KMimeType::iconNameForUrl( KUrl(d->scriptfile) );
    setIconName( icon );

    const QString code = element.attribute("code");
    if( ! code.isNull() )
        setCode(code.toUtf8());

    for(QDomNode node = element.firstChild(); ! node.isNull(); node = node.nextSibling()) {
        QDomElement e = node.toElement();
        if( ! e.isNull() ) {
            if( e.tagName() == "property" ) {
                const QString n = e.attribute("name", QString());
                if( ! n.isNull() ) {
                    #ifdef KROSS_ACTION_DEBUG
                        krossdebug(QString("Action::readDomElement: Setting property name=%1 value=%2").arg(n).arg(e.text()));
                    #endif
                    setProperty(n.toLatin1().constData(), QVariant(e.text()));
                }
            }
        }
    }
}

QDomElement Action::toDomElement() const
{
    return toDomElement(QStringList());
}

QDomElement Action::toDomElement(const QStringList& searchPath) const
{
    QDomDocument doc;
    QDomElement e = doc.createElement("script");
    e.setAttribute("name", objectName());
    if( d->version > 0 )
        e.setAttribute("version", QString(d->version));
    if( ! text().isNull() )
        e.setAttribute("text", text());
    if( ! description().isNull() )
        e.setAttribute("comment", description());
    if( ! iconName().isNull() )
        e.setAttribute("icon", iconName());
    if( ! isEnabled() )
        e.setAttribute("enabled", "false");
    if( ! interpreter().isNull() )
        e.setAttribute("interpreter", interpreter());


    QString fileName=file();
    if (!searchPath.isEmpty()) {
        //fileName=QDir(searchPath.first()).relativeFilePath(fileName); //prefer absname if it is short?
        foreach(const QString& packagepath, searchPath) {
            QString nfn=QDir(packagepath).relativeFilePath(file());
            if (nfn.length()<fileName.length())
                fileName=nfn;
        }
    }

    if( ! fileName.isNull() ) {
        e.setAttribute("file", fileName);
    }
    
    QList<QByteArray> props=dynamicPropertyNames();
    foreach(const QByteArray& prop, props) {
        QDomElement p = doc.createElement("property");
        p.setAttribute("name", QString::fromLatin1(prop));
        p.appendChild(doc.createTextNode(property(prop.constData()).toString()));
        e.appendChild(p);
    }
    /*
    else if( ! code().isNull() ) {
        e.setAttribute("code", code());
    }
    */

    return e;
}

Kross::Script* Action::script() const
{
    return d->script;
}

QString Action::name() const
{
    return objectName();
}

int Action::version() const
{
    return d->version;
}

QString Action::description() const
{
    return d->description;
}

void Action::setDescription(const QString& description)
{
    d->description = description;
    emit dataChanged(this);
    emit updated();
}

QString Action::iconName() const
{
    return d->iconname;
}

void Action::setIconName(const QString& iconname)
{
    setIcon( KIcon(iconname) );
    d->iconname = iconname;
    emit dataChanged(this);
    emit updated();
}

bool Action::isEnabled() const
{
    return QAction::isEnabled();
}

void Action::setEnabled(bool enabled)
{
    QAction::setEnabled(enabled);
    emit dataChanged(this);
    emit updated();
}

QByteArray Action::code() const
{
    return d->code;
}

void Action::setCode(const QByteArray& code)
{
    if( d->code != code ) {
        finalize();
        d->code = code;
        emit dataChanged(this);
        emit updated();
    }
}

QString Action::interpreter() const
{
    return d->interpretername;
}

void Action::setInterpreter(const QString& interpretername)
{
    if( d->interpretername != interpretername ) {
        finalize();
        d->interpretername = interpretername;
        setEnabled( Manager::self().interpreters().contains(interpretername) );
        if (!isEnabled())
            krosswarning("Action::setInterpreter: interpreter not found: "+interpretername);
        emit dataChanged(this);
        emit updated();
    }
}

QString Action::file() const
{
    return d->scriptfile;
}

bool Action::setFile(const QString& scriptfile)
{
    if( d->scriptfile != scriptfile ) {
        finalize();
        if ( scriptfile.isNull() ) {
            if( ! d->scriptfile.isNull() )
                d->interpretername.clear();
            d->scriptfile.clear();
            d->searchpath.clear();
        }
        else {
            d->scriptfile = scriptfile;
            d->interpretername = Manager::self().interpreternameForFile(scriptfile);
            if( d->interpretername.isNull() )
                return false;
        }
    }
    return true;
}

QString Action::currentPath() const
{
    return file().isEmpty()?QString():QFileInfo(file()).absolutePath();//obey Qt docs and don't cheat
}

QVariantMap Action::options() const
{
    return d->options;
}

void Action::addQObject(QObject* obj, const QString &name)
{
    this->addObject(obj, name);
}

QObject* Action::qobject(const QString &name) const
{
    return this->object(name);
}

QStringList Action::qobjectNames() const
{
    return this->objects().keys();
}

QVariant Action::option(const QString& name, const QVariant& defaultvalue)
{
    if(d->options.contains(name))
        return d->options[name];
    InterpreterInfo* info = Manager::self().interpreterInfo( d->interpretername );
    return info ? info->optionValue(name, defaultvalue) : defaultvalue;
}

bool Action::setOption(const QString& name, const QVariant& value)
{
    InterpreterInfo* info = Manager::self().interpreterInfo( d->interpretername );
    if(info) {
        if(info->hasOption(name)) {
            d->options.insert(name, value);
            return true;
        } else krosswarning( QString("Kross::Action::setOption(%1, %2): No such option").arg(name).arg(value.toString()) );
    } else krosswarning( QString("Kross::Action::setOption(%1, %2): No such interpreterinfo").arg(name).arg(value.toString()) );
    return false;
}

QStringList Action::functionNames()
{
    if(! d->script) {
        if(! initialize())
            return QStringList();
    }
    return d->script->functionNames();
}

QVariant Action::callFunction(const QString& name, const QVariantList& args)
{
    if(! d->script) {
        if(! initialize())
            return QVariant();
    }
    return d->script->callFunction(name, args);
}

QVariant Action::evaluate(const QByteArray& code)
{
    if(! d->script) {
        if(! initialize())
            return QVariant();
    }
    return d->script->evaluate(code);
}

bool Action::initialize()
{
    finalize();

    if( ! d->scriptfile.isNull() ) {
        QFile f( d->scriptfile );
        if( ! f.exists() ) {
            setError(i18n("Scriptfile \"%1\" does not exist.", d->scriptfile));
            return false;
        }
        if( d->interpretername.isNull() ) {
            setError(i18n("Failed to determine interpreter for scriptfile \"%1\"", d->scriptfile));
            return false;
        }
        if( ! f.open(QIODevice::ReadOnly) ) {
            setError(i18n("Failed to open scriptfile \"%1\"", d->scriptfile));
            return false;
        }
        d->code = f.readAll();
        f.close();
    }

    Interpreter* interpreter = Manager::self().interpreter(d->interpretername);
    if( ! interpreter ) {
        InterpreterInfo* info = Manager::self().interpreterInfo(d->interpretername);
        if( info )
            setError(i18n("Failed to load interpreter \"%1\"", d->interpretername));
        else
            setError(i18n("No such interpreter \"%1\"", d->interpretername));
        return false;
    }

    d->script = interpreter->createScript(this);
    if( ! d->script ) {
        setError(i18n("Failed to create script for interpreter \"%1\"", d->interpretername));
        return false;
    }

    if( d->script->hadError() ) {
        setError(d->script);
        finalize();
        return false;
    }

    clearError(); // clear old exception
    return true;
}

void Action::finalize()
{
    if( d->script )
        emit finalized(this);
    delete d->script;
    d->script = 0;
}

bool Action::isFinalized() const
{
    return d->script == 0;
}

void Action::slotTriggered()
{
    #ifdef KROSS_ACTION_DEBUG
        krossdebug( QString("Action::slotTriggered() name=%1").arg(objectName()) );
    #endif

    emit started(this);

    if( ! d->script ) {
        if( ! initialize() )
            Q_ASSERT( hadError() );
    }

    if( hadError() ) {
        #ifdef KROSS_ACTION_DEBUG
            krossdebug( QString("Action::slotTriggered() on init, errorMessage=%2").arg(errorMessage()) );
        #endif
    }
    else {
        d->script->execute();
        if( d->script->hadError() ) {
            #ifdef KROSS_ACTION_DEBUG
                krossdebug( QString("Action::slotTriggered() after exec, errorMessage=%2").arg(errorMessage()) );
            #endif
            setError(d->script);
            //emit finished(this);
            finalize();
            //return;
        }
    }

    emit finished(this);
}

// --------

// interface files
WrapperInterface::~WrapperInterface()
{
}

#include "action.moc"
