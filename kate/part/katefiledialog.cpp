/* This file is part of the KDE libraries
  Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>

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

// $Id$

#include "katefiledialog.h"

#include <kcombobox.h>
#include <ktoolbar.h>
#include <kglobal.h>
#include <kcharsets.h>
#include <qstringlist.h>
#include <qtextcodec.h>

KateFileDialog::KateFileDialog (const QString& startDir,
                    const QString& encoding,
                    QWidget *parent,
                    const QString& caption,
                    KFileDialog::OperationMode opMode )
  : KFileDialog (startDir, QString::null, parent, "", true)
{
  QString sEncoding (encoding);

  setCaption (caption);

  toolBar()->insertCombo(QStringList(), 33333, false, 0L, 0L, 0L, true);

  setOperationMode( opMode );
  if (opMode == Opening)
    setMode(KFile::Files);
  else
    setMode(KFile::File);

  m_encoding = toolBar()->getCombo(33333);

  m_encoding->clear ();
  QStringList encodings (KGlobal::charsets()->availableEncodingNames());
  int insert = 0;
  for (uint i=0; i < encodings.count(); i++)
  {
    bool found = false;
    QTextCodec *codecForEnc = KGlobal::charsets()->codecForName(encodings[i], found);

    if (found)
    {
      m_encoding->insertItem (encodings[i]);

      if ( codecForEnc->name() == encoding )
      {
        m_encoding->setCurrentItem(insert);
      }

      insert++;
    }
  }
}

KateFileDialog::~KateFileDialog ()
{

}

KateFileDialogData KateFileDialog::exec()
{
  int n = KDialogBase::exec();

  KateFileDialogData data = KateFileDialogData ();

  if (n)
  {
    data.encoding = m_encoding->currentText();
    data.url = selectedURL ();
    data.urls = selectedURLs ();
  }

  return data;
}

void KateFileDialog::slotApply()
{

}

#include "katefiledialog.moc"
