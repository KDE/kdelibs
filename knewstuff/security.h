/***************************************************************************
                          security.h  -  description
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


#ifndef SECURITY_H
#define SECURITY_H

//qt includes
#include <qmap.h>
#include <qobject.h>

class KProcIO;
class KProcess;

struct KeyStruct {
   QString id;
   QString name;
   QString mail;
   bool trusted;
   bool secret;
};

/**
Handles security releated issues, like signing, verifying.
It is a private class, not meant to be used by third party applications.

@author Andras Mantia <amantia@kde.org>
*/

namespace KNS {

class Security : public QObject
{
Q_OBJECT
public:
  static Security* const ref()
  {
    static Security *m_ref;
    if (!m_ref) m_ref = new Security();
    return m_ref;
  }
  ~Security();


  /** Verifies the integrity and the signature of a tarball file.
   * @param fileName the file to be verified. It should be a tar.gz (.tgz) file. The directory where
   *               the file is should contain a "signature" and a "md5sum" file, otherwise verification will fail.
   *               The method is asynchronous and the result is signalled with @ref validityResult.
   */
  void checkValidity(const QString &fileName);

  /** Creates a signature and an md5sum file for the fileName and packs
   * everything into a gzipped tarball.
   * @param fileName the file with full path to sign
   *
   * The method is asynchronous and the result is signalled with @ref fileSigned.
   */
  void signFile(const QString &fileName);
  /** Get the key used for signing. This method is valid only if:
  *  - the checkValidity was called
  *  - the result of the validity check does not have the UNKNOWN bit set
  *
  *  @return the key used for signing the file
  */
  KeyStruct signatureKey() {return m_signatureKey;}

  enum Results {
     MD5_OK = 1, /// The MD5 sum check is OK
     SIGNED_OK = 2, /// The file is signed with a good signature
     SIGNED_BAD = 4, /// The file is signed with a bad signature
     TRUSTED = 8, /// The signature is trusted
     UNKNOWN = 16, ///The key is unknown
     SIGNED_BAD_CLEAR = 27, ///used to clear the SIGNED_BAD flag
     BAD_PASSPHRASE = 32 ///wrong passhprase entered
  };

public Q_SLOTS:
  /** Reads the available public keys */
  void readKeys();
  /** Reads the available secret keys */
  void readSecretKeys();
  /** Verifies the integrity and the signature of a tarball file (see m_fileName).
   */
  void slotCheckValidity();

  /** Creates a signature and an md5sum file for the m_fileName and packs
   * everything into a gzipped tarball.
   */
  void slotSignFile();

private:
   Security();

   enum RunMode {
       List = 0, ///read the public keys
       ListSecret, ///read the secret keys
       Verify, ///verify the signature
       Sign ///create signature
   };

   KeyStruct m_signatureKey;
   int m_result;
   int m_runMode;
   bool m_gpgRunning; /// true if gpg is currently running
   bool m_keysRead; /// true if all the keys were read
   QMap<QString, KeyStruct> m_keys; /// holds information about the available key
   QString m_fileName; /// the file to sign/verify
   QString m_secretKey; /// the key used for signing

private Q_SLOTS:
   void slotProcessExited(KProcess *process);
   void slotDataArrived(KProcIO *process);

Q_SIGNALS:
   /** Sent when the validity check is done.
   *
   *    @return the result of the check. See @ref Results
   */
    void validityResult(int result);
   /** Sent when the signing is done.
     *
     *    @return the result of the operation. See @ref Results
    */
    void fileSigned(int result);
};

}

#endif
