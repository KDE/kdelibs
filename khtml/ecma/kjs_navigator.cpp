// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
 *  Copyright (c) 2000 Daniel Molkentin (molkentin@kde.org)
 *  Copyright (c) 2000 Stefan Schimanski (schimmi@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <klocale.h>

#include <kstandarddirs.h>
#include <kconfig.h>
#include <kdebug.h>

#include <kio/kprotocolmanager.h>
#include "kjs_navigator.h"
#include "kjs_binding.h"
#include "khtml_part.h"

using namespace KJS;

namespace KJS {

    class PluginBase : public ObjectImp {
    public:
        PluginBase();
        virtual ~PluginBase();

        struct MimeClassInfo;
        struct PluginInfo;

        struct MimeClassInfo {
            QString type;
            QString desc;
            QString suffixes;
            PluginInfo *plugin;
        };

        struct PluginInfo {
            QString name;
            QString file;
            QString desc;
            QList<MimeClassInfo> mimes;
        };

        static QList<PluginInfo> *plugins;
        static QList<MimeClassInfo> *mimes;

    private:
        static int m_refCount;
    };


    class Plugins : public PluginBase {
    public:
        Plugins() {};
        virtual Value get(ExecState *exec, const UString &propertyName) const;
        virtual const ClassInfo* classInfo() const { return &info; }
        static const ClassInfo info;
    private:
    };
    const ClassInfo Plugins::info = { "PluginArray", 0, 0, 0 };


    class MimeTypes : public PluginBase {
    public:
        MimeTypes() { };
        virtual Value get(ExecState *exec, const UString &propertyName) const;
        virtual const ClassInfo* classInfo() const { return &info; }
        static const ClassInfo info;
    private:
    };
    const ClassInfo MimeTypes::info = { "MimeTypeArray", 0, 0, 0 };


    class Plugin : public ObjectImp {
    public:
        Plugin( PluginBase::PluginInfo *info ) { m_info = info; };
        virtual Value get(ExecState *exec, const UString &propertyName) const;
        virtual const ClassInfo* classInfo() const { return &info; }
        static const ClassInfo info;
    private:
        PluginBase::PluginInfo *m_info;
    };
    const ClassInfo Plugin::info = { "Plugin", 0, 0, 0 };


    class MimeType : public ObjectImp {
    public:
        MimeType( PluginBase::MimeClassInfo *info ) { m_info = info; };
        virtual Value get(ExecState *exec, const UString &propertyName) const;
        virtual const ClassInfo* classInfo() const { return &info; }
        static const ClassInfo info;
    private:
        PluginBase::MimeClassInfo *m_info;
    };
    const ClassInfo MimeType::info = { "MimeType", 0, 0, 0 };

    class PluginsFunc : public DOMFunction {
    public:
        PluginsFunc() : DOMFunction() { }
        virtual Value tryCall(ExecState *exec, Object &thisObj, const List &args);
    };


    class NavigatorFunc : public DOMFunction /*InternalFunctionImp*/ {
    public:
        NavigatorFunc(KHTMLPart *p) : DOMFunction(), part(p) { }
        virtual Value tryCall(ExecState *exec, Object &thisObj, const List &args);
    private:
        KHTMLPart *part;
    };
};


QList<PluginBase::PluginInfo> *KJS::PluginBase::plugins = 0;
QList<PluginBase::MimeClassInfo> *KJS::PluginBase::mimes = 0;
int KJS::PluginBase::m_refCount = 0;

const ClassInfo Navigator::info = { "Navigator", 0, 0, 0 };

bool Navigator::hasProperty(ExecState *exec, const UString &p, bool recursive) const
{
  if (p == "javaEnabled" ||
      p == "appCodeName" ||
      p == "appName" ||
      p == "appVersion" ||
      p == "language" ||
      p == "userAgent" ||
      p == "platform" ||
      p == "plugins" ||
      p == "mimeTypes" ||
      ObjectImp::hasProperty(exec, p, recursive) )
    return true;
  return false;
}

Value Navigator::get(ExecState *exec, const UString &propertyName) const
{
  KURL url = part->url();
  QString userAgent = KProtocolManager::userAgentForHost(url.host());

  if (propertyName == "javaEnabled")
     return /*Function*/ (new NavigatorFunc(part));
  else if (propertyName == "appCodeName")
    return String("Mozilla");
  else if (propertyName == "appName") {
    // If we find "Mozilla" but not "(compatible, ...)" we are a real Netscape
    if (userAgent.find(QString::fromLatin1("Mozilla")) >= 0 &&
        userAgent.find(QString::fromLatin1("compatible")) == -1)
    {
      //kdDebug() << "appName -> Mozilla" << endl;
      return String("Netscape");
    }
    if (userAgent.find(QString::fromLatin1("Microsoft")) >= 0 ||
        userAgent.find(QString::fromLatin1("MSIE")) >= 0)
    {
      //kdDebug() << "appName -> IE" << endl;
      return String("Microsoft Internet Explorer");
    }
    //kdDebug() << "appName -> Konqueror" << endl;
    return String("Konqueror");
  } else if (propertyName == "appVersion"){
    // We assume the string is something like Mozilla/version (properties)
    return String(userAgent.mid(userAgent.find('/') + 1));
  } else if (propertyName == "product") {
      return String("Konqueror/khtml");
  } else if (propertyName == "vendor") {
      return String("KDE");
  } else if (propertyName == "language") {
    return String(KGlobal::locale()->language() == "C" ?
                  QString::fromLatin1("en") : KGlobal::locale()->language());
  } else if (propertyName == "userAgent") {
    return String(userAgent);
  } else if (propertyName == "platform") {
    // yet another evil hack, but necessary to spoof some sites...
    if ( (userAgent.find(QString::fromLatin1("Win"),0,false)>=0) )
      return String(QString::fromLatin1("Win32"));
    else if ( (userAgent.find(QString::fromLatin1("Macintosh"),0,false)>=0) ||
              (userAgent.find(QString::fromLatin1("Mac_PowerPC"),0,false)>=0) )
      return String(QString::fromLatin1("MacPPC"));
    else
      return String(QString::fromLatin1("X11"));
  } else if (propertyName == "plugins") {
      return Value(new Plugins());
  } else if (propertyName == "mimeTypes") {
      return Value(new MimeTypes());
  } else
    return ObjectImp::get(exec, propertyName);
}


UString Navigator::toString(ExecState *) const
{
  return "[object Navigator]";
}

/*******************************************************************/

PluginBase::PluginBase()
{
    if ( !plugins ) {
        plugins = new QList<PluginInfo>;
        mimes = new QList<MimeClassInfo>;
        plugins->setAutoDelete( true );
        mimes->setAutoDelete( true );

        // read configuration
        KConfig c(KGlobal::dirs()->saveLocation("data","nsplugins")+"/pluginsinfo");
        unsigned num = (unsigned int)c.readNumEntry("number");
        for ( unsigned n=0; n<num; n++ ) {

            c.setGroup( QString::number(n) );
            PluginInfo *plugin = new PluginInfo;

            plugin->name = c.readEntry("name");
            plugin->file = c.readEntry("file");
            plugin->desc = c.readEntry("description");

            //kdDebug(6070) << "plugin : " << plugin->name << " - " << plugin->desc << endl;

            plugins->append( plugin );

            // get mime types from string
            QStringList types = QStringList::split( ';', c.readEntry("mime") );
            QStringList::Iterator type;
            for ( type=types.begin(); type!=types.end(); ++type ) {

                // get mime information
                MimeClassInfo *mime = new MimeClassInfo;
                QStringList tokens = QStringList::split(':', *type, TRUE);
                QStringList::Iterator token;

                token = tokens.begin();
                mime->type = (*token).lower();
                //kdDebug(6070) << "mime->type=" << mime->type << endl;
                ++token;

                mime->suffixes = *token;
                ++token;

                mime->desc = *token;
                ++token;

                mime->plugin = plugin;

                mimes->append( mime );
                plugin->mimes.append( mime );
            }
        }
    }

    m_refCount++;
}

PluginBase::~PluginBase()
{
    m_refCount--;
    if ( m_refCount==0 ) {
        delete plugins;
        delete mimes;
        plugins = 0;
        mimes = 0;
    }
}


/*******************************************************************/


Value Plugins::get(ExecState *exec, const UString &propertyName) const
{
    if (propertyName == "refresh")
        return /*Function*/(new PluginsFunc());
    else if ( propertyName =="length" )
        return Number(plugins->count());
    else {

        // plugins[#]
        bool ok;
        unsigned int i = propertyName.toULong(&ok);
        if( ok && i<plugins->count() )
            return Value( new Plugin( plugins->at(i) ) );

        // plugin[name]
        for ( PluginInfo *pl = plugins->first(); pl!=0; pl = plugins->next() ) {
            if ( pl->name==propertyName.string() )
                return Value( new Plugin( pl ) );
        }
    }

    return PluginBase::get(exec, propertyName);
}

/*******************************************************************/

Value MimeTypes::get(ExecState *exec, const UString &propertyName) const
{
    if( propertyName=="length" )
        return Number( mimes->count() );
    else {

        // mimeTypes[#]
        bool ok;
        unsigned int i = propertyName.toULong(&ok);
        if( ok && i<mimes->count() )
            return Value( new MimeType( mimes->at(i) ) );

        // mimeTypes[name]
        //kdDebug(6070) << "MimeTypes[" << propertyName.ascii() << "]" << endl;
        for ( MimeClassInfo *m=mimes->first(); m!=0; m=mimes->next() ) {
            //kdDebug(6070) << "m->type=" << m->type.ascii() << endl;
            if ( m->type == propertyName.string() )
                return Value( new MimeType( m ) );
        }
    }

    return PluginBase::get(exec, propertyName);
}


/************************************************************************/

Value Plugin::get(ExecState *exec, const UString &propertyName) const
{
    if ( propertyName=="name" )
        return String( m_info->name );
    else if ( propertyName == "filename" )
        return String( m_info->file );
    else if ( propertyName == "description" )
        return String( m_info->desc );
    else if ( propertyName == "length" )
        return Number( m_info->mimes.count() );
    else {

        // plugin[#]
        bool ok;
        unsigned int i = propertyName.toULong(&ok);
        //kdDebug(6070) << "Plugin::get plugin[" << i << "]" << endl;
        if( ok && i<m_info->mimes.count() )
        {
            //kdDebug(6070) << "returning mimetype " << m_info->mimes.at(i)->type << endl;
            return new MimeType( m_info->mimes.at(i) );
        }

        // plugin["name"]
        for ( PluginBase::MimeClassInfo *m=m_info->mimes.first();
              m!=0; m=m_info->mimes.next() ) {
            if ( m->type==propertyName.string() )
                return new MimeType( m );
        }

    }

    return ObjectImp::get(exec,propertyName);
}


/*****************************************************************************/

Value MimeType::get(ExecState *exec, const UString &propertyName) const
{
    if ( propertyName == "type" )
        return String( m_info->type );
    else if ( propertyName == "suffixes" )
        return String( m_info->suffixes );
    else if ( propertyName == "description" )
        return String( m_info->desc );
    else if ( propertyName == "enabledPlugin" )
        return new Plugin( m_info->plugin );

    return ObjectImp::get(exec,propertyName);
}


Value PluginsFunc::tryCall(ExecState *, Object &, const List &)
{
  return Undefined();
}


Value NavigatorFunc::tryCall(ExecState *, Object &, const List &)
{
  // javaEnabled()
  return Boolean(part->javaEnabled());
}
