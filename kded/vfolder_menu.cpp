/*  This file is part of the KDE libraries
 *  Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "vfolder_menu.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <config.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kservice.h>
#include <kde_file.h>

#include <QtCore/QMap>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QRegExp>

static void foldNode(QDomElement &docElem, QDomElement &e, QMap<QString,QDomElement> &dupeList, QString s=QString()) //krazy:exclude=passbyvalue
{
   if (s.isEmpty())
      s = e.text();
   QMap<QString,QDomElement>::iterator it = dupeList.find(s);
   if (it != dupeList.end())
   {
      kDebug(7021) << e.tagName() << "and" << s << "requires combining!";

      docElem.removeChild(*it);
      dupeList.erase(it);
   }
   dupeList.insert(s, e);
}

static void replaceNode(QDomElement &docElem, QDomNode &n, const QStringList &list, const QString &tag)
{
   for(QStringList::ConstIterator it = list.begin();
       it != list.end(); ++it)
   {
      QDomElement e = docElem.ownerDocument().createElement(tag);
      QDomText txt = docElem.ownerDocument().createTextNode(*it);
      e.appendChild(txt);
      docElem.insertAfter(e, n);
   }

   QDomNode next = n.nextSibling();
   docElem.removeChild(n);
   n = next;
//   kDebug(7021) << "Next tag = " << n.toElement().tagName();
}

void VFolderMenu::registerFile(const QString &file)
{
   int i = file.lastIndexOf('/');
   if (i < 0)
      return;

   QString dir = file.left(i+1); // Include trailing '/'
   registerDirectory(dir);
}

void VFolderMenu::registerDirectory(const QString &directory)
{
   m_allDirectories.append(directory);
}

QStringList VFolderMenu::allDirectories()
{
   if (m_allDirectories.isEmpty())
     return m_allDirectories;
   m_allDirectories.sort();

   QStringList::Iterator it = m_allDirectories.begin();
   QString previous = *it++;
   for(;it != m_allDirectories.end();)
   {
     if ((*it).startsWith(previous))
     {
        it = m_allDirectories.erase(it);
     }
     else
     {
        previous = *it;
        ++it;
     }
   }
   return m_allDirectories;
}

static void
track(const QString &menuId, const QString &menuName, const QHash<QString,KService::Ptr>& includeList, const QHash<QString,KService::Ptr>& excludeList, const QHash<QString,KService::Ptr>& itemList, const QString &comment)
{
   if (itemList.contains(menuId))
      printf("%s: %s INCL %d EXCL %d\n", qPrintable(menuName), qPrintable(comment), includeList.contains(menuId) ? 1 : 0, excludeList.contains(menuId) ? 1 : 0);
}

void
VFolderMenu::includeItems(QHash<QString,KService::Ptr>& items1, const QHash<QString,KService::Ptr>& items2)
{
   foreach (const KService::Ptr &p, items2) {
       items1.insert(p->menuId(), p);
   }
}

void
VFolderMenu::matchItems(QHash<QString,KService::Ptr>& items1, const QHash<QString,KService::Ptr>& items2)
{
   foreach (const KService::Ptr &p, items1)
   {
       QString id = p->menuId();
       if (!items2.contains(id))
          items1.remove(id);
   }
}

void
VFolderMenu::excludeItems(QHash<QString,KService::Ptr>& items1, const QHash<QString,KService::Ptr>& items2)
{
   foreach (const KService::Ptr &p, items2)
       items1.remove(p->menuId());
}

VFolderMenu::SubMenu*
VFolderMenu::takeSubMenu(SubMenu *parentMenu, const QString &menuName)
{
   const int i = menuName.indexOf('/');
   const QString s1 = i > 0 ? menuName.left(i) : menuName;
   const QString s2 = menuName.mid(i+1);

   // Look up menu
   for (QList<SubMenu*>::Iterator it = parentMenu->subMenus.begin(); it != parentMenu->subMenus.end(); ++it)
   {
      SubMenu* menu = *it;
      if (menu->name == s1)
      {
         if (i == -1)
         {
            // Take it out
            parentMenu->subMenus.erase(it);
            return menu;
         }
         else
         {
            return takeSubMenu(menu, s2);
         }
      }
   }
   return 0; // Not found
}

void
VFolderMenu::mergeMenu(SubMenu *menu1, SubMenu *menu2, bool reversePriority)
{
   if (m_track)
   {
      track(m_trackId, menu1->name, menu1->items, menu1->excludeItems, menu2->items, QString("Before MenuMerge w. %1 (incl)").arg(menu2->name));
      track(m_trackId, menu1->name, menu1->items, menu1->excludeItems, menu2->excludeItems, QString("Before MenuMerge w. %1 (excl)").arg(menu2->name));
   }
   if (reversePriority)
   {
      // Merge menu1 with menu2, menu1 takes precedent
      excludeItems(menu2->items, menu1->excludeItems);
      includeItems(menu1->items, menu2->items);
      excludeItems(menu2->excludeItems, menu1->items);
      includeItems(menu1->excludeItems, menu2->excludeItems);
   }
   else
   {
      // Merge menu1 with menu2, menu2 takes precedent
      excludeItems(menu1->items, menu2->excludeItems);
      includeItems(menu1->items, menu2->items);
      includeItems(menu1->excludeItems, menu2->excludeItems);
      menu1->isDeleted = menu2->isDeleted;
   }
   while (!menu2->subMenus.isEmpty())
   {
      SubMenu *subMenu = menu2->subMenus.takeFirst();
      insertSubMenu(menu1, subMenu->name, subMenu, reversePriority);
   }

   if (reversePriority)
   {
      // Merge menu1 with menu2, menu1 takes precedent
      if (menu1->directoryFile.isEmpty())
         menu1->directoryFile = menu2->directoryFile;
      if (menu1->defaultLayoutNode.isNull())
         menu1->defaultLayoutNode = menu2->defaultLayoutNode;
      if (menu1->layoutNode.isNull())
         menu1->layoutNode = menu2->layoutNode;
   }
   else
   {
      // Merge menu1 with menu2, menu2 takes precedent
      if (!menu2->directoryFile.isEmpty())
         menu1->directoryFile = menu2->directoryFile;
      if (!menu2->defaultLayoutNode.isNull())
         menu1->defaultLayoutNode = menu2->defaultLayoutNode;
      if (!menu2->layoutNode.isNull())
         menu1->layoutNode = menu2->layoutNode;
   }

   if (m_track)
   {
      track(m_trackId, menu1->name, menu1->items, menu1->excludeItems, menu2->items, QString("After MenuMerge w. %1 (incl)").arg(menu2->name));
      track(m_trackId, menu1->name, menu1->items, menu1->excludeItems, menu2->excludeItems, QString("After MenuMerge w. %1 (excl)").arg(menu2->name));
   }

   delete menu2;
}

void
VFolderMenu::insertSubMenu(SubMenu *parentMenu, const QString &menuName, SubMenu *newMenu, bool reversePriority)
{
   const int i = menuName.indexOf('/');
   const QString s1 = menuName.left(i);
   const QString s2 = menuName.mid(i+1);

   // Look up menu
   foreach (SubMenu *menu, parentMenu->subMenus)
   {
      if (menu->name == s1)
      {
         if (i == -1)
         {
            mergeMenu(menu, newMenu, reversePriority);
            return;
         }
         else
         {
            insertSubMenu(menu, s2, newMenu, reversePriority);
            return;
         }
      }
   }
   if (i == -1)
   {
     // Add it here
     newMenu->name = menuName;
     parentMenu->subMenus.append(newMenu);
   }
   else
   {
     SubMenu *menu = new SubMenu;
     menu->name = s1;
     parentMenu->subMenus.append(menu);
     insertSubMenu(menu, s2, newMenu);
   }
}

void
VFolderMenu::insertService(SubMenu *parentMenu, const QString &name, KService::Ptr newService)
{
   const int i = name.indexOf('/');

   if (i == -1)
   {
     // Add it here
     parentMenu->items.insert(newService->menuId(), newService);
     return;
   }

   QString s1 = name.left(i);
   QString s2 = name.mid(i+1);

   // Look up menu
   foreach (SubMenu *menu, parentMenu->subMenus)
   {
      if (menu->name == s1)
      {
         insertService(menu, s2, newService);
         return;
      }
   }

   SubMenu *menu = new SubMenu;
   menu->name = s1;
   parentMenu->subMenus.append(menu);
   insertService(menu, s2, newService);
}


VFolderMenu::VFolderMenu() : m_track(false)
{
   m_usedAppsDict.reserve(797);
   m_rootMenu = 0;
   initDirs();
}

VFolderMenu::~VFolderMenu()
{
   delete m_rootMenu;
   delete m_appsInfo;
}

#define FOR_ALL_APPLICATIONS(it) \
   foreach (appsInfo *info, m_appsInfoStack) \
   { \
      QHashIterator<QString,KService::Ptr> it = info->applications; \
      while (it.hasNext()) \
      { \
         it.next();
#define FOR_ALL_APPLICATIONS_END } }

#define FOR_CATEGORY(category, it) \
   foreach (appsInfo *info, m_appsInfoStack) \
   { \
      KService::List *list = info->dictCategories[category]; \
      if (list) for(KService::List::ConstIterator it = list->constBegin(); \
             it != list->constEnd(); ++it) \
      {
#define FOR_CATEGORY_END } }

KService::Ptr
VFolderMenu::findApplication(const QString &relPath)
{
   foreach(appsInfo *info, m_appsInfoStack)
   {
      if (info->applications.contains(relPath)) {
         KService::Ptr s = info->applications[relPath];
         if (s)
            return s;
      }
   }
   return KService::Ptr();
}

void
VFolderMenu::addApplication(const QString &id, KService::Ptr service)
{
   service->setMenuId(id);
   m_appsInfo->applications.insert(id, service);
}

void
VFolderMenu::buildApplicationIndex(bool unusedOnly)
{
   foreach (appsInfo *info, m_appsInfoList)
   {
      info->dictCategories.clear();
      QMutableHashIterator<QString,KService::Ptr> it = info->applications;
      while (it.hasNext())
      {
         KService::Ptr s = it.next().value();
         if (unusedOnly && m_usedAppsDict.contains(s->menuId()))
         {
            // Remove and skip this one
            it.remove();
            continue;
         }

         const QStringList cats = s->categories();
         for(QStringList::ConstIterator it2 = cats.begin();
             it2 != cats.end(); ++it2)
         {
            const QString &cat = *it2;
            KService::List *list = info->dictCategories[cat];
            if (!list)
            {
               list = new KService::List();
               info->dictCategories.insert(cat, list);
            }
            list->append(s);
         }
      }
   }
}

void
VFolderMenu::createAppsInfo()
{
   if (m_appsInfo) return;

   m_appsInfo = new appsInfo;
   m_appsInfoStack.prepend(m_appsInfo);
   m_appsInfoList.append(m_appsInfo);
   m_currentMenu->apps_info = m_appsInfo;
}

void
VFolderMenu::loadAppsInfo()
{
   m_appsInfo = m_currentMenu->apps_info;
   if (!m_appsInfo)
      return; // No appsInfo for this menu

   if (m_appsInfoStack.count() && m_appsInfoStack.first() == m_appsInfo)
      return; // Already added (By createAppsInfo?)

   m_appsInfoStack.prepend(m_appsInfo); // Add
}

void
VFolderMenu::unloadAppsInfo()
{
   m_appsInfo = m_currentMenu->apps_info;
   if (!m_appsInfo)
      return; // No appsInfo for this menu

   if (m_appsInfoStack.first() != m_appsInfo)
   {
      return; // Already removed (huh?)
   }

   m_appsInfoStack.removeAll(m_appsInfo); // Remove
   m_appsInfo = 0;
}

QString
VFolderMenu::absoluteDir(const QString &_dir, const QString &baseDir, bool keepRelativeToCfg)
{
   QString dir = _dir;
   if (QDir::isRelativePath(dir))
   {
      dir = baseDir + dir;
   }
   if (!dir.endsWith('/'))
      dir += '/';

   if (QDir::isRelativePath(dir) && !keepRelativeToCfg)
   {
      dir = KGlobal::dirs()->findResource("xdgconf-menu", dir);
   }

   dir = KGlobal::dirs()->realPath(dir);

   return dir;
}

static void tagBaseDir(QDomDocument &doc, const QString &tag, const QString &dir)
{
   QDomNodeList mergeFileList = doc.elementsByTagName(tag);
   for(int i = 0; i < (int)mergeFileList.count(); i++)
   {
      QDomAttr attr = doc.createAttribute("__BaseDir");
      attr.setValue(dir);
      mergeFileList.item(i).toElement().setAttributeNode(attr);
   }
}

static void tagBasePath(QDomDocument &doc, const QString &tag, const QString &path)
{
   QDomNodeList mergeFileList = doc.elementsByTagName(tag);
   for(int i = 0; i < (int)mergeFileList.count(); i++)
   {
      QDomAttr attr = doc.createAttribute("__BasePath");
      attr.setValue(path);
      mergeFileList.item(i).toElement().setAttributeNode(attr);
   }
}

QDomDocument
VFolderMenu::loadDoc()
{
   QDomDocument doc;
   if ( m_docInfo.path.isEmpty() )
   {
      return doc;
   }
   QFile file( m_docInfo.path );
   if ( !file.open( QIODevice::ReadOnly ) )
   {
      kWarning(7021) << "Could not open " << m_docInfo.path;
      return doc;
   }
   QString errorMsg;
   int errorRow;
   int errorCol;
   if ( !doc.setContent( &file, &errorMsg, &errorRow, &errorCol ) ) {
      kWarning(7021) << "Parse error in " << m_docInfo.path << ", line " << errorRow << ", col " << errorCol << ": " << errorMsg;
      file.close();
      return doc;
   }
   file.close();

   tagBaseDir(doc, "MergeFile", m_docInfo.baseDir);
   tagBasePath(doc, "MergeFile", m_docInfo.path);
   tagBaseDir(doc, "MergeDir", m_docInfo.baseDir);
   tagBaseDir(doc, "DirectoryDir", m_docInfo.baseDir);
   tagBaseDir(doc, "AppDir", m_docInfo.baseDir);
   tagBaseDir(doc, "LegacyDir", m_docInfo.baseDir);

   return doc;
}


void
VFolderMenu::mergeFile(QDomElement &parent, const QDomNode &mergeHere)
{
kDebug(7021) << "VFolderMenu::mergeFile:" << m_docInfo.path;
   QDomDocument doc = loadDoc();

   QDomElement docElem = doc.documentElement();
   QDomNode n = docElem.firstChild();
   QDomNode last = mergeHere;
   while( !n.isNull() )
   {
      QDomElement e = n.toElement(); // try to convert the node to an element.
      QDomNode next = n.nextSibling();

      if (e.isNull())
      {
         // Skip
      }
      // The spec says we must ignore any Name nodes
      else if (e.tagName() != "Name")
      {
         parent.insertAfter(n, last);
         last = n;
      }

      docElem.removeChild(n);
      n = next;
   }
}


void
VFolderMenu::mergeMenus(QDomElement &docElem, QString &name)
{
   QMap<QString,QDomElement> menuNodes;
   QMap<QString,QDomElement> directoryNodes;
   QMap<QString,QDomElement> appDirNodes;
   QMap<QString,QDomElement> directoryDirNodes;
   QMap<QString,QDomElement> legacyDirNodes;
   QDomElement defaultLayoutNode;
   QDomElement layoutNode;

   QDomNode n = docElem.firstChild();
   while( !n.isNull() ) {
      QDomElement e = n.toElement(); // try to convert the node to an element.
      if( e.isNull() ) {
// kDebug(7021) << "Empty node";
      }
      else if( e.tagName() == "DefaultAppDirs") {
         // Replace with m_defaultAppDirs
         replaceNode(docElem, n, m_defaultAppDirs, "AppDir");
         continue;
      }
      else if( e.tagName() == "DefaultDirectoryDirs") {
         // Replace with m_defaultDirectoryDirs
         replaceNode(docElem, n, m_defaultDirectoryDirs, "DirectoryDir");
         continue;
      }
      else if( e.tagName() == "DefaultMergeDirs") {
         // Replace with m_defaultMergeDirs
         replaceNode(docElem, n, m_defaultMergeDirs, "MergeDir");
         continue;
      }
      else if( e.tagName() == "AppDir") {
         // Filter out dupes
         foldNode(docElem, e, appDirNodes);
      }
      else if( e.tagName() == "DirectoryDir") {
         // Filter out dupes
         foldNode(docElem, e, directoryDirNodes);
      }
      else if( e.tagName() == "LegacyDir") {
         // Filter out dupes
         foldNode(docElem, e, legacyDirNodes);
      }
      else if( e.tagName() == "Directory") {
         // Filter out dupes
         foldNode(docElem, e, directoryNodes);
      }
      else if( e.tagName() == "Move") {
         // Filter out dupes
         QString orig;
         QDomNode n2 = e.firstChild();
         while( !n2.isNull() ) {
            QDomElement e2 = n2.toElement(); // try to convert the node to an element.
            if( e2.tagName() == "Old")
            {
               orig = e2.text();
               break;
            }
            n2 = n2.nextSibling();
         }
         foldNode(docElem, e, appDirNodes, orig);
      }
      else if( e.tagName() == "Menu") {
         QString name;
         mergeMenus(e, name);
         QMap<QString,QDomElement>::iterator it = menuNodes.find(name);
         if (it != menuNodes.end())
         {
           QDomElement docElem2 = *it;
           QDomNode n2 = docElem2.firstChild();
           QDomNode first = e.firstChild();
           while( !n2.isNull() ) {
             QDomElement e2 = n2.toElement(); // try to convert the node to an element.
             QDomNode n3 = n2.nextSibling();
             e.insertBefore(n2, first);
             docElem2.removeChild(n2);
             n2 = n3;
           }
           // We still have duplicated Name entries
           // but we don't care about that

           docElem.removeChild(docElem2);
           menuNodes.erase(it);
         }
         menuNodes.insert(name, e);
      }
      else if( e.tagName() == "MergeFile") {
         if ((e.attribute("type") == "parent"))
            pushDocInfoParent(e.attribute("__BasePath"), e.attribute("__BaseDir"));
         else
            pushDocInfo(e.text(), e.attribute("__BaseDir"));

         if (!m_docInfo.path.isEmpty())
            mergeFile(docElem, n);
         popDocInfo();

         QDomNode last = n;
         n = n.nextSibling();
         docElem.removeChild(last); // Remove the MergeFile node
         continue;
      }
      else if( e.tagName() == "MergeDir") {
         QString dir = absoluteDir(e.text(), e.attribute("__BaseDir"), true);

         const QStringList dirs = KGlobal::dirs()->findDirs("xdgconf-menu", dir);
         for(QStringList::ConstIterator it=dirs.begin();
             it != dirs.end(); ++it)
         {
            registerDirectory(*it);
         }

         QStringList fileList;
         if (!QDir::isRelativePath(dir))
         {
            // Absolute
            fileList = KGlobal::dirs()->findAllResources("xdgconf-menu", dir+"*.menu");
         }
         else
         {
            // Relative
            (void) KGlobal::dirs()->findAllResources("xdgconf-menu", dir+"*.menu",
                                                     KStandardDirs::NoDuplicates, fileList);
         }

         for(QStringList::ConstIterator it=fileList.constBegin();
             it != fileList.constEnd(); ++it)
         {
            pushDocInfo(*it);
            mergeFile(docElem, n);
            popDocInfo();
         }

         QDomNode last = n;
         n = n.nextSibling();
         docElem.removeChild(last); // Remove the MergeDir node

         continue;
      }
      else if( e.tagName() == "Name") {
         name = e.text();
      }
      else if( e.tagName() == "DefaultLayout") {
         if (!defaultLayoutNode.isNull())
            docElem.removeChild(defaultLayoutNode);
         defaultLayoutNode = e;
      }
      else if( e.tagName() == "Layout") {
         if (!layoutNode.isNull())
            docElem.removeChild(layoutNode);
         layoutNode = e;
      }
      n = n.nextSibling();
   }
}

void
VFolderMenu::pushDocInfo(const QString &fileName, const QString &baseDir)
{
   m_docInfoStack.push(m_docInfo);
   if (!baseDir.isEmpty())
   {
      if (!QDir::isRelativePath(baseDir))
         m_docInfo.baseDir = KGlobal::dirs()->relativeLocation("xdgconf-menu", baseDir);
      else
         m_docInfo.baseDir = baseDir;
   }

   QString baseName = fileName;
   if (!QDir::isRelativePath(baseName))
      registerFile(baseName);
   else
      baseName = m_docInfo.baseDir + baseName;

   m_docInfo.path = locateMenuFile(fileName);
   if (m_docInfo.path.isEmpty())
   {
      m_docInfo.baseDir.clear();
      m_docInfo.baseName.clear();
      kDebug(7021) << "Menu" << fileName << "not found.";
      return;
   }
   int i;
   i = baseName.lastIndexOf('/');
   if (i > 0)
   {
      m_docInfo.baseDir = baseName.left(i+1);
      m_docInfo.baseName = baseName.mid(i+1, baseName.length() - i - 6);
   }
   else
   {
      m_docInfo.baseDir.clear();
      m_docInfo.baseName = baseName.left( baseName.length() - 5 );
   }
}

void
VFolderMenu::pushDocInfoParent(const QString &basePath, const QString &baseDir)
{
    m_docInfoStack.push(m_docInfo);

   m_docInfo.baseDir = baseDir;

   QString fileName = basePath.mid(basePath.lastIndexOf('/')+1);
   m_docInfo.baseName = fileName.left( fileName.length() - 5 );
   QString baseName = QDir::cleanPath(m_docInfo.baseDir + fileName);

   QStringList result = KGlobal::dirs()->findAllResources("xdgconf-menu", baseName);

   while( !result.isEmpty() && (result[0] != basePath))
      result.erase(result.begin());

   if (result.count() <= 1)
   {
      m_docInfo.path.clear(); // No parent found
      return;
   }
   m_docInfo.path = result[1];
}

void
VFolderMenu::popDocInfo()
{
   m_docInfo = m_docInfoStack.pop();
}

QString
VFolderMenu::locateMenuFile(const QString &fileName)
{
   if (!QDir::isRelativePath(fileName))
   {
      if (KStandardDirs::exists(fileName))
         return fileName;
      return QString();
   }

   QString result;

   QString xdgMenuPrefix = QString::fromLocal8Bit(qgetenv("XDG_MENU_PREFIX"));
   if (!xdgMenuPrefix.isEmpty())
   {
      QFileInfo fileInfo(fileName);

      QString fileNameOnly = fileInfo.fileName();
      if (!fileNameOnly.startsWith(xdgMenuPrefix))
         fileNameOnly = xdgMenuPrefix + fileNameOnly;

      QString baseName = QDir::cleanPath(m_docInfo.baseDir +
                                         fileInfo.path() + '/' + fileNameOnly);
      result = KStandardDirs::locate("xdgconf-menu", baseName);
   }

   if (result.isEmpty())
   {
       QString baseName = QDir::cleanPath(m_docInfo.baseDir + fileName);
       result = KStandardDirs::locate("xdgconf-menu", baseName);
   }

   return result;
}

QString
VFolderMenu::locateDirectoryFile(const QString &fileName)
{
   if (fileName.isEmpty())
      return QString();

   if (!QDir::isRelativePath(fileName))
   {
      if (KStandardDirs::exists(fileName))
         return fileName;
      return QString();
   }

   // First location in the list wins
   for(QStringList::ConstIterator it = m_directoryDirs.constBegin();
       it != m_directoryDirs.constEnd();
       ++it)
   {
      QString tmp = (*it)+fileName;
      if (KStandardDirs::exists(tmp))
         return tmp;
   }

   return QString();
}

void
VFolderMenu::initDirs()
{
   m_defaultDataDirs = KGlobal::dirs()->kfsstnd_prefixes().split(':', QString::SkipEmptyParts);
   const QString localDir = m_defaultDataDirs.first();
   m_defaultDataDirs.removeAll(localDir); // Remove local dir

   m_defaultAppDirs = KGlobal::dirs()->findDirs("xdgdata-apps", QString());
   m_defaultDirectoryDirs = KGlobal::dirs()->findDirs("xdgdata-dirs", QString());
   m_defaultLegacyDirs = KGlobal::dirs()->resourceDirs("apps");
}

void
VFolderMenu::loadMenu(const QString &fileName)
{
   m_defaultMergeDirs.clear();

   if (!fileName.endsWith(".menu"))
      return;

   pushDocInfo(fileName);
   m_defaultMergeDirs << m_docInfo.baseName+"-merged/";
   m_doc = loadDoc();
   popDocInfo();

   if (m_doc.isNull())
   {
      if (m_docInfo.path.isEmpty())
         kError(7021) << fileName << " not found in " << m_allDirectories << endl;
      else
         kWarning(7021) << "Load error (" << m_docInfo.path << ")";
      return;
   }

   QDomElement e = m_doc.documentElement();
   QString name;
   mergeMenus(e, name);
}

void
VFolderMenu::processCondition(QDomElement &domElem, QHash<QString,KService::Ptr>& items)
{
   if (domElem.tagName() == "And")
   {
      QDomNode n = domElem.firstChild();
      // Look for the first child element
      while (!n.isNull()) // loop in case of comments
      {
         QDomElement e = n.toElement();
         n = n.nextSibling();
         if ( !e.isNull() ) {
             processCondition(e, items);
             break; // we only want the first one
         }
      }

      QHash<QString,KService::Ptr> andItems;
      while( !n.isNull() ) {
         QDomElement e = n.toElement();
         if (e.tagName() == "Not")
         {
            // Special handling for "and not"
            QDomNode n2 = e.firstChild();
            while( !n2.isNull() ) {
               QDomElement e2 = n2.toElement();
               andItems.clear();
               processCondition(e2, andItems);
               excludeItems(items, andItems);
               n2 = n2.nextSibling();
            }
         }
         else
         {
            andItems.clear();
            processCondition(e, andItems);
            matchItems(items, andItems);
         }
         n = n.nextSibling();
      }
   }
   else if (domElem.tagName() == "Or")
   {
      QDomNode n = domElem.firstChild();
      // Look for the first child element
      while (!n.isNull()) // loop in case of comments
      {
         QDomElement e = n.toElement();
         n = n.nextSibling();
         if ( !e.isNull() ) {
             processCondition(e, items);
             break; // we only want the first one
         }
      }

      QHash<QString,KService::Ptr> orItems;
      while( !n.isNull() ) {
         QDomElement e = n.toElement();
         if ( !e.isNull() ) {
             orItems.clear();
             processCondition(e, orItems);
             includeItems(items, orItems);
         }
         n = n.nextSibling();
      }
   }
   else if (domElem.tagName() == "Not")
   {
      FOR_ALL_APPLICATIONS(it)
      {
         KService::Ptr s = it.value();
         items.insert(s->menuId(), s);
      }
      FOR_ALL_APPLICATIONS_END

      QHash<QString,KService::Ptr> notItems;
      QDomNode n = domElem.firstChild();
      while( !n.isNull() ) {
         QDomElement e = n.toElement();
         if ( !e.isNull() ) {
             notItems.clear();
             processCondition(e, notItems);
             excludeItems(items, notItems);
         }
         n = n.nextSibling();
      }
   }
   else if (domElem.tagName() == "Category")
   {
      FOR_CATEGORY(domElem.text(), it)
      {
         KService::Ptr s = *it;
         items.insert(s->menuId(), s);
      }
      FOR_CATEGORY_END
   }
   else if (domElem.tagName() == "All")
   {
      FOR_ALL_APPLICATIONS(it)
      {
         KService::Ptr s = it.value();
         items.insert(s->menuId(), s);
      }
      FOR_ALL_APPLICATIONS_END
   }
   else if (domElem.tagName() == "Filename")
   {
      QString filename = domElem.text();
kDebug(7021) << "Adding file" << filename;
      KService::Ptr s = findApplication(filename);
      if (s)
         items.insert(filename, s);
   }
}

void
VFolderMenu::loadApplications(const QString &dir, const QString &prefix)
{
   kDebug(7021) << "Looking up applications under" << dir;

   // We look for a set of files.
   DIR *dp = opendir( QFile::encodeName(dir));
   if (!dp)
      return;

   KDE_struct_dirent *ep;

   while( ( ep = KDE_readdir( dp ) ) != 0L )
   {
      QString fn( QFile::decodeName(ep->d_name));
      if (fn == "." || fn == ".." || fn.at(fn.length() - 1) == '~')
         continue;

      bool isDir;
      bool isReg;
      QString pathfn = dir + fn;

#ifdef HAVE_DIRENT_D_TYPE
      isDir = ep->d_type == DT_DIR;
      isReg = ep->d_type == DT_REG;

      if (ep->d_type == DT_UNKNOWN || ep->d_type == DT_LNK)
#endif
      {
        KDE_struct_stat buff;
        if ( KDE_stat( QFile::encodeName(pathfn), &buff ) != 0 ) {
           continue; // Couldn't stat (e.g. no read permissions)
        }
        isDir = S_ISDIR ( buff.st_mode );
        isReg = S_ISREG ( buff.st_mode );
      }
      if (isDir) {
         loadApplications(pathfn + '/', prefix + fn + '-');
         continue;
      }

      if (isReg)
      {
         if (!fn.endsWith(".desktop"))
            continue;

         KService::Ptr service;
         emit newService(pathfn, &service); // calls KBuildSycoca::slotCreateEntry
         if (service)
            addApplication(prefix+fn, service);
      }
    }
    closedir( dp );
}

void
VFolderMenu::processKDELegacyDirs()
{
kDebug(7021) << "processKDELegacyDirs()";

   QHash<QString,KService::Ptr> items;
   QString prefix = "kde4-";

   QStringList relFiles;

   (void) KGlobal::dirs()->findAllResources( "apps",
                                             QString(),
                                             KStandardDirs::Recursive |
                                             KStandardDirs::NoDuplicates,
                                             relFiles);
   for(QStringList::ConstIterator it = relFiles.constBegin();
       it != relFiles.constEnd(); ++it)
   {
      if (!m_forcedLegacyLoad && (*it).endsWith(".directory"))
      {
         QString name = *it;
         if (!name.endsWith("/.directory"))
            continue; // Probably ".directory", skip it.

         name = name.left(name.length()-11);

         SubMenu *newMenu = new SubMenu;
         newMenu->directoryFile = KStandardDirs::locate("apps", *it);

         insertSubMenu(m_currentMenu, name, newMenu);
         continue;
      }

      if ((*it).endsWith(".desktop"))
      {
         QString name = *it;
         KService::Ptr service;
         emit newService(name, &service);

         if (service && !m_forcedLegacyLoad)
         {
            QString id = name;
            // Strip path from id
            int i = id.lastIndexOf('/');
            if (i >= 0)
               id = id.mid(i+1);

            id.prepend(prefix);

            // TODO: add Legacy category
            addApplication(id, service);
            items.insert(service->menuId(), service);
            if (service->categories().isEmpty())
               insertService(m_currentMenu, name, service);

         }
      }
   }
   markUsedApplications(items);
   m_legacyLoaded = true;
}

void
VFolderMenu::processLegacyDir(const QString &dir, const QString &relDir, const QString &prefix)
{
kDebug(7021).nospace() << "processLegacyDir(" << dir << ", " << relDir << ", " << prefix << ")";

   QHash<QString,KService::Ptr> items;
   // We look for a set of files.
   DIR *dp = opendir( QFile::encodeName(dir));
   if (!dp)
      return;

   KDE_struct_dirent *ep;

   while( ( ep = KDE_readdir( dp ) ) != 0L )
   {
      QString fn( QFile::decodeName(ep->d_name));
      if (fn == "." || fn == ".." || fn.at(fn.length() - 1) == '~')
         continue;

      bool isDir;
      bool isReg;
      QString pathfn = dir + fn;

#ifdef HAVE_DIRENT_D_TYPE
      isDir = ep->d_type == DT_DIR;
      isReg = ep->d_type == DT_REG;

      if (ep->d_type == DT_UNKNOWN || ep->d_type == DT_LNK)
#endif
      {
	KDE_struct_stat buff;
        if ( KDE_stat( QFile::encodeName(pathfn), &buff ) != 0 ) {
           continue; // Couldn't stat (e.g. no read permissions)
        }
        isDir = S_ISDIR ( buff.st_mode );
        isReg = S_ISREG ( buff.st_mode );
      }
      if ( isDir ) {
         SubMenu *parentMenu = m_currentMenu;

         m_currentMenu = new SubMenu;
         m_currentMenu->name = fn;
         m_currentMenu->directoryFile = dir + fn + "/.directory";

         parentMenu->subMenus.append(m_currentMenu);

         processLegacyDir(pathfn + '/', relDir+fn+'/', prefix);
         m_currentMenu = parentMenu;
         continue;
      }

      if ( isReg )
      {
         if (!fn.endsWith(".desktop"))
            continue;

         KService::Ptr service;
         emit newService(pathfn, &service);
         if (service)
         {
            QString id = prefix+fn;

            // TODO: Add legacy category
            addApplication(id, service);
            items.insert(service->menuId(), service);

            if (service->categories().isEmpty())
               m_currentMenu->items.insert(id, service);
         }
      }
    }
    closedir( dp );
    markUsedApplications(items);
}



void
VFolderMenu::processMenu(QDomElement &docElem, int pass)
{
   SubMenu *parentMenu = m_currentMenu;
   int oldDirectoryDirsCount = m_directoryDirs.count();

   QString name;
   QString directoryFile;
   bool onlyUnallocated = false;
   bool isDeleted = false;
   bool kdeLegacyDirsDone = false;
   QDomElement defaultLayoutNode;
   QDomElement layoutNode;

   QDomElement query;
   QDomNode n = docElem.firstChild();
   while( !n.isNull() ) {
      QDomElement e = n.toElement(); // try to convert the node to an element.
      if (e.tagName() == "Name")
      {
         name = e.text();
      }
      else if (e.tagName() == "Directory")
      {
         directoryFile = e.text();
      }
      else if (e.tagName() == "DirectoryDir")
      {
         QString dir = absoluteDir(e.text(), e.attribute("__BaseDir"));

         m_directoryDirs.prepend(dir);
      }
      else if (e.tagName() == "OnlyUnallocated")
      {
         onlyUnallocated = true;
      }
      else if (e.tagName() == "NotOnlyUnallocated")
      {
         onlyUnallocated = false;
      }
      else if (e.tagName() == "Deleted")
      {
         isDeleted = true;
      }
      else if (e.tagName() == "NotDeleted")
      {
         isDeleted = false;
      }
      else if (e.tagName() == "DefaultLayout")
      {
         defaultLayoutNode = e;
      }
      else if (e.tagName() == "Layout")
      {
         layoutNode = e;
      }
      n = n.nextSibling();
   }

   // Setup current menu entry
   if (pass == 0)
   {
      m_currentMenu = 0;
      // Look up menu
      if (parentMenu)
      {
         foreach (SubMenu *menu, parentMenu->subMenus)
         {
            if (menu->name == name)
            {
               m_currentMenu = menu;
               break;
            }
         }
      }

      if (!m_currentMenu) // Not found?
      {
         // Create menu
         m_currentMenu = new SubMenu;
         m_currentMenu->name = name;

         if (parentMenu)
            parentMenu->subMenus.append(m_currentMenu);
         else
            m_rootMenu = m_currentMenu;
      }
      if (directoryFile.isEmpty())
      {
         kDebug(7021) << "Menu" << name << "does not specify a directory file.";
      }

      // Override previous directoryFile iff available
      QString tmp = locateDirectoryFile(directoryFile);
      if (! tmp.isEmpty())
         m_currentMenu->directoryFile = tmp;
      m_currentMenu->isDeleted = isDeleted;

      m_currentMenu->defaultLayoutNode = defaultLayoutNode;
      m_currentMenu->layoutNode = layoutNode;
   }
   else
   {
      // Look up menu
      if (parentMenu)
      {
         foreach (SubMenu *menu, parentMenu->subMenus)
         {
            if (menu->name == name)
            {
               m_currentMenu = menu;
               break;
            }
         }
      }
      else
      {
         m_currentMenu = m_rootMenu;
      }
   }

   // Process AppDir and LegacyDir
   if (pass == 0)
   {
      QDomElement query;
      QDomNode n = docElem.firstChild();
      while( !n.isNull() ) {
         QDomElement e = n.toElement(); // try to convert the node to an element.
         if (e.tagName() == "AppDir")
         {
            createAppsInfo();
            QString dir = absoluteDir(e.text(), e.attribute("__BaseDir"));

            registerDirectory(dir);

            loadApplications(dir, QString());
         }
         else if (e.tagName() == "KDELegacyDirs")
         {
            createAppsInfo();
            if (!kdeLegacyDirsDone)
            {
kDebug(7021) << "Processing KDE Legacy dirs for <KDE>";
               SubMenu *oldMenu = m_currentMenu;
               m_currentMenu = new SubMenu;

               processKDELegacyDirs();

               m_legacyNodes.insert("<KDE>", m_currentMenu);
               m_currentMenu = oldMenu;

               kdeLegacyDirsDone = true;
            }
         }
         else if (e.tagName() == "LegacyDir")
         {
            createAppsInfo();
            QString dir = absoluteDir(e.text(), e.attribute("__BaseDir"));

            QString prefix = e.attributes().namedItem("prefix").toAttr().value();

            if (m_defaultLegacyDirs.contains(dir))
            {
               if (!kdeLegacyDirsDone)
               {
kDebug(7021) << "Processing KDE Legacy dirs for" << dir;
                  SubMenu *oldMenu = m_currentMenu;
                  m_currentMenu = new SubMenu;

                  processKDELegacyDirs();

                  m_legacyNodes.insert("<KDE>", m_currentMenu);
                  m_currentMenu = oldMenu;

                  kdeLegacyDirsDone = true;
               }
            }
            else
            {
               SubMenu *oldMenu = m_currentMenu;
               m_currentMenu = new SubMenu;

               registerDirectory(dir);

               processLegacyDir(dir, QString(), prefix);

               m_legacyNodes.insert(dir, m_currentMenu);
               m_currentMenu = oldMenu;
            }
         }
         n = n.nextSibling();
      }
   }

   loadAppsInfo(); // Update the scope wrt the list of applications

   if (((pass == 1) && !onlyUnallocated) || ((pass == 2) && onlyUnallocated))
   {
      n = docElem.firstChild();

      while( !n.isNull() ) {
         QDomElement e = n.toElement(); // try to convert the node to an element.
         if (e.tagName() == "Include")
         {
            QHash<QString,KService::Ptr> items;

            QDomNode n2 = e.firstChild();
            while( !n2.isNull() ) {
               QDomElement e2 = n2.toElement();
               items.clear();
               processCondition(e2, items);
               if (m_track)
                  track(m_trackId, m_currentMenu->name, m_currentMenu->items, m_currentMenu->excludeItems, items, "Before <Include>");
               includeItems(m_currentMenu->items, items);
               excludeItems(m_currentMenu->excludeItems, items);
               markUsedApplications(items);

               if (m_track)
                  track(m_trackId, m_currentMenu->name, m_currentMenu->items, m_currentMenu->excludeItems, items, "After <Include>");

               n2 = n2.nextSibling();
            }
         }

         else if (e.tagName() == "Exclude")
         {
            QHash<QString,KService::Ptr> items;

            QDomNode n2 = e.firstChild();
            while( !n2.isNull() ) {
               QDomElement e2 = n2.toElement();
               items.clear();
               processCondition(e2, items);
               if (m_track)
                  track(m_trackId, m_currentMenu->name, m_currentMenu->items, m_currentMenu->excludeItems, items, "Before <Exclude>");
               excludeItems(m_currentMenu->items, items);
               includeItems(m_currentMenu->excludeItems, items);
               if (m_track)
                  track(m_trackId, m_currentMenu->name, m_currentMenu->items, m_currentMenu->excludeItems, items, "After <Exclude>");
               n2 = n2.nextSibling();
            }
         }

         n = n.nextSibling();
      }
   }

   n = docElem.firstChild();
   while( !n.isNull() ) {
      QDomElement e = n.toElement(); // try to convert the node to an element.
      if (e.tagName() == "Menu")
      {
         processMenu(e, pass);
      }
// We insert legacy dir in pass 0, this way the order in the .menu-file determines
// which .directory file gets used, but the menu-entries of legacy-menus will always
// have the lowest priority.
//      else if (((pass == 1) && !onlyUnallocated) || ((pass == 2) && onlyUnallocated))
      else if (pass == 0)
      {
         if (e.tagName() == "LegacyDir")
         {
            // Add legacy nodes to Menu structure
            QString dir = absoluteDir(e.text(), e.attribute("__BaseDir"));
            SubMenu *legacyMenu = m_legacyNodes[dir];
            if (legacyMenu)
            {
               mergeMenu(m_currentMenu, legacyMenu);
            }
         }

         else if (e.tagName() == "KDELegacyDirs")
         {
            // Add legacy nodes to Menu structure
            QString dir = "<KDE>";
            SubMenu *legacyMenu = m_legacyNodes[dir];
            if (legacyMenu)
            {
               mergeMenu(m_currentMenu, legacyMenu);
            }
         }
      }
      n = n.nextSibling();
   }

   if (pass == 2)
   {
      n = docElem.firstChild();
      while( !n.isNull() ) {
         QDomElement e = n.toElement(); // try to convert the node to an element.
         if (e.tagName() == "Move")
         {
            QString orig;
            QString dest;
            QDomNode n2 = e.firstChild();
            while( !n2.isNull() ) {
               QDomElement e2 = n2.toElement(); // try to convert the node to an element.
               if( e2.tagName() == "Old")
                  orig = e2.text();
               if( e2.tagName() == "New")
                  dest = e2.text();
               n2 = n2.nextSibling();
            }
            kDebug(7021) << "Moving" << orig << "to" << dest;
            if (!orig.isEmpty() && !dest.isEmpty())
            {
              SubMenu *menu = takeSubMenu(m_currentMenu, orig);
              if (menu)
              {
                insertSubMenu(m_currentMenu, dest, menu, true); // Destination has priority
              }
            }
         }
         n = n.nextSibling();
      }

   }

   unloadAppsInfo(); // Update the scope wrt the list of applications

   while (m_directoryDirs.count() > oldDirectoryDirsCount)
      m_directoryDirs.pop_front();

   m_currentMenu = parentMenu;
}



static QString parseAttribute( const QDomElement &e)
{
    QString option;
    if ( e.hasAttribute( "show_empty" ) )
    {
        QString str = e.attribute( "show_empty" );
        if ( str=="true" )
            option= "ME ";
        else if ( str=="false" )
            option= "NME ";
        else
            kDebug()<<" Error in parsing show_empty attribute :"<<str;
    }
    if ( e.hasAttribute( "inline" ) )
    {
        QString str = e.attribute( "inline" );
        if (  str=="true" )
            option+="I ";
        else if ( str=="false" )
            option+="NI ";
        else
            kDebug()<<" Error in parsing inlibe attribute :"<<str;
    }
    if ( e.hasAttribute( "inline_limit" ) )
    {
        bool ok;
        int value = e.attribute( "inline_limit" ).toInt(&ok);
        if ( ok )
            option+=QString( "IL[%1] " ).arg( value );
    }
    if ( e.hasAttribute( "inline_header" ) )
    {
        QString str = e.attribute( "inline_header" );
        if ( str=="true")
            option+="IH ";
        else if ( str == "false" )
            option+="NIH ";
        else
            kDebug()<<" Error in parsing of inline_header attribute :"<<str;

    }
    if ( e.hasAttribute( "inline_alias" ) && e.attribute( "inline_alias" )=="true")
    {
        QString str = e.attribute( "inline_alias" );
        if ( str=="true" )
            option+="IA";
        else if ( str=="false" )
            option+="NIA";
        else
            kDebug()<<" Error in parsing inline_alias attribute :"<<str;
    }
    if( !option.isEmpty())
    {
        option = option.prepend(":O");
    }
    return option;

}

static QStringList parseLayoutNode(const QDomElement &docElem)
{
   QStringList layout;

   QString optionDefaultLayout;
   if( docElem.tagName()=="DefaultLayout")
       optionDefaultLayout =  parseAttribute( docElem);
   if ( !optionDefaultLayout.isEmpty() )
       layout.append( optionDefaultLayout );

   QDomNode n = docElem.firstChild();
   while( !n.isNull() ) {
      QDomElement e = n.toElement(); // try to convert the node to an element.
      if (e.tagName() == "Separator")
      {
         layout.append(":S");
      }
      else if (e.tagName() == "Filename")
      {
         layout.append(e.text());
      }
      else if (e.tagName() == "Menuname")
      {
         layout.append('/'+e.text());
         QString option = parseAttribute( e );
         if( !option.isEmpty())
             layout.append( option );
      }
      else if (e.tagName() == "Merge")
      {
         QString type = e.attributeNode("type").value();
         if (type == "files")
            layout.append(":F");
         else if (type == "menus")
            layout.append(":M");
         else if (type == "all")
            layout.append(":A");
      }

      n = n.nextSibling();
   }
   return layout;
}

void
VFolderMenu::layoutMenu(VFolderMenu::SubMenu *menu, QStringList defaultLayout) //krazy:exclude=passbyvalue
{
   if (!menu->defaultLayoutNode.isNull())
   {
      defaultLayout = parseLayoutNode(menu->defaultLayoutNode);
   }

   if (menu->layoutNode.isNull())
   {
     menu->layoutList = defaultLayout;
   }
   else
   {
     menu->layoutList = parseLayoutNode(menu->layoutNode);
     if (menu->layoutList.isEmpty())
        menu->layoutList = defaultLayout;
   }

   foreach (VFolderMenu::SubMenu *subMenu, menu->subMenus)
   {
      layoutMenu(subMenu, defaultLayout);
   }
}

void
VFolderMenu::markUsedApplications(const QHash<QString,KService::Ptr>& items)
{
   foreach(const KService::Ptr &p, items)
      m_usedAppsDict.insert(p->menuId(), p);
}

VFolderMenu::SubMenu *
VFolderMenu::parseMenu(const QString &file, bool forceLegacyLoad)
{
   m_forcedLegacyLoad = false;
   m_legacyLoaded = false;
   m_appsInfo = 0;

   const QStringList dirs = KGlobal::dirs()->resourceDirs("xdgconf-menu");
   for(QStringList::ConstIterator it=dirs.begin();
       it != dirs.end(); ++it)
   {
      registerDirectory(*it);
   }

   loadMenu(file);

   delete m_rootMenu;
   m_rootMenu = m_currentMenu = 0;

   QDomElement docElem = m_doc.documentElement();

   for (int pass = 0; pass <= 2; pass++)
   {
      processMenu(docElem, pass);

      if (pass == 0)
      {
         buildApplicationIndex(false);
      } else
      if (pass == 1)
      {
         buildApplicationIndex(true);
      } else
      if (pass == 2)
      {
         QStringList defaultLayout;
         defaultLayout << ":M"; // Sub-Menus
         defaultLayout << ":F"; // Individual entries
         layoutMenu(m_rootMenu, defaultLayout);
      }
   }

   if (!m_legacyLoaded && forceLegacyLoad)
   {
      m_forcedLegacyLoad = true;
      processKDELegacyDirs();
   }

   return m_rootMenu;
}

void
VFolderMenu::setTrackId(const QString &id)
{
   m_track = !id.isEmpty();
   m_trackId = id;
}

#include "vfolder_menu.moc"
