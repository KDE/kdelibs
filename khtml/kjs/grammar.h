typedef union {
  int                 ival;
  double              dval;
  CString             *cstr;
  UString             *ustr;
  void                *rxp; /* TODO */
  Node                *node;
  StatementNode       *stat;
  ParameterNode       *param;
  FuncDeclNode        *func;
  ProgramNode         *prog;
  AssignExprNode      *init;
  SourceElementNode   *src;
  SourceElementsNode  *srcs;
  StatListNode        *slist;
  ArgumentsNode       *args;
  ArgumentListNode    *alist;
  VarDeclNode         *decl;
  VarDeclListNode     *vlist;
  CaseBlockNode       *cblk;
  ClauseListNode      *clist;
  CaseClauseNode      *ccl;
  Operator            op;
} YYSTYPE;
#define	LF	258
#define	NULLTOKEN	259
#define	TRUETOKEN	260
#define	FALSETOKEN	261
#define	STRING	262
#define	DECIMAL	263
#define	INTEGER	264
#define	REGEXP	265
#define	BREAK	266
#define	CASE	267
#define	DEFAULT	268
#define	FOR	269
#define	NEW	270
#define	VAR	271
#define	CONTINUE	272
#define	FUNCTION	273
#define	RETURN	274
#define	VOID	275
#define	DELETE	276
#define	IF	277
#define	THIS	278
#define	DO	279
#define	WHILE	280
#define	ELSE	281
#define	IN	282
#define	INSTANCEOF	283
#define	TYPEOF	284
#define	SWITCH	285
#define	WITH	286
#define	RESERVED	287
#define	THROW	288
#define	TRY	289
#define	CATCH	290
#define	FINALLY	291
#define	DEBUG	292
#define	EQEQ	293
#define	NE	294
#define	STREQ	295
#define	STRNEQ	296
#define	LE	297
#define	GE	298
#define	OR	299
#define	AND	300
#define	PLUSPLUS	301
#define	MINUSMINUS	302
#define	LSHIFT	303
#define	RSHIFT	304
#define	URSHIFT	305
#define	PLUSEQUAL	306
#define	MINUSEQUAL	307
#define	MULTEQUAL	308
#define	DIVEQUAL	309
#define	LSHIFTEQUAL	310
#define	RSHIFTEQUAL	311
#define	URSHIFTEQUAL	312
#define	ANDEQUAL	313
#define	MODEQUAL	314
#define	XOREQUAL	315
#define	OREQUAL	316
#define	DOUBLE	317
#define	IDENT	318


extern YYSTYPE kjsyylval;
