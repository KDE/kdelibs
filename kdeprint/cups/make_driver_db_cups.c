#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <zlib.h>

#include "driverparse.h"

void initPpd(const char *dirname)
{
	DIR	*dir = opendir(dirname);
	struct dirent	*entry;
	char		buffer[4096] = {0};
	char	drFile[256];
	int		len = strlen(dirname);

	if (dir == NULL)
	{
		fprintf(stderr, "Can't open drivers directory : %s\n", dirname);
		return;
	}
	while ((entry=readdir(dir)) != NULL)
	{
		if (strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0)
		{
			continue;
		}
		if (len+strlen(entry->d_name)+1 < 4096)
		{
			struct stat	st;

			strcpy(buffer,dirname);
			strcat(buffer,"/");
			strcat(buffer,entry->d_name);
			if (stat(buffer,&st) == 0)
			{
				if (S_ISDIR(st.st_mode))
				{
					initPpd(buffer);
				}
				else if (S_ISREG(st.st_mode))
				{
					char	*c = strrchr(buffer,'.');
					snprintf(drFile, 255, "ppd:%s", buffer);
					if (c && strncmp(c,".ppd",4) == 0)
					{
						addFile(drFile);
					}
					else if (c && strncmp(c, ".gz", 3) == 0)
					{ /* keep also compressed driver files */
						while (c != buffer)
						{
							if (*(--c) == '.') break;
						}
						if (*c == '.' && strncmp(c, ".ppd",4) == 0)
						{
							addFile(drFile);
						}
					}
				}
			}
		}
	}
	closedir(dir);
}

int parsePpdFile(const char *filename, FILE *output_file)
{
	gzFile	ppd_file;
	char	line[4096], value[256], langver[64] = {0}, desc[256] = {0};
	char	*c1, *c2;
	int	count = 0;

	ppd_file = gzopen(filename,"r");
	if (ppd_file == NULL)
	{
		fprintf(stderr, "Can't open driver file : %s\n", filename);
		return 0;
	}
	fprintf(output_file,"FILE=ppd:%s\n",filename);

	while (gzgets(ppd_file,line,4095) != Z_NULL)
	{
		memset(value,0,256);
		c1 = strchr(line,':');
		if (c1)
		{
			c2 = strchr(c1,'"');
			if (c2)
			{
				c2++;
				c1 = strchr(c2,'"');
				if (c1) strncpy(value,c2,c1-c2);
			}
			else
			{
				c1++;
				while (*c1 && isspace(*c1))
					c1++;
				if (!*c1)
					continue;
				c2 = line+strlen(line)-1;	/* point to \n */
				while (*c2 && isspace(*c2))
					c2--;
				strncpy(value,c1,c2-c1+1);
			}
		}
		count++;
		if (strncmp(line,"*Manufacturer:",14) == 0) fprintf(output_file,"MANUFACTURER=%s\n",value);
		else if (strncmp(line,"*ShortNickName:",15) == 0) fprintf(output_file,"MODEL=%s\n",value);
		else if (strncmp(line,"*ModelName:",11) == 0) fprintf(output_file,"MODELNAME=%s\n",value);
		else if (strncmp(line,"*NickName:",10) == 0) strncat(desc,value,255-strlen(desc));
		else if (strncmp(line,"*pnpManufacturer:",17) == 0) fprintf(output_file,"PNPMANUFACTURER=%s\n",value);
		else if (strncmp(line,"*pnpModel:",10) == 0) fprintf(output_file,"PNPMODEL=%s\n",value);
		else if (strncmp(line,"*LanguageVersion:",17) == 0) strncat(langver,value,63-strlen(langver));
		else count--;
		/* Either we got everything we needed, or we encountered an "OpenUI" directive
		 * and it's reasonable to assume that there's no needed info further in the file,
		 * just stop here */
		if (count >= 7 || strncmp(line, "*OpenUI", 7) == 0)
		{
			if (strlen(langver) > 0)
			{
				strncat(desc, " [", 255-strlen(desc));
				strncat(desc, langver, 255-strlen(desc));
				strncat(desc, "]", 255-strlen(desc));
			}
			if (strlen(desc) > 0)
				fprintf(output_file, "DESCRIPTION=%s\n", desc);
			break;
		}
	}
	fprintf(output_file,"\n");

	gzclose(ppd_file);
	return 1;
}

int main(int argc, char *argv[])
{
	registerHandler("ppd:", initPpd, parsePpdFile);
	initFoomatic();
	return execute(argc, argv);
}
