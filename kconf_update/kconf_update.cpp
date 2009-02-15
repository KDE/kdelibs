/*
 *
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <kde_file.h>

#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QTextCodec>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <ktemporaryfile.h>
#include <kurl.h>

class KonfUpdate
{
public:
   KonfUpdate();
   ~KonfUpdate();
   QStringList findUpdateFiles(bool dirtyOnly);

   QTextStream &log();

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
   void gotScriptArguments(const QString &_arguments);
   void resetOptions();

   void copyGroup(KConfig *cfg1, const QString &group1,
                  KConfig *cfg2, const QString &group2);

protected:
   KConfig *config;
   QString currentFilename;
   bool skip;
   bool skipFile;
   bool debug;
   QString id;

   QString oldFile;
   QString newFile;
   QString newFileName;
   KConfig *oldConfig1; // Config to read keys from.
   KConfig *oldConfig2; // Config to delete keys from.
   KConfig *newConfig;

   QString oldGroup;
   QString newGroup;
   QString oldKey;
   QString newKey;

   bool m_bCopy;
   bool m_bOverwrite;
   bool m_bUseConfigInfo;
   QString m_arguments;
   QTextStream *m_textStream;
   QFile *m_file;
   QString m_line;
   int m_lineCount;
};

KonfUpdate::KonfUpdate()
 : m_textStream(0), m_file(0)
{
   bool updateAll = false;
   oldConfig1 = 0;
   oldConfig2 = 0;
   newConfig = 0;

   config = new KConfig("kconf_updaterc");
   KConfigGroup cg(config, QString());

   QStringList updateFiles;
   KCmdLineArgs *args=KCmdLineArgs::parsedArgs();

   debug = args->isSet("debug");

   m_bUseConfigInfo = false;
   if (args->isSet("check"))
   {
      m_bUseConfigInfo = true;
      QString file = KStandardDirs::locate("data", "kconf_update/"+args->getOption("check"));
      if (file.isEmpty())
      {
         qWarning("File '%s' not found.", args->getOption("check").toLocal8Bit().data());
         log() << "File '" << args->getOption("check") << "' passed on command line not found" << endl;
         return;
      }
      updateFiles.append(file);
   }
   else if (args->count())
   {
      for(int i = 0; i < args->count(); i++)
      {
         KUrl url = args->url(i);
         if (!url.isLocalFile())
            KCmdLineArgs::usageError(i18n("Only local files are supported."));
         updateFiles.append(url.path());
      }
   }
   else
   {
      if (cg.readEntry("autoUpdateDisabled", false))
         return;
      updateFiles = findUpdateFiles(true);
      updateAll = true;
   }

   for(QStringList::ConstIterator it = updateFiles.constBegin();
       it != updateFiles.constEnd();
       ++it)
   {
      updateFile(*it);
   }

   if (updateAll && !cg.readEntry("updateInfoAdded", false))
   {
       cg.writeEntry("updateInfoAdded", true);
       updateFiles = findUpdateFiles(false);

       for(QStringList::ConstIterator it = updateFiles.constBegin();
           it != updateFiles.constEnd();
           ++it)
       {
           checkFile(*it);
       }
       updateFiles.clear();
   }
}

KonfUpdate::~KonfUpdate()
{
   delete config;
   delete m_file;
   delete m_textStream;
}

QTextStream &
KonfUpdate::log()
{
   if (!m_textStream)
   {
      QString file = KStandardDirs::locateLocal("data", "kconf_update/log/update.log");
      m_file = new QFile(file);
      if (m_file->open(QIODevice::WriteOnly | QIODevice::Append))
      {
        m_textStream = new QTextStream(m_file);
      }
      else
      {
        // Error
        m_textStream = new QTextStream(stderr, QIODevice::WriteOnly);
      }
   }

   (*m_textStream) << QDateTime::currentDateTime().toString( Qt::ISODate ) << " ";

   return *m_textStream;
}

QStringList KonfUpdate::findUpdateFiles(bool dirtyOnly)
{
   QStringList result;
   const QStringList list = KGlobal::dirs()->findAllResources("data", "kconf_update/*.upd",
                                                              KStandardDirs::NoDuplicates);
   for(QStringList::ConstIterator it = list.constBegin();
       it != list.constEnd();
       ++it)
   {
      QString file = *it;
      KDE_struct_stat buff;
      if (KDE::stat(file, &buff) == 0)
      {
         int i = file.lastIndexOf('/');
         if (i != -1)
            file = file.mid(i+1);
         KConfigGroup cg(config, file);
         time_t ctime = cg.readEntry("ctime", 0);
         time_t mtime = cg.readEntry("mtime", 0);
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
   int i = currentFilename.lastIndexOf('/');
   if (i != -1)
      currentFilename = currentFilename.mid(i+1);
   skip = true;
   QFile file(filename);
   if (!file.open(QIODevice::ReadOnly))
      return false;

   QTextStream ts(&file);
   ts.setCodec(QTextCodec::codecForName("ISO-8859-1"));
   int lineCount = 0;
   resetOptions();
   QString id;
   while(!ts.atEnd())
   {
      QString line = ts.readLine().trimmed();
      lineCount++;
      if (line.isEmpty() || (line[0] == '#'))
         continue;
      if (line.startsWith("Id="))
         id = currentFilename+':'+line.mid(3);
      else if (line.startsWith("File="))
         checkGotFile(line.mid(5), id);
   }

   return true;
}

void KonfUpdate::checkGotFile(const QString &_file, const QString &id)
{
   QString file;
   int i = _file.indexOf(',');
   if (i == -1)
   {
      file = _file.trimmed();
   }
   else
   {
      file = _file.mid(i+1).trimmed();
   }

//   qDebug("File %s, id %s", file.toLatin1().constData(), id.toLatin1().constData());

   KConfig cfg(file, KConfig::SimpleConfig);
   KConfigGroup cg(&cfg, "$Version");
   QStringList ids = cg.readEntry("update_info", QStringList());
   if (ids.contains(id))
       return;
   ids.append(id);
   cg.writeEntry("update_info", ids);
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
 * ScriptArguments=arguments
 * Script=scriptfile[,interpreter]
 *
 * Sequence:
 * (Id,(File(Group,Keys)*)*)*
 **/
bool KonfUpdate::updateFile(const QString &filename)
{
   currentFilename = filename;
   int i = currentFilename.lastIndexOf('/');
   if (i != -1)
       currentFilename = currentFilename.mid(i+1);
   skip = true;
   QFile file(filename);
   if (!file.open(QIODevice::ReadOnly))
      return false;

   log() << "Checking update-file '" << filename << "' for new updates" << endl;

   QTextStream ts(&file);
   ts.setCodec(QTextCodec::codecForName("ISO-8859-1"));
   m_lineCount = 0;
   resetOptions();
   while(!ts.atEnd())
   {
      m_line = ts.readLine().trimmed();
      m_lineCount++;
      if (m_line.isEmpty() || (m_line[0] == '#'))
         continue;
      if (m_line.startsWith("Id="))
         gotId(m_line.mid(3));
      else if (skip)
         continue;
      else if (m_line.startsWith("Options="))
         gotOptions(m_line.mid(8));
      else if (m_line.startsWith("File="))
         gotFile(m_line.mid(5));
      else if(skipFile)
         continue;
      else if (m_line.startsWith("Group="))
         gotGroup(m_line.mid(6));
      else if (m_line.startsWith("RemoveGroup="))
      {
         gotRemoveGroup(m_line.mid(12));
         resetOptions();
      }
      else if (m_line.startsWith("Script="))
      {
         gotScript(m_line.mid(7));
         resetOptions();
      }
      else if (m_line.startsWith("ScriptArguments="))
         gotScriptArguments(m_line.mid(16));
      else if (m_line.startsWith("Key="))
      {
         gotKey(m_line.mid(4));
         resetOptions();
      }
      else if (m_line.startsWith("RemoveKey="))
      {
         gotRemoveKey(m_line.mid(10));
         resetOptions();
      }
      else if (m_line == "AllKeys")
      {
         gotAllKeys();
         resetOptions();
      }
      else if (m_line == "AllGroups")
      {
         gotAllGroups();
         resetOptions();
      }
      else
      {
         log() << currentFilename << ": parse error in line " << m_lineCount << " : '" << m_line << "'" << endl;
      }
   }
   // Flush.
   gotId(QString());

   KDE_struct_stat buff;
   KDE::stat(filename, &buff);
   KConfigGroup cg(config, currentFilename);
   cg.writeEntry("ctime", int(buff.st_ctime));
   cg.writeEntry("mtime", int(buff.st_mtime));
   cg.sync();
   return true;
}



void KonfUpdate::gotId(const QString &_id)
{
   if (!id.isEmpty() && !skip)
   {
       KConfigGroup cg(config, currentFilename);

       QStringList ids = cg.readEntry("done", QStringList());
       if (!ids.contains(id))
       {
          ids.append(id);
          cg.writeEntry("done", ids);
          cg.sync();
       }
   }

   // Flush pending changes
   gotFile(QString());
   KConfigGroup cg(config, currentFilename);

   QStringList ids = cg.readEntry("done", QStringList());
   if (!_id.isEmpty())
   {
       if (ids.contains(_id))
       {
          //qDebug("Id '%s' was already in done-list", _id.toLatin1().constData());
          if (!m_bUseConfigInfo)
          {
             skip = true;
             return;
          }
       }
       skip = false;
       skipFile = false;
       id = _id;
       if (m_bUseConfigInfo)
          log() << currentFilename << ": Checking update '" << _id << "'" << endl;
       else
          log() << currentFilename << ": Found new update '" << _id << "'" << endl;
   }
}

void KonfUpdate::gotFile(const QString &_file)
{
   // Reset group
   gotGroup(QString());

   if (!oldFile.isEmpty())
   {
      // Close old file.
      delete oldConfig1;
      oldConfig1 = 0;

      KConfigGroup cg(oldConfig2, "$Version");
      QStringList ids = cg.readEntry("update_info", QStringList());
      QString cfg_id = currentFilename + ':' + id;
      if (!ids.contains(cfg_id) && !skip)
      {
         ids.append(cfg_id);
         cg.writeEntry("update_info", ids);
      }
      cg.sync();
      delete oldConfig2;
      oldConfig2 = 0;

      QString file = KStandardDirs::locateLocal("config", oldFile);
      KDE_struct_stat s_buf;
      if (KDE::stat(file, &s_buf) == 0)
      {
         if (s_buf.st_size == 0)
         {
            // Delete empty file.
            QFile::remove(file);
         }
      }

      oldFile.clear();
   }
   if (!newFile.isEmpty())
   {
      // Close new file.
      KConfigGroup cg(newConfig, "$Version");
      QStringList ids = cg.readEntry("update_info", QStringList());
      QString cfg_id = currentFilename + ':' + id;
      if (!ids.contains(cfg_id) && !skip)
      {
         ids.append(cfg_id);
         cg.writeEntry("update_info", ids);
      }
      newConfig->sync();
      delete newConfig;
      newConfig = 0;

      newFile.clear();
   }
   newConfig = 0;

   int i = _file.indexOf(',');
   if (i == -1)
   {
      oldFile = _file.trimmed();
   }
   else
   {
      oldFile = _file.left(i).trimmed();
      newFile = _file.mid(i+1).trimmed();
      if (oldFile == newFile)
         newFile.clear();
   }

   if (!oldFile.isEmpty())
   {
      oldConfig2 = new KConfig(oldFile, KConfig::NoGlobals);
      QString cfg_id = currentFilename + ':' + id;
      KConfigGroup cg(oldConfig2, "$Version");
      QStringList ids = cg.readEntry("update_info", QStringList());
      if (ids.contains(cfg_id))
      {
         skip = true;
         newFile.clear();
         log() << currentFilename << ": Skipping update '" << id << "'" << endl;
      }

      if (!newFile.isEmpty())
      {
         newConfig = new KConfig(newFile, KConfig::NoGlobals);
         KConfigGroup cg(newConfig, "$Version");
         ids = cg.readEntry("update_info", QStringList());
         if (ids.contains(cfg_id))
         {
            skip = true;
            log() << currentFilename << ": Skipping update '" << id << "'" << endl;
         }
      }
      else
      {
         newConfig = oldConfig2;
      }

      oldConfig1 = new KConfig(oldFile, KConfig::NoGlobals);
   }
   else
   {
      newFile.clear();
   }
   newFileName = newFile;
   if (newFileName.isEmpty())
      newFileName = oldFile;

   skipFile = false;
   if( !oldFile.isEmpty())
   { // if File= is specified, it doesn't exist, is empty or contains only kconf_update's [$Version] group, skip
      if( oldConfig1 != NULL
          && ( oldConfig1->groupList().isEmpty()
              || ( oldConfig1->groupList().count() == 1 && oldConfig1->groupList().first() == "$Version" )))
      {
         log() << currentFilename << ": File '" << oldFile << "' does not exist or empty, skipping" << endl;
         skipFile = true;
      }
   }
}

void KonfUpdate::gotGroup(const QString &_group)
{
   int i = _group.indexOf(',');
   if (i == -1)
   {
      oldGroup = _group.trimmed();
      newGroup = oldGroup;
   }
   else
   {
      oldGroup = _group.left(i).trimmed();
      newGroup = _group.mid(i+1).trimmed();
   }
}

void KonfUpdate::gotRemoveGroup(const QString &_group)
{
   oldGroup = _group.trimmed();

   if (!oldConfig1)
   {
      log() << currentFilename << ": !! RemoveGroup without previous File specification in line " << m_lineCount << " : '" << m_line << "'" << endl;
      return;
   }

   if (!oldConfig1->hasGroup(oldGroup))
      return;
   // Delete group.
   oldConfig2->deleteGroup(oldGroup);
   log() << currentFilename << ": RemoveGroup removes group " << oldFile << ":" << oldGroup << endl;
}


void KonfUpdate::gotKey(const QString &_key)
{
   int i = _key.indexOf(',');
   if (i == -1)
   {
      oldKey = _key.trimmed();
      newKey = oldKey;
   }
   else
   {
      oldKey = _key.left(i).trimmed();
      newKey = _key.mid(i+1).trimmed();
   }

   if (oldKey.isEmpty() || newKey.isEmpty())
   {
      log() << currentFilename << ": !! Key specifies invalid key in line " << m_lineCount << " : '" << m_line << "'" << endl;
      return;
   }
   if (!oldConfig1)
   {
      log() << currentFilename << ": !! Key without previous File specification in line " << m_lineCount << " : '" << m_line << "'" << endl;
      return;
   }
   KConfigGroup cg1( oldConfig1, oldGroup);
   if (!cg1.hasKey(oldKey))
      return;
   QString value = cg1.readEntry(oldKey, QString());
   KConfigGroup newFGroup( newConfig, newGroup);
   if (!m_bOverwrite && newFGroup.hasKey(newKey))
   {
      log() << currentFilename << ": Skipping " << newFileName << ":" << newGroup << ":" << newKey << ", already exists."<< endl;
      return;
   }
   log() << currentFilename << ": Updating " << newFileName << ":" << newGroup << ":" << newKey << " to '" << value << "'" << endl;
   newFGroup.writeEntry(newKey, value);

   if (m_bCopy)
      return; // Done.

   // Delete old entry
   if ((oldConfig2 == newConfig) &&
       (oldGroup == newGroup) &&
       (oldKey == newKey))
      return; // Don't delete!
   KConfigGroup oldGroup2( oldConfig2, oldGroup);
   oldGroup2.deleteEntry(oldKey);
   log() << currentFilename << ": Removing " << oldFile << ":" << oldGroup << ":" << oldKey << ", moved." << endl;
   /*if (oldConfig2->deleteGroup(oldGroup, KConfig::Normal)) { // Delete group if empty.
      log() << currentFilename << ": Removing empty group " << oldFile << ":" << oldGroup << endl;
   }  (this should be automatic)  */
}

void KonfUpdate::gotRemoveKey(const QString &_key)
{
   oldKey = _key.trimmed();

   if (oldKey.isEmpty())
   {
      log() << currentFilename << ": !! RemoveKey specifies invalid key in line " << m_lineCount << " : '" << m_line << "'" << endl;
      return;
   }

   if (!oldConfig1)
   {
      log() << currentFilename << ": !! Key without previous File specification in line " << m_lineCount << " : '" << m_line << "'" << endl;
      return;
   }

   KConfigGroup cg1(oldConfig1, oldGroup);
   if (!cg1.hasKey(oldKey))
      return;
   log() << currentFilename << ": RemoveKey removes " << oldFile << ":" << oldGroup << ":" << oldKey << endl;

   // Delete old entry
   KConfigGroup cg2( oldConfig2, oldGroup);
   cg2.deleteEntry(oldKey);
   /*if (oldConfig2->deleteGroup(oldGroup, KConfig::Normal)) { // Delete group if empty.
      log() << currentFilename << ": Removing empty group " << oldFile << ":" << oldGroup << endl;
   }   (this should be automatic)*/
}

void KonfUpdate::gotAllKeys()
{
   if (!oldConfig1)
   {
      log() << currentFilename << ": !! AllKeys without previous File specification in line " << m_lineCount << " : '" << m_line << "'" << endl;
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
      log() << currentFilename << ": !! AllGroups without previous File specification in line " << m_lineCount << " : '" << m_line << "'" << endl;
      return;
   }

   const QStringList allGroups = oldConfig1->groupList();
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
   const QStringList options = _options.split(',');
   for(QStringList::ConstIterator it = options.begin();
       it != options.end();
       ++it)
   {
       if ( (*it).toLower().trimmed() == "copy")
          m_bCopy = true;

       if ( (*it).toLower().trimmed() == "overwrite")
          m_bOverwrite = true;
   }
}

void KonfUpdate::copyGroup(KConfig *cfg1, const QString &group1,
                           KConfig *cfg2, const QString &group2)
{
   KConfigGroup cg1(cfg1, group1);
   KConfigGroup cg2(cfg2, group2);
   QMap<QString, QString> list = cg1.entryMap();
   for(QMap<QString, QString>::Iterator it = list.begin();
       it != list.end(); ++it)
   {
      cg2.writeEntry(it.key(), cg1.readEntry(it.key(), QString()));
   }
}

void KonfUpdate::gotScriptArguments(const QString &_arguments)
{
   m_arguments = _arguments;
}

void KonfUpdate::gotScript(const QString &_script)
{
   QString script, interpreter;
   int i = _script.indexOf(',');
   if (i == -1)
   {
      script = _script.trimmed();
   }
   else
   {
      script = _script.left(i).trimmed();
      interpreter = _script.mid(i+1).trimmed();
   }


   if (script.isEmpty())
   {
      log() << currentFilename << ": !! Script fails to specify filename in line " << m_lineCount << " : '" << m_line << "'" << endl;
      skip = true;
      return;
   }



   QString path = KStandardDirs::locate("data","kconf_update/"+script);
   if (path.isEmpty())
   {
      if (interpreter.isEmpty())
         path = KStandardDirs::locate("lib", "kconf_update_bin/"+script);

      if (path.isEmpty())
      {
        log() << currentFilename << ": !! Script '" << script << "' not found in line " << m_lineCount << " : '" << m_line << "'" << endl;
        skip = true;
        return;
      }
   }

   if( !m_arguments.isNull())
      log() << currentFilename << ": Running script '" << script << "' with arguments '" << m_arguments << "'" << endl;
   else
      log() << currentFilename << ": Running script '" << script << "'" << endl;

   QString cmd;
   if (interpreter.isEmpty())
      cmd = path;
   else
      cmd = interpreter + ' ' + path;

   if( !m_arguments.isNull())
   {
      cmd += ' ';
      cmd += m_arguments;
   }

   KTemporaryFile tmp1;
   tmp1.open();
   KTemporaryFile tmp2;
   tmp2.open();
   KTemporaryFile tmp3;
   tmp3.open();

   int result;
   if (oldConfig1)
   {
       if (debug)
       {
           tmp1.setAutoRemove(false);
           log() << "Script input stored in " << tmp1.fileName() << endl;
       }
       KConfig cfg(tmp1.fileName(), KConfig::SimpleConfig);

       if (oldGroup.isEmpty())
       {
           // Write all entries to tmpFile;
           const QStringList grpList = oldConfig1->groupList();
           for(QStringList::ConstIterator it = grpList.begin();
               it != grpList.end();
               ++it)
           {
               copyGroup(oldConfig1, *it, &cfg, *it);
           }
       }
       else
       {
           copyGroup(oldConfig1, oldGroup, &cfg, QString());
       }
       cfg.sync();
       result = system(QFile::encodeName(QString("%1 < %2 > %3 2> %4").arg(cmd, tmp1.fileName(), tmp2.fileName(), tmp3.fileName())));
   }
   else
   {
       // No config file
       result = system(QFile::encodeName(QString("%1 2> %2").arg(cmd, tmp3.fileName())));
   }

   // Copy script stderr to log file
   {
     QFile output(tmp3.fileName());
     if (output.open(QIODevice::ReadOnly))
     {
       QTextStream ts( &output );
       ts.setCodec(QTextCodec::codecForName("UTF-8"));
       while(!ts.atEnd())
       {
         QString line = ts.readLine();
         log() << "[Script] " << line << endl;
       }
     }
   }

   if (result)
   {
      log() << currentFilename << ": !! An error occurred while running '" << cmd << "'" << endl;
      return;
   }

   if (!oldConfig1)
      return; // Nothing to merge

   if (debug)
   {
      tmp2.setAutoRemove(false);
      log() << "Script output stored in " << tmp2.fileName() << endl;
   }

   // Deleting old entries
   {
     QString group = oldGroup;
     QFile output(tmp2.fileName());
     if (output.open(QIODevice::ReadOnly))
     {
       QTextStream ts( &output );
       ts.setCodec(QTextCodec::codecForName("UTF-8"));
       while(!ts.atEnd())
       {
         QString line = ts.readLine();
         if (line.startsWith('['))
         {
            int j = line.indexOf(']')+1;
            if (j > 0)
               group = line.mid(1, j-2);
         }
         else if (line.startsWith("# DELETE "))
         {
            QString key = line.mid(9);
            if (key[0] == '[')
            {
               int j = key.indexOf(']')+1;
               if (j > 0)
               {
                  group = key.mid(1,j-2);
                  key = key.mid(j);
               }
            }
            KConfigGroup cg(oldConfig2, group);
            cg.deleteEntry(key);
            log() << currentFilename << ": Script removes " << oldFile << ":" << group << ":" << key << endl;
            /*if (oldConfig2->deleteGroup(group, KConfig::Normal)) { // Delete group if empty.
               log() << currentFilename << ": Removing empty group " << oldFile << ":" << group << endl;
	    } (this should be automatic)*/
         }
         else if (line.startsWith("# DELETEGROUP"))
         {
            QString key = line.mid(13).trimmed();
            if (key[0] == '[')
            {
               int j = key.indexOf(']')+1;
               if (j > 0)
               {
                  group = key.mid(1,j-2);
               }
            }
            oldConfig2->deleteGroup(group);
            log() << currentFilename << ": Script removes group " << oldFile << ":" << group << endl;
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
     oldConfig1 = new KConfig(tmp2.fileName(), KConfig::NoGlobals);

     // For all groups...
     const QStringList grpList = oldConfig1->groupList();
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
   m_arguments.clear();
}


extern "C" KDE_EXPORT int kdemain(int argc, char **argv)
{
   KCmdLineOptions options;
   options.add("debug", ki18n("Keep output results from scripts"));
   options.add("check <update-file>", ki18n("Check whether config file itself requires updating"));
   options.add("+[file]", ki18n("File to read update instructions from"));

   KAboutData aboutData("kconf_update", 0, ki18n("KConf Update"),
                        "1.0.2",
                        ki18n("KDE Tool for updating user configuration files"),
                        KAboutData::License_GPL,
                        ki18n("(c) 2001, Waldo Bastian"));

   aboutData.addAuthor(ki18n("Waldo Bastian"), KLocalizedString(), "bastian@kde.org");

   KCmdLineArgs::init(argc, argv, &aboutData);
   KCmdLineArgs::addCmdLineOptions(options);

   KComponentData componentData(&aboutData);

   KonfUpdate konfUpdate;

   return 0;
}
