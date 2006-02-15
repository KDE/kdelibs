// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
                  2003 Andras Mantia <amantia@freemail.hu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config-kfile.h"

#include "kencodingfiledialog.h"
#include <kcombobox.h>
#include <ktoolbar.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcharsets.h>
#include <qtextcodec.h>
#include <kdiroperator.h>
#include <krecentdocument.h>

struct KEncodingFileDialogPrivate
{
    KComboBox *encoding;
};

KEncodingFileDialog::KEncodingFileDialog(const QString& startDir, const QString& encoding , const QString& filter,
			 const QString& caption, KFileDialog::OperationMode type, QWidget *parent)
   : KFileDialog(startDir,filter,parent), d(new KEncodingFileDialogPrivate)
{
  setCaption(caption);
  
  setOperationMode( type );
    
  KToolBar *tb = toolBar();
  tb->insertSeparator();
  int index = tb->insertCombo(QStringList(), -1 /*id*/, false /*writable*/, 0 /*signal*/, 0 /*receiver*/, 0 /*slot*/ );
  d->encoding = tb->getCombo( tb->idAt( index ) );
  if ( !d->encoding )
      return;

  d->encoding->clear ();
  QString sEncoding = encoding;
  if (sEncoding.isEmpty())
     sEncoding = QLatin1String(KGlobal::locale()->encoding());
  
  QStringList encodings (KGlobal::charsets()->availableEncodingNames());
  int insert = 0;
  for (int i=0; i < encodings.count(); i++)
  {
    bool found = false;
    QTextCodec *codecForEnc = KGlobal::charsets()->codecForName(encodings[i], found);

    if (found)
    {
      d->encoding->addItem (encodings[i]);
      if ( (codecForEnc->name() == sEncoding) || (encodings[i] == sEncoding) )
      {
        d->encoding->setCurrentIndex(insert);
      }

      insert++;
    }
  }
        
     
}

KEncodingFileDialog::~KEncodingFileDialog()
{
    delete d;
}


QString KEncodingFileDialog::selectedEncoding() const
{
  if (d->encoding)
     return d->encoding->currentText();
  else
    return QString();     
}


KEncodingFileDialog::Result KEncodingFileDialog::getOpenFileNameAndEncoding(const QString& encoding,
 				     const QString& startDir,
                                     const QString& filter,
                                     QWidget *parent, const QString& caption)
{
    KEncodingFileDialog dlg(startDir, encoding,filter,caption.isNull() ? i18n("Open") : caption,Opening,parent);

    dlg.setMode( KFile::File | KFile::LocalOnly );
    dlg.ops->clearHistory();
    dlg.exec();
 
    Result res;
    res.fileNames<<dlg.selectedFile();
    res.encoding=dlg.selectedEncoding();	
    return res;
}

KEncodingFileDialog::Result KEncodingFileDialog::getOpenFileNamesAndEncoding(const QString& encoding,
					  const QString& startDir,
                                          const QString& filter,
                                          QWidget *parent,
                                          const QString& caption)
{
    KEncodingFileDialog dlg(startDir, encoding,filter,caption.isNull() ? i18n("Open") : caption,Opening,parent);
    dlg.setMode(KFile::Files | KFile::LocalOnly);
    dlg.ops->clearHistory();
    dlg.exec();

    Result res;
    res.fileNames=dlg.selectedFiles();
    res.encoding=dlg.selectedEncoding();
    return res;
}

KEncodingFileDialog::Result KEncodingFileDialog::getOpenURLAndEncoding(const QString& encoding, const QString& startDir, 
				const QString& filter, QWidget *parent, const QString& caption)
{
    KEncodingFileDialog dlg(startDir, encoding,filter,caption.isNull() ? i18n("Open") : caption,Opening,parent);

    dlg.setMode( KFile::File );
    dlg.ops->clearHistory();
    dlg.exec();

    Result res;
    res.URLs<<dlg.selectedURL();
    res.encoding=dlg.selectedEncoding();
    return res;
}

KEncodingFileDialog::Result KEncodingFileDialog::getOpenURLsAndEncoding(const QString& encoding, const QString& startDir,
                                          const QString& filter,
                                          QWidget *parent,
                                          const QString& caption)
{
    KEncodingFileDialog dlg(startDir, encoding,filter,caption.isNull() ? i18n("Open") : caption,Opening,parent);

    dlg.setMode(KFile::Files);
    dlg.ops->clearHistory();
    dlg.exec();

    Result res;
    res.URLs=dlg.selectedURLs();
    res.encoding=dlg.selectedEncoding();
    return res;
}


KEncodingFileDialog::Result KEncodingFileDialog::getSaveFileNameAndEncoding(const QString& encoding,
			             const QString& dir, 
				     const QString& filter,
                                     QWidget *parent,
                                     const QString& caption)
{
    bool specialDir = dir.at(0) == ':';
    KEncodingFileDialog dlg(specialDir?dir:QString(), encoding,filter,caption.isNull() ? i18n("Save As") : caption);
    dlg.setMode(KFile::File);

    if ( !specialDir )
        dlg.setSelection( dir ); // may also be a filename
    dlg.exec();

    QString filename = dlg.selectedFile();
    if (!filename.isEmpty())
        KRecentDocument::add(filename);

    Result res;
    res.fileNames<<filename;
    res.encoding=dlg.selectedEncoding();
    return res;
}


KEncodingFileDialog::Result  KEncodingFileDialog::getSaveURLAndEncoding(const QString& encoding,
			     const QString& dir, const  QString& filter,
                             QWidget *parent, const QString& caption)
{
    bool specialDir = !dir.isEmpty() && dir.at(0) == ':';
    KEncodingFileDialog dlg(specialDir?dir:QString(), encoding,filter,caption.isNull() ? i18n("Save As") : 
	caption, Saving,parent);
    dlg.setMode(KFile::File);
    if ( !specialDir )
    dlg.setSelection( dir ); // may also be a filename

    dlg.exec();

    KUrl url = dlg.selectedURL();
    if (url.isValid())
        KRecentDocument::add( url );

    Result res;
    res.URLs<<url;
    res.encoding=dlg.selectedEncoding();
    return res;
}



void KEncodingFileDialog::virtual_hook( int id, void* data ) 
{
 KFileDialog::virtual_hook( id, data ); 
}


#include "kencodingfiledialog.moc"
