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

#include <qmap.h>
#include <qfile.h>
#include <qdir.h>
#include <qregexp.h>

#include "vfolder_menu.h"

static VFolderMenu* g_this = 0;

static QString relPath(KService *service)
{
   QString *result = g_this->m_appRelPaths.find(service);
   if (result)
      return *result;
kdWarning(7021) << "Panic! " << service->desktopEntryPath() << " not found!" << endl;
   return QString::null;
}

static QDomDocument loadDoc(const QString &filename)
{
   QDomDocument doc;
   if ( filename.isEmpty() )
   {
      return doc;
   }
   QFile file( filename );
   if ( !file.open( IO_ReadOnly ) )
   {
      kdWarning(7021) << "Could not open " << filename << endl;
      return doc;
   }
   QString errorMsg;
   int errorRow;
   int errorCol;
   if ( !doc.setContent( &file, &errorMsg, &errorRow, &errorCol ) ) {
      kdWarning(7021) << "Parse error in " << filename << ", line " << errorRow << ", col " << errorCol << ": " << errorMsg << endl;
      file.close();
      return doc;
   }
   file.close();
   return doc;
}                                            

static void foldNode(QDomElement &docElem, QDomElement &e, QMap<QString,QDomElement> &dupeList, QString s=QString::null)
{
   if (s.isEmpty())
      s = e.text();
   QMap<QString,QDomElement>::iterator it = dupeList.find(s);
   if (it != dupeList.end())
   {
      kdDebug(7021) << e.tagName() << " and " << s << " requires combining!" << endl;
          
      docElem.removeChild(*it);
      dupeList.remove(it);
   }
   dupeList.insert(s, e);
}

static void replaceNode(QDomElement &docElem, QDomNode &n, const QStringList &list, const QString &tag)
{
   for(QStringList::ConstIterator it = list.begin();
       it != list.end(); ++it)
   {
kdDebug(7021) << "Inserting " << tag << " -> " << (*it) << endl;
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

static void
includeItems(QDict<KService> *items1, QDict<KService> *items2)
{
   for(QDictIterator<KService> it(*items2); it.current(); ++it)
   {
       items1->replace(relPath(it.current()), it.current());
   }
}

static void
matchItems(QDict<KService> *items1, QDict<KService> *items2)
{
   for(QDictIterator<KService> it(*items1); it.current(); )
   {
       QString entry = relPath(it.current());
       ++it;
       if (!items2->find(entry))
          items1->remove(entry);
   }
}

static void
excludeItems(QDict<KService> *items1, QDict<KService> *items2)
{
   for(QDictIterator<KService> it(*items2); it.current(); ++it)
   {
       items1->remove(relPath(it.current()));
   }
}

static VFolderMenu::SubMenu* takeSubMenu(VFolderMenu::SubMenu *parentMenu, const QString &name)
{
   int i = name.find('/');
   QString s1 = i > 0 ? name.left(i) : name;
   QString s2 = name.mid(i+1);
      
   // Look up menu
   for(VFolderMenu::SubMenu *menu = parentMenu->subMenus.first(); menu; menu = parentMenu->subMenus.next())
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

static bool insertSubMenu(VFolderMenu::SubMenu *parentMenu, const QString &name, VFolderMenu::SubMenu *newMenu){
   int i = name.find('/');
   
   QString s1 = name.left(i);
   QString s2 = name.mid(i+1);
      
   // Look up menu
   for(VFolderMenu::SubMenu *menu = parentMenu->subMenus.first(); menu; menu = parentMenu->subMenus.next())
   {
      if (menu->name == s1)
      {
         if (i == -1)
         {
kdDebug(7021) << "Merging " << name << " submenu's" << endl;
            // Merge newMenu with menu.
            includeItems(&(menu->items), &(newMenu->items));
            for(; newMenu->subMenus.first(); menu->subMenus.append(newMenu->subMenus.take()));

            if (!newMenu->directoryFile.isEmpty())
               menu->directoryFile = newMenu->directoryFile;

            delete newMenu;

            return true;
         }
         else
         {
            return insertSubMenu(menu, s2, newMenu);
         }
      }
   }
   if (i == -1)
   {
kdDebug(7021) << "Adding new " << name << " submenu" << endl;
     // Add it here
     newMenu->name = name;
     parentMenu->subMenus.append(newMenu);
     return true;
   }
   else
   {
kdDebug(7021) << "insertSubMenu: Creating new sub menu " << s1 << endl;
     VFolderMenu::SubMenu *menu = new VFolderMenu::SubMenu;
     menu->name = s1;
     parentMenu->subMenus.append(menu);
     return insertSubMenu(menu, s2, newMenu);
   }
   return false; // Not found
}

// TODO: bool makes no sense, always succeeds.
// Insert a service into a menu, creating any missing submenu
static bool insertService(VFolderMenu::SubMenu *parentMenu, const QString &name, KService *newService)
{
   int i = name.find('/');
   
   if (i == -1)
   {
     // Add it here
     parentMenu->items.replace(name, newService);
     return true;
   }

   QString s1 = name.left(i);
   QString s2 = name.mid(i+1);
      
   // Look up menu
   for(VFolderMenu::SubMenu *menu = parentMenu->subMenus.first(); menu; menu = parentMenu->subMenus.next())
   {
      if (menu->name == s1)
      {
         return insertService(menu, s2, newService);
      }
   }
   
kdDebug(7021) << "insertService: Creating new sub menu " << s1 << endl;
   VFolderMenu::SubMenu *menu = new VFolderMenu::SubMenu;
   menu->name = s1;
   parentMenu->subMenus.append(menu);
   return insertService(menu, s2, newService);
}


VFolderMenu::VFolderMenu()
{
   m_rootMenu = 0;
   m_dictCategories = 0;
   initDirs();
}

VFolderMenu::~VFolderMenu()
{
   delete m_rootMenu;
}

QString 
VFolderMenu::absoluteDir(const QString &_dir)
{
   QString dir = _dir;
   if (!dir.startsWith("/"))
   {
      dir = m_docInfo.absBaseDir + dir;
   }
   if (!dir.endsWith("/"))
      dir += '/';
   
   dir = KGlobal::dirs()->realPath(dir);

   return dir;
}

void 
VFolderMenu::mergeFile(QDomElement &parent, const QDomNode &mergeHere)
{
kdDebug(7021) << "VFolderMenu::mergeFile: " << m_docInfo.path << endl;
   QDomDocument doc = loadDoc(m_docInfo.path);
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
kdDebug(7021) << "Merging Tag: " << e.tagName() << endl;
         parent.insertAfter(n, last);
         last = n;
      }
      else
      {
kdDebug(7021) << "Skipping Tag: " << e.tagName() << endl;
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
           menuNodes.remove(it);
         }
         menuNodes.insert(name, e);
      }
      else if( e.tagName() == "MergeFile") {
         pushDocInfo(e.text());
         mergeFile(docElem, n);
         popDocInfo();

         QDomNode last = n;
         n = n.nextSibling();
         docElem.removeChild(last); // Remove the MergeFile node
         continue;
      }
      else if( e.tagName() == "MergeDir") {
         QString dir = absoluteDir(e.text());

         // We look for a set of files.
         DIR *dp = opendir( QFile::encodeName(dir));
         if (dp)
         {
            struct dirent *ep;
            while( ( ep = readdir( dp ) ) != 0L )
            {
               QString fn( QFile::decodeName(ep->d_name));
               if (!fn.endsWith(".menu"))
                  continue;

               pushDocInfo(dir + fn);
               mergeFile(docElem, n);
               popDocInfo();
            }
            closedir( dp );
         }
         
         QDomNode last = n;
         n = n.nextSibling();
         docElem.removeChild(last); // Remove the MergeDir node

         continue;
      }
      else if( e.tagName() == "Name") {
         name = e.text();
      }
      n = n.nextSibling();
   }
}

static QString locateFile(const QStringList &files)
{
   for(QStringList::ConstIterator it = files.begin();
       it != files.end();
       ++it)
   {
kdDebug(7021) << "Checking for file " << (*it) << " (#1)" << endl;
      if (KStandardDirs::exists(*it))
         return (*it);
   }
   return QString::null;
}

void 
VFolderMenu::pushDocInfo(const QString &fileName)
{
   m_docInfoStack.push(m_docInfo);
   QString baseName = fileName;
   if (!baseName.startsWith("/"))
      baseName = m_docInfo.baseDir + baseName;
   int i;
   
   i = baseName.findRev('/');
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

   m_docInfo.path = locateMenuFile(fileName);
   if (baseName.isEmpty())
   {
      m_docInfo.absBaseDir = QString::null;
      m_docInfo.baseName = QString::null;
      kdDebug(7021) << "Menu " << fileName << " not found." << endl;
      return;
   }
   i = baseName.findRev('/');
   m_docInfo.absBaseDir = baseName.left(i+1);
}

void
VFolderMenu::popDocInfo()
{
   m_docInfo = m_docInfoStack.pop();
}

QStringList
VFolderMenu::allConfLocations(const QString &fileName)
{
   QStringList locations;
   QString tmp;

   tmp = KStandardDirs::realPath(m_desktopUserDir+fileName);
   if (!locations.contains(tmp)) locations.append(tmp);

   for(QStringList::ConstIterator it = m_desktopSystemDirs.begin();
       it != m_desktopSystemDirs.end();
       ++it)
   {
      tmp = KStandardDirs::realPath((*it) + "etc/"+fileName);
      if (!locations.contains(tmp)) locations.append(tmp);
   }

   // TODO: The spec refers to "sysconfdir" here
   for(QStringList::ConstIterator it = m_defaultDataDirs.begin();
       it != m_defaultDataDirs.end();
       ++it)
   {
      tmp = KStandardDirs::realPath((*it) + "share/" + fileName);
      if (!locations.contains(tmp)) locations.append(tmp);
   }
   
   tmp = KStandardDirs::realPath("/etc/desktop/" + fileName);
   if (!locations.contains(tmp)) locations.append(tmp);

   return locations;
}

QStringList
VFolderMenu::allDataLocations(const QString &fileName)
{
   QStringList locations;
   QString tmp;

   tmp = KStandardDirs::realPath(m_desktopUserDir+fileName);
   if (!locations.contains(tmp)) locations.append(tmp);
   
   for(QStringList::ConstIterator it = m_desktopSystemDirs.begin();
       it != m_desktopSystemDirs.end();
       ++it)
   {
      tmp = KStandardDirs::realPath((*it) + "share/" + fileName);
      if (!locations.contains(tmp)) locations.append(tmp);
   }

   for(QStringList::ConstIterator it = m_defaultDataDirs.begin();
       it != m_defaultDataDirs.end();
       ++it)
   {
      tmp = KStandardDirs::realPath((*it) + "share/" + fileName);
      if (!locations.contains(tmp)) locations.append(tmp);
   }

   tmp = KStandardDirs::realPath("/usr/share/desktop/" + fileName);
   if (!locations.contains(tmp)) locations.append(tmp);

   return locations;
}

QString
VFolderMenu::locateMenuFile(const QString &fileName)
{
   if (fileName.startsWith("/"))
   {
kdDebug(7021) << "Checking for file " << fileName << " (#2)" << endl;
      if (KStandardDirs::exists(fileName))
         return fileName;
      return QString::null;
   }

   QString baseName = m_docInfo.baseDir + fileName;
   QStringList locations = allConfLocations("menus/"+baseName);
   // First location in the list wins
    
   QString result = locateFile(locations);

   if (result.isEmpty() && !m_docInfo.baseDir.startsWith("/"))
   {
      result = locate("apps", m_docInfo.baseDir+"default_"+fileName);
   }
   return result;
}

QString
VFolderMenu::locateDirectoryFile(const QString &fileName)
{
   if (fileName.isEmpty())
      return QString::null;
      
kdDebug(7021) << "locateDirectoryFile(" << fileName << ")" << endl;
   if (fileName.startsWith("/"))
   {
kdDebug(7021) << "Checking for file " << fileName << " (#3)" << endl;
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
kdDebug(7021) << "Checking for file " << tmp << " (#4)" << endl;
      if (KStandardDirs::exists(tmp))
         return tmp;
   }
   
   return QString::null;
}

void
VFolderMenu::initDirs()
{
   QString home = QDir::homeDirPath();
    
   m_desktopUserDir = QFile::decodeName(getenv("DESKTOP_USER_DIR"));
   if (m_desktopUserDir.isEmpty())
      m_desktopUserDir = home+"/.desktop/";
   if (!m_desktopUserDir.endsWith("/"))
      m_desktopUserDir += "/";

   QString tmp = QFile::decodeName(getenv("DESKTOP_DIRS"));
   m_desktopSystemDirs = QStringList::split(':', tmp);
   for(QStringList::Iterator it = m_desktopSystemDirs.begin();
       it != m_desktopSystemDirs.end();
       ++it)
   {
      if (!(*it).endsWith("/"))
         *it += "/";
   }

   m_defaultDataDirs = QStringList::split(':', KGlobal::dirs()->kfsstnd_prefixes());
   m_defaultDataDirs.remove(m_defaultDataDirs.first()); // Remove local dir
   
   m_defaultAppDirs = allDataLocations("applications/");
   m_defaultDirectoryDirs = allDataLocations("desktop-directories/");
   m_defaultLegacyDirs = KGlobal::dirs()->resourceDirs("apps");
}

void
VFolderMenu::loadMenu(const QString &fileName)
{
   m_defaultMergeDirs.clear();
    
   if (!fileName.endsWith(".menu"))
      return;

   pushDocInfo(fileName);
   m_defaultMergeDirs = allConfLocations("menus/"+m_docInfo.baseName+"-merged/");
   m_doc = loadDoc(m_docInfo.path);
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
VFolderMenu::processCondition(QDomElement &domElem, QDict<KService> *items)
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

      QDict<KService> andItems;
      while( !n.isNull() ) {
         QDomElement e = n.toElement(); 
         if (e.tagName() == "Not")
         {
kdDebug(7021) << "Special case 'and not'" << endl;
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

      QDict<KService> orItems;
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
kdDebug(7021) << "Adding not" << endl;
      for(KService::List::ConstIterator it = m_applications.begin();
            it != m_applications.end(); ++it)
      {
         KService *s = *it;
         items->replace(relPath(s), s);
      }
      QDict<KService> notItems;
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
kdDebug(7021) << "Adding Category " << domElem.text() << endl;
      KService::List *list = m_dictCategories->find(domElem.text());
      if (!list) return;
      
      for(KService::List::ConstIterator it = list->begin();
            it != list->end(); ++it)
      {
         KService *s = *it;
         items->replace(relPath(s), s);
      }
   }
   else if (domElem.tagName() == "All")
   {
kdDebug(7021) << "Adding all" << endl;
      for(KService::List::ConstIterator it = m_applications.begin();
            it != m_applications.end(); ++it)
      {
         KService *s = *it;
         items->replace(relPath(s), s);
      }
   }
   else if (domElem.tagName() == "Filename")
   {
      QString filename = domElem.text();
kdDebug(7021) << "Adding file " << filename << endl;
      // TODO: Argh, we need to search m_applications :(
      for(KService::List::ConstIterator it = m_applications.begin();
            it != m_applications.end(); ++it)
      {
         KService *s = *it;
         if (relPath(s) == filename)
         {
            items->replace(relPath(s), s);
         }
      }
   }
}

void
VFolderMenu::loadApplications(const QString &dir, const QString &relDir)
{
   kdDebug(7021) << "Looking up applications under " << dir << endl;
   
   // We look for a set of files.
   DIR *dp = opendir( QFile::encodeName(dir));
   if (!dp)
      return;

   struct dirent *ep;
   struct stat buff;

   QString _dot(".");
   QString _dotdot("..");

   while( ( ep = readdir( dp ) ) != 0L )
   {
      QString fn( QFile::decodeName(ep->d_name));
      if (fn == _dot || fn == _dotdot || fn.at(fn.length() - 1).latin1() == '~')
         continue;

      QString pathfn = dir + fn;
      if ( stat( QFile::encodeName(pathfn), &buff ) != 0 ) {
         continue; // Couldn't stat (e.g. no read permissions)
      }
      if ( S_ISDIR( buff.st_mode )) {
         loadApplications(pathfn + '/', relDir + fn + '/');
         continue;
      }
      
      if ( S_ISREG( buff.st_mode))
      {
         if (!fn.endsWith(".desktop"))
            continue;
            
         KService *service = 0;
kdDebug(7021) << "Found " << pathfn << " (" << (relDir+fn) << ")" << endl;
         emit newService(pathfn, &service); 
         if (service)
         {
            m_appRelPaths.insert(service, new QString(relDir+fn));
            m_applications.prepend(service);
         }
         else
         {
// This is not an error!
kdDebug(7021) << "Could not load " << pathfn << endl;
         }
         // list.append( pathfn );
         // relList.append( relPart + fn );
      }
    }
    closedir( dp );
}

void
VFolderMenu::processKDELegacyDir()
{
kdDebug(7021) << "processLegacyDir()" << endl;

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
      if (!m_forcedLegacyLoad && (dirs.search(*it) != -1))
      {
         QString name = *it;
kdDebug(7021) << "KDE Legacy: Found menu " << name << endl;
         if (name.endsWith("/.directory"))
            name = name.left(name.length()-11);
      
         SubMenu *newMenu = new SubMenu;
         newMenu->directoryFile = locate("apps", *it);
kdDebug(7021) << "KDE Legacy: Directory file = " << newMenu->directoryFile << endl;

         insertSubMenu(m_currentMenu, name, newMenu);
         continue;
      }
      
      if (files.search(*it) != -1)
      {
         QString name = *it;
         KService *service = 0;
kdDebug(7021) << "KDE Legacy Found " << name << endl;
         emit newService(name, &service);

         if (service && !m_forcedLegacyLoad)
         {
            m_appRelPaths.insert(service, new QString(name));
            if (service->categories().isEmpty())
            {
               insertService(m_currentMenu, name, service);
            }
            else
            {
               m_applications.prepend(service); // TODO: Should always happen according to spec, not only as part of the else.
            }
         }
      }
   }
   m_legacyLoaded = true;
}

void
VFolderMenu::processLegacyDir(const QString &dir, const QString &relDir)
{
kdDebug(7021) << "processLegacyDir(" << dir << ", " << relDir << ")" << endl;

   // We look for a set of files.
   DIR *dp = opendir( QFile::encodeName(dir));
   if (!dp)
      return;

   struct dirent *ep;
   struct stat buff;

   QString _dot(".");
   QString _dotdot("..");

   while( ( ep = readdir( dp ) ) != 0L )
   {
      QString fn( QFile::decodeName(ep->d_name));
      if (fn == _dot || fn == _dotdot || fn.at(fn.length() - 1).latin1() == '~')
         continue;

      QString pathfn = dir + fn;
      if ( stat( QFile::encodeName(pathfn), &buff ) != 0 ) {
         continue; // Couldn't stat (e.g. no read permissions)
      }
      if ( S_ISDIR( buff.st_mode )) {
         SubMenu *parentMenu = m_currentMenu;

kdDebug(7021) << "Legacy: adding menu " << fn << endl;
         m_currentMenu = new SubMenu;
         m_currentMenu->name = fn;
         m_currentMenu->directoryFile = dir + fn + "/.directory";
      
         parentMenu->subMenus.append(m_currentMenu);
         
         processLegacyDir(pathfn + '/', relDir+fn+'/');
         m_currentMenu = parentMenu;
         continue;
      }
      
      if ( S_ISREG( buff.st_mode))
      {
         if (!fn.endsWith(".desktop"))
            continue;
            
         KService *service = 0;
kdDebug(7021) << "Found " << pathfn << " (" << (relDir+fn) << ")" << endl;
         emit newService(pathfn, &service);
         if (service)
         {
            m_appRelPaths.insert(service, new QString(relDir+fn));
            if (service->categories().isEmpty())
               m_currentMenu->items.replace(relDir+fn, service);
            else
               m_applications.prepend(service); // TODO: Should always happen according to spec.
         }
         // list.append( pathfn );
         // relList.append( relPart + fn );
      }
    }
    closedir( dp );
}



void 
VFolderMenu::processMenu(QDomElement &docElem, int pass)
{
   SubMenu *parentMenu = m_currentMenu;
   unsigned int oldDirectoryDirsCount = m_directoryDirs.count();
   unsigned int oldApplicationsCount = m_applications.count();

   QString name;
   QString directoryFile;
   QStringList legacyDirs;
   bool onlyUnallocated = false;
   bool isDeleted = false;
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
      else if (e.tagName() == "AppDir")
      {
         if (pass == 1)
         {
            QString dir = absoluteDir(e.text());

            loadApplications(dir, QString::null);
         }
      }
      else if (e.tagName() == "KDELegacyDirs")
      {
         if (pass == 1)
         {
            legacyDirs.append("<KDE>");
         }
      }
      else if (e.tagName() == "LegacyDir")
      {
         if (pass == 1)
         {
            QString dir = absoluteDir(e.text());

            legacyDirs.append(dir);
         }
      }
      else if (e.tagName() == "DirectoryDir")
      {
         QString dir = absoluteDir(e.text());

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
      n = n.nextSibling();
   }

   if (pass == 1)
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
kdDebug(7021) << "VFolder: adding menu " << name << endl;
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

   bool kdeLegacyDirsDone = false;
   for(QStringList::ConstIterator it = legacyDirs.begin();
       it != legacyDirs.end(); ++it)
   {
      kdDebug(7021) << "Loading legacy dir " << (*it) << endl;
      if (*it == "<KDE>")
      {
        if (!kdeLegacyDirsDone)
        {
kdDebug(7021) << "Processing KDE Legacy dirs for <KDE>" << endl;
           processKDELegacyDir();
           kdeLegacyDirsDone = true;
        }
      }
      else
      {
        if (m_defaultLegacyDirs.contains(*it))
        {
           if (!kdeLegacyDirsDone)
           {
kdDebug(7021) << "Processing KDE Legacy dirs for " << (*it) << endl;
              processKDELegacyDir();
              kdeLegacyDirsDone = true;
           }
        }
        else
        {
           processLegacyDir(*it, QString::null);
        }
      }
   }

   if (((pass == 1) && !onlyUnallocated) || ((pass == 2) && onlyUnallocated))
   {
      if (!m_dictCategories)
      {
         m_dictCategories = new QDict<KService::List>(43);
         m_dictCategories->setAutoDelete(true);
         for(KService::List::ConstIterator it = m_applications.begin();
            it != m_applications.end(); ++it)
         {
            QStringList cats = (*it)->categories();
            for(QStringList::ConstIterator it2 = cats.begin();
                it2 != cats.end(); ++it2)
            {
               const QString &cat = *it2;
               KService::List *list = m_dictCategories->find(cat);
               if (!list)
               {
                  list = new KService::List();
                  m_dictCategories->insert(cat, list);
               }
               list->append(*it);
            }
         }
      }
   
      n = docElem.firstChild();

      while( !n.isNull() ) {
         QDomElement e = n.toElement(); // try to convert the node to an element.
         if (e.tagName() == "Include")
         {
            QDict<KService> items;
            
            QDomNode n2 = e.firstChild();
            while( !n2.isNull() ) {
               QDomElement e2 = n2.toElement(); 
               items.clear();
               processCondition(e2, &items);
               includeItems(&(m_currentMenu->items), &items);
               n2 = n2.nextSibling();
            }
         }
         if (e.tagName() == "Exclude")
         {
            QDict<KService> items;
            
            QDomNode n2 = e.firstChild();
            while( !n2.isNull() ) {
               QDomElement e2 = n2.toElement(); 
               items.clear();
               processCondition(e2, &items);
               excludeItems(&(m_currentMenu->items), &items);
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
                insertSubMenu(m_currentMenu, dest, menu);
              } 
            }
         }
         n = n.nextSibling();
      }
   
   }

   while (m_directoryDirs.count() > oldDirectoryDirsCount)
      m_directoryDirs.pop_front();

   if (m_applications.count() > oldApplicationsCount)
   {
      delete m_dictCategories;
      m_dictCategories = 0;
      while (m_applications.count() > oldApplicationsCount)
         m_applications.pop_front();
   }
   
   m_currentMenu = parentMenu;
}

VFolderMenu::SubMenu *
VFolderMenu::parseMenu(const QString &file, bool forceLegacyLoad)
{
   g_this = this;

   m_forcedLegacyLoad = false;
   m_legacyLoaded = false;

   loadMenu(file);

   delete m_rootMenu;
   m_rootMenu = m_currentMenu = 0;

   QDomElement docElem = m_doc.documentElement();

   for (int pass = 1; pass <= 2; pass++)
   {  
     processMenu(docElem, pass);
   }
   
   if (!m_legacyLoaded && forceLegacyLoad)
   {
     m_forcedLegacyLoad = true;
     processKDELegacyDir();
   }
   
   return m_rootMenu;   
}

#include "vfolder_menu.moc"
