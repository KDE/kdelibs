/* This file is part of the KDE libraries
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <dcopclient.h>

#include "kuniqueapp.moc"

KUniqueApplication::KUniqueApplication(int& argc, char** argv,
				       const QCString& rAppName,
                                       bool allowStyles)
  : KApplication(argc, argv, rAppName, allowStyles), DCOPObject(name())
{
  DCOPClient *dc = dcopClient();
  if (dc->attach()) {
    if (name() != 0L) {
      if (dc->isApplicationRegistered(name())) {
	QByteArray data;
	QDataStream ds(data, IO_WriteOnly);
	QValueList<QCString> params;
	for (int i = 0; i < argc; i++)
	  params.append(argv[i]);

	ds << params;
	dc->send(name(), name(), "newInstance(QValueList<QCString>)", data);
	::exit(0);
      }
    }
  }
  dc->registerAs(name());
}

KUniqueApplication::~KUniqueApplication()
{
}

bool KUniqueApplication::process(const QCString &fun, const QByteArray &data,
				 QCString &replyType, QByteArray &/*reply*/)
{
  if (fun == "newInstance(QValueList<QCString>)") {
    QDataStream ds(data, IO_ReadOnly);
    QValueList<QCString> params;
    ds >> params;
    newInstance(params);
    replyType = "void";
    return true;
  } else
    return false;
}

void KUniqueApplication::newInstance(QValueList<QCString> params)
{
  return; // do nothing in default implementation
}
