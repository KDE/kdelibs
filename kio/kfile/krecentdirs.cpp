/* -*- c++ -*-
 * Copyright (C)2000 Waldo Bastian <bastian@kde.org>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#include <krecentdirs.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>

#define MAX_DIR_HISTORY 3

static void recentdirs_done(KConfig *config)
{
   if (config == KGlobal::config())
   {
      config->sync();
   }
   else
   {
      delete config;
   }
}

static KConfig *recentdirs_readList(QString &key, QStringList &result, bool readOnly)
{
   KConfig *config;
   if ((key.length() < 2) || (key[0] != ':'))
     key = ":default";
   if (key[1] == ':') 
   {
      key = key.mid(2);
      config = new KSimpleConfig(QLatin1String("krecentdirsrc"), readOnly);
   }
   else
   {
      key = key.mid(1);
      config = KGlobal::config();
      config->setGroup(QLatin1String("Recent Dirs"));
   }

   result=config->readPathListEntry(key);
   if (result.isEmpty())
   {
      result.append(KGlobalSettings::documentPath());
   }
   return config;
}

QStringList KRecentDirs::list(const QString &fileClass)
{
   QString key = fileClass;
   QStringList result;
   recentdirs_done(recentdirs_readList(key, result, true));
   return result;
}
    
QString KRecentDirs::dir(const QString &fileClass)
{
   QStringList result = list(fileClass);
   return result[0];
}

void KRecentDirs::add(const QString &fileClass, const QString &directory)
{
   QString key = fileClass;
   QStringList result;
   KConfig *config = recentdirs_readList(key, result, false);
   // make sure the dir is first in history
   result.removeAll(directory);
   result.prepend(directory);
   while(result.count() > MAX_DIR_HISTORY)
      result.removeLast();
   config->writePathEntry(key, result);
   recentdirs_done(config);
}

