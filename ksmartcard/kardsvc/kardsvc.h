/*
  This file is part of the KDE libraries

  Copyright (c) 2001 George Staikos <staikos@kde.org>
  Copyright (C) 2001 Fernando Llobregat <fernando.llobregat@free.fr>

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
#ifndef __KardSvc_H__
#define __KardSvc_H__

#include <kded/kdedmodule.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>


class KPCSC;
class KCardStatus;
class KSimpleConfig;
class QTimer;


class KardSvc : public KDEDModule
{
  Q_OBJECT
  K_DCOP

public:

  KardSvc(const QCString &name);

  virtual ~KardSvc();



k_dcop:

  QStringList getSlotList();
  bool isCardPresent(QString slot);
  QString getCardATR(QString slot);
  void reconfigure();


 k_dcop_signals:

  void signalReaderListChanged(QStringList newReadersList);
  void signalCardStateChanged(QString reader,
                              bool cardInserted,
                              QString cardATR);

 private slots:
  void poll();




 private:

  void emitreaderListChanged();


  KPCSC *_pcsc;
  KSimpleConfig *_cfg;
  QTimer *_timer;
  QStringList _readers;
  QMap<QString,KCardStatus> _states;
  bool _beepOnEvent, _enablePolling, _launchManager;

};


#endif
