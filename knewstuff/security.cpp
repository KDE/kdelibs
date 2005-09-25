/***************************************************************************
                          security.cpp  -  description
                             -------------------
    begin                : Thu Jun 24 11:22:12 2004
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
#include <qfile.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtimer.h>

 //kde includes
#include <kdebug.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kcodecs.h>
#include <kmessagebox.h>
#include <kpassdlg.h>
#include <kprocio.h>

 //app includes
#include "security.h"

using namespace KNS;

Security::Security()
{
   m_keysRead = false;
   m_gpgRunning = false;
   readKeys();
   readSecretKeys();
}


Security::~Security()
{
}

void Security::readKeys()
{
  if (m_gpgRunning)
  {
    QTimer::singleShot(5, this, SLOT(readKeys()));
    return;
  }
  m_runMode = List;
  m_keys.clear();
  KProcIO *readProcess=new KProcIO();
  *readProcess << "gpg"<<"--no-secmem-warning"<<"--no-tty"<<"--with-colon"<<"--list-keys";
  connect(readProcess, SIGNAL(processExited(KProcess *)), this, SLOT(slotProcessExited(KProcess *)));
  connect(readProcess, SIGNAL(readReady(KProcIO *)) ,this, SLOT(slotDataArrived(KProcIO *)));
  if (!readProcess->start(KProcess::NotifyOnExit, true))
    KMessageBox::error(0L, i18n("<qt>Cannot start <i>gpg</i> and retrieve the available keys. Make sure that <i>gpg</i> is installed, otherwise verification of downloaded resources will not be possible.</qt>"));
  else
    m_gpgRunning = true;
}

void Security::readSecretKeys()
{
  if (m_gpgRunning)
  {
    QTimer::singleShot(5, this, SLOT(readSecretKeys()));
    return;
  }
  m_runMode = ListSecret;
  KProcIO *readProcess=new KProcIO();
  *readProcess << "gpg"<<"--no-secmem-warning"<<"--no-tty"<<"--with-colon"<<"--list-secret-keys";
  connect(readProcess, SIGNAL(processExited(KProcess *)), this, SLOT(slotProcessExited(KProcess *)));
  connect(readProcess, SIGNAL(readReady(KProcIO *)) ,this, SLOT(slotDataArrived(KProcIO *)));
  if (readProcess->start(KProcess::NotifyOnExit, true))
    m_gpgRunning = true;  
}

void Security::slotProcessExited(KProcess *process)
{
  switch (m_runMode)
   {
     case ListSecret:
                  m_keysRead = true;
                  break;
     case Verify: emit validityResult(m_result);
                  break;
     case Sign:   emit fileSigned(m_result);
                  break;

   }
   m_gpgRunning = false;
   delete process;
}

void Security::slotDataArrived(KProcIO *procIO)
{
  QString data;
  while (procIO->readln(data, true) != -1)
  {
     switch (m_runMode)
     {
        case List:
        case ListSecret:  
          if (data.startsWith("pub") || data.startsWith("sec"))
          {
              KeyStruct key;
              if (data.startsWith("pub"))
                key.secret = false;
              else
                key.secret = true;
              QStringList line = QStringList::split(":", data, true);
              key.id = line[4];
              QString shortId = key.id.right(8);
              QString trustStr = line[1];
              key.trusted = false;
              if (trustStr == "u" || trustStr == "f")
                  key.trusted = true;
              data = line[9];
              key.mail=data.section('<', -1, -1);
              key.mail.truncate(key.mail.length() - 1);
              key.name=data.section('<',0,0);
              if (key.name.find("(")!=-1)
                  key.name=key.name.section('(',0,0);
              m_keys[shortId] = key;
          }
          break;
       case Verify:
          data = data.section("]",1,-1).trimmed();
          if (data.startsWith("GOODSIG"))
          {
              m_result &= SIGNED_BAD_CLEAR;
              m_result |= SIGNED_OK;
              QString id = data.section(" ", 1 , 1).right(8);
              if (!m_keys.contains(id))
              {
                  m_result |= UNKNOWN;
              } else
              {
                 m_signatureKey = m_keys[id];
              }
          } else
          if (data.startsWith("NO_PUBKEY"))
          {
              m_result &= SIGNED_BAD_CLEAR;
              m_result |= UNKNOWN;
          } else
          if (data.startsWith("BADSIG"))
          {
              m_result |= SIGNED_BAD;
              QString id = data.section(" ", 1 , 1).right(8);
              if (!m_keys.contains(id))
              {
                  m_result |= UNKNOWN;
              } else
              {
                 m_signatureKey = m_keys[id];
              }
          } else
          if (data.startsWith("TRUST_ULTIMATE"))
          {
            m_result &= SIGNED_BAD_CLEAR;
            m_result |= TRUSTED;
          }
          break;

       case Sign:
         if (data.find("passphrase.enter") != -1)
         {
           QByteArray password;
           KeyStruct key = m_keys[m_secretKey];
           int result = KPasswordDialog::getPassword((QWidget*)0,password, i18n("<qt>Enter passphrase for key <b>0x%1</b>, belonging to<br><i>%2&lt;%3&gt;</i>:</qt>").arg(m_secretKey).arg(key.name).arg(key.mail));
           if (result == KPasswordDialog::Accepted)
           {
             procIO->writeStdin(password, true);
             password.fill(' ');
           }
           else
           {
             m_result |= BAD_PASSPHRASE;
             slotProcessExited(procIO);
             return;
           }
         } else
         if (data.find("BAD_PASSPHRASE") != -1)
         {
           m_result |= BAD_PASSPHRASE;
         }
         break;
     }
  }
}

void Security::checkValidity(const QString& filename)
{
  m_fileName = filename;
  slotCheckValidity();
}  

void Security::slotCheckValidity()
{  
  if (!m_keysRead || m_gpgRunning)
  {
    QTimer::singleShot(5, this, SLOT(slotCheckValidity()));
    return;
  }
  if (m_keys.count() == 0)
  {    
    emit validityResult(-1);
    return;
  }  

  m_result = 0;
  m_runMode = Verify;
  QFileInfo f(m_fileName);
  //check the MD5 sum
  QString md5sum;
  const char* c = "";
  KMD5 context(c);
  QFile file(m_fileName);
  if (file.open(QIODevice::ReadOnly))
  {
     context.reset();
     context.update(file);
     md5sum = context.hexDigest();
     file.close();
  }
  file.setName(f.path() + "/md5sum");
  if (file.open(QIODevice::ReadOnly))
  {
     QByteArray md5sum_file;
     file.readLine(md5sum_file.data(), 50);
     if (!md5sum_file.isEmpty() && QString( md5sum_file ).startsWith(md5sum))
       m_result |= MD5_OK;
     file.close();
  }
  m_result |= SIGNED_BAD;
  m_signatureKey.id = "";
  m_signatureKey.name = "";
  m_signatureKey.mail = "";
  m_signatureKey.trusted = false;

  //verify the signature
  KProcIO *verifyProcess=new KProcIO();
  *verifyProcess<<"gpg"<<"--no-secmem-warning"<<"--status-fd=2"<<"--command-fd=0"<<"--verify" << f.path() + "/signature"<< m_fileName;
  connect(verifyProcess, SIGNAL(processExited(KProcess *)),this, SLOT(slotProcessExited(KProcess *)));
  connect(verifyProcess, SIGNAL(readReady(KProcIO *)),this, SLOT(slotDataArrived(KProcIO *)));
  if (verifyProcess->start(KProcess::NotifyOnExit,true))
      m_gpgRunning = true;
  else
  {
      KMessageBox::error(0L, i18n("<qt>Cannot start <i>gpg</i> and check the validity of the file. Make sure that <i>gpg</i> is installed, otherwise verification of downloaded resources will not be possible.</qt>"));
      emit validityResult(0);
      delete verifyProcess;
  }
}

void Security::signFile(const QString &fileName)
{
  m_fileName = fileName;
  slotSignFile();
}

void Security::slotSignFile()
{
  if (!m_keysRead || m_gpgRunning)
  {
    QTimer::singleShot(5, this, SLOT(slotSignFile()));
    return;
  }
  
  QStringList secretKeys;
  for (QMap<QString, KeyStruct>::Iterator it = m_keys.begin(); it != m_keys.end(); ++it)
  {
    if (it.data().secret)
      secretKeys.append(it.key());
  }
  
  if (secretKeys.count() == 0)
  {    
    emit fileSigned(-1);
    return;
  }  
  
  m_result = 0;
  QFileInfo f(m_fileName);

  //create the MD5 sum
  QString md5sum;
  const char* c = "";
  KMD5 context(c);
  QFile file(m_fileName);
  if (file.open(QIODevice::ReadOnly))
  {
    context.reset();
    context.update(file);
    md5sum = context.hexDigest();
    file.close();
  }
  file.setName(f.path() + "/md5sum");
  if (file.open(QIODevice::WriteOnly))
  {
    QTextStream stream(&file);
    stream << md5sum;
    m_result |= MD5_OK;
    file.close();
  }
  
  if (secretKeys.count() > 1)
  {
    bool ok;
    secretKeys = KInputDialog::getItemList(i18n("Select Signing Key"), i18n("Key used for signing:"), secretKeys, QStringList( secretKeys[0] ), false, &ok);    
    if (ok)
      m_secretKey = secretKeys[0];
    else
    {
      emit fileSigned(0);
      return;
    }
  } else
    m_secretKey = secretKeys[0];

  //verify the signature
  KProcIO *signProcess=new KProcIO();
  *signProcess<<"gpg"<<"--no-secmem-warning"<<"--status-fd=2"<<"--command-fd=0"<<"--no-tty"<<"--detach-sign" << "-u" << m_secretKey << "-o" << f.path() + "/signature" << m_fileName;
  connect(signProcess, SIGNAL(processExited(KProcess *)),this, SLOT(slotProcessExited(KProcess *)));
  connect(signProcess, SIGNAL(readReady(KProcIO *)),this, SLOT(slotDataArrived(KProcIO *)));
  m_runMode = Sign;
  if (signProcess->start(KProcess::NotifyOnExit,true))
    m_gpgRunning = true;
  else
  {
    KMessageBox::error(0L, i18n("<qt>Cannot start <i>gpg</i> and sign the file. Make sure that <i>gpg</i> is installed, otherwise signing of the resources will not be possible.</qt>"));
    emit fileSigned(0);
    delete signProcess;
  }
}

#include "security.moc"
