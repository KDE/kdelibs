typedef union {
	QList<QString>	*list;
	QString		*string;
	BlockT		*block;
} YYSTYPE;
#define	PRINTER	257
#define	DRIVER	258
#define	INTERFACE	259
#define	LANGUAGE_DRIVER	260
#define	LOCATION	261
#define	MODEL	262
#define	DRIVER_OPTS	263
#define	DRIVER_ARGS	264
#define	INTERFACE_OPTS	265
#define	INTERFACE_ARGS	266
#define	JOB_DIR	267
#define	HELP	268
#define	OPTION	269
#define	ARGUMENT	270
#define	VAR	271
#define	DESC	272
#define	CHOICE	273
#define	REQUIRES	274
#define	REQUIRED_ARGS	275
#define	DEF_VALUE	276
#define	DEFAULT_CHOICE	277
#define	VALUE	278
#define	FILETYPE_REGX	279
#define	DRIVER_COMMAND_PATH	280
#define	INTERFACE_COMMAND_PATH	281
#define	TRY_INCLUDE	282
#define	INCLUDE	283
#define	DEFAULT_PRINTER	284
#define	STRING	285
#define	IDENTIFIER	286
#define	SCRIPT_START	287
#define	SCRIPT_BODY	288
#define	COMMENT	289


extern YYSTYPE yylval;
