/* This file is part of the KDE project
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999 Lars Knoll <knoll@kde.org>
 *                     1999 Antti Koivisto <koivisto@kde.org>
 *                     2000 Simon Hausmann <hausmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include "khtml_run.h"
#include "khtml_part.h"
#include <kio/job.h>

KHTMLRun::KHTMLRun( KHTMLPart *part, khtml::ChildFrame *child, const KURL &url, KParts::URLArgs *args )
: KRun( url, 0, false, false /* No GUI */ )
{
  m_part = part;
  m_child = child;
  m_args = args;
}

void KHTMLRun::foundMimeType( const QString &mimetype )
{
  m_part->processObjectRequest( m_child, m_strURL, mimetype );
  m_bFinished = true;
  m_timer.start( 0, true );
}

void KHTMLRun::scanFile()
{
  if (m_strURL.protocol().left(4) != "http") // http and https
  {
     KRun::scanFile();
     return;
  }

  // No check for well-known extensions, since we don't trust HTTP

  KIO::TransferJob *job = KIO::get(m_strURL, false, false);
  job->addMetaData(m_args->metaData());

  //job->setWindow((KMainWindow *)m_pMainWindow);
  connect( job, SIGNAL( result( KIO::Job *)),
           this, SLOT( slotKHTMLScanFinished(KIO::Job *)));
  connect( job, SIGNAL( mimetype( KIO::Job *, const QString &)),
           this, SLOT( slotKHTMLMimetype(KIO::Job *, const QString &)));
  m_job = job;
}

void KHTMLRun::slotKHTMLScanFinished(KIO::Job *job)
{
  KRun::slotScanFinished(job);
}

void KHTMLRun::slotKHTMLMimetype(KIO::Job *, const QString &type)
{
  KIO::SimpleJob *job = (KIO::SimpleJob *) m_job;
  // Update our URL in case of a redirection
  m_strURL = job->url();

  // Make copy to avoid dead reference
  QString _type = type;
  job->putOnHold();
  m_job = 0;

  foundMimeType( _type );
}


#include "khtml_run.moc"
