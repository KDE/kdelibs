#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <zlib.h>

typedef struct _Node
{
	char		*str;
	struct _Node	*next;
} Node;
Node	*start = 0, *current = 0;
int	node_number = 0;

void exit_with_error(const char *txt);
int process_file(const char *filename, FILE *output_file);
void append_node(const char *str);
void delete_all_nodes(void);
int list_files(const char *dirname);

int main(int argc, char *argv[])
{
	FILE	*db_file;
	char	*db_filename;
	int	count = 0;
	int	i1, i2;

	if (argc < 2 || argc > 3)
	{
		exit_with_error("usage: make_driver_db <db_directory> [output_filename]");
	}
	if (argc == 3)
	{
		i1 = argc-2;
		i2 = argc-1;
	}
	else
	{
		i1 = argc-1;
		i2 = i1;
	}

	fprintf(stderr, "PPDs:|%s|%s|\n", argv[i1], argv[i2] );

	if (list_files(argv[i1]) == -1)
	{
		exit_with_error(0);
	}
	printf("%d\n",node_number);
	fflush(stdout);

	db_filename = (char*)calloc(strlen(argv[i2])+32,sizeof(char));
	strcpy(db_filename, argv[i2]);
	/*strcat(db_filename,"/");
	strcat(db_filename,"printers_db.txt");*/
	db_file = fopen(db_filename,"w");
	if (db_file == NULL)
	{
		char str[256];
		sprintf(str, "Unable to open driver database file : %s\n", db_filename);
		free(db_filename);
		exit_with_error(str);
	}
	free(db_filename);

	current = start;
	while (current)
	{
		printf("%d\n",count++);
		fflush(stdout);
		
		if (process_file(current->str,db_file) == -1)
		{
			fclose(db_file);
			exit_with_error("Problem while scanning driver file");
		}
		current = current->next;
	}
	fclose(db_file);

	delete_all_nodes();

	return 0;
}

int list_files(const char *dirname)
{
	DIR	*dir = opendir(dirname);
	struct dirent	*entry;
	char		buffer[4096] = {0};
	int		len = strlen(dirname);

	if (dir == NULL)
	{
		fprintf(stderr, "Can't open drivers directory : %s\n", dirname);
		return -1;
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
					if (list_files(buffer) == -1)
					{
						closedir(dir);
						return -1;
					}
				}
				else if (S_ISREG(st.st_mode))
				{
					char	*c = strrchr(buffer,'.');
					if (c && strncmp(c,".ppd",4) == 0)
					{
						append_node(buffer);
					}
                                        else if (c && strncmp(c, ".gz", 3) == 0)
                                        { /* keep also compressed driver files */
                                        	while (c != buffer)
                                                {
                                                	if (*(--c) == '.') break;
                                                }
                                                if (*c == '.' && strncmp(c, ".ppd",4) == 0)
                                                {
                                                	append_node(buffer);
                                                }
                                        }
				}
			}
		}
	}
	closedir(dir);
	return 0;
}

int process_file(const char *filename, FILE *output_file)
{
	gzFile	ppd_file;
	char	line[4096], value[256];
	char	*c1, *c2;

	ppd_file = gzopen(filename,"r");
	if (ppd_file == NULL)
	{
		fprintf(stderr, "Can't open driver file : %s\n", filename);
		return -1;
	}
	fprintf(output_file,"FILE=%s\n",filename);

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
				while (*c1 && isspace(*c1))
					c1++;
				c2 = line+strlen(line)-1;	/* point to \n */
				while (*c2 && isspace(*c2))
					c2--;
				strncpy(value,c1,c2-c1);
			}
		}
		if (strncmp(line,"*Manufacturer:",14) == 0) fprintf(output_file,"MANUFACTURER=%s\n",value);
		else if (strncmp(line,"*ShortNickName:",15) == 0) fprintf(output_file,"MODEL=%s\n",value);
		else if (strncmp(line,"*ModelName:",11) == 0) fprintf(output_file,"MODELNAME=%s\n",value);
		else if (strncmp(line,"*NickName:",10) == 0) fprintf(output_file,"DESCRIPTION=%s\n",value);
		else if (strncmp(line,"*pnpManufacturer:",17) == 0) fprintf(output_file,"PNPMANUFACTURER=%s\n",value);
		else if (strncmp(line,"*pnpModel:",10) == 0) fprintf(output_file,"PNPMODEL=%s\n",value);
	}
	fprintf(output_file,"\n");

	gzclose(ppd_file);
	return 0;
}

void append_node(const char *str)
{
	if (!start)
	{
		start = (Node*)malloc(sizeof(Node));
		current = start;
	}
	else
	{
		current->next = (Node*)malloc(sizeof(Node));
		current = current->next;
	}
	current->next = 0;
	current->str = (char*)calloc(strlen(str)+1,sizeof(char));
	strncpy(current->str,str,strlen(str));
	node_number++;
}

void delete_all_nodes()
{
	while (start)
	{
		current = start;
		start = start->next;
		free(current->str);
		free(current);
	}
}

void exit_with_error(const char *txt)
{
	if (txt) fprintf(stderr,"%s\n",txt);
	delete_all_nodes();
	exit(-1);
}
