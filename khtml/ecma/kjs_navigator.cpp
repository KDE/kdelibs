// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
 *  Copyright (c) 2000 Daniel Molkentin (molkentin@kde.org)
 *  Copyright (c) 2000 Stefan Schimanski (schimmi@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <klocale.h>

#include <kstandarddirs.h>
#include <kconfig.h>
#include <kdebug.h>

#include <kio/kprotocolmanager.h>
#include "kjs_navigator.h"
#include "kjs/lookup.h"
#include "kjs_binding.h"
#include "khtml_part.h"
#include <sys/utsname.h>
#include "kjs_navigator.lut.h"

using namespace KJS;

namespace KJS {

    // All objects that need plugin info must inherit from PluginBase
    // Its ctor and dtor take care of the refcounting on the static lists.
    class PluginBase : public ObjectImp {
    public:
        PluginBase(ExecState *exec);
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
            QPtrList<MimeClassInfo> mimes;
        };

        static QPtrList<PluginInfo> *plugins;
        static QPtrList<MimeClassInfo> *mimes;

    private:
        static int m_refCount;
    };


    class Plugins : public PluginBase {
    public:
        Plugins(ExecState *exec) : PluginBase(exec) {};
        virtual Value get(ExecState *exec, const UString &propertyName) const;
        virtual const ClassInfo* classInfo() const { return &info; }
        static const ClassInfo info;
    private:
    };
    const ClassInfo Plugins::info = { "PluginArray", 0, 0, 0 };


    class MimeTypes : public PluginBase {
    public:
        MimeTypes(ExecState *exec) : PluginBase(exec) { };
        virtual Value get(ExecState *exec, const UString &propertyName) const;
        virtual const ClassInfo* classInfo() const { return &info; }
        static const ClassInfo info;
    private:
    };
    const ClassInfo MimeTypes::info = { "MimeTypeArray", 0, 0, 0 };


    class Plugin : public PluginBase {
    public:
        Plugin( ExecState *exec, PluginBase::PluginInfo *info )
          : PluginBase( exec )
        { m_info = info; };
        virtual Value get(ExecState *exec, const UString &propertyName) const;
        virtual const ClassInfo* classInfo() const { return &info; }
        static const ClassInfo info;
    private:
        PluginBase::PluginInfo *m_info;
    };
    const ClassInfo Plugin::info = { "Plugin", 0, 0, 0 };


    class MimeType : public PluginBase {
    public:
        MimeType( ExecState *exec, PluginBase::MimeClassInfo *info )
          : PluginBase( exec )
        { m_info = info; };
        virtual Value get(ExecState *exec, const UString &propertyName) const;
        virtual const ClassInfo* classInfo() const { return &info; }
        static const ClassInfo info;
    private:
        PluginBase::MimeClassInfo *m_info;
    };
    const ClassInfo MimeType::info = { "MimeType", 0, 0, 0 };

};


QPtrList<PluginBase::PluginInfo> *KJS::PluginBase::plugins = 0;
QPtrList<PluginBase::MimeClassInfo> *KJS::PluginBase::mimes = 0;
int KJS::PluginBase::m_refCount = 0;

const ClassInfo Navigator::info = { "Navigator", 0, &NavigatorTable, 0 };
/*
@begin NavigatorTable 11
  appCodeName	Navigator::AppCodeName	DontDelete|ReadOnly
  appName	Navigator::AppName	DontDelete|ReadOnly
  appVersion	Navigator::AppVersion	DontDelete|ReadOnly
  language	Navigator::Language	DontDelete|ReadOnly
  userAgent	Navigator::UserAgent	DontDelete|ReadOnly
  userLanguage	Navigator::UserLanguage	DontDelete|ReadOnly
  platform	Navigator::Platform	DontDelete|ReadOnly
  plugins	Navigator::_Plugins	DontDelete|ReadOnly
  mimeTypes	Navigator::_MimeTypes	DontDelete|ReadOnly
  product	Navigator::Product	DontDelete|ReadOnly
  vendor	Navigator::Vendor	DontDelete|ReadOnly
  cookieEnabled	Navigator::CookieEnabled DontDelete|ReadOnly
  javaEnabled	Navigator::JavaEnabled	DontDelete|Function 0
@end
*/
IMPLEMENT_PROTOFUNC_DOM(NavigatorFunc)

Navigator::Navigator(ExecState *exec, KHTMLPart *p)
  : ObjectImp(exec->interpreter()->builtinObjectPrototype()), m_part(p) { }

Value Navigator::get(ExecState *exec, const UString &propertyName) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Navigator::get " << propertyName.ascii() << endl;
#endif
  return lookupGet<NavigatorFunc,Navigator,ObjectImp>(exec,propertyName,&NavigatorTable,this);
}

Value Navigator::getValueProperty(ExecState *exec, int token) const
{
  KURL url = m_part->url();
  QString userAgent = KProtocolManager::userAgentForHost(url.host());
  switch (token) {
  case AppCodeName:
    return String("Mozilla");
  case AppName:
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
  case AppVersion:
    // We assume the string is something like Mozilla/version (properties)
    return String(userAgent.mid(userAgent.find('/') + 1));
  case Product:
    return String("Konqueror/khtml");
  case Vendor:
    return String("KDE");
  case Language:
  case UserLanguage:
    return String(KGlobal::locale()->language());
  case UserAgent:
    return String(userAgent);
  case Platform:
    // yet another evil hack, but necessary to spoof some sites...
    if ( (userAgent.find(QString::fromLatin1("Win"),0,false)>=0) )
      return String(QString::fromLatin1("Win32"));
    else if ( (userAgent.find(QString::fromLatin1("Macintosh"),0,false)>=0) ||
              (userAgent.find(QString::fromLatin1("Mac_PowerPC"),0,false)>=0) )
      return String(QString::fromLatin1("MacPPC"));
    else
    {
        struct utsname name;
        int ret = uname(&name);
        if ( ret >= 0 )
            return String(QString::fromLatin1("%1 %1 X11").arg(name.sysname).arg(name.machine));
        else // can't happen
            return String(QString::fromLatin1("Unix X11"));
    }
  case _Plugins:
    return Value(new Plugins(exec));
  case _MimeTypes:
    return Value(new MimeTypes(exec));
  case CookieEnabled:
    return Boolean(true); /// ##### FIXME
  default:
    kdWarning() << "Unhandled token in DOMEvent::getValueProperty : " << token << endl;
    return Value();
  }
}

/*******************************************************************/

PluginBase::PluginBase(ExecState *exec)
  : ObjectImp(exec->interpreter()->builtinObjectPrototype() )
{
    if ( !plugins ) {
        plugins = new QPtrList<PluginInfo>;
        mimes = new QPtrList<MimeClassInfo>;
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
IMPLEMENT_PROTOFUNC_DOM(PluginsFunc)

Value Plugins::get(ExecState *exec, const UString &propertyName) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Plugins::get " << propertyName.qstring() << endl;
#endif
    if (propertyName == "refresh")
      return lookupOrCreateFunction<PluginsFunc>(exec,propertyName,this,0,0,DontDelete|Function);
    else if ( propertyName =="length" )
      return Number(plugins->count());
    else {

        // plugins[#]
        bool ok;
        unsigned int i = propertyName.toULong(&ok);
        if( ok && i<plugins->count() )
            return Value( new Plugin( exec, plugins->at(i) ) );

        // plugin[name]
        for ( PluginInfo *pl = plugins->first(); pl!=0; pl = plugins->next() ) {
            if ( pl->name==propertyName.string() )
                return Value( new Plugin( exec, pl ) );
        }
    }

    return PluginBase::get(exec, propertyName);
}

/*******************************************************************/

Value MimeTypes::get(ExecState *exec, const UString &propertyName) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "MimeTypes::get " << propertyName.qstring() << endl;
#endif
    if( propertyName=="length" )
        return Number( mimes->count() );
    else {

        // mimeTypes[#]
        bool ok;
        unsigned int i = propertyName.toULong(&ok);
        if( ok && i<mimes->count() )
            return Value( new MimeType( exec, mimes->at(i) ) );

        // mimeTypes[name]
        //kdDebug(6070) << "MimeTypes[" << propertyName.ascii() << "]" << endl;
        for ( MimeClassInfo *m=mimes->first(); m!=0; m=mimes->next() ) {
            //kdDebug(6070) << "m->type=" << m->type.ascii() << endl;
            if ( m->type == propertyName.string() )
                return Value( new MimeType( exec, m ) );
        }
    }

    return PluginBase::get(exec, propertyName);
}


/************************************************************************/

Value Plugin::get(ExecState *exec, const UString &propertyName) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Plugin::get " << propertyName.qstring() << endl;
#endif
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
            return Value(new MimeType(exec, m_info->mimes.at(i)));
        }

        // plugin["name"]
        for ( PluginBase::MimeClassInfo *m=m_info->mimes.first();
              m!=0; m=m_info->mimes.next() ) {
            if ( m->type==propertyName.string() )
                return Value(new MimeType(exec, m));
        }

    }

    return ObjectImp::get(exec,propertyName);
}


/*****************************************************************************/

Value MimeType::get(ExecState *exec, const UString &propertyName) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "MimeType::get " << propertyName.qstring() << endl;
#endif
    if ( propertyName == "type" )
        return String( m_info->type );
    else if ( propertyName == "suffixes" )
        return String( m_info->suffixes );
    else if ( propertyName == "description" )
        return String( m_info->desc );
    else if ( propertyName == "enabledPlugin" )
        return Value(new Plugin(exec, m_info->plugin));

    return ObjectImp::get(exec,propertyName);
}


Value PluginsFunc::tryCall(ExecState *, Object &, const List &)
{
  return Undefined();
}


Value NavigatorFunc::tryCall(ExecState *exec, Object &thisObj, const List &)
{
  KJS_CHECK_THIS( KJS::Navigator, thisObj );
  Navigator *nav = static_cast<Navigator *>(thisObj.imp());
  // javaEnabled()
  return Boolean(nav->part()->javaEnabled());
}
