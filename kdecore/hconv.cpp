
/* hconv.c
 *
 * Copyright (c) 1992 by Waleed A. Muhanna
 *
 * Permission for nonprofit use and redistribution of this software and 
 * its documentation is hereby granted without fee, provided that the 
 * above copyright notice appear in all copies and that both that copyright 
 * notice and this permission notice appear in supporting documentation.
 *
 * No representation is made about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * Send any comments/suggestions/fixes/additions to:
 *		wmuhanna@magnus.acs.ohio-state.edu
 *
 */

#include <math.h>
#include "hconv.h"
#include <stdlib.h>

#define RPD     (0.01745329251994329577) /* radians per degree (pi/180) */

/*
 * Given an integer _n_ and a phase selector (nph=0,1,2,3 for
 * new,first,full,last quarters respectively, function returns the
 * Julian date/time of the Nth such phase since January 1900.
 * Adapted from "Astronomical  Formulae for Calculators" by
 * Jean Meeus, Third Edition, Willmann-Bell, 1985.
 */
static double
tmoonphase( long n, int nph)
{
	double jd, t, t2, t3, k, ma, sa, tf, xtra;
	k = n + nph/4.0;  t = k/1236.85;  t2 = t*t; t3 = t2*t;
	jd =  2415020.75933 + 29.53058868*k - 1.178e-4 * t2
	    - 1.55e-7 * t3
	    + 3.3e-4 * sin (RPD * (166.56 +132.87*t -0.009173*t2));

	/* Sun's mean anomaly */
	sa =  RPD * (359.2242 + 29.10535608*k - 3.33e-5 * t2 - 3.47e-6 * t3);

	/* Moon's mean anomaly */
	ma =  RPD * (306.0253 + 385.81691806*k + 0.0107306*t2 +1.236e-5 *t3);

	/* Moon's argument of latitude */
	tf = RPD * 2.0 * (21.2964 + 390.67050646*k -0.0016528*t2
		      -2.39e-6 * t3);

	/* should reduce to interval 0-1.0 before calculating further */
	if (nph==0 || nph==2)
		/* Corrections for New and Full Moon */
		xtra = (0.1734 - 0.000393*t) * sin(sa)
		      +0.0021*sin(sa*2)
		      -0.4068*sin(ma) +0.0161*sin(2*ma) -0.0004*sin(3*ma)
		      +0.0104*sin(tf)
		      -0.0051*sin(sa+ma) -0.0074*sin(sa-ma)
		      +0.0004*sin(tf+sa) -0.0004*sin(tf-sa)
		      -0.0006*sin(tf+ma) +0.0010*sin(tf-ma)
		      +0.0005*sin(sa+ 2*ma);
	else if (nph==1 || nph==3) {
		xtra = (0.1721 - 0.0004*t) * sin(sa)
		      +0.0021*sin(sa*2)
		      -0.6280*sin(ma) +0.0089*sin(2*ma) -0.0004*sin(3*ma)
		      +0.0079*sin(tf)
		      -0.0119*sin(sa+ma) -0.0047*sin(sa-ma)
		      +0.0003*sin(tf+sa) -0.0004*sin(tf-sa)
		      -0.0006*sin(tf+ma) +0.0021*sin(tf-ma)
		      +0.0003*sin(sa+ 2*ma) +0.0004*sin(sa-2*ma)
		      -0.0003*sin(2*sa+ma);
		if (nph==1)
			xtra = xtra +0.0028 -0.0004*cos(sa) +0.0003*cos(ma);
		else
			xtra = xtra -0.0028 +0.0004*cos(sa) -0.0003*cos(ma);
	} else {
		/*printf("tmoonphase: illegal phase number\n"); */
	        exit(1); // ### HPB Remove this
	}
	/* convert from Ephemeris Time (ET) to (approximate) 
	   Universal Time (UT) */
	jd += xtra - (0.41 +1.2053*t +0.4992*t2)/1440;
	return (jd);
}


/* parameters for Makkah: for a new moon to be visible after sunset on
   a the same day in which it started, it has to have started before
   (SUNSET-MINAGE)-TIMZ=3 A.M. local time. */

#define TIMZ 3.0
#define MINAGE 13.5
#define SUNSET 19.5 /*approximate */
#define TIMDIF (SUNSET-MINAGE) 

static double
visible(long n, double * rjd)
{
	double jd;
	float tf;
	long d;

	jd = tmoonphase(n,0);  *rjd = jd;
	d = jd;
	tf = (jd - d);
	if (tf<=0.5)  /*new moon starts in the afternoon */
		return(jd+1.0); 
	/* new moon starts before noon */
	tf = (tf-0.5)*24 +TIMZ;  /* local time */ 
	if (tf>TIMDIF) return(jd+1.0);  /*age at sunset < min for visiblity*/
	return(jd);
}

/*
 * Returns the Julian date (the Julian day number at _time_) of the
 * given calendar date specified by _year_, _month_, and _day_.
 * Positive year signifies A.D.; negative, B.C.
 */
double
julianday( int year, int month, int day, float time)
{
	double jul;
	long m,y,ja;

	if (year<0) year++;
	if (month>2) {
		y = year; m =month;
	} else {
		y = year-1; m =month+12;
	}
	jul = ((double)y) * 365.25;
	if (y<1) jul -= 0.75;
	jul = ((long) jul) + ( (long) (30.6001*(m+1))) +day +time +1720994.5;
	if (year +month*1e-2 +(day+time)*1e-4 >= 1582.1015) {
		/* cross-over to Gregorian calendar */
		ja = 0.01*y;
		jul = jul +2 -ja + ((long) (0.25*ja));
	}
	return jul;
}

/* 
 * inverse function of Julianday.
 */
SDATE * julianToGregorian( double julday )
{
	static SDATE sd;
	long	z, alpha, a, b, c, d, e;
	double	f;

	julday += 0.5; z = julday; f = julday-z;
	if ( z<2299161L )  /* test whether to change to Gregorian cal */
		a = z;
	else {
		alpha = (z-1867216.25) / 36524.25;
		a     = z + 1 +alpha - alpha / 4;
	}
	b = a + 1524;  c = (b - 122.1) / 365.25; d = 365.25 * c;
	e = (b-d) / 30.6001;
	f += b -d - ((long) (30.6001*e));
	sd.day = f;
	sd.time = f-sd.day;
	sd.mon = (e>13) ? e-13 : e-1;
	sd.year = (sd.mon>2) ? c-4716 : c-4715;
	/*
	sd.dw = ((long) (julianday(sd.year,sd.mon, sd.day, 0.0) + 1.5)) % 7;
	*/
	sd.dw = ((long) (julday -sd.time + 1.1)) % 7;
	if ( sd.year<=0) sd.year--;
	return(&sd);
}

		

/*
 * Given a gregorian/julian date, compute corresponding Hijri date structure 
 * As a reference point, the routine uses the fact that the year
 * 1405 A.H. started immediatly after lunar conjunction number 1048
 * which occured on September 1984 25d 3h 10m UT.
 */
SDATE * gregorianToHijri (int y, int m, int d)
{
	static SDATE h;
	double jd, mjd, rjd;
	long k, hm;

	jd = julianday(y,m,d,0.00); 
	/* obtain first approx. of how many new moons since the beginning
	   of the year 1900 */
	k = 0.6 + (y + ((int) (m-0.5)) /12.0 + d/365.0 - 1900) *12.3685;
	do  {mjd = visible(k--, &rjd);} while (mjd>jd);  k++;
	/*first of the month is the following day*/
	hm = k -1048;  
	h.year = 1405 + (hm / 12);

	h.mon =  (hm % 12) +1; 
	if (hm !=0 && h.mon<=0) {h.mon +=12; h.year--; }
	if (h.year<=0) h.year--;
	h.day = jd -mjd +1.0;
	h.time = 0.5;
	h.dw = ((long) (jd+1.5)) % 7;
	return(&h);
}

#define NMONTHS  (1405*12+1)

/*
 * Given a Hijri date, compute corresponding C.E. date structure 
 */
SDATE * hijriToGregorian(int y, int m, int d)
{
	double jd, rjd;
	long k;
	SDATE *sd;

	if (y<0) y++;
	k = m+ y*12 - NMONTHS;   /* # of months since 1/1/1405 */
	jd = visible(k+1048L, &rjd) +d;
	sd = julianToGregorian(jd);
	sd->nmtime = rjd;
	return (sd);
}


int hijriDaysInMonth(int m, int y)
{
	static short  ndmnth[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

	int nd = ndmnth[m];
	if (m==2 && (y%4==0 && (y<1582 || y%100!=0 || y%400==0))) nd++;
	return nd;
}
