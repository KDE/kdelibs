/***************************************************************************
                          knewstuffsecure.cpp  -  description
                             -------------------
    begin                : Tue Jun 22 12:19:55 2004
    copyright          : (C) 2004, 2005 by Andras Mantia <amantia@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; version 2 of the License.  *
 *                                                                         *
 ***************************************************************************/
//qt includes
#include <qfileinfo.h>

//kde includes
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktar.h>
#include <ktempdir.h>

//app includes
#include "engine.h"
#include "knewstuffsecure.h"
#include "security.h"

using namespace KNS;

KNewStuffSecure::KNewStuffSecure(const QString &type,  QWidget *parentWidget)
 : KNewStuff(type, parentWidget)
{
  m_tempDir = 0L;
  connect(engine(), SIGNAL(uploadFinished(bool)), SLOT(slotUploadFinished(bool)));
}


KNewStuffSecure::~KNewStuffSecure()
{
  removeTempDirectory();
}

bool KNewStuffSecure::install(const QString &fileName)
{
  bool ok = true;

  removeTempDirectory();
  m_tempDir = new KTempDir();
  m_tempDir->setAutoDelete(true);
  KTar tar(fileName, "application/x-gzip");
  if (tar.open(QIODevice::ReadOnly))
  {
      const KArchiveDirectory *directory = tar.directory();
      directory->copyTo(m_tempDir->name(), true);
      m_tarName = "";
      QStringList entries = directory->entries();
      for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it)
      {
        if (*it != "signature" && *it != "md5sum")
        {
          m_tarName = *it;
          break;
        }
      }
      tar.close();
      if (m_tarName.isEmpty())
        ok = false;
      else
      {
         m_tarName.prepend(m_tempDir->name());
         connect(Security::ref(), SIGNAL(validityResult(int)), this, SLOT(slotValidated(int)));
         Security::ref()->checkValidity(m_tarName);
      }
  } else
      ok = false;
  if (!ok)
    KMessageBox::error(parentWidget(), i18n("There was an error with the downloaded resource tarball file. Possible causes are damaged archive or invalid directory structure in the archive."), i18n("Resource Installation Error"));
  return ok;
}

void KNewStuffSecure::slotValidated(int result)
{
   QString errorString;
   QString signatureStr;
   bool valid = true;
   if (result == -1)
   {
     errorString ="<br>-    " +  i18n("No keys were found.");
     valid = false;
   } else
   if (result == 0)
   {
       errorString ="<br>-    " +  i18n("The validation failed for unknown reason.");
       valid = false;
   } else
   {
      KeyStruct key = Security::ref()->signatureKey();
      if (!(result & Security::MD5_OK ))
      {
          errorString = "<br>-    " + i18n("The MD5SUM check failed, the archive might be broken.");
          valid = false;
      }
      if (result & Security::SIGNED_BAD)
      {
          errorString += "<br>-    " + i18n("The signature is bad, the archive might be broken or altered.");
          valid = false;
      }
      if (result & Security::SIGNED_OK)
      {
         if (result & Security::TRUSTED)
         {
            kDebug() << "Signed and trusted " << endl;
         } else
         {
            errorString += "<br>-    " + i18n("The signature is valid, but untrusted.");
            valid = false;
         }
      }
      if (result & Security::UNKNOWN)
      {
          errorString += "<br>-    " + i18n("The signature is unknown.");
          valid = false;
      } else
      {
          signatureStr = i18n("The resource was signed with key <i>0x%1</i>, belonging to <i>%2 &lt;%3&gt;</i>.").arg(key.id.right(8)).arg(key.name).arg(key.mail);
      }
   }
  if (!valid)
  {
      signatureStr.prepend( "<br>");
      if (KMessageBox::warningContinueCancel(parentWidget(), i18n("<qt>There is a problem with the resource file you have downloaded. The errors are :<b>%1</b><br>%2<br><br>Installation of the resource is <b>not recommended</b>.<br><br>Do you want to proceed with the installation?</qt>").arg(errorString).arg(signatureStr), i18n("Problematic Resource File")) == KMessageBox::Continue)
          valid = true;
  } else
    KMessageBox::information(parentWidget(), i18n("<qt>%1<br><br>Press OK to install it.</qt>").arg(signatureStr), i18n("Valid Resource"), "Show Valid Signature Information");
  if (valid)
  {
     installResource();
     emit installFinished();
  } else
  {
    KConfig *cfg = KGlobal::config();
    cfg->deleteGroup("KNewStuffStatus");
    cfg->setGroup("KNewStuffStatus");
    for (QMap<QString, QString>::ConstIterator it = m_installedResources.constBegin(); it != m_installedResources.constEnd(); ++it)
    {
      cfg->writeEntry(it.key(), it.data());
    }
    cfg->sync();
  }
  removeTempDirectory();
  disconnect(Security::ref(), SIGNAL(validityResult(int)), this, SLOT(slotValidated(int)));
}

void KNewStuffSecure::downloadResource()
{
  KConfig *cfg = KGlobal::config();
  m_installedResources = cfg->entryMap("KNewStuffStatus");
  engine()->ignoreInstallResult(true);
  KNewStuff::download();
}

bool KNewStuffSecure::createUploadFile(const QString &fileName)
{
  Q_UNUSED(fileName);
  return true;
}

void KNewStuffSecure::uploadResource(const QString& fileName)
{
  connect(Security::ref(), SIGNAL(fileSigned(int)), this, SLOT(slotFileSigned(int)));
  removeTempDirectory();
  m_tempDir = new KTempDir();
  m_tempDir->setAutoDelete(true);
  QFileInfo f(fileName);
  m_signedFileName = m_tempDir->name() + "/" + f.fileName();
  KIO::NetAccess::file_copy(KUrl::fromPathOrURL(fileName), KUrl::fromPathOrURL(m_signedFileName), -1, true);
  Security::ref()->signFile(m_signedFileName);
}

void KNewStuffSecure::slotFileSigned(int result)
{
  if (result == 0)
  {
    KMessageBox::error(parentWidget(), i18n("The signing failed for unknown reason."));
  } else
  {
    if (result & Security::BAD_PASSPHRASE)
    {
      if (KMessageBox::warningContinueCancel(parentWidget(), i18n("There are no keys usable for signing or you did not entered the correct passphrase.\nProceed without signing the resource?")) == KMessageBox::Cancel)
      {
        disconnect(Security::ref(), SIGNAL(fileSigned(int)), this, SLOT(slotFileSigned(int)));
        removeTempDirectory();
        return;
      }
    }
    KTar tar(m_signedFileName + ".signed", "application/x-gzip");
    tar.open(QIODevice::WriteOnly);
    QStringList files;
    files << m_signedFileName;
    files << m_tempDir->name() + "/md5sum";
    files << m_tempDir->name() + "/signature";

    for (QStringList::Iterator it_f = files.begin(); it_f != files.end(); ++it_f)
    {
      QFile file(*it_f);
      file.open(QIODevice::ReadOnly);
      QByteArray bArray = file.readAll();
      tar.writeFile(QFileInfo(file).fileName(), "user", "group", bArray.data(), bArray.size());
      file.close();
    }
    tar.close();
    KIO::NetAccess::file_move(KUrl::fromPathOrURL(m_signedFileName + ".signed"), KUrl::fromPathOrURL(m_signedFileName), -1, true);
    KNewStuff::upload(m_signedFileName, QString());
    disconnect(Security::ref(), SIGNAL(fileSigned(int)), this, SLOT(slotFileSigned(int)));
  }
}

void KNewStuffSecure::slotUploadFinished(bool result)
{
  Q_UNUSED(result);
  removeTempDirectory();
}

void KNewStuffSecure::removeTempDirectory()
{
  if (m_tempDir)
  {
    KIO::NetAccess::del(KUrl().fromPathOrURL(m_tempDir->name()), parentWidget());
    delete m_tempDir;
    m_tempDir = 0L;
  }
}

#include "knewstuffsecure.moc"
