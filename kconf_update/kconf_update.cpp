/*
 *
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
 *
 * $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
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
#include <stdlib.h>

#include <qfile.h>
#include <qtextstream.h>

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>
#include <kinstance.h>
#include <ktempfile.h>

static KCmdLineOptions options[] =
{
	{ "check <update-file>", I18N_NOOP("Check whether config file itself requires updating"), 0 },
	{ "+[file]", I18N_NOOP("File to read update instructions from"), 0 },
        { 0, 0, 0 }
};

class KonfUpdate
{
public:
   KonfUpdate();
   ~KonfUpdate();
   QStringList findUpdateFiles(bool dirtyOnly);

   bool checkFile(const QString &filename);
   void checkGotFile(const QString &_file, const QString &id);

   bool updateFile(const QString &filename);

   void gotId(const QString &_id);
   void gotFile(const QString &_file);
   void gotGroup(const QString &_group);
   void gotRemoveGroup(const QString &_group);
   void gotKey(const QString &_key);
   void gotRemoveKey(const QString &_key);
   void gotAllKeys();
   void gotAllGroups();
   void gotOptions(const QString &_options);
   void gotScript(const QString &_script);
   void resetOptions();

   void copyGroup(KConfigBase *cfg1, const QString &grp1, 
                  KConfigBase *cfg2, const QString &grp2);

protected:
   KConfig *config;
   QString currentFilename;
   bool skip;
   QString id;

   QString oldFile;
   QString newFile;
   KConfig *oldConfig1; // Config to read keys from.
   KSimpleConfig *oldConfig2; // Config to delete keys from.
   KSimpleConfig *newConfig;

   QString oldGroup;
   QString newGroup;
   QString oldKey;
   QString newKey;

   bool m_bCopy;
   bool m_bOverwrite;
   bool m_bUseConfigInfo;
};

KonfUpdate::KonfUpdate()
{
   oldConfig1 = 0;
   oldConfig2 = 0;
   newConfig = 0;

   config = new KConfig("kconf_updaterc");

   QStringList updateFiles;
   KCmdLineArgs *args=KCmdLineArgs::parsedArgs();
   if (!config->readBoolEntry("updateInfoAdded", false))
   {
       config->writeEntry("updateInfoAdded", true);
       updateFiles = findUpdateFiles(false);

       for(QStringList::ConstIterator it = updateFiles.begin();
           it != updateFiles.end();
           ++it)
       {
           QString file = *it;
           checkFile(file);
       }
       updateFiles.clear();
   }

   m_bUseConfigInfo = false;
   if (args->isSet("check"))
   {
      m_bUseConfigInfo = true;
      QString file = locate("data", "kconf_update/"+QFile::decodeName(args->getOption("check")));
      if (file.isEmpty())
      {
         qWarning("File '%s' not found.", args->getOption("check").data());
         return;
      }
      updateFiles.append(file);
   }
   
   else if (args->count())
   {
      for(int i = 0; i < args->count(); i++)
      {
         KURL url = args->url(i);
         if (!url.isLocalFile())
            KCmdLineArgs::usage(i18n("Only local files are supported."));
         updateFiles.append(url.path());
      }
   }
   else
   {
      updateFiles = findUpdateFiles(true);
   }

   for(QStringList::ConstIterator it = updateFiles.begin();
       it != updateFiles.end();
       ++it)
   {
      QString file = *it;
      updateFile(file);
   }
}

KonfUpdate::~KonfUpdate()
{
   delete config;
}

QStringList KonfUpdate::findUpdateFiles(bool dirtyOnly)
{
   QStringList result;
   QStringList list = KGlobal::dirs()->findAllResources("data", "kconf_update/*.upd", false, true);
   for(QStringList::ConstIterator it = list.begin();
       it != list.end();
       ++it)
   {
      QString file = *it;
      struct stat buff;
      if (stat( QFile::encodeName(file), &buff) == 0)
      {
         int i = file.findRev('/');
         if (i != -1) 
            file = file.mid(i+1);
         config->setGroup(file);
         time_t ctime = config->readUnsignedLongNumEntry("ctime");
         time_t mtime = config->readUnsignedLongNumEntry("mtime");
         if (!dirtyOnly ||
             (ctime != buff.st_ctime) || (mtime != buff.st_mtime))
         {
            result.append(*it);
         }
      }
   }
   return result;
}

bool KonfUpdate::checkFile(const QString &filename)
{
   currentFilename = filename;
   int i = currentFilename.findRev('/');
   if (i != -1) 
      currentFilename = currentFilename.mid(i+1);
   skip = true;
   QFile file(filename);
   if (!file.open(IO_ReadOnly))
      return false;

   QTextStream ts(&file);
   ts.setEncoding(QTextStream::Latin1);
   int lineCount = 0;
   resetOptions();
   QString id;
   while(!ts.atEnd())
   {
      QString line = ts.readLine().stripWhiteSpace();
      lineCount++;
      if (line.isEmpty() || (line[0] == '#'))
         continue;
      if (line.startsWith("Id="))
         id = currentFilename+":"+line.mid(3);
      else if (line.startsWith("File="))
         checkGotFile(line.mid(5), id);
   }
  
   return true;
}

void KonfUpdate::checkGotFile(const QString &_file, const QString &id)
{
   QString file;
   int i = _file.find(',');
   if (i == -1)
   {
      file = _file.stripWhiteSpace();
   }
   else
   {
      file = _file.mid(i+1).stripWhiteSpace();
   }

   qDebug("File %s, id %s", file.latin1(), id.latin1());

   KSimpleConfig cfg(file);
   cfg.setGroup("$Version");
   QStringList ids = cfg.readListEntry("update_info");
   if (ids.contains(id))
       return;
   ids.append(id);
   cfg.writeEntry("update_info", ids);
}

/**
 * Syntax:
 * # Comment
 * Id=id
 * File=oldfile[,newfile]
 * AllGroups
 * Group=oldgroup[,newgroup]
 * RemoveGroup=oldgroup
 * Options=[copy,][overwrite,]
 * Key=oldkey[,newkey]
 * RemoveKey=ldkey
 * AllKeys
 * Keys= [Options](AllKeys|(Key|RemoveKey)*)
 * Script=scriptfile[,interpreter]
 *
 * Sequence:
 * (Id,(File(Group,Keys)*)*)*
 **/
bool KonfUpdate::updateFile(const QString &filename)
{
   currentFilename = filename;
   int i = currentFilename.findRev('/');
   if (i != -1) 
       currentFilename = currentFilename.mid(i+1);
   skip = true;
   QFile file(filename);
   if (!file.open(IO_ReadOnly))
      return false;

   QTextStream ts(&file);
   ts.setEncoding(QTextStream::Latin1);
   int lineCount = 0;
   resetOptions();
   while(!ts.atEnd())
   {
      QString line = ts.readLine().stripWhiteSpace();
      lineCount++;
      if (line.isEmpty() || (line[0] == '#'))
         continue;
      if (line.startsWith("Id="))
         gotId(line.mid(3));
      else if (skip)
         continue;
      else if (line.startsWith("Options="))
         gotOptions(line.mid(8));
      else if (line.startsWith("File="))
         gotFile(line.mid(5));
      else if (line.startsWith("Group="))
         gotGroup(line.mid(6));
      else if (line.startsWith("RemoveGroup="))
      {
         gotRemoveGroup(line.mid(12));
         resetOptions();
      }
      else if (line.startsWith("Script="))
      {
         gotScript(line.mid(7));
         resetOptions();
      }
      else if (line.startsWith("Key="))
      {
         gotKey(line.mid(4));
         resetOptions();
      }
      else if (line.startsWith("RemoveKey="))
      {
         gotRemoveKey(line.mid(10));
         resetOptions();
      }
      else if (line == "AllKeys")
      {
         gotAllKeys();
         resetOptions();
      }
      else if (line == "AllGroups")
      {
         gotAllGroups();
         resetOptions();
      }
      else
         qWarning("%s:%d parse error '%s'", filename.latin1(), lineCount, line.latin1());
   }
   // Flush.
   gotId(QString::null);
  
   struct stat buff;
   stat( QFile::encodeName(filename), &buff);
   config->setGroup(currentFilename);
   config->writeEntry("ctime", buff.st_ctime);
   config->writeEntry("mtime", buff.st_mtime);
   config->sync();
   return true;
}



void KonfUpdate::gotId(const QString &_id)
{
   // Flush pending changes
   gotFile(QString::null);

   config->setGroup(currentFilename);
   QStringList ids = config->readListEntry("done");
   if (!_id.isEmpty())
   {
       if (!ids.contains(_id))
       {
          ids.append(_id);
          config->writeEntry("done", ids);
          config->sync();
       }
       else
       {
          qDebug("Id '%s' was already in done-list!", _id.latin1());
          if (!m_bUseConfigInfo)
          {
             skip = true;
             return;
          }
       }
       skip = false;
       id = _id;
   }
}

void KonfUpdate::gotFile(const QString &_file)
{
   // Reset group
   gotGroup(QString::null);
 
   if (!oldFile.isEmpty())
   {
      // Close old file.
      delete oldConfig1;
      oldConfig1 = 0;
      oldConfig2->sync();
      delete oldConfig2;
      oldConfig2 = 0;
      
      QString file = locateLocal("config", oldFile);
      struct stat s_buf;
      if (stat(QFile::encodeName(file), &s_buf) == 0)
      {
         if (s_buf.st_size == 0)
         {
            // Delete empty file.
            unlink(QFile::encodeName(file));
         }   
      }

      oldFile = QString::null;
   }
   if (!newFile.isEmpty())
   {
      // Close new file.
      newConfig->sync();
      delete newConfig;
      newConfig = 0;

      newFile = QString::null;
   }
   newConfig = 0; 

   int i = _file.find(',');
   if (i == -1)
   {
      oldFile = _file.stripWhiteSpace();
   }
   else
   {
      oldFile = _file.left(i).stripWhiteSpace();
      newFile = _file.mid(i+1).stripWhiteSpace();
      if (oldFile == newFile)
         newFile = QString::null;
   }
   
   if (!oldFile.isEmpty())
   {
      oldConfig1 = new KConfig(oldFile, true, false);
      oldConfig2 = new KSimpleConfig(oldFile);
      if (!newFile.isEmpty())
      {
         newConfig = new KSimpleConfig(newFile);
      }
      else
      {
         newConfig = oldConfig2;
      }
      newConfig->setGroup("$Version");
      QStringList ids = newConfig->readListEntry("update_info");
      QString cfg_id = currentFilename + ":" + id;
      if (!ids.contains(cfg_id))
      {
         ids.append(cfg_id);
         newConfig->writeEntry("update_info", ids);
      }
      else
      {
         if (m_bUseConfigInfo)
         {
            skip = true;
         }
      }
   }
   else
   {
      newFile = QString::null;
   }
}

void KonfUpdate::gotGroup(const QString &_group)
{
   int i = _group.find(',');
   if (i == -1)
   {
      oldGroup = _group.stripWhiteSpace();
      newGroup = oldGroup;
   }
   else
   {
      oldGroup = _group.left(i).stripWhiteSpace();
      newGroup = _group.mid(i+1).stripWhiteSpace();
   }
}

void KonfUpdate::gotRemoveGroup(const QString &_group)
{
   oldGroup = _group.stripWhiteSpace();

   if (!oldConfig1)
   {
      qWarning("RemoveGroup without file specification.");
      return;
   }

   if (!oldConfig1->hasGroup(oldGroup))
      return;
   // Delete group.
   oldConfig2->deleteGroup(oldGroup, true);
   qDebug("Removing group %s (FORCED)", oldGroup.isEmpty() ? "<empty>" : oldGroup.latin1());
}


void KonfUpdate::gotKey(const QString &_key)
{
   int i = _key.find(',');
   if (i == -1)
   {
      oldKey = _key.stripWhiteSpace();
      newKey = oldKey;
   }
   else
   {
      oldKey = _key.left(i).stripWhiteSpace();
      newKey = _key.mid(i+1).stripWhiteSpace();
   }

   if (oldKey.isEmpty() || newKey.isEmpty())
   {
      qWarning("Invalid key.");
      return;
   }
   if (!oldConfig1)
   {
      qWarning("Key without file specification.");
      return;
   }
   oldConfig1->setGroup(oldGroup);
   if (!oldConfig1->hasKey(oldKey))
      return;
   QString value = oldConfig1->readEntry(oldKey);
   newConfig->setGroup(newGroup);
   if (!m_bOverwrite && newConfig->hasKey(newKey))
   {
      qWarning("Skipping %s", newKey.latin1());
      return;
   }
   qDebug("Write %s -> %s", newKey.latin1(), value.isEmpty() ? "<empty>" : value.latin1());
   newConfig->writeEntry(newKey, value);

   if (m_bCopy)
      return; // Done.

   // Delete old entry
   if ((oldConfig2 == newConfig) && 
       (oldGroup == newGroup) &&
       (oldKey == newKey))
      return; // Don't delete!
   oldConfig2->setGroup(oldGroup);
   oldConfig2->deleteEntry(oldKey, false);
   if (oldConfig2->deleteGroup(oldGroup, false)) { // Delete group if empty.
      qDebug("Removing group %s", oldGroup.isEmpty() ? "<empty>" : oldGroup.latin1());
   }
}

void KonfUpdate::gotRemoveKey(const QString &_key)
{
   oldKey = _key.stripWhiteSpace();

   if (oldKey.isEmpty())
   {
      qWarning("Invalid key.");
      return;
   }

   if (!oldConfig1)
   {
      qWarning("RemoveKey without file specification.");
      return;
   }

   oldConfig1->setGroup(oldGroup);
   if (!oldConfig1->hasKey(oldKey))
      return;
   qDebug("Remove Key '%s'/'%s'", oldGroup.isEmpty() ? "empty" : oldGroup.latin1(), oldKey.latin1());

   // Delete old entry
   oldConfig2->setGroup(oldGroup);
   oldConfig2->deleteEntry(oldKey, false);
   if (oldConfig2->deleteGroup(oldGroup, false)) { // Delete group if empty.
      qDebug("Removing group %s", oldGroup.isEmpty() ? "<empty>" : oldGroup.latin1());
   }
}

void KonfUpdate::gotAllKeys()
{
   if (!oldConfig1)
   {
      qWarning("AllKeys without file specification.");
      return;
   }

   QMap<QString, QString> list = oldConfig1->entryMap(oldGroup);
   for(QMap<QString, QString>::Iterator it = list.begin();
       it != list.end(); ++it)
   {
      gotKey(it.key());
   }
}

void KonfUpdate::gotAllGroups()
{
   if (!oldConfig1)
   {
      qWarning("AllGroups without file specification.");
      return;
   }

   QStringList allGroups = oldConfig1->groupList();
   for(QStringList::ConstIterator it = allGroups.begin();
       it != allGroups.end(); ++it)
   {
     oldGroup = *it;
     newGroup = oldGroup;
     gotAllKeys();
   }
}

void KonfUpdate::gotOptions(const QString &_options)
{
   QStringList options = QStringList::split(',', _options);
   for(QStringList::ConstIterator it = options.begin();
       it != options.end();
       ++it)
   {
       if ( (*it).lower().stripWhiteSpace() == "copy")
          m_bCopy = true;

       if ( (*it).lower().stripWhiteSpace() == "overwrite")
          m_bOverwrite = true;
   }
}

void KonfUpdate::copyGroup(KConfigBase *cfg1, const QString &grp1, 
                           KConfigBase *cfg2, const QString &grp2)
{
   cfg2->setGroup(grp2);
   QMap<QString, QString> list = cfg1->entryMap(grp1);
   for(QMap<QString, QString>::Iterator it = list.begin();
       it != list.end(); ++it)
   {
      cfg2->writeEntry(it.key(), it.data());
   }
}

void KonfUpdate::gotScript(const QString &_script)
{
   QString script, interpreter;
   int i = _script.find(',');
   if (i == -1)
   {
      script = _script.stripWhiteSpace();
   }
   else
   {
      script = _script.left(i).stripWhiteSpace();
      interpreter = _script.mid(i+1).stripWhiteSpace();
   }

   if (!oldConfig1)
   {
      qWarning("Script without file specification.");
      return;
   }

   if (script.isEmpty())
   {
      qWarning("No script specified.");
      return;
   } 
   qDebug("Running script '%s'", script.latin1());

   QString path = locate("data","kconf_update/"+script);
   if (path.isEmpty())
   {
      qWarning("Script '%s' not found.", script.latin1());
      return;
   }

   KTempFile tmp1;
   tmp1.setAutoDelete(true);
   KTempFile tmp2;
   tmp2.setAutoDelete(true);
   KSimpleConfig cfg(tmp1.name());

   if (oldGroup.isEmpty())
   {
       // Write all entries to tmpFile;
       QStringList grpList = oldConfig1->groupList();
       for(QStringList::ConstIterator it = grpList.begin();
           it != grpList.end();
           ++it)
       {
          copyGroup(oldConfig1, *it, &cfg, *it);
       }
   }
   else 
   {
       copyGroup(oldConfig1, oldGroup, &cfg, QString::null);
   }
   cfg.sync();
   qDebug("Script: Writing entries to %s", tmp1.name().latin1());

   QString cmd;
   if (interpreter.isEmpty())
      cmd = path;
   else
      cmd = interpreter + " " + path;

   int result = system(QFile::encodeName(QString("%1 < %2 > %3").arg(cmd).arg(tmp1.name()).arg(tmp2.name())));
   if (result)
   {
      qWarning("Script: Error running '%s'", cmd.latin1());
      return;
   }

   qDebug("Script: Filtered entries written to %s", tmp2.name().latin1());

   // Deleting old entries
   {
     QString group = oldGroup;
     QFile output(tmp2.name());
     if (output.open(IO_ReadOnly))
     { 
       QTextStream ts( &output );
       ts.setEncoding(QTextStream::UnicodeUTF8);
       while(!ts.atEnd())
       {
         QString line = ts.readLine();
         if (line.startsWith("# DELETE "))
         {  
            QString key = line.mid(9);
            if (key[0] == '[')
            {
               int j = key.find(']')+1;
               if (j > 0)
               {
                  group = key.mid(1,j-2);
                  key = key.mid(j);
               }
            }
            oldConfig2->setGroup(group);
            oldConfig2->deleteEntry(key, false);
            if (oldConfig2->deleteGroup(group, false)) { // Delete group if empty.
               qDebug("Removing group %s", group.isEmpty() ? "<empty>" : group.latin1());
	    }
         }
         else if (line.startsWith("# DELETEGROUP"))
         {
            QString key = line.mid(13).stripWhiteSpace();
            if (key[0] == '[')
            {
               int j = key.find(']')+1;
               if (j > 0)
               {
                  group = key.mid(1,j-2);
               }
            }
            if (oldConfig2->deleteGroup(group, true)) { // Delete group
               qDebug("Removing group %s (FORCED)", group.isEmpty() ? "<empty>" : group.latin1());
	    }
          }
       }
     }
   }

   // Merging in new entries.
   m_bCopy = true;
   {
     KConfig *saveOldConfig1 = oldConfig1;
     QString saveOldGroup = oldGroup;
     QString saveNewGroup = newGroup;
     oldConfig1 = new KConfig(tmp2.name(), true, false);

     // For all groups...
     QStringList grpList = oldConfig1->groupList();
     for(QStringList::ConstIterator it = grpList.begin();
         it != grpList.end();
         ++it)
     {
        oldGroup = *it;
        if (oldGroup != "<default>")
        {
           newGroup = oldGroup;
        }
        gotAllKeys(); // Copy all keys
     }
     delete oldConfig1;
     oldConfig1 = saveOldConfig1;
     oldGroup = saveOldGroup;
     newGroup = saveNewGroup;
   }
}

void KonfUpdate::resetOptions()
{
   m_bCopy = false;
   m_bOverwrite = false;
}


int main(int argc, char **argv)
{
   KAboutData aboutData("kconf_update", I18N_NOOP("KConf Update"),
                        "1.0.2",
                        I18N_NOOP("KDE Tool for updating user configuration files"),
                        KAboutData::License_GPL,
                        "(c) 2001, Waldo Bastian");

   aboutData.addAuthor("Waldo Bastian", 0, "bastian@kde.org");

   KCmdLineArgs::init(argc, argv, &aboutData);
   KCmdLineArgs::addCmdLineOptions(options);

   KInstance instance(&aboutData);

   KonfUpdate konfUpdate;
}
