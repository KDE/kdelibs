#ifndef _KDEBUG_H
#define _KDEBUG_H

void kdebug( unsigned short, unsigned short, const char*, ... );

#ifndef NDEBUG
#define KDEBUG( Level, Area, String ) kdebug( (Level), (Area), (String) );
#define KDEBUG1( Level, Area, String, Par1 ) kdebug( (Level), (Area), (String), (Par1) );
#define KDEBUG2( Level, Area, String, Par1, Par2 ) kdebug( (Level), (Area), (String), (Par1), (Par2) );
#define KDEBUG3( Level, Area, String, Par1, Par2, Par3 ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3) );
#define KDEBUG4( Level, Area, String, Par1, Par2, Par3, Par4 ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4) );
#define KDEBUG5( Level, Area, String, Par1, Par2, Par3, Par4, Par5 ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5) );
#define KDEBUG6( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6 ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5), (Par6) );
#define KDEBUG7( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7 ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5), (Par6), (Par7) );
#define KDEBUG8( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7, Par8 ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5), (Par6), (Par7), (Par8) );
#define KDEBUG9( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7, Par8, Par9 ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5), (Par6), (Par7), (Par8), (Par9) );
#define KASSERT( Cond, Level, Area, String ) { if( !Cond ) kdebug( (Level), (Area), (String) ); }
#define KASSERT1( Cond, Level, Area, String, Par1 ) { if( !Cond ) kdebug( (Level), (Area), (String), (Par1) ); }
#define KASSERT2( Cond, Level, Area, String, Par1, Par2 ) { if( !Cond ) kdebug( (Level), (Area), (String), (Par1), (Par2) ); }
#define KASSERT3( Cond, Level, Area, String, Par1, Par2, Par3 ) { if( !Cond ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3) ); }
#define KASSERT4( Cond, Level, Area, String, Par1, Par2, Par3, Par4 ) { if( !Cond ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4) ); }
#define KASSERT5( Cond, Level, Area, String, Par1, Par2, Par3, Par4, Par5 ) { if( !Cond ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5) ); }
#define KASSERT6( Cond, Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6 ) { if( !Cond ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5), (Par6) ); }
#define KASSERT7( Cond, Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7 ) { if( !Cond ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5), (Par6), (Par7) ); }
#define KASSERT8( Cond, Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7, Par8 ) { if( !Cond ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5), (Par6), (Par7), (Par8) ); }
#define KASSERT9( Cond, Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7, Par8, Par9 ) { if( !Cond ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5), (Par6), (Par7), (Par8), (Par9) ); }
#else
#define KDEBUG( Level, Area, String )
#define KDEBUG1( Level, Area, String, Par1 )
#define KDEBUG2( Level, Area, String, Par1, Par2 )
#define KDEBUG3( Level, Area, String, Par1, Par2, Par3 )
#define KDEBUG4( Level, Area, String, Par1, Par2, Par3, Par4 )
#define KDEBUG5( Level, Area, String, Par1, Par2, Par3, Par4, Par5 )
#define KDEBUG6( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6 )
#define KDEBUG7( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7 )
#define KDEBUG8( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7, Par8 )
#define KDEBUG9( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7, Par8, Par9 )
#define KASSERT( Level, Area, String )
#define KASSERT1( Level, Area, String, Par1 )
#define KASSERT2( Level, Area, String, Par1, Par2 )
#define KASSERT3( Level, Area, String, Par1, Par2, Par3 )
#define KASSERT4( Level, Area, String, Par1, Par2, Par3, Par4 )
#define KASSERT5( Level, Area, String, Par1, Par2, Par3, Par4, Par5 )
#define KASSERT6( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6 )
#define KASSERT7( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7 )
#define KASSERT8( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7, Par8 )
#define KASSERT9( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7, Par8, Par9 )
#endif

#define KDEBUG_INFO 0
#define KDEBUG_WARN 1
#define KDEBUG_ERROR 2
#define KDEBUG_FATAL 3

#endif

