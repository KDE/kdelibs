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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _VFOLDER_MENU_H_
#define _VFOLDER_MENU_H_

#include <qobject.h>
#include <qdom.h>
#include <qstringlist.h>
#include <qptrdict.h>
#include <qvaluestack.h>

#include <kservice.h>

class VFolderMenu : public QObject
{
  Q_OBJECT
public:
  class SubMenu;

public:
  VFolderMenu();
  ~VFolderMenu();

  /**
   * Parses VFolder menu defintion and generates a menu layout.
   * The @ref newService signals is used as callback to load
   * a specific service description.
   *
   * @param file Menu file to load
   * @param forceLegacyLoad flag indicating whether the KDE "applnk"
   * directory should be processed at least once.
   */
  SubMenu *parseMenu(const QString &file, bool forceLegacyLoad=false);

signals:
  void newService(const QString &path, KService **entry);

public:
  class SubMenu {
  public:
     ~SubMenu() { subMenus.setAutoDelete(true); }
  
  public:
     QString name;
     QString directoryFile;
     QPtrList<SubMenu> subMenus;
     QDict<KService> items;
  };

public:  
  QStringList m_desktopSystemDirs;

  QStringList m_defaultDataDirs;
  QStringList m_defaultAppDirs;
  QStringList m_defaultDirectoryDirs;
  QStringList m_defaultMergeDirs;
  QStringList m_defaultLegacyDirs;

  QStringList m_directoryDirs; // Current set of applicable <DirectoryDir> dirs
  QDict<KService> m_applications; // All applications
  
  QPtrDict<QString> m_appRelPaths; // Dictionary with relative paths
  QDict<SubMenu> m_legacyNodes; // Dictionary that stores Menu nodes 
                                // associated with legacy tree.

  class docInfo {
  public:
     QString baseDir; // Relative base dir of current menu file
     QString absBaseDir; // Absolute base dir of current menu file
     QString baseName; // Filename of current menu file without ".menu"
     QString path; // Filename of current menu file without ".menu"
  };

  docInfo m_docInfo;
  QValueStack<VFolderMenu::docInfo> m_docInfoStack;
  
  QString m_desktopUserDir;
  QDict<KService> *m_serviceDict;
  QDict<KService::List> *m_dictCategories;
  QDomDocument m_doc;
  SubMenu *m_rootMenu;
  SubMenu *m_currentMenu;
  bool m_forcedLegacyLoad;
  bool m_legacyLoaded;

private:
  void mergeMenus(QDomElement &docElem, QString &name);
  void mergeFile(QDomElement &docElem, const QDomNode &mergeHere);
  void loadMenu(const QString &filename);

  void processKDELegacyDirs();
  void processLegacyDir(const QString &dir, const QString &relDir);
  void processMenu(QDomElement &docElem, int pass);
  void processCondition(QDomElement &docElem, QDict<KService> *items);

  void initDirs();
  QStringList allDataLocations(const QString &fileName);
  QStringList allConfLocations(const QString &fileName);
  
  void pushDocInfo(const QString &fileName);
  void popDocInfo();
  
  QString absoluteDir(const QString &);
  QString locateMenuFile(const QString &fileName); 
  QString locateDirectoryFile(const QString &fileName);
  void loadApplications(const QString&, const QString&);
};

#endif
