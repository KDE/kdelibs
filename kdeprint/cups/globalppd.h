#ifndef	GLOBALPPD_H
#define	GLOBALPPD_H

#include <cups/ppd.h>
#include <cups/cups.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* data structure for numerical options of CUPS-O-MATIC */
typedef struct {
  char keyword[41], /* option keyword */
       text[81];    /* human-readable description for GUIs */
  int type;         /* 0: integer, 1: float */
  double min,       /* minumum value */
         max,       /* maximum value */
         defvalue,  /* default value */
         value;	    /* current value */
} numerical_opt_t;

int getNumericalOptions(const char *filename, numerical_opt_t **numerical_options);

typedef struct {
	ppd_file_t	*ppd;
	int		num_numerical_options;
	numerical_opt_t	*numerical_options;
	char		*ppd_filename;
        int		is_compressed;
        char		*compr_filename;	/* for internal use only */
} global_ppd_file_t;

global_ppd_file_t* globalPpdOpenFile(const char *filename);
void globalPpdClose(global_ppd_file_t *ppd);
int writeGlobalPpdFile(global_ppd_file_t *ppd, const char *filename, const char *template_file);
numerical_opt_t* globalPpdFindNumericalOption(global_ppd_file_t *ppd, const char *keyword);
int globalPpdMarkOption(global_ppd_file_t *ppd, const char *keyword, const char *option);
int globalPpdMarkNumericalOption(global_ppd_file_t *ppd, const char *keyword, double value);
void globalCupsMarkOptions(global_ppd_file_t *ppd, int num_options, cups_option_t *options);
void globalPpdMarkDefaults(global_ppd_file_t *ppd);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
