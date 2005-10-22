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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h> // getenv

#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kservice.h>
#include <kde_file.h>

#include <qmap.h>
#include <qfile.h>
#include <qdir.h>
#include <qregexp.h>

#include "vfolder_menu.h"

static void foldNode(QDomElement &docElem, QDomElement &e, QMap<QString,QDomElement> &dupeList, QString s=QString::null)
{
   if (s.isEmpty())
      s = e.text();
   QMap<QString,QDomElement>::iterator it = dupeList.find(s);
   if (it != dupeList.end())
   {
      kdDebug(7021) << e.tagName() << " and " << s << " requires combining!" << endl;

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
//   kdDebug(7021) << "Next tag = " << n.toElement().tagName() << endl;
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
track(const QString &menuId, const QString &menuName, Q3Dict<KService> *includeList, Q3Dict<KService> *excludeList, Q3Dict<KService> *itemList, const QString &comment)
{
   if (itemList->find(menuId))
      printf("%s: %s INCL %d EXCL %d\n", qPrintable(menuName), qPrintable(comment), includeList->find(menuId) ? 1 : 0, excludeList->find(menuId) ? 1 : 0);
}

void
VFolderMenu::includeItems(Q3Dict<KService> *items1, Q3Dict<KService> *items2)
{
   for(Q3DictIterator<KService> it(*items2); it.current(); ++it)
   {
       items1->replace(it.current()->menuId(), it.current());
   }
}

void
VFolderMenu::matchItems(Q3Dict<KService> *items1, Q3Dict<KService> *items2)
{
   for(Q3DictIterator<KService> it(*items1); it.current(); )
   {
       QString id = it.current()->menuId();
       ++it;
       if (!items2->find(id))
          items1->remove(id);
   }
}

void
VFolderMenu::excludeItems(Q3Dict<KService> *items1, Q3Dict<KService> *items2)
{
   for(Q3DictIterator<KService> it(*items2); it.current(); ++it)
   {
       items1->remove(it.current()->menuId());
   }
}

VFolderMenu::SubMenu*
VFolderMenu::takeSubMenu(SubMenu *parentMenu, const QString &menuName)
{
   const int i = menuName.indexOf('/');
   const QString s1 = i > 0 ? menuName.left(i) : menuName;
   const QString s2 = menuName.mid(i+1);

   // Look up menu
   for(SubMenu *menu = parentMenu->subMenus.first(); menu; menu = parentMenu->subMenus.next())
   {
      if (menu->name == s1)
      {
         if (i == -1)
         {
            // Take it out
            return parentMenu->subMenus.take();
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
      track(m_trackId, menu1->name, &(menu1->items), &(menu1->excludeItems), &(menu2->items), QString("Before MenuMerge w. %1 (incl)").arg(menu2->name));
      track(m_trackId, menu1->name, &(menu1->items), &(menu1->excludeItems), &(menu2->excludeItems), QString("Before MenuMerge w. %1 (excl)").arg(menu2->name));
   }
   if (reversePriority)
   {
      // Merge menu1 with menu2, menu1 takes precedent
      excludeItems(&(menu2->items), &(menu1->excludeItems));
      includeItems(&(menu1->items), &(menu2->items));
      excludeItems(&(menu2->excludeItems), &(menu1->items));
      includeItems(&(menu1->excludeItems), &(menu2->excludeItems));
   }
   else
   {
      // Merge menu1 with menu2, menu2 takes precedent
      excludeItems(&(menu1->items), &(menu2->excludeItems));
      includeItems(&(menu1->items), &(menu2->items));
      includeItems(&(menu1->excludeItems), &(menu2->excludeItems));
      menu1->isDeleted = menu2->isDeleted;
   }
   for(; menu2->subMenus.first(); )
   {
      SubMenu *subMenu = menu2->subMenus.take();
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
      track(m_trackId, menu1->name, &(menu1->items), &(menu1->excludeItems), &(menu2->items), QString("After MenuMerge w. %1 (incl)").arg(menu2->name));
      track(m_trackId, menu1->name, &(menu1->items), &(menu1->excludeItems), &(menu2->excludeItems), QString("After MenuMerge w. %1 (excl)").arg(menu2->name));
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
   for(SubMenu *menu = parentMenu->subMenus.first(); menu; menu = parentMenu->subMenus.next())
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
VFolderMenu::insertService(SubMenu *parentMenu, const QString &name, KService *newService)
{
   const int i = name.indexOf('/');

   if (i == -1)
   {
     // Add it here
     parentMenu->items.replace(newService->menuId(), newService);
     return;
   }

   QString s1 = name.left(i);
   QString s2 = name.mid(i+1);

   // Look up menu
   for(SubMenu *menu = parentMenu->subMenus.first(); menu; menu = parentMenu->subMenus.next())
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


VFolderMenu::VFolderMenu() : m_usedAppsDict(797), m_track(false)
{
   m_rootMenu = 0;
   initDirs();
}

VFolderMenu::~VFolderMenu()
{
   delete m_rootMenu;
}

#define FOR_ALL_APPLICATIONS(it) \
   for(appsInfo *info = m_appsInfoStack.first(); \
       info; info = m_appsInfoStack.next()) \
   { \
      for(Q3DictIterator<KService> it( info->applications ); \
          it.current(); ++it ) \
      {
#define FOR_ALL_APPLICATIONS_END } }

#define FOR_CATEGORY(category, it) \
   for(appsInfo *info = m_appsInfoStack.first(); \
       info; info = m_appsInfoStack.next()) \
   { \
      KService::List *list = info->dictCategories.find(category); \
      if (list) for(KService::List::ConstIterator it = list->begin(); \
             it != list->end(); ++it) \
      {
#define FOR_CATEGORY_END } }

KService *
VFolderMenu::findApplication(const QString &relPath)
{
   for(appsInfo *info = m_appsInfoStack.first();
       info; info = m_appsInfoStack.next())
   {
      KService *s = info->applications.find(relPath);
      if (s)
         return s;
   }
   return 0;
}

void
VFolderMenu::addApplication(const QString &id, KService *service)
{
   service->setMenuId(id);
   m_appsInfo->applications.replace(id, service);
}

void
VFolderMenu::buildApplicationIndex(bool unusedOnly)
{
   Q3PtrList<appsInfo>::ConstIterator appsInfo_it =  m_appsInfoList.begin();
   for( ; appsInfo_it != m_appsInfoList.end(); ++appsInfo_it )
   {
      appsInfo *info = *appsInfo_it;
      info->dictCategories.clear();
      for(Q3DictIterator<KService> it( info->applications );
          it.current(); )
      {
         KService *s = it.current();
         Q3DictIterator<KService> tmpIt = it;
         ++it;
         if (unusedOnly && m_usedAppsDict.find(s->menuId()))
         {
            // Remove and skip this one
            info->applications.remove(tmpIt.currentKey());
            continue;
         }

         QStringList cats = s->categories();
         for(QStringList::ConstIterator it2 = cats.begin();
             it2 != cats.end(); ++it2)
         {
            const QString &cat = *it2;
            KService::List *list = info->dictCategories.find(cat);
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

   if (m_appsInfoStack.first() == m_appsInfo)
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

   m_appsInfoStack.remove(m_appsInfo); // Remove
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
   if (!dir.endsWith("/"))
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
      kdWarning(7021) << "Could not open " << m_docInfo.path << endl;
      return doc;
   }
   QString errorMsg;
   int errorRow;
   int errorCol;
   if ( !doc.setContent( &file, &errorMsg, &errorRow, &errorCol ) ) {
      kdWarning(7021) << "Parse error in " << m_docInfo.path << ", line " << errorRow << ", col " << errorCol << ": " << errorMsg << endl;
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
kdDebug(7021) << "VFolderMenu::mergeFile: " << m_docInfo.path << endl;
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
// kdDebug(7021) << "Empty node" << endl;
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

         QStringList dirs = KGlobal::dirs()->findDirs("xdgconf-menu", dir);
         for(QStringList::ConstIterator it=dirs.begin();
             it != dirs.end(); ++it)
         {
            registerDirectory(*it);
         }

         QStringList fileList;
         if (!QDir::isRelativePath(dir))
         {
            // Absolute
            fileList = KGlobal::dirs()->findAllResources("xdgconf-menu", dir+"*.menu", false, false);
         }
         else
         {
            // Relative
            (void) KGlobal::dirs()->findAllResources("xdgconf-menu", dir+"*.menu", false, true, fileList);
         }

         for(QStringList::ConstIterator it=fileList.begin();
             it != fileList.end(); ++it)
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
      m_docInfo.baseDir = QString::null;
      m_docInfo.baseName = QString::null;
      kdDebug(7021) << "Menu " << fileName << " not found." << endl;
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
      m_docInfo.baseDir = QString::null;
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
      m_docInfo.path = QString::null; // No parent found
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
      return QString::null;
   }

   QString baseName = QDir::cleanPath(m_docInfo.baseDir + fileName);
   QString result = locate("xdgconf-menu", baseName);

   return result;
}

QString
VFolderMenu::locateDirectoryFile(const QString &fileName)
{
   if (fileName.isEmpty())
      return QString::null;

   if (!QDir::isRelativePath(fileName))
   {
      if (KStandardDirs::exists(fileName))
         return fileName;
      return QString::null;
   }

   // First location in the list wins
   QString tmp;
   for(QStringList::ConstIterator it = m_directoryDirs.begin();
       it != m_directoryDirs.end();
       ++it)
   {
      tmp = (*it)+fileName;
      if (KStandardDirs::exists(tmp))
         return tmp;
   }

   return QString::null;
}

void
VFolderMenu::initDirs()
{
   m_defaultDataDirs = KGlobal::dirs()->kfsstnd_prefixes().split(':', QString::SkipEmptyParts);
   const QString localDir = m_defaultDataDirs.first();
   m_defaultDataDirs.removeAll(localDir); // Remove local dir

   m_defaultAppDirs = KGlobal::dirs()->findDirs("xdgdata-apps", QString::null);
   m_defaultDirectoryDirs = KGlobal::dirs()->findDirs("xdgdata-dirs", QString::null);
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
      kdWarning(7021) << "Load error (" << m_docInfo.path << ")" << endl;
      return;
   }

   QDomElement e = m_doc.documentElement();
   QString name;
   mergeMenus(e, name);
}

void
VFolderMenu::processCondition(QDomElement &domElem, Q3Dict<KService> *items)
{
   if (domElem.tagName() == "And")
   {
      QDomNode n = domElem.firstChild();
      if (!n.isNull())
      {
         QDomElement e = n.toElement();
         processCondition(e, items);
         n = n.nextSibling();
      }

      Q3Dict<KService> andItems;
      while( !n.isNull() ) {
         QDomElement e = n.toElement();
         if (e.tagName() == "Not")
         {
            // Special handling for "and not"
            QDomNode n2 = e.firstChild();
            while( !n2.isNull() ) {
               QDomElement e2 = n2.toElement();
               andItems.clear();
               processCondition(e2, &andItems);
               excludeItems(items, &andItems);
               n2 = n2.nextSibling();
            }
         }
         else
         {
            andItems.clear();
            processCondition(e, &andItems);
            matchItems(items, &andItems);
         }
         n = n.nextSibling();
      }
   }
   else if (domElem.tagName() == "Or")
   {
      QDomNode n = domElem.firstChild();
      if (!n.isNull())
      {
         QDomElement e = n.toElement();
         processCondition(e, items);
         n = n.nextSibling();
      }

      Q3Dict<KService> orItems;
      while( !n.isNull() ) {
         QDomElement e = n.toElement();
         orItems.clear();
         processCondition(e, &orItems);
         includeItems(items, &orItems);
         n = n.nextSibling();
      }
   }
   else if (domElem.tagName() == "Not")
   {
      FOR_ALL_APPLICATIONS(it)
      {
         KService *s = it.current();
         items->replace(s->menuId(), s);
      }
      FOR_ALL_APPLICATIONS_END

      Q3Dict<KService> notItems;
      QDomNode n = domElem.firstChild();
      while( !n.isNull() ) {
         QDomElement e = n.toElement();
         notItems.clear();
         processCondition(e, &notItems);
         excludeItems(items, &notItems);
         n = n.nextSibling();
      }
   }
   else if (domElem.tagName() == "Category")
   {
      FOR_CATEGORY(domElem.text(), it)
      {
         KService *s = (*it).get();
         items->replace(s->menuId(), s);
      }
      FOR_CATEGORY_END
   }
   else if (domElem.tagName() == "All")
   {
      FOR_ALL_APPLICATIONS(it)
      {
         KService *s = it.current();
         items->replace(s->menuId(), s);
      }
      FOR_ALL_APPLICATIONS_END
   }
   else if (domElem.tagName() == "Filename")
   {
      QString filename = domElem.text();
kdDebug(7021) << "Adding file " << filename << endl;
      KService *s = findApplication(filename);
      if (s)
         items->replace(filename, s);
   }
}

void
VFolderMenu::loadApplications(const QString &dir, const QString &prefix)
{
   kdDebug(7021) << "Looking up applications under " << dir << endl;

   // We look for a set of files.
   DIR *dp = opendir( QFile::encodeName(dir));
   if (!dp)
      return;

   struct dirent *ep;
   KDE_struct_stat buff;

   QString _dot(".");
   QString _dotdot("..");

   while( ( ep = readdir( dp ) ) != 0L )
   {
      QString fn( QFile::decodeName(ep->d_name));
      if (fn == _dot || fn == _dotdot || fn.at(fn.length() - 1) == '~')
         continue;

      QString pathfn = dir + fn;
      if ( KDE_stat( QFile::encodeName(pathfn), &buff ) != 0 ) {
         continue; // Couldn't stat (e.g. no read permissions)
      }
      if ( S_ISDIR( buff.st_mode )) {
         loadApplications(pathfn + '/', prefix + fn + '-');
         continue;
      }

      if ( S_ISREG( buff.st_mode))
      {
         if (!fn.endsWith(".desktop"))
            continue;

         KService *service = 0;
         emit newService(pathfn, &service);
         if (service)
            addApplication(prefix+fn, service);
      }
    }
    closedir( dp );
}

void
VFolderMenu::processKDELegacyDirs()
{
kdDebug(7021) << "processKDELegacyDirs()" << endl;

   Q3Dict<KService> items;
   QString prefix = "kde-";

   QStringList relFiles;
   QRegExp files("\\.(desktop|kdelnk)$");
   QRegExp dirs("\\.directory$");

   (void) KGlobal::dirs()->findAllResources( "apps",
                                             QString::null,
                                             true, // Recursive!
                                             true, // uniq
                                             relFiles);
   for(QStringList::ConstIterator it = relFiles.begin();
       it != relFiles.end(); ++it)
   {
      if (!m_forcedLegacyLoad && (dirs.indexIn(*it) != -1))
      {
         QString name = *it;
         if (!name.endsWith("/.directory"))
            continue; // Probably ".directory", skip it.

         name = name.left(name.length()-11);

         SubMenu *newMenu = new SubMenu;
         newMenu->directoryFile = locate("apps", *it);

         insertSubMenu(m_currentMenu, name, newMenu);
         continue;
      }

      if (files.indexIn(*it) != -1)
      {
         QString name = *it;
         KService *service = 0;
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
            items.replace(service->menuId(), service);
            if (service->categories().isEmpty())
               insertService(m_currentMenu, name, service);

         }
      }
   }
   markUsedApplications(&items);
   m_legacyLoaded = true;
}

void
VFolderMenu::processLegacyDir(const QString &dir, const QString &relDir, const QString &prefix)
{
kdDebug(7021) << "processLegacyDir(" << dir << ", " << relDir << ", " << prefix << ")" << endl;

   Q3Dict<KService> items;
   // We look for a set of files.
   DIR *dp = opendir( QFile::encodeName(dir));
   if (!dp)
      return;

   struct dirent *ep;
   KDE_struct_stat buff;

   QString _dot(".");
   QString _dotdot("..");

   while( ( ep = readdir( dp ) ) != 0L )
   {
      QString fn( QFile::decodeName(ep->d_name));
      if (fn == _dot || fn == _dotdot || fn.at(fn.length() - 1) == '~')
         continue;

      QString pathfn = dir + fn;
      if ( KDE_stat( QFile::encodeName(pathfn), &buff ) != 0 ) {
         continue; // Couldn't stat (e.g. no read permissions)
      }
      if ( S_ISDIR( buff.st_mode )) {
         SubMenu *parentMenu = m_currentMenu;

         m_currentMenu = new SubMenu;
         m_currentMenu->name = fn;
         m_currentMenu->directoryFile = dir + fn + "/.directory";

         parentMenu->subMenus.append(m_currentMenu);

         processLegacyDir(pathfn + '/', relDir+fn+'/', prefix);
         m_currentMenu = parentMenu;
         continue;
      }

      if ( S_ISREG( buff.st_mode))
      {
         if (!fn.endsWith(".desktop"))
            continue;

         KService *service = 0;
         emit newService(pathfn, &service);
         if (service)
         {
            QString id = prefix+fn;

            // TODO: Add legacy category
            addApplication(id, service);
            items.replace(service->menuId(), service);

            if (service->categories().isEmpty())
               m_currentMenu->items.replace(id, service);
         }
      }
    }
    closedir( dp );
    markUsedApplications(&items);
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
         for(SubMenu *menu = parentMenu->subMenus.first(); menu; menu = parentMenu->subMenus.next())
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
         kdDebug(7021) << "Menu " << name << " does not specify a directory file." << endl;
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
         for(SubMenu *menu = parentMenu->subMenus.first(); menu; menu = parentMenu->subMenus.next())
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

            loadApplications(dir, QString::null);
         }
         else if (e.tagName() == "KDELegacyDirs")
         {
            createAppsInfo();
            if (!kdeLegacyDirsDone)
            {
kdDebug(7021) << "Processing KDE Legacy dirs for <KDE>" << endl;
               SubMenu *oldMenu = m_currentMenu;
               m_currentMenu = new SubMenu;

               processKDELegacyDirs();

               m_legacyNodes.replace("<KDE>", m_currentMenu);
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
kdDebug(7021) << "Processing KDE Legacy dirs for " << dir << endl;
                  SubMenu *oldMenu = m_currentMenu;
                  m_currentMenu = new SubMenu;

                  processKDELegacyDirs();

                  m_legacyNodes.replace("<KDE>", m_currentMenu);
                  m_currentMenu = oldMenu;

                  kdeLegacyDirsDone = true;
               }
            }
            else
            {
               SubMenu *oldMenu = m_currentMenu;
               m_currentMenu = new SubMenu;

               registerDirectory(dir);

               processLegacyDir(dir, QString::null, prefix);

               m_legacyNodes.replace(dir, m_currentMenu);
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
            Q3Dict<KService> items;

            QDomNode n2 = e.firstChild();
            while( !n2.isNull() ) {
               QDomElement e2 = n2.toElement();
               items.clear();
               processCondition(e2, &items);
               if (m_track)
                  track(m_trackId, m_currentMenu->name, &(m_currentMenu->items), &(m_currentMenu->excludeItems), &items, "Before <Include>");
               includeItems(&(m_currentMenu->items), &items);
               excludeItems(&(m_currentMenu->excludeItems), &items);
               markUsedApplications(&items);

               if (m_track)
                  track(m_trackId, m_currentMenu->name, &(m_currentMenu->items), &(m_currentMenu->excludeItems), &items, "After <Include>");

               n2 = n2.nextSibling();
            }
         }

         else if (e.tagName() == "Exclude")
         {
            Q3Dict<KService> items;

            QDomNode n2 = e.firstChild();
            while( !n2.isNull() ) {
               QDomElement e2 = n2.toElement();
               items.clear();
               processCondition(e2, &items);
               if (m_track)
                  track(m_trackId, m_currentMenu->name, &(m_currentMenu->items), &(m_currentMenu->excludeItems), &items, "Before <Exclude>");
               excludeItems(&(m_currentMenu->items), &items);
               includeItems(&(m_currentMenu->excludeItems), &items);
               if (m_track)
                  track(m_trackId, m_currentMenu->name, &(m_currentMenu->items), &(m_currentMenu->excludeItems), &items, "After <Exclude>");
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
            SubMenu *legacyMenu = m_legacyNodes.find(dir);
            if (legacyMenu)
            {
               mergeMenu(m_currentMenu, legacyMenu);
            }
         }

         else if (e.tagName() == "KDELegacyDirs")
         {
            // Add legacy nodes to Menu structure
            QString dir = "<KDE>";
            SubMenu *legacyMenu = m_legacyNodes.find(dir);
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
            kdDebug(7021) << "Moving " << orig << " to " << dest << endl;
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
            kdDebug()<<" Error in parsing show_empty attribute :"<<str<<endl;
    }
    if ( e.hasAttribute( "inline" ) )
    {
        QString str = e.attribute( "inline" );
        if (  str=="true" )
            option+="I ";
        else if ( str=="false" )
            option+="NI ";
        else
            kdDebug()<<" Error in parsing inlibe attribute :"<<str<<endl;
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
            kdDebug()<<" Error in parsing of inline_header attribute :"<<str<<endl;

    }
    if ( e.hasAttribute( "inline_alias" ) && e.attribute( "inline_alias" )=="true")
    {
        QString str = e.attribute( "inline_alias" );
        if ( str=="true" )
            option+="IA";
        else if ( str=="false" )
            option+="NIA";
        else
            kdDebug()<<" Error in parsing inline_alias attribute :"<<str<<endl;
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
         layout.append("/"+e.text());
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
VFolderMenu::layoutMenu(VFolderMenu::SubMenu *menu, QStringList defaultLayout)
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

   for(VFolderMenu::SubMenu *subMenu = menu->subMenus.first(); subMenu; subMenu = menu->subMenus.next())
   {
      layoutMenu(subMenu, defaultLayout);
   }
}

void
VFolderMenu::markUsedApplications(Q3Dict<KService> *items)
{
   for(Q3DictIterator<KService> it(*items); it.current(); ++it)
   {
      m_usedAppsDict.replace(it.current()->menuId(), it.current());
   }
}

VFolderMenu::SubMenu *
VFolderMenu::parseMenu(const QString &file, bool forceLegacyLoad)
{
   m_forcedLegacyLoad = false;
   m_legacyLoaded = false;
   m_appsInfo = 0;

   QStringList dirs = KGlobal::dirs()->resourceDirs("xdgconf-menu");
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
      }
      if (pass == 1)
      {
         buildApplicationIndex(true);
      }
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
