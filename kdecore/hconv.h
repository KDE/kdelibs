#ifndef KCALENDARSYSTEM_HCONV_H
#define KCALENDARSYSTEM_HCONV_H



#ifdef __cplusplus
	extern "C" {
#endif

typedef struct sdate {
	float time;
	int day;
	int mon;
	int year;
	int dw;
	double nmtime;
} SDATE;

extern SDATE *julianToGregorian(double julian);
extern SDATE *gregorianToHijri(int,int,int);
extern SDATE *hijriToGregorian(int,int,int);
extern double gregorianToJulian(int,int,int,float);
extern int hijriDaysInMonth(int m, int y);

#ifdef __cplusplus
	}
#endif

#define NULLP    ((char *)0)

/* for portability */
#include <sys/types.h>

#endif
