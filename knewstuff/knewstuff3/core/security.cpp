/*
    This file is part of KNewStuff2.
    Copyright (c) 2004, 2005 Andras Mantia <amantia@kde.org>
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

//app includes
#include "security.h"

//qt includes
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QPointer>
#include <QtCore/QStringList>
#include <QtCore/QTextIStream>
#include <QtCore/QTimer>

//kde includes
#include <kdebug.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kcodecs.h>
#include <kmessagebox.h>
#include <kpassworddialog.h>
#include <kprocess.h>

using namespace KNS3;

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
    if (m_gpgRunning) {
        QTimer::singleShot(5, this, SLOT(readKeys()));
        return;
    }
    m_runMode = List;
    m_keys.clear();
    m_process = new KProcess();
    *m_process << "gpg"
    << "--no-secmem-warning"
    << "--no-tty"
    << "--with-colon"
    << "--list-keys";
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotFinished(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(readyReadStandardOutput()),
            this, SLOT(slotReadyReadStandardOutput()));
    m_process->start();
    if (!m_process->waitForStarted()) {
        KMessageBox::error(0L, i18n("<qt>Cannot start <i>gpg</i> and retrieve the available keys. Make sure that <i>gpg</i> is installed, otherwise verification of downloaded resources will not be possible.</qt>"));
        delete m_process;
        m_process = 0;
    } else
        m_gpgRunning = true;
}

void Security::readSecretKeys()
{
    if (m_gpgRunning) {
        QTimer::singleShot(5, this, SLOT(readSecretKeys()));
        return;
    }
    m_runMode = ListSecret;
    m_process = new KProcess();
    *m_process << "gpg"
    << "--no-secmem-warning"
    << "--no-tty"
    << "--with-colon"
    << "--list-secret-keys";
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotFinished(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(readyReadStandardOutput()),
            this, SLOT(slotReadyReadStandardOutput()));
    m_process->start();
    if (!m_process->waitForStarted()) {
        delete m_process;
        m_process = 0;
    } else
        m_gpgRunning = true;
}

void Security::slotFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus != QProcess::NormalExit) {
        m_gpgRunning = false;
        delete m_process;
        m_process = 0;
        return;
    }
    switch (m_runMode) {
    case ListSecret:
        m_keysRead = true;
        break;
    case Verify: emit validityResult(m_result);
        break;
    case Sign:   emit fileSigned(m_result);
        break;

    }
    m_gpgRunning = false;
    delete m_process;
    m_process = 0;

    Q_UNUSED(exitCode)
}

void Security::slotReadyReadStandardOutput()
{
    QString data;
    while (m_process->canReadLine()) {
        data = QString::fromLocal8Bit(m_process->readLine());
        switch (m_runMode) {
        case List:
        case ListSecret:
            if (data.startsWith(QLatin1String("pub")) || data.startsWith(QLatin1String("sec"))) {
                KeyStruct key;
                if (data.startsWith(QLatin1String("pub")))
                    key.secret = false;
                else
                    key.secret = true;
                QStringList line = data.split(':', QString::KeepEmptyParts);
                key.id = line[4];
                QString shortId = key.id.right(8);
                QString trustStr = line[1];
                key.trusted = false;
                if (trustStr == "u" || trustStr == "f")
                    key.trusted = true;
                data = line[9];
                key.mail = data.section('<', -1, -1);
                key.mail.truncate(key.mail.length() - 1);
                key.name = data.section('<', 0, 0);
                if (key.name.contains("("))
                    key.name = key.name.section('(', 0, 0);
                m_keys[shortId] = key;
            }
            break;
        case Verify:
            data = data.section(']', 1, -1).trimmed();
            if (data.startsWith(QLatin1String("GOODSIG"))) {
                m_result &= SIGNED_BAD_CLEAR;
                m_result |= SIGNED_OK;
                QString id = data.section(' ', 1 , 1).right(8);
                if (!m_keys.contains(id)) {
                    m_result |= UNKNOWN;
                } else {
                    m_signatureKey = m_keys[id];
                }
            } else
                if (data.startsWith(QLatin1String("NO_PUBKEY"))) {
                    m_result &= SIGNED_BAD_CLEAR;
                    m_result |= UNKNOWN;
                } else
                    if (data.startsWith(QLatin1String("BADSIG"))) {
                        m_result |= SIGNED_BAD;
                        QString id = data.section(' ', 1 , 1).right(8);
                        if (!m_keys.contains(id)) {
                            m_result |= UNKNOWN;
                        } else {
                            m_signatureKey = m_keys[id];
                        }
                    } else
                        if (data.startsWith(QLatin1String("TRUST_ULTIMATE"))) {
                            m_result &= SIGNED_BAD_CLEAR;
                            m_result |= TRUSTED;
                        }
            break;

        case Sign:
            if (data.contains("passphrase.enter")) {
                KeyStruct key = m_keys[m_secretKey];
                QPointer<KPasswordDialog> dlg = new KPasswordDialog(NULL);
                dlg->setPrompt(i18n("<qt>Enter passphrase for key <b>0x%1</b>, belonging to<br /><i>%2&lt;%3&gt;</i><br />:</qt>", m_secretKey, key.name, key.mail));
                if (dlg->exec()) {
                    m_process->write(dlg->password().toLocal8Bit() + '\n');
                } else {
                    m_result |= BAD_PASSPHRASE;
                    m_process->kill();
                    return;
                }
            } else
                if (data.contains("BAD_PASSPHRASE")) {
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
    if (!m_keysRead || m_gpgRunning) {
        QTimer::singleShot(5, this, SLOT(slotCheckValidity()));
        return;
    }
    if (m_keys.count() == 0) {
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
    if (file.open(QIODevice::ReadOnly)) {
        context.reset();
        context.update(file);
        md5sum = context.hexDigest();
        file.close();
    }
    file.setFileName(f.path() + "/md5sum");
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray md5sum_file;
        file.readLine(md5sum_file.data(), 50);
        if (!md5sum_file.isEmpty() && QString(md5sum_file).startsWith(md5sum))
            m_result |= MD5_OK;
        file.close();
    }
    m_result |= SIGNED_BAD;
    m_signatureKey.id = "";
    m_signatureKey.name = "";
    m_signatureKey.mail = "";
    m_signatureKey.trusted = false;

    //verify the signature
    m_process = new KProcess();
    *m_process << "gpg"
    << "--no-secmem-warning"
    << "--status-fd=2"
    << "--command-fd=0"
    << "--verify"
    << f.path() + "/signature"
    << m_fileName;
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotFinished(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(readyReadStandardOutput()),
            this, SLOT(slotReadyReadStandardOutput()));
    m_process->start();
    if (m_process->waitForStarted())
        m_gpgRunning = true;
    else {
        KMessageBox::error(0L, i18n("<qt>Cannot start <i>gpg</i> and check the validity of the file. Make sure that <i>gpg</i> is installed, otherwise verification of downloaded resources will not be possible.</qt>"));
        emit validityResult(0);
        delete m_process;
        m_process = 0;
    }
}

void Security::signFile(const QString &fileName)
{
    m_fileName = fileName;
    slotSignFile();
}

void Security::slotSignFile()
{
    if (!m_keysRead || m_gpgRunning) {
        QTimer::singleShot(5, this, SLOT(slotSignFile()));
        return;
    }

    QStringList secretKeys;
    for (QMap<QString, KeyStruct>::Iterator it = m_keys.begin(); it != m_keys.end(); ++it) {
        if (it.value().secret)
            secretKeys.append(it.key());
    }

    if (secretKeys.count() == 0) {
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
    if (file.open(QIODevice::ReadOnly)) {
        context.reset();
        context.update(file);
        md5sum = context.hexDigest();
        file.close();
    }
    file.setFileName(f.path() + "/md5sum");
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << md5sum;
        m_result |= MD5_OK;
        file.close();
    }

    if (secretKeys.count() > 1) {
        bool ok;
        secretKeys = KInputDialog::getItemList(i18n("Select Signing Key"), i18n("Key used for signing:"), secretKeys, QStringList(secretKeys[0]), false, &ok);
        if (ok)
            m_secretKey = secretKeys[0];
        else {
            emit fileSigned(0);
            return;
        }
    } else
        m_secretKey = secretKeys[0];

    //verify the signature
    m_process = new KProcess();
    *m_process << "gpg"
    << "--no-secmem-warning"
    << "--status-fd=2"
    << "--command-fd=0"
    << "--no-tty"
    << "--detach-sign"
    << "-u"
    << m_secretKey
    << "-o"
    << f.path() + "/signature"
    << m_fileName;
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotFinished(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(readyReadStandardOutput()),
            this, SLOT(slotReadyReadStandardOutput()));
    m_runMode = Sign;
    m_process->start();
    if (m_process->waitForStarted())
        m_gpgRunning = true;
    else {
        KMessageBox::error(0L, i18n("<qt>Cannot start <i>gpg</i> and sign the file. Make sure that <i>gpg</i> is installed, otherwise signing of the resources will not be possible.</qt>"));
        emit fileSigned(0);
        delete m_process;
        m_process = 0;
    }
}

#include "security.moc"
