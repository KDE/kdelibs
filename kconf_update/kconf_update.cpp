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

#include <qdatetime.h>
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
#include <kurl.h>

static KCmdLineOptions options[] =
{
        { "debug", I18N_NOOP("Keep output results from scripts"), 0 },
	{ "check <update-file>", I18N_NOOP("Check whether config file itself requires updating"), 0 },
	{ "+[file]", I18N_NOOP("File to read update instructions from"), 0 },
        KCmdLineLastOption
};

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

   void copyGroup(KConfigBase *cfg1, const QString &grp1, 
                  KConfigBase *cfg2, const QString &grp2);

protected:
   KConfig *config;
   QString currentFilename;
   bool skip;
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

   QStringList updateFiles;
   KCmdLineArgs *args=KCmdLineArgs::parsedArgs();
   
   debug = args->isSet("debug");

   m_bUseConfigInfo = false;
   if (args->isSet("check"))
   {
      m_bUseConfigInfo = true;
      QString file = locate("data", "kconf_update/"+QFile::decodeName(args->getOption("check")));
      if (file.isEmpty())
      {
         qWarning("File '%s' not found.", args->getOption("check").data());
         log() << "File '" << QFile::decodeName(args->getOption("check")) << "' passed on command line not found" << endl;
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
      if (config->readBoolEntry("autoUpdateDisabled", false))
         return;
      updateFiles = findUpdateFiles(true);
      updateAll = true;
   }

   for(QStringList::ConstIterator it = updateFiles.begin();
       it != updateFiles.end();
       ++it)
   {
      QString file = *it;
      updateFile(file);
   }

   config->setGroup(QString::null);
   if (updateAll && !config->readBoolEntry("updateInfoAdded", false))
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
      QString file = locateLocal("data", "kconf_update/log/update.log");
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
   QStringList list = KGlobal::dirs()->findAllResources("data", "kconf_update/*.upd", false, true);
   for(QStringList::ConstIterator it = list.begin();
       it != list.end();
       ++it)
   {
      QString file = *it;
      struct stat buff;
      if (stat( QFile::encodeName(file), &buff) == 0)
      {
         int i = file.lastIndexOf('/');
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
   int i = currentFilename.lastIndexOf('/');
   if (i != -1) 
      currentFilename = currentFilename.mid(i+1);
   skip = true;
   QFile file(filename);
   if (!file.open(QIODevice::ReadOnly))
      return false;

   QTextStream ts(&file);
   ts.setEncoding(QTextStream::Latin1);
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
         id = currentFilename+":"+line.mid(3);
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

//   qDebug("File %s, id %s", file.latin1(), id.latin1());

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
   ts.setEncoding(QTextStream::Latin1);
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
   if (!id.isEmpty() && !skip)
   {
       config->setGroup(currentFilename);
       QStringList ids = config->readListEntry("done");
       if (!ids.contains(id))
       {
          ids.append(id);
          config->writeEntry("done", ids);
          config->sync();
       }
   }

   // Flush pending changes
   gotFile(QString::null);

   config->setGroup(currentFilename);
   QStringList ids = config->readListEntry("done");
   if (!_id.isEmpty())
   {
       if (ids.contains(_id))
       {
          //qDebug("Id '%s' was already in done-list", _id.latin1());
          if (!m_bUseConfigInfo)
          {
             skip = true;
             return;
          }
       }
       skip = false;
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
   gotGroup(QString::null);
 
   if (!oldFile.isEmpty())
   {
      // Close old file.
      delete oldConfig1;
      oldConfig1 = 0;

      oldConfig2->setGroup("$Version");
      QStringList ids = oldConfig2->readListEntry("update_info");
      QString cfg_id = currentFilename + ":" + id;
      if (!ids.contains(cfg_id) && !skip)
      {
         ids.append(cfg_id);
         oldConfig2->writeEntry("update_info", ids);
      }
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
      newConfig->setGroup("$Version");
      QStringList ids = newConfig->readListEntry("update_info");
      QString cfg_id = currentFilename + ":" + id;
      if (!ids.contains(cfg_id) && !skip)
      {
         ids.append(cfg_id);
         newConfig->writeEntry("update_info", ids);
      }
      newConfig->sync();
      delete newConfig;
      newConfig = 0;

      newFile = QString::null;
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
         newFile = QString::null;
   }
   
   if (!oldFile.isEmpty())
   {
      oldConfig2 = new KConfig(oldFile, false, false);
      QString cfg_id = currentFilename + ":" + id;
      oldConfig2->setGroup("$Version");
      QStringList ids = oldConfig2->readListEntry("update_info");
      if (ids.contains(cfg_id))
      {
         skip = true;
         newFile = QString::null;
         log() << currentFilename << ": Skipping update '" << id << "'" << endl;
      }

      if (!newFile.isEmpty())
      {
         newConfig = new KConfig(newFile, false, false);
         newConfig->setGroup("$Version");
         ids = newConfig->readListEntry("update_info");
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

      oldConfig1 = new KConfig(oldFile, true, false);
   }
   else
   {
      newFile = QString::null;
   }
   newFileName = newFile;
   if (newFileName.isEmpty())
      newFileName = oldFile;
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
   oldConfig2->deleteGroup(oldGroup, true);
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
   oldConfig1->setGroup(oldGroup);
   if (!oldConfig1->hasKey(oldKey))
      return;
   QString value = oldConfig1->readEntry(oldKey);
   newConfig->setGroup(newGroup);
   if (!m_bOverwrite && newConfig->hasKey(newKey))
   {
      log() << currentFilename << ": Skipping " << newFileName << ":" << newGroup << ":" << newKey << ", already exists."<< endl;
      return;
   }
   log() << currentFilename << ": Updating " << newFileName << ":" << newGroup << ":" << newKey << " to '" << value << "'" << endl;
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
   log() << currentFilename << ": Removing " << oldFile << ":" << oldGroup << ":" << oldKey << ", moved." << endl;
   if (oldConfig2->deleteGroup(oldGroup, false)) { // Delete group if empty.
      log() << currentFilename << ": Removing empty group " << oldFile << ":" << oldGroup << endl;
   }
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

   oldConfig1->setGroup(oldGroup);
   if (!oldConfig1->hasKey(oldKey))
      return;
   log() << currentFilename << ": RemoveKey removes " << oldFile << ":" << oldGroup << ":" << oldKey << endl;

   // Delete old entry
   oldConfig2->setGroup(oldGroup);
   oldConfig2->deleteEntry(oldKey, false);
   if (oldConfig2->deleteGroup(oldGroup, false)) { // Delete group if empty.
      log() << currentFilename << ": Removing empty group " << oldFile << ":" << oldGroup << endl;
   }
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
   QStringList options = _options.split(',');
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

void KonfUpdate::copyGroup(KConfigBase *cfg1, const QString &grp1, 
                           KConfigBase *cfg2, const QString &grp2)
{
   cfg1->setGroup(grp1);
   cfg2->setGroup(grp2);
   QMap<QString, QString> list = cfg1->entryMap(grp1);
   for(QMap<QString, QString>::Iterator it = list.begin();
       it != list.end(); ++it)
   {
      cfg2->writeEntry(it.key(), cfg1->readEntry(it.key()));
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
      log() << currentFilename << ": !! Script fails to specifiy filename in line " << m_lineCount << " : '" << m_line << "'" << endl;
      skip = true;
      return;
   } 



   QString path = locate("data","kconf_update/"+script);
   if (path.isEmpty())
   {
      if (interpreter.isEmpty())
         path = locate("lib", "kconf_update_bin/"+script);

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
      cmd = interpreter + " " + path;

   if( !m_arguments.isNull())
   {
      cmd += ' ';
      cmd += m_arguments;
   }

   KTempFile tmp1;
   tmp1.setAutoDelete(true);
   KTempFile tmp2;
   tmp2.setAutoDelete(true);
   KTempFile tmp3;
   tmp3.setAutoDelete(true);

   int result;
   if (oldConfig1)
   {
       if (debug)
       {
           tmp1.setAutoDelete(false);
           log() << "Script input stored in " << tmp1.name() << endl;
       }
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
       result = system(QFile::encodeName(QString("%1 < %2 > %3 2> %4").arg(cmd, tmp1.name(), tmp2.name(), tmp3.name())));
   }
   else
   {
       // No config file
       result = system(QFile::encodeName(QString("%1 2> %2").arg(cmd, tmp3.name())));
   }

   // Copy script stderr to log file
   {
     QFile output(tmp3.name());
     if (output.open(QIODevice::ReadOnly))
     { 
       QTextStream ts( &output );
       ts.setEncoding(QTextStream::UnicodeUTF8);
       while(!ts.atEnd())
       {
         QString line = ts.readLine();
         log() << "[Script] " << line << endl;
       }
     }
   }

   if (result)
   {
      log() << currentFilename << ": !! An error occured while running '" << cmd << "'" << endl;
      return;
   }

   if (!oldConfig1)
      return; // Nothing to merge

   if (debug)
   {
      tmp2.setAutoDelete(false);
      log() << "Script output stored in " << tmp2.name() << endl;
   }

   // Deleting old entries
   {
     QString group = oldGroup;
     QFile output(tmp2.name());
     if (output.open(QIODevice::ReadOnly))
     { 
       QTextStream ts( &output );
       ts.setEncoding(QTextStream::UnicodeUTF8);
       while(!ts.atEnd())
       {
         QString line = ts.readLine();
         if (line.startsWith("["))
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
            oldConfig2->setGroup(group);
            oldConfig2->deleteEntry(key, false);
            log() << currentFilename << ": Script removes " << oldFile << ":" << group << ":" << key << endl;
            if (oldConfig2->deleteGroup(group, false)) { // Delete group if empty.
               log() << currentFilename << ": Removing empty group " << oldFile << ":" << group << endl;
	    }
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
            if (oldConfig2->deleteGroup(group, true)) { // Delete group
               log() << currentFilename << ": Script removes group " << oldFile << ":" << group << endl;
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
   m_arguments = QString::null;
}


extern "C" KDE_EXPORT int kdemain(int argc, char **argv)
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

   return 0;
}
