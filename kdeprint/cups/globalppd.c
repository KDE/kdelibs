#include "globalppd.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <zlib.h>

/*
 * open a PPD file and create a global_ppd_file_t structure
 */
global_ppd_file_t* globalPpdOpenFile(const char *filename)
{
	ppd_file_t		*ppd;	/* regular PPD structure */
	global_ppd_file_t	*gppd;	/* global PPD structure */
        int			compr;
        char			tmpfile[1024], *c;

 	gppd = NULL;
        compr = 0;
        strncpy(tmpfile, filename, 1024);
        c = strrchr(tmpfile, '.');
        if (c && strncmp(c, ".gz", 3) == 0)
        { /* compressed file -> first decompress it in temporary file
             as CUPS API doesn't support compressed drivers */
        	gzFile	*src;
                FILE	*dest;
                char	buffer[1024];
                int	n = 0;

                compr = 1;
                cupsTempFile(tmpfile, 1024);
                src = gzopen(filename, "rb");
                if (src == NULL)
                {
                	return gppd;
		}
                dest = fopen(tmpfile, "wb");
                if (dest == NULL)
                {
                	gzclose(src);
                        return gppd;
                }
                while ((n = gzread(src, buffer, 1024)) > 0)
                {
                	if (fwrite(buffer, 1, n, dest) != n)
                        { /* error while writing */
                        	n = -1;
                                break;
                        }
                }
                gzclose(src);
                fclose(dest);
                if (n == -1)
                { /* an error occured, remove temp file and return NULL */
                	unlink(tmpfile);
                        return gppd;
                }
        }
 	ppd = ppdOpenFile(tmpfile);
	if (ppd != NULL)
	{
		/* initialize global PPD structure */
		gppd = (global_ppd_file_t*)malloc(sizeof(global_ppd_file_t));
		gppd->ppd = ppd;
		gppd->num_numerical_options = 0;
		gppd->numerical_options = NULL;
		gppd->ppd_filename = strdup(filename);
                gppd->is_compressed = 0;
                gppd->compr_filename = NULL;
                /* set compressed attributes */
                if (compr)
                {
			gppd->is_compressed = 1;
                        gppd->compr_filename = strdup(tmpfile);
                }
		/* load numerical options from PPD file */
		gppd->num_numerical_options = getNumericalOptions(tmpfile, &(gppd->numerical_options));
	}
	return gppd;
}

/*
 * mark defaults options
 */
void globalPpdMarkDefaults(global_ppd_file_t *ppd)
{
	int	i;
	numerical_opt_t	*opt;

	ppdMarkDefaults(ppd->ppd);
	for (i=0;i<ppd->num_numerical_options;i++)
	{
		opt = ppd->numerical_options+i;
		opt->value = opt->defvalue;
	}
}

/*
 * close a PPD file, deleting the allocated globale structure
 */
void globalPpdClose(global_ppd_file_t *ppd)
{
	/* first close the regular PPD structure */
	ppdClose(ppd->ppd);
	/* then delete the numerical options array */
	if (ppd->num_numerical_options > 0) free(ppd->numerical_options);
	/* free ppd filename */
	free(ppd->ppd_filename);
        /* check if compressed driver */
        if (ppd->is_compressed)
        {
        	unlink(ppd->compr_filename);
                free(ppd->compr_filename);
        }
	/* then delete the structure */
	free(ppd);
}

/*
 * write a global PPD file, using a template. This function is useful to set
 * default options in a PPD file and then sending it to the CUPS server. All
 * default options are assumed to be marked (regular and numerical).
 */
int writeGlobalPpdFile(global_ppd_file_t *ppd, const char *filename, const char *template_file)
{
	FILE	*out;
        gzFile	in;	/* possible compressed template file */
	int	retVal;

	retVal = 0;
	if (template_file == NULL)	/* if no template file given, use the source PPD file */
		in = gzopen(ppd->ppd_filename,"r");
	else
		in = gzopen(template_file,"r");
	out = fopen(filename,"w");
	if (in && out)
	{
		char	linebuf[1024];
		char	keyword[64] = {0};
		char	*c, *cc;
		int	type;

		type = -1;
		while (gzgets(in,linebuf,1024) != Z_NULL)
		{
			if (strncmp(linebuf,"*% COMDATA #",12) == 0)
			{
				if ((c=strstr(linebuf,"'name'")) != NULL)
				{
					c += 11;
					cc = strchr(c,'\'');
					memset(keyword,0,64);
					strncpy(keyword,c,cc-c);
				}
				else if ((c=strstr(linebuf,"'type'")) != NULL)
				{
					c += 11;
					if (strncmp(c,"float",5) == 0) type = 1;
					else if (strncmp(c,"int",3) == 0) type = 0;
					else type = -1;
				}
				else if ((c=strstr(linebuf,"'default'")) != NULL && keyword[0] != 0)
				{
					numerical_opt_t	*opt;
					double	value;
					char	buffer[1024];
					opt = globalPpdFindNumericalOption(ppd,keyword);
					if (opt && opt->type == type)
					{
						memset(buffer,0,1024);
						strncpy(buffer,linebuf,c-linebuf+13);
						fprintf(out,"%s",buffer);
						value = opt->value;
						if (opt->type == 0)
							fprintf(out,"'%d'",(int)(value));
						else
							fprintf(out,"'%.3f'",value);
						if (strchr(c+13,',') != NULL)
							fprintf(out,",");
						fprintf(out,"\n");
						continue;
					}
				}
				fprintf(out,"%s",linebuf);
			}
			else if (strncmp(linebuf,"*Default",8) != 0)
				fprintf(out,"%s",linebuf);
			else
			{
				ppd_choice_t	*choice;

				strncpy(keyword,linebuf+8,63);
				for (c = keyword;*c;c++)
					if (*c == ':' || isspace(*c)) break;
				*c = '\0';
				choice = ppdFindMarkedChoice(ppd->ppd,(strcmp(keyword,"PageRegion") == 0 ? "PageSize" : keyword));
				if (choice)
					fprintf(out,"*Default%s: %s\n",keyword,choice->choice);
				else
					fprintf(out,"%s",linebuf);
			}
		}
	}
	else retVal = -1;

	gzclose(in);
	fclose(out);
	
	return retVal;
}

/*
 * find a numerical option
 */
numerical_opt_t* globalPpdFindNumericalOption(global_ppd_file_t *ppd, const char *keyword)
{
	numerical_opt_t	*opt;
	int	i;

	for (i=0;i<ppd->num_numerical_options;i++)
	{
		opt = ppd->numerical_options+i;
		if (strcmp(opt->keyword, keyword) == 0) return opt;
	}
	return NULL;
}

/*
 * mark a numerical option
 */
int globalPpdMarkNumericalOption(global_ppd_file_t *ppd, const char *keyword, double value)
{
	numerical_opt_t	*opt;

	opt = globalPpdFindNumericalOption(ppd, keyword);
	if (opt != NULL)
	{
		opt->value = value;
	}
	return 0;
}

/*
 * mark a global option (regular or numerical)
 */
int globalPpdMarkOption(global_ppd_file_t *ppd, const char *keyword, const char *option)
{
	numerical_opt_t	*opt;

	opt = globalPpdFindNumericalOption(ppd, keyword);
	if (opt != NULL)
	{
		double	value;
		value = atof(option);
		opt->value = value;
		return 0;
	}
	else
	{
		return ppdMarkOption(ppd->ppd, keyword, option);
	}
}

/*
 * mark all options (regular and numerical)
 */
void globalCupsMarkOptions(global_ppd_file_t *ppd, int num_options, cups_option_t *options)
{
	cups_option_t	*opt;
	int	i;

	for (i=0;i<num_options;i++)
	{
		opt = options+i;
		if (opt->name != NULL && opt->value != NULL)
			globalPpdMarkOption(ppd, opt->name, opt->value);
	}
}

/*
 * code taken from XPP (Author: Till Kamppeter)
 */
int getNumericalOptions(const char *filename, numerical_opt_t **numerical_opts_arr)
{
  int           options_found;  /* how many options found */
  int		i;              /* Looping vars */

  FILE *ppdfile;
  char line[1024], /* buffer for reading PPD file line by
                      line to search numerical options */
       item[1024], /* item to be defined (left of "=>") */
       value[1024], /* value for item (right of "=>") */
       argname[1024], /* name of the current argument */
       comment[1024], /* human-readable argument name */
       numbuffer[128]; /* buffer to store the value as a CUPS option */
  const char *line_contents; /* contents of line */
  const char *scan; /* pointer scanning the line */
  char *writepointer;
  const char *optionvalue; /* Value of the option if it is set by the user */
  double min, max, defvalue; /* Range of numerical
                                CUPS-O-MATIC option */
  int opttype; /* 0 = other, 1 = int, 2 = float */
  int openbrackets; /* How many curled brackets are open? */
  int inquotes; /* are we in quotes now? */
  int inargspart; /* are we in the arguments part now? */

  /* Open PPD file to search for numerical options of CUPS-O-MATIC */
  if ((ppdfile = fopen(filename,"r")) == NULL) return(0);

  /* Reset all variables */
  opttype = 0;
  min = 0.0; max = 0.0; defvalue = 0.0;
  openbrackets = 0;
  inquotes = 0;
  writepointer = item;
  inargspart = 0;
  options_found = 0;
  item[0] = '\0';
  value[0] = '\0';
  argname[0] = '\0';
  comment[0] = '\0';
  /* Read the PPD file, line by line. */
  while (fgets(line,sizeof(line),ppdfile)) {
    /* evaluate only lines with CUPS-O-MATIC info */
    if ((line_contents = strstr(line,"*% COMDATA #"))) {
      line_contents += 12; /* Go to the text after
                              "*% COMDATA #" */
      for (scan = line_contents;
           (*scan != '\n') && (*scan != '\0') && (*scan != '\r');
           scan ++) {
        switch(*scan) {
          case '{': /* open curled bracket */
            if (!inquotes) {
              openbrackets ++;
              /* we are on the left hand side now */
              *writepointer = '\0';
              writepointer = item;
              /* in which type of block are we now? */
              if ((openbrackets == 2) &&
                  (strcasecmp(item,"args_byname") == 0)) {
                /* we are entering the arguments section now */
                inargspart = 1;
      	      }
              if ((openbrackets == 3) &&
                  (inargspart == 1)) {
                /* new argument, get its name */
                strcpy(argname,item);
	      }
              /* item already evaluated now */
              item[0] = '\0';
            } else {*writepointer = *scan; writepointer ++;}
            break;
	  case ',': /* end of logical line */
	  case '}': /* close curled bracket */
            if (!inquotes) {
              /* right hand side completed, go to left hand side */
              *writepointer = '\0';
              writepointer = item;
              /* evaluate logical line */
              if (item[0]) {
                /* Human-readable argument name */
                if ((openbrackets == 3) &&
                    (inargspart == 1) &&
                    (strcasecmp(item,"comment") == 0)) {
                  strcpy(comment,value);
	        }
                /* argument type */
                if ((openbrackets == 3) &&
                    (inargspart == 1) &&
                    (strcasecmp(item,"type") == 0)) {
                  if (strcasecmp(value,"int") == 0) opttype = 1;
                  if (strcasecmp(value,"float") == 0) opttype = 2;
		}
                /* minimum value */
                if ((openbrackets == 3) &&
                    (inargspart == 1) &&
                    (strcasecmp(item,"min") == 0)) {
                  min = atof(value);
		}
                /* maximum value */
                if ((openbrackets == 3) &&
                    (inargspart == 1) &&
                    (strcasecmp(item,"max") == 0)) {
                  max = atof(value);
		}
                /* default value */
                if ((openbrackets == 3) &&
                    (inargspart == 1) &&
                    (strcasecmp(item,"default") == 0)) {
                  defvalue = atof(value);
		}
                /* item already evaluated now */
                item[0] = '\0';
              }
              /* close curled bracket */
              if (*scan == '}') {
                /* which block did we complete now? */
                if ((openbrackets == 2) &&
                    (inargspart == 1)) {
                  /* We are leaving the arguments part now */
                  inargspart = 0;
                }
                if ((openbrackets == 3) &&
                    (inargspart == 1)) {
                  /* The current option is completely parsed
                     Is the option a valid numerical option? */
                  if ((opttype > 0) &&
                      (min != max) &&
                      (argname[0])) {
                    /* Correct the default value, if necessary */
                    if (min < max) {
                      if (defvalue < min) defvalue = min;
                      if (defvalue > max) defvalue = max;
		    } else {
                      if (defvalue < max) defvalue = max;
                      if (defvalue > min) defvalue = min;
                    }
                    /* Store the found argument */
                    options_found++;
                    *numerical_opts_arr = (numerical_opt_t *)realloc
                      (*numerical_opts_arr,
                       sizeof(numerical_opt_t)*options_found);
                    strncpy((*numerical_opts_arr)[options_found-1].keyword,
                            argname,40);
                    strncpy((*numerical_opts_arr)[options_found-1].text,
                            comment,80);
                    (*numerical_opts_arr)[options_found-1].type=opttype-1;
		    (*numerical_opts_arr)[options_found-1].min=min;
		    (*numerical_opts_arr)[options_found-1].max=max;
		    (*numerical_opts_arr)[options_found-1].defvalue=defvalue;
		    (*numerical_opts_arr)[options_found-1].value=min;
                    /* Set an option in the printer's option list for every
                       numerical option to guarantee that the printer uses the
                       shown value, even when the driver default and the PPD
                       default are not equal. */
                    if (opttype == 1) {
                      sprintf(numbuffer,"%d",(int)(defvalue));
                    } else {
                      sprintf(numbuffer,"%.3f",defvalue);
                    }
                  }
                  /* reset the values */
                  argname[0] = '\0';
                  comment[0] = '\0';
                  opttype = 0;
                  min = 0.0; max = 0.0; defvalue = 0.0;
                }
                openbrackets --;
              }
            } else {*writepointer = *scan; writepointer ++;}
            break;
	  case '\'': /* quote */
            if (!inquotes) { /* open quote pair */
              inquotes = 1;
	    } else { /* close quote pair */
              inquotes = 0;
            }
            break;
	  case '=': /* "=>" */
            if ((!inquotes) && (*(scan + 1) == '>')) {
              scan ++;
              /* left hand side completed, go to right hand side */
              *writepointer = '\0';
              writepointer = value;
            } else {*writepointer = *scan; writepointer ++;}
            break;
	  case ' ':  /* white space */
	  case '\t':
            if (!inquotes) {
              /* ignore white space outside quotes */
            } else {*writepointer = *scan; writepointer ++;}
            break;
	  default:
            /* write all other characters */
            *writepointer = *scan; writepointer ++;
            break;
        }
      }
      inquotes = 0; /* quote pairs cannot enclose more
                       than one line */
    }
  }
  fclose(ppdfile);
  return(options_found);
}
