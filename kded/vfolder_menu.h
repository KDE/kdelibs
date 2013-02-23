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

#ifndef KDED_VFOLDER_MENU_H
#define KDED_VFOLDER_MENU_H

#include <QtCore/QObject>
#include <QtXml/QDomDocument>
#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QStack>

#include <kservice.h>

class KBuildSycocaInterface;
class KBuildServiceFactory;

class VFolderMenu : public QObject
{
  Q_OBJECT
public:
  class AppsInfo;
  class SubMenu {
  public:
     SubMenu() : isDeleted(false),apps_info(0) { items.reserve(43); }
     ~SubMenu() { qDeleteAll(subMenus); }

  public:
     QString name;
     QString directoryFile;
     QList<SubMenu*> subMenus;
     QHash<QString,KService::Ptr> items;
     QHash<QString,KService::Ptr> excludeItems; // Needed when merging due to Move.
     QDomElement defaultLayoutNode;
     QDomElement layoutNode;
     bool isDeleted;
     QStringList layoutList;
     AppsInfo *apps_info;
  };

  VFolderMenu(KBuildServiceFactory* serviceFactory, KBuildSycocaInterface* kbuildsycocaInterface);
  ~VFolderMenu();

  /**
   * Parses VFolder menu definition and generates a menu layout.
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

public:
  struct MenuItem
  {
    enum Type { MI_Service, MI_SubMenu, MI_Separator };
    Type type;
    KService::Ptr service;
    SubMenu  *submenu;
  };

public:
  QStringList m_allDirectories; // A list of all the directories that we touch

  QStringList m_defaultAppDirs;
  QStringList m_defaultDirectoryDirs;
  QStringList m_defaultMergeDirs;
  QStringList m_defaultLegacyDirs;

  QStringList m_directoryDirs; // Current set of applicable <DirectoryDir> dirs
  QHash<QString, SubMenu*> m_legacyNodes; // Dictionary that stores Menu nodes
                                          // associated with legacy tree.

  class DocInfo {
  public:
     QString baseDir; // Relative base dir of current menu file
     QString baseName; // Filename of current menu file without ".menu"
     QString path; // Full path of current menu file including ".menu"
  };


  DocInfo m_docInfo; // DocInfo for current doc
  QStack<VFolderMenu::DocInfo> m_docInfoStack;

  class AppsInfo {
  public:
     AppsInfo()
     {
        dictCategories.reserve(53);
        applications.reserve(997);
        appRelPaths.reserve(997);
     }

     ~AppsInfo()
     {
     }

     QHash<QString,KService::List> dictCategories; // category -> apps
     QHash<QString,KService::Ptr> applications; // rel path -> service
     QHash<KService::Ptr,QString> appRelPaths; // service -> rel path
  };

  AppsInfo *m_appsInfo; // AppsInfo for current menu
  QList<AppsInfo*> m_appsInfoStack; // All applicable AppsInfo for current menu
  QList<AppsInfo*> m_appsInfoList; // List of all AppsInfo objects.
    QSet<QString /*menuId*/> m_usedAppsDict; // all applications that have been allocated

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
  KService::Ptr findApplication(const QString &relPath);

  /**
   * Lookup applications by category
   */
  QList<KService::List*> findCategory(const QString &category);

  /**
   * Add new application
   */
  void addApplication(const QString &id, KService::Ptr service);

  /**
   * Build application indices
   */
  void buildApplicationIndex(bool unusedOnly);

  /**
   * Create a AppsInfo frame for current menu
   */
  void createAppsInfo();

  /**
   * Load additional AppsInfo frame for current menu
   */
  void loadAppsInfo();

  /**
   * Unload additional AppsInfo frame for current menu
   */
  void unloadAppsInfo();

  QDomDocument loadDoc();
  void mergeMenus(QDomElement &docElem, QString &name);
  void mergeFile(QDomElement &docElem, const QDomNode &mergeHere);
  void loadMenu(const QString &filename);

  /**
   * Merge the items2 set into the items1 set
   */
  void includeItems(QHash<QString,KService::Ptr>& items1, const QHash<QString,KService::Ptr>& items2);

  /**
   * Remove all items from the items1 set that aren't also in the items2 set
   */
  void matchItems(QHash<QString,KService::Ptr>& items1, const QHash<QString,KService::Ptr>& items2);

  /**
   * Remove all items in the items2 set from the items1 set
   */
  void excludeItems(QHash<QString,KService::Ptr>& items1, const QHash<QString,KService::Ptr>& items2);

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
   * Merge menu2 and its submenus into menu1 and its submenus
   * If reversePriority is false, menu2 has priority over menu1
   * If reversePriority is true, menu1 has priority over menu2
   */
  void mergeMenu(SubMenu *menu1, SubMenu *menu2, bool reversePriority=false);

  /**
   * Inserts service into the menu using name relative to parentMenu
   * Any missing sub-menus are created.
   */
  void insertService(SubMenu *parentMenu, const QString &name, KService::Ptr newService);

  /**
   * Register the directory that @p file is in.
   * @see allDirectories()
   */
  void registerFile(const QString &file);

  /**
   * Fill m_usedAppsDict with all applications from @p items
   */
  void markUsedApplications(const QHash<QString,KService::Ptr>& items);

  /**
   * Register @p directory
   * @see allDirectories()
   */
  void registerDirectory(const QString &directory);

  void processKDELegacyDirs();
  void processLegacyDir(const QString &dir, const QString &relDir, const QString &prefix);
  void processMenu(QDomElement &docElem, int pass);
  void layoutMenu(VFolderMenu::SubMenu *menu, QStringList defaultLayout);
  void processCondition(QDomElement &docElem, QHash<QString,KService::Ptr>& items);

  void initDirs();

  void pushDocInfo(const QString &fileName, const QString &baseDir = QString());
  void pushDocInfoParent(const QString &basePath, const QString &baseDir);
  void popDocInfo();

  QString absoluteDir(const QString &_dir, const QString &baseDir, bool keepRelativeToCfg=false);
  QString locateMenuFile(const QString &fileName);
  QString locateDirectoryFile(const QString &fileName);
  void loadApplications(const QString&, const QString&);

private:
    KBuildServiceFactory* m_serviceFactory;
    KBuildSycocaInterface* m_kbuildsycocaInterface;
};

#endif
