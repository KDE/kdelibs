// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
 *  Copyright (c) 2000 Daniel Molkentin (molkentin@kde.org)
 *  Copyright (c) 2000 Stefan Schimanski (schimmi@kde.org)
 *  Copyright (C) 2003 Apple Computer, Inc.
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <klocale.h>

#include <kstandarddirs.h>
#include <kconfig.h>
#include <kdebug.h>

#include <kio/kprotocolmanager.h>
#include <kio/kmimetype.h>
#include <kio/kservice.h>
#include <kio/ktrader.h>
#include "kjs_navigator.h"
#include "kjs/lookup.h"
#include "kjs_binding.h"
#include "khtml_part.h"
#include <sys/utsname.h>
#include "kjs_navigator.lut.h"

#include <q3ptrlist.h>

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
            Q3PtrList<MimeClassInfo> mimes;
        };

        static Q3PtrList<PluginInfo> *plugins;
        static Q3PtrList<MimeClassInfo> *mimes;

    private:
        static int m_refCount;
    };


    class Plugins : public PluginBase {
    public:
        Plugins(ExecState *exec) : PluginBase(exec) {};
        virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
        ValueImp *getValueProperty(ExecState *exec, int token) const;
        virtual const ClassInfo* classInfo() const { return &info; }
        static const ClassInfo info;
        static ValueImp *pluginByName( ExecState* exec, const QString& name );
    private:
        static ValueImp *indexGetter(ExecState *, const Identifier&, const PropertySlot&);
        static ValueImp *nameGetter(ExecState *, const Identifier&, const PropertySlot&);
    };


    class MimeTypes : public PluginBase {
    public:
        MimeTypes(ExecState *exec) : PluginBase(exec) { };
        virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
        virtual const ClassInfo* classInfo() const { return &info; }
        static const ClassInfo info;
        ValueImp *getValueProperty(ExecState *exec, int token) const;
        static ValueImp *mimeTypeByName( ExecState* exec, const QString& name );
    private:
        static ValueImp *indexGetter(ExecState *, const Identifier&, const PropertySlot&);
        static ValueImp *nameGetter(ExecState *, const Identifier&, const PropertySlot&);
    };


    class Plugin : public PluginBase {
    public:
        Plugin( ExecState *exec, PluginBase::PluginInfo *info )
          : PluginBase( exec )
        { m_info = info; };
        virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
        virtual const ClassInfo* classInfo() const { return &info; }
        static const ClassInfo info;
        ValueImp *mimeByName(ExecState* exec, const QString& name ) const;
        ValueImp *getValueProperty(ExecState *exec, int token) const;
        PluginBase::PluginInfo *pluginInfo() const { return m_info; }
    private:
        PluginBase::PluginInfo *m_info;
        static ValueImp *indexGetter(ExecState *, const Identifier&, const PropertySlot&);
        static ValueImp *nameGetter(ExecState *, const Identifier&, const PropertySlot&);
    };


    class MimeType : public PluginBase {
    public:
        MimeType( ExecState *exec, PluginBase::MimeClassInfo *info )
          : PluginBase( exec )
        { m_info = info; };
        virtual bool getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot);
        virtual const ClassInfo* classInfo() const { return &info; }
        static const ClassInfo info;
        ValueImp *getValueProperty(ExecState *exec, int token) const;
    private:
        PluginBase::MimeClassInfo *m_info;
    };

}


Q3PtrList<PluginBase::PluginInfo> *KJS::PluginBase::plugins = 0;
Q3PtrList<PluginBase::MimeClassInfo> *KJS::PluginBase::mimes = 0;
int KJS::PluginBase::m_refCount = 0;

const ClassInfo Navigator::info = { "Navigator", 0, &NavigatorTable, 0 };
/*
@begin NavigatorTable 12
  appCodeName	Navigator::AppCodeName	DontDelete|ReadOnly
  appName	Navigator::AppName	DontDelete|ReadOnly
  appVersion	Navigator::AppVersion	DontDelete|ReadOnly
  language	Navigator::Language	DontDelete|ReadOnly
  userAgent	Navigator::UserAgent	DontDelete|ReadOnly
  userLanguage	Navigator::UserLanguage	DontDelete|ReadOnly
  browserLanguage Navigator::BrowserLanguage	DontDelete|ReadOnly
  platform	Navigator::Platform	DontDelete|ReadOnly
  cpuClass      Navigator::CpuClass     DontDelete|ReadOnly
  plugins	Navigator::_Plugins	DontDelete|ReadOnly
  mimeTypes	Navigator::_MimeTypes	DontDelete|ReadOnly
  product	Navigator::Product	DontDelete|ReadOnly
  vendor	Navigator::Vendor	DontDelete|ReadOnly
  productSub    Navigator::ProductSub   DontDelete|ReadOnly
  cookieEnabled	Navigator::CookieEnabled DontDelete|ReadOnly
  javaEnabled	Navigator::JavaEnabled	DontDelete|Function 0
@end
*/
IMPLEMENT_PROTOFUNC(NavigatorFunc)

Navigator::Navigator(ExecState *exec, KHTMLPart *p)
  : ObjectImp(exec->lexicalInterpreter()->builtinObjectPrototype()), m_part(p) { }

bool Navigator::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Navigator::getOwnPropertySlot " << propertyName.ascii() << endl;
#endif
  return getStaticPropertySlot<NavigatorFunc, Navigator, ObjectImp>(exec, &NavigatorTable, this, propertyName, slot);
}

ValueImp *Navigator::getValueProperty(ExecState *exec, int token) const
{
  KUrl url = m_part->url();
  QString userAgent = url.host();
  if (userAgent.isEmpty())
     userAgent = "localhost";
  userAgent = KProtocolManager::userAgentForHost(userAgent);
  switch (token) {
  case AppCodeName:
    return String("Mozilla");
  case AppName:
    // If we find "Mozilla" but not "(compatible, ...)" we are a real Netscape
    if (userAgent.find(QLatin1String("Mozilla")) >= 0 &&
        userAgent.find(QLatin1String("compatible")) == -1)
    {
      //kdDebug() << "appName -> Mozilla" << endl;
      return String("Netscape");
    }
    if (userAgent.find(QLatin1String("Microsoft")) >= 0 ||
        userAgent.find(QLatin1String("MSIE")) >= 0)
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
    // We are pretending to be Mozilla or Safari
    if (userAgent.find(QLatin1String("Mozilla")) >= 0 &&
        userAgent.find(QLatin1String("compatible")) == -1)
    {
        return String("Gecko");
    }
    // When spoofing as IE, we use Undefined().
    if (userAgent.find(QLatin1String("Microsoft")) >= 0 ||
        userAgent.find(QLatin1String("MSIE")) >= 0)
    {
        return Undefined();
    }
    // We are acting straight
    return String("Konqueror/khtml");
  case ProductSub:
    {
      int ix = userAgent.find("Gecko");
      if (ix >= 0 && userAgent.length() >= ix+14 && userAgent.at(ix+5) == '/' &&
          userAgent.find(QRegExp("\\d{8}"), ix+6) == ix+6)
      {
          // We have Gecko/<productSub> in the UA string
          return String(userAgent.mid(ix+6, 8));
      }
      else if (ix >= 0)
      {
          return String("20040107");
      }
    }
    return Undefined();
  case Vendor:
    return String("KDE");
  case BrowserLanguage:
  case Language:
  case UserLanguage:
    return String(KGlobal::locale()->language());
  case UserAgent:
    return String(userAgent);
  case Platform:
    // yet another evil hack, but necessary to spoof some sites...
    if ( (userAgent.find(QLatin1String("Win"),0,false)>=0) )
      return String("Win32");
    else if ( (userAgent.find(QLatin1String("Macintosh"),0,false)>=0) ||
              (userAgent.find(QLatin1String("Mac_PowerPC"),0,false)>=0) )
      return String("MacPPC");
    else
    {
        struct utsname name;
        int ret = uname(&name);
        if ( ret >= 0 )
            return String(QString::fromLatin1("%1 %1 X11").arg(name.sysname).arg(name.machine));
        else // can't happen
            return String("Unix X11");
    }
  case CpuClass:
  {
    struct utsname name;
    int ret = uname(&name);
    if ( ret >= 0 )
      return String(name.machine);
    else // can't happen
      return String("x86");
  }
  case _Plugins:
    return new Plugins(exec);
  case _MimeTypes:
    return new MimeTypes(exec);
  case CookieEnabled:
    return Boolean(true); /// ##### FIXME
  default:
    kdDebug(6070) << "WARNING: Unhandled token in DOMEvent::getValueProperty : " << token << endl;
    return Null();
  }
}

/*******************************************************************/

PluginBase::PluginBase(ExecState *exec)
  : ObjectImp(exec->lexicalInterpreter()->builtinObjectPrototype() )
{
    if ( !plugins ) {
        plugins = new Q3PtrList<PluginInfo>;
        mimes = new Q3PtrList<MimeClassInfo>;
        plugins->setAutoDelete( true );
        mimes->setAutoDelete( true );

        // FIXME: add domain support here
        KConfig kc("konquerorrc", true);
        if (!KConfigGroup(&kc, "Java/JavaScript Settings").readEntry("EnablePlugins", true))
            return; // plugins disabled

        // read in using KTrader
        KTrader::OfferList offers = KTrader::self()->query("Browser/View");
        KTrader::OfferList::iterator it;
        for ( it = offers.begin(); it != offers.end(); ++it ) {

            QVariant pluginsinfo = (**it).property( "X-KDE-BrowserView-PluginsInfo" );
            if ( !pluginsinfo.isValid() ) {
                // <backwards compatible>
                if ((**it).library() == QString("libnsplugin"))
                    pluginsinfo = QVariant("nsplugins/pluginsinfo");
                else
                // </backwards compatible>
                    continue;
            }
            // read configuration
            KConfig kc( locate ("data", pluginsinfo.toString()) );
            const int num = kc.readEntry("number", 0);
            for ( int n = 0; n < num; n++ ) {
                kc.setGroup( QString::number(n) );
                PluginInfo *plugin = new PluginInfo;

                plugin->name = kc.readEntry("name");
                plugin->file = kc.readPathEntry("file");
                plugin->desc = kc.readEntry("description");

                plugins->append( plugin );

                QStringList types = kc.readEntry("mime", QStringList(), ';');
                QStringList::Iterator type;
                for ( type=types.begin(); type!=types.end(); ++type ) {

                    // get mime information
                    QStringList tokens = QStringList::split(':', *type, true);
                    if ( tokens.count() < 3 ) // we need 3 items
                        continue;

                    MimeClassInfo *mime = new MimeClassInfo;
                    QStringList::Iterator token = tokens.begin();
                    mime->type = (*token).toLower();
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

const ClassInfo Plugins::info = { "PluginArray", 0, &PluginsTable, 0 };
/*
@begin PluginsTable 4
  length	Plugins_Length  	DontDelete|ReadOnly
  refresh	Plugins_Refresh 	DontDelete|Function 0
  item  	Plugins_Item    	DontDelete|Function 1
  namedItem  	Plugins_NamedItem   	DontDelete|Function 1
@end
*/
IMPLEMENT_PROTOFUNC(PluginsFunc)

ValueImp *Plugins::getValueProperty(ExecState *exec, int token) const
{
  assert(token == Plugins_Length);
  return Number(plugins->count());
}

ValueImp *Plugins::indexGetter(ExecState *exec, const Identifier& propertyName, const PropertySlot& slot)
{
    return new Plugin(exec, plugins->at(slot.index()));
}

ValueImp *Plugins::nameGetter(ExecState *exec, const Identifier& propertyName, const PropertySlot& slot)
{

  return pluginByName(exec, propertyName.qstring());
}

bool Plugins::getOwnPropertySlot(ExecState *exec, const Identifier &propertyName, PropertySlot& slot)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Plugins::getOwnPropertySlot " << propertyName.qstring() << endl;
#endif
  if (getStaticOwnPropertySlot<PluginsFunc, Plugins>(&PluginsTable, this, propertyName, slot))
      return true;
      
  // plugins[#]
  bool ok;
  unsigned int i = propertyName.toUInt32(&ok);
  if (ok && i < plugins->count()) {
      slot.setCustomIndex(this, i, indexGetter);
      return true;
  }

  // plugin[name]
  for (PluginInfo *pl = plugins->first(); pl; pl = plugins->next()) {
      if (pl->name == propertyName.qstring()) {
          slot.setCustom(this, nameGetter);
          return true;
      }
  }

  return PluginBase::getOwnPropertySlot(exec, propertyName, slot);
}

ValueImp *Plugins::pluginByName( ExecState* exec, const QString& name )
{
  for ( PluginInfo *pl = plugins->first(); pl!=0; pl = plugins->next() ) {
    if ( pl->name == name )
      return new Plugin( exec, pl );
  }
  return Undefined();
}

ValueImp *PluginsFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::Plugins, thisObj );
  KJS::Plugins* base = static_cast<KJS::Plugins *>(thisObj);
  switch( id ) {
  case Plugins_Refresh:
    return Undefined(); //## TODO
  case Plugins_Item:
  {
    bool ok;
    unsigned int i = args[0]->toString(exec).toArrayIndex(&ok);
    if( ok && i<base->plugins->count() )
      return new Plugin( exec, base->plugins->at(i) );
    return Undefined();
  }
  case Plugins_NamedItem:
  {
    UString s = args[0]->toString(exec);
    return base->pluginByName( exec, s.qstring() );
  }
  default:
    kdDebug(6070) << "WARNING: Unhandled token in PluginsFunc::callAsFunction : " << id << endl;
    return Undefined();
  }
}

/*******************************************************************/

const ClassInfo MimeTypes::info = { "MimeTypeArray", 0, &MimeTypesTable, 0 };
/*
@begin MimeTypesTable 3
  length	MimeTypes_Length  	DontDelete|ReadOnly
  item  	MimeTypes_Item    	DontDelete|Function 1
  namedItem  	MimeTypes_NamedItem   	DontDelete|Function 1
@end
*/
IMPLEMENT_PROTOFUNC(MimeTypesFunc)

ValueImp *MimeTypes::indexGetter(ExecState *exec, const Identifier& propertyName, const PropertySlot& slot)
{
    return new MimeType(exec, mimes->at(slot.index()));
}

ValueImp *MimeTypes::nameGetter(ExecState *exec, const Identifier& propertyName, const PropertySlot& slot)
{
    return mimeTypeByName(exec, propertyName.qstring());
}

bool MimeTypes::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "MimeTypes::getOwnPropertySlot " << propertyName.qstring() << endl;
#endif
  if (getStaticOwnPropertySlot<MimeTypesFunc, MimeTypes>(&MimeTypesTable, this, propertyName, slot))
      return true;

  // mimeTypes[#]
  bool ok;
  unsigned int i = propertyName.toUInt32(&ok);
  if (ok && i < mimes->count()) {
      slot.setCustomIndex(this, i, indexGetter);
      return true;
  }

  // mimeTypes[name]
  for (MimeClassInfo *m = mimes->first(); m; m = mimes->next()) {
      if (m->type == propertyName.qstring()) {
          slot.setCustom(this, nameGetter);
          return true;
      }
  }

  return PluginBase::getOwnPropertySlot(exec, propertyName, slot);
}

ValueImp *MimeTypes::mimeTypeByName( ExecState* exec, const QString& name )
{
  //kdDebug(6070) << "MimeTypes[" << name << "]" << endl;
  for ( MimeClassInfo *m = mimes->first(); m!=0; m = mimes->next() ) {
    if ( m->type == name )
      return new MimeType( exec, m );
  }
  return Undefined();
}

ValueImp *MimeTypes::getValueProperty(ExecState* /*exec*/, int token) const
{
  assert(token == MimeTypes_Length);
  return Number(mimes->count());
}

ValueImp *MimeTypesFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::MimeTypes, thisObj );
  KJS::MimeTypes* base = static_cast<KJS::MimeTypes *>(thisObj);
  switch( id ) {
  case MimeTypes_Item:
  {
    bool ok;
    unsigned int i = args[0]->toString(exec).toArrayIndex(&ok);
    if( ok && i<base->mimes->count() )
      return new MimeType( exec, base->mimes->at(i) );
    return Undefined();
  }
  case MimeTypes_NamedItem:
  {
    UString s = args[0]->toString(exec);
    return base->mimeTypeByName( exec, s.qstring() );
  }
  default:
    kdDebug(6070) << "WARNING: Unhandled token in MimeTypesFunc::callAsFunction : " << id << endl;
    return Undefined();
  }
}

/************************************************************************/
const ClassInfo Plugin::info = { "Plugin", 0, &PluginTable, 0 };
/*
@begin PluginTable 7
  name  	Plugin_Name	  	DontDelete|ReadOnly
  filename  	Plugin_FileName    	DontDelete|ReadOnly
  description  	Plugin_Description    	DontDelete|ReadOnly
  length  	Plugin_Length    	DontDelete|ReadOnly
  item  	Plugin_Item	   	DontDelete|Function 1
  namedItem  	Plugin_NamedItem   	DontDelete|Function 1
@end
*/
IMPLEMENT_PROTOFUNC(PluginFunc)

ValueImp *Plugin::indexGetter(ExecState *exec, const Identifier& propertyName, const PropertySlot& slot)
{
    Plugin *thisObj = static_cast<Plugin *>(slot.slotBase());
    return new MimeType(exec, thisObj->m_info->mimes.at(slot.index()));
}

ValueImp *Plugin::nameGetter(ExecState *exec, const Identifier& propertyName, const PropertySlot& slot)
{
  Plugin *thisObj = static_cast<Plugin *>(slot.slotBase());
  return thisObj->mimeByName(exec, propertyName.qstring());
}

bool Plugin::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Plugin::getOwnPropertySlot " << propertyName.qstring() << endl;
#endif
  if (getStaticOwnPropertySlot<PluginFunc, Plugin>(&PluginsTable, this, propertyName, slot))
      return true;

  // plugin[#]
  bool ok;
  unsigned int i = propertyName.toUInt32(&ok);
  if (ok && i < m_info->mimes.count()) {
    slot.setCustomIndex(this, i, indexGetter);
    return true;
  }

  // plugin["name"]
  for (MimeClassInfo *m=m_info->mimes.first(); m; m = m_info->mimes.next()) {
    if (m->type == propertyName.qstring()) {
      slot.setCustom(this, nameGetter);
      return true;
    }
  }

  return PluginBase::getOwnPropertySlot(exec, propertyName, slot);
}

ValueImp *Plugin::mimeByName(ExecState* exec, const QString& name) const
{
  for ( PluginBase::MimeClassInfo *m = m_info->mimes.first();
        m != 0; m = m_info->mimes.next() ) {
    if ( m->type == name )
      return new MimeType(exec, m);
  }
  return Undefined();
}

ValueImp *Plugin::getValueProperty(ExecState* /*exec*/, int token) const
{
  switch( token ) {
  case Plugin_Name:
    return String( UString(m_info->name) );
  case Plugin_FileName:
    return String( UString(m_info->file) );
  case Plugin_Description:
    return String( UString(m_info->desc) );
  case Plugin_Length:
    return Number( m_info->mimes.count() );
  default:
    kdDebug(6070) << "WARNING: Unhandled token in Plugin::getValueProperty : " << token << endl;
    return Undefined();
  }
}

ValueImp *PluginFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::Plugin, thisObj );
  KJS::Plugin* plugin = static_cast<KJS::Plugin *>(thisObj);
  switch( id ) {
  case Plugin_Item:
  {
    bool ok;
    unsigned int i = args[0]->toString(exec).toArrayIndex(&ok);
    if( ok && i< plugin->pluginInfo()->mimes.count() )
      return new MimeType( exec, plugin->pluginInfo()->mimes.at(i) );
    return Undefined();
  }
  case Plugin_NamedItem:
  {
    UString s = args[0]->toString(exec);
    return plugin->mimeByName( exec, s.qstring() );
  }
  default:
    kdDebug(6070) << "WARNING: Unhandled token in PluginFunc::callAsFunction : " << id << endl;
    return Undefined();
  }
}

/*****************************************************************************/

const ClassInfo MimeType::info = { "MimeType", 0, &MimeTypesTable, 0 };
/*
@begin MimeTypeTable 4
  description  	MimeType_Description    	DontDelete|ReadOnly
  enabledPlugin MimeType_EnabledPlugin    	DontDelete|ReadOnly
  suffixes	MimeType_Suffixes	    	DontDelete|ReadOnly
  type  	MimeType_Type			DontDelete|ReadOnly
@end
*/

bool MimeType::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "MimeType::get " << propertyName.qstring() << endl;
#endif
  return getStaticValueSlot<MimeType, ObjectImp>(exec, &MimeTypesTable, this, propertyName, slot);
}

ValueImp *MimeType::getValueProperty(ExecState* exec, int token) const
{
  switch( token ) {
  case MimeType_Type:
    return String( UString(m_info->type) );
  case MimeType_Suffixes:
    return String( UString(m_info->suffixes) );
  case MimeType_Description:
    return String( UString(m_info->desc) );
  case MimeType_EnabledPlugin:
    return new Plugin(exec, m_info->plugin);
  default:
    kdDebug(6070) << "WARNING: Unhandled token in MimeType::getValueProperty : " << token << endl;
    return Undefined();
  }
}


ValueImp *NavigatorFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &)
{
  KJS_CHECK_THIS( KJS::Navigator, thisObj );
  Navigator *nav = static_cast<Navigator *>(thisObj);
  // javaEnabled()
  return Boolean(nav->part()->javaEnabled());
}
