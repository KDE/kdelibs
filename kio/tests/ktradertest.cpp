
#include <kcmdlineargs.h>
#include <ktrader.h>
#include <kapplication.h>
#include <stdio.h>

static KCmdLineOptions options[] =
{
  { "+query", "the query", 0 },
  { "+[genericServiceType]", "Application (default), or KParts/ReadOnlyPart", 0 },
  { "+[constraint]", "constraint", 0 },
  { "+[preference]", "preference", 0 },
  { 0, 0, 0 }
};

int main( int argc, char **argv )
{
  KCmdLineArgs::init( argc, argv, "ktradertest", "A KTrader testing tool", "0.0" );
 
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

  printf( "query is : %s\n", query.local8Bit().data() );
  printf( "genericServiceType is : %s\n", genericServiceType.local8Bit().data() );
  printf( "constraint is : %s\n", constraint.local8Bit().data() );
  printf( "preference is : %s\n", preference.local8Bit().data() );
  
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
        printf("Invalid property %s\n", (*propIt).local8Bit().data());
	continue;
      }
      
      QString outp = *propIt;
      outp += " : '";
      
      switch ( prop.type() )
      {
        case QVariant::String:
  	  outp += prop.toString();
	break;
        case QVariant::StringList:
  	  outp += prop.toStringList().join(" - ");
	break;
        default: outp = QString::null; break;
      }

      if ( !outp.isEmpty() )
        printf("%s'\n", outp.local8Bit().data());
    }
  }
}
