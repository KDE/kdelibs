// Simplest example using two kde calendar systems (gregorian and hijri)
// Carlos Moro <cfmoro@correo.uniovi.es>
// GNU-GPL v.2

#include "kcalendarsystemfactory.h"
#include "kcalendarsystem.h"

#include <qstringlist.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcmdlineargs.h>

class KLocale;

void test(QDate & date);

static const char description[] = "KCalendarTest";

static KCmdLineOptions options[] =
{
  { "help", I18N_NOOP("Prints this help"), 0 },
  { "type hijri|gregorian|jalali|hebrew", I18N_NOOP("Supported calendar types"), 0 },
  { "date <date>", I18N_NOOP("Show day info"), 0 },
};

int main(int argc, char **argv) {

	QDate date;
	QString calType, option;
	
        KAboutData aboutData( "kcalendartest", "KCalendarTest" ,
                        "0.1", description, KAboutData::License_GPL,
                        "(c) 2002, Carlos Moro", 0, 0,
                        "cfmoro@correo.uniovi.es");
  	aboutData.addAuthor("Carlos Moro",0, "cfmoro@correo.uniovi.es");
	

        KCmdLineArgs::init( argc, argv, &aboutData );
        KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	KApplication app(false, false);

        QStringList lst = KCalendarSystemFactory::calendarSystems();
	kdDebug() << "Supported calendar types: " << endl;
	for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it)
            kdDebug() << *it << endl;
        kdDebug() << endl;

	
        if ( args->isSet("type") )
		calType = args->getOption("type");
	
	
	KGlobal::locale()->setCalendar(calType);

  /*
   *  If we like to see some date
   *
   */
      	if ( args->isSet("date") ) {
    		option = args->getOption("date");
                date = KGlobal::locale()->readDate(option);
  	} else 
    		date = QDate::currentDate();

	args->clear(); // Free up some memory.
	
	test(date);

	return 0;	
  
	

}

void test(QDate & date) {

        kdDebug() << "(KLocale) readDate" << endl;

        kdDebug() << "Created calendar: " << KGlobal::locale()->calendar()->calendarName() << endl;

	kdDebug() << "Day name for first day of week is " << KGlobal::locale()->calendar()->weekDayName(1) << endl;
	kdDebug() << "Short month name for second month is " << KGlobal::locale()->calendar()->weekDayName(1, true) << endl;

	kdDebug() << "Month name for second month is " << KGlobal::locale()->calendar()->monthName(2, KGlobal::locale()->calendar()->year(date)) << endl;
	kdDebug() << "Short month name for second month is " << KGlobal::locale()->calendar()->monthName(2, KGlobal::locale()->calendar()->year(date), true) << endl;
	kdDebug() << "Month name possessive for second month is " << KGlobal::locale()->calendar()->monthNamePossessive(2, KGlobal::locale()->calendar()->year(date)) << endl;
	kdDebug() << "Short month name possessive for second month is " << KGlobal::locale()->calendar()->monthNamePossessive(2, KGlobal::locale()->calendar()->year(date), true) << endl;
	kdDebug() << "Month name for fifth month is " << KGlobal::locale()->calendar()->monthName(5, KGlobal::locale()->calendar()->year(date)) << endl;
	kdDebug() << "Short month name for fifth month is " << KGlobal::locale()->calendar()->monthName(5, KGlobal::locale()->calendar()->year(date), true) << endl;
	kdDebug() << "Month name possessive for fifth month is " << KGlobal::locale()->calendar()->monthNamePossessive(5, KGlobal::locale()->calendar()->year(date)) << endl;
	kdDebug() << "Short month name possessive for fifth month is " << KGlobal::locale()->calendar()->monthNamePossessive(5, KGlobal::locale()->calendar()->year(date), true) << endl;

	kdDebug() << "Day for date " << date.toString() << " is " << KGlobal::locale()->calendar()->day(date) << endl;
	kdDebug() << "Month for date " << date.toString() << " is " << KGlobal::locale()->calendar()->month(date) << endl;
	kdDebug() << "Year for date " << date.toString() << " is " << KGlobal::locale()->calendar()->year(date) << endl;

	kdDebug() << "Day for date " << date.toString() << " as a string is " << KGlobal::locale()->calendar()->dayString(date, true) << endl;
	kdDebug() << "Month for date " << date.toString() << " as a string is " << KGlobal::locale()->calendar()->monthString(date, true) << endl;
	kdDebug() << "Year for date " << date.toString() << " as a string is " << KGlobal::locale()->calendar()->yearString(date, true) << endl;

	kdDebug() << "Day of week for date " << date.toString() << " is number " << KGlobal::locale()->calendar()->dayOfWeek(date) << endl;
	kdDebug() << "Week name for date " << date.toString() << " is " << KGlobal::locale()->calendar()->weekDayName(date) << endl;
	kdDebug() << "Short week name for date " << date.toString() << " is " << KGlobal::locale()->calendar()->weekDayName(date, true) << endl;

	kdDebug() << "Month name for date " << date.toString() <<  " is "  << KGlobal::locale()->calendar()->monthName(date) << endl;
	kdDebug() << "Short month name for date " << date.toString() << " is "  << KGlobal::locale()->calendar()->monthName(date, true) << endl;
	kdDebug() << "Month name possessive for date " << date.toString() <<  " is "  << KGlobal::locale()->calendar()->monthNamePossessive(date) << endl;
	kdDebug() << "Short month name possessive for date " << date.toString() << " is "  << KGlobal::locale()->calendar()->monthNamePossessive(date, true) << endl;

 	kdDebug() << "It's week number " << KGlobal::locale()->calendar()->weekNumber(date) << endl;


	kdDebug() << "(KLocale) Formatted date: " << KGlobal::locale()->formatDate(date) << endl;
	kdDebug() << "(KLocale) Short formatted date: " << KGlobal::locale()->formatDate(date, true) << endl;

	kdDebug() << "That month have : " << KGlobal::locale()->calendar()->daysInMonth(date) << " days" << endl;

	kdDebug() << "That year has " << KGlobal::locale()->calendar()->monthsInYear(date) << " months" << endl;
	kdDebug() << "There are " << KGlobal::locale()->calendar()->weeksInYear(KGlobal::locale()->calendar()->year(date)) << " weeks that year" << endl;
	kdDebug() << "There are " << KGlobal::locale()->calendar()->daysInYear(date) << " days that year" << endl;
	
	kdDebug() << "The day of pray is number " << KGlobal::locale()->calendar()->weekDayOfPray() << endl;
	
	kdDebug() << "Max valid year supported is " << KGlobal::locale()->calendar()->maxValidYear() << endl;
	kdDebug() << "Min valid year supported is " << KGlobal::locale()->calendar()->minValidYear() << endl;
	
	kdDebug() << "It's the day number " << KGlobal::locale()->calendar()->dayOfYear(date) << " of year" << endl;
	
	kdDebug() << "Add 3 days" << endl;
	date = KGlobal::locale()->calendar()->addDays(date, 3);
	kdDebug() << "It's " << KGlobal::locale()->formatDate(date) << endl;

	kdDebug() << "Then add 3 months" << endl;
	date = KGlobal::locale()->calendar()->addMonths(date, 3);
	kdDebug() << "It's " << KGlobal::locale()->formatDate(date) << endl;

	kdDebug() << "And last, add -3 years" << endl;
	date = KGlobal::locale()->calendar()->addYears(date, -3);
	kdDebug() << "It's " << KGlobal::locale()->formatDate(date) << endl;
	
	kdDebug() << "Is lunar based: " << KGlobal::locale()->calendar()->isLunar() << endl;
	kdDebug() << "Is lunisolar based: " << KGlobal::locale()->calendar()->isLunisolar() << endl;
	kdDebug() << "Is solar based: " << KGlobal::locale()->calendar()->isSolar() << endl;

}
