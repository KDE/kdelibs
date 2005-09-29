/* 
   This file is part of the KDE libraries
   Copyright (c) 2003 Waldo Bastian <bastian@kde.org>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _VFOLDER_MENU_H_
#define _VFOLDER_MENU_H_

#include <qobject.h>
#include <qdom.h>
#include <qstringlist.h>
#include <qptrdict.h>
#include <qptrlist.h>
#include <qvaluestack.h>

#include <kservice.h>

class VFolderMenu : public QObject
{
  Q_OBJECT
public:
  class appsInfo;
  class SubMenu {
  public:
     SubMenu() : items(43),isDeleted(false),apps_info(0) { }
     ~SubMenu() { subMenus.setAutoDelete(true); }
  
  public:
     QString name;
     QString directoryFile;
     QPtrList<SubMenu> subMenus;
     QDict<KService> items;
     QDict<KService> excludeItems; // Needed when merging due to Move.
     QDomElement defaultLayoutNode;
     QDomElement layoutNode;
     bool isDeleted;
     QStringList layoutList;
     appsInfo *apps_info;
  };

  VFolderMenu();
  ~VFolderMenu();

  /**
   * Parses VFolder menu defintion and generates a menu layout.
   * The newService signals is used as callback to load
   * a specific service description.
   *
   * @param file Menu file to load
   * @param forceLegacyLoad flag indicating whether the KDE "applnk"
   * directory should be processed at least once.
   */
  SubMenu *parseMenu(const QString &file, bool forceLegacyLoad=false);
  
  /**
   * Returns a list of all directories involved in the last call to 
   * parseMenu(), excluding the KDE Legacy directories.
   *
   * A change in any of these directories or in any of their child-
   * directories can result in changes to the menu.
   */
  QStringList allDirectories();

  /**
   * Debug function to enable tracking of what happens with a specific
   * menu item id
   */
  void setTrackId(const QString &id);

signals:
  void newService(const QString &path, KService **entry);

public:
  struct MenuItem 
  {
    enum Type { MI_Service, MI_SubMenu, MI_Separator };
    Type type;
    union { 
       KService *service;
       SubMenu  *submenu;
    } data;
  };

public:  
  QStringList m_allDirectories; // A list of all the directories that we touch

  QStringList m_defaultDataDirs;
  QStringList m_defaultAppDirs;
  QStringList m_defaultDirectoryDirs;
  QStringList m_defaultMergeDirs;
  QStringList m_defaultLegacyDirs;

  QStringList m_directoryDirs; // Current set of applicable <DirectoryDir> dirs
  QDict<SubMenu> m_legacyNodes; // Dictionary that stores Menu nodes 
                                // associated with legacy tree.

  class docInfo {
  public:
     QString baseDir; // Relative base dir of current menu file
     QString baseName; // Filename of current menu file without ".menu"
     QString path; // Full path of current menu file including ".menu"
  };
  

  docInfo m_docInfo; // docInfo for current doc
  QValueStack<VFolderMenu::docInfo> m_docInfoStack;

  class appsInfo {
  public:
     appsInfo() : dictCategories(53), applications(997), appRelPaths(997)
     {
        dictCategories.setAutoDelete(true);
     }

     QDict<KService::List> dictCategories; // category -> apps
     QDict<KService> applications; // rel path -> service
     QPtrDict<QString> appRelPaths; // service -> rel path
  };
  
  appsInfo *m_appsInfo; // appsInfo for current menu
  QPtrList<appsInfo> m_appsInfoStack; // All applicable appsInfo for current menu
  QPtrList<appsInfo> m_appsInfoList; // List of all appsInfo objects.
  QDict<KService> m_usedAppsDict; // all applications that have been allocated
  
  QDomDocument m_doc;
  SubMenu *m_rootMenu;
  SubMenu *m_currentMenu;
  bool m_forcedLegacyLoad;
  bool m_legacyLoaded;
  bool m_track;
  QString m_trackId;

private:
  /**
   * Lookup application by relative path
   */
  KService *findApplication(const QString &relPath);

  /**
   * Lookup applications by category
   */
  QPtrList<KService::List> findCategory(const QString &category);
  
  /**
   * Add new application
   */
  void addApplication(const QString &id, KService *service);
  
  /**
   * Build application indices
   */
  void buildApplicationIndex(bool unusedOnly);
  
  /**
   * Create a appsInfo frame for current menu
   */
  void createAppsInfo();

  /**
   * Load additional appsInfo frame for current menu
   */
  void loadAppsInfo();

  /**
   * Unload additional appsInfo frame for current menu
   */
  void unloadAppsInfo();

  QDomDocument loadDoc();
  void mergeMenus(QDomElement &docElem, QString &name);
  void mergeFile(QDomElement &docElem, const QDomNode &mergeHere);
  void loadMenu(const QString &filename);

  /**
   * Merge the items2 set into the items1 set
   */
  void includeItems(QDict<KService> *items1, QDict<KService> *items2);

  /**
   * Remove all items from the items1 set that aren't also in the items2 set
   */
  void matchItems(QDict<KService> *items1, QDict<KService> *items2);

  /**
   * Remove all items in the items2 set from the items1 set
   */
  void excludeItems(QDict<KService> *items1, QDict<KService> *items2);

  /**
   * Search the parentMenu tree for the menu menuName and takes it
   * out.
   *
   * This function returns a pointer to the menu if it was found 
   * or 0 if it was not found.
   */
  SubMenu* takeSubMenu(SubMenu *parentMenu, const QString &menuName);

  /**
   * Insert the menu newMenu with name menuName into the parentMenu.
   * If such menu already exist the result is merged, if any additional
   * submenus are required they are created.
   * If reversePriority is false, newMenu has priority over the existing 
   * menu during merging.
   * If reversePriority is true, the existing menu has priority over newMenu
   * during merging.
   */
  void insertSubMenu(VFolderMenu::SubMenu *parentMenu, const QString &menuName, VFolderMenu::SubMenu *newMenu, bool reversePriority=false);

  /**
   * Merge menu2 and it's submenus into menu1 and it's submenus
   * If reversePriority is false, menu2 has priority over menu1
   * If reversePriority is true, menu1 has priority over menu2
   */
  void mergeMenu(SubMenu *menu1, SubMenu *menu2, bool reversePriority=false);

  /**
   * Inserts service into the menu using name relative to parentMenu
   * Any missing sub-menus are created.
   */
  void insertService(SubMenu *parentMenu, const QString &name, KService *newService);

  /**
   * Register the directory that @p file is in.
   * @see allDirectories()
   */
  void registerFile(const QString &file);

  /**
   * Fill m_usedAppsDict with all applications from @p items
   */
  void markUsedApplications(QDict<KService> *items);

  /**
   * Register @p directory
   * @see allDirectories()
   */
  void registerDirectory(const QString &directory);

  void processKDELegacyDirs();
  void processLegacyDir(const QString &dir, const QString &relDir, const QString &prefix);
  void processMenu(QDomElement &docElem, int pass);
  void layoutMenu(VFolderMenu::SubMenu *menu, QStringList defaultLayout);
  void processCondition(QDomElement &docElem, QDict<KService> *items);

  void initDirs();
  
  void pushDocInfo(const QString &fileName, const QString &baseDir = QString::null);
  void pushDocInfoParent(const QString &basePath, const QString &baseDir);
  void popDocInfo();
  
  QString absoluteDir(const QString &_dir, const QString &baseDir, bool keepRelativeToCfg=false);
  QString locateMenuFile(const QString &fileName); 
  QString locateDirectoryFile(const QString &fileName);
  void loadApplications(const QString&, const QString&);
};

#endif
