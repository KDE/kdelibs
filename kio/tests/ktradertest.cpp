/*
 *  Copyright (C) 2002, 2003 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <kcmdlineargs.h>
#include <ktrader.h>
#include <kmimetype.h>
#include <kapplication.h>
#include <klocale.h>
#include <stdio.h>

static KCmdLineOptions options[] =
{
  { "+query", "the query", 0 },
  { "+[genericServiceType]", "Application (default), or KParts/ReadOnlyPart", 0 },
  { "+[constraint]", "constraint", 0 },
  { "+[preference]", "preference", 0 },
  KCmdLineLastOption
};

int main( int argc, char **argv )
{
  KCmdLineArgs::init( argc, argv, "ktradertest", I18N_NOOP("KTradertest"), I18N_NOOP("A testing tool for the KDE trader system"), "0.0" );

  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app( false, false ); // no GUI

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if ( args->count() < 1 )
      KCmdLineArgs::usage();

  QString query = QString::fromLocal8Bit( args->arg( 0 ) );

  QString genericServiceType, constraint, preference;

  if ( args->count() >= 2 )
    genericServiceType = QString::fromLocal8Bit( args->arg( 1 ) );

  if ( args->count() >= 3 )
    constraint = QString::fromLocal8Bit( args->arg( 2 ) );

  if ( args->count() == 4 )
    preference = QString::fromLocal8Bit( args->arg( 3 ) );

  printf( "query is : %s\n", query.toLocal8Bit().data() );
  printf( "genericServiceType is : %s\n", genericServiceType.toLocal8Bit().data() );
  printf( "constraint is : %s\n", constraint.toLocal8Bit().data() );
  printf( "preference is : %s\n", preference.toLocal8Bit().data() );

  KTrader::OfferList offers = KTrader::self()->query( query, genericServiceType, constraint, preference );

  printf("got %d offers.\n", offers.count());

  int i = 0;
  KTrader::OfferList::ConstIterator it = offers.begin();
  KTrader::OfferList::ConstIterator end = offers.end();
  for (; it != end; ++it, ++i )
  {
    printf("---- Offer %d ----\n", i);
    QStringList props = (*it)->propertyNames();
    QStringList::ConstIterator propIt = props.begin();
    QStringList::ConstIterator propEnd = props.end();
    for (; propIt != propEnd; ++propIt )
    {
      QVariant prop = (*it)->property( *propIt );

      if ( !prop.isValid() )
      {
        printf("Invalid property %s\n", (*propIt).toLocal8Bit().data());
	continue;
      }

      QString outp = *propIt;
      outp += " : '";

      switch ( prop.type() )
      {
        case QVariant::StringList:
          outp += prop.toStringList().join(" - ");
        break;
        case QVariant::Bool:
          outp += prop.toBool() ? "TRUE" : "FALSE";
          break;
        default:
          outp += prop.toString();
        break;
      }

      if ( !outp.isEmpty() )
        printf("%s'\n", outp.toLocal8Bit().data());
    }
  }
}
