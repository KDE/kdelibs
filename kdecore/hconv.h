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

extern SDATE *caldate(double), *hdate(int,int,int), *gdate(int,int,int);
extern double julianday(int,int,int,float);
extern int ndays(int,int);

#ifdef __cplusplus
	}
#endif

#define NULLP    ((char *)0)

/* for portability */
#include <sys/types.h>

#endif
