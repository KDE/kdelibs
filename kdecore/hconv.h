#ifndef KCALENDARSYSTEM_HCONV_H
#define KCALENDARSYSTEM_HCONV_H



#ifdef __cplusplus
	extern "C" {
#endif

extern const char* const mname[];
extern const char* const hmname[];
extern const char* const dow[];
extern const char* const sdow[];
extern const char* const hmnameshort[];

typedef struct sdate {
	float time;
	int day;
	int mon;
	int year;
	int dw;
	double nmtime;
} SDATE;

extern SDATE *caldate(double), *hdate(int,int,int), *gdate(int,int,int);

extern double tmoonphase(long,int), julianday(int,int,int,float);
extern int ndays(int,int);

extern double visible(long, double*);

#ifdef __cplusplus
	}
#endif

#define NULLP    ((char *)0)

/* for portability */
#include <sys/types.h>

#endif
