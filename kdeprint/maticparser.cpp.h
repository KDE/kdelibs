typedef union {
	QString	*string;
	MHash	*hash;
	int	num;
} YYSTYPE;
#define	STRING	257
#define	UNDEF	258
#define	VAR	259
#define	NUM	260


extern YYSTYPE maticlval;
