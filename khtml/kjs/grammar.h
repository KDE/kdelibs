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
  ElementNode         *elm;
  ElisionNode         *eli;
  Operator            op;
} YYSTYPE;
#define	NULLTOKEN	258
#define	TRUETOKEN	259
#define	FALSETOKEN	260
#define	STRING	261
#define	DECIMAL	262
#define	INTEGER	263
#define	REGEXP	264
#define	BREAK	265
#define	CASE	266
#define	DEFAULT	267
#define	FOR	268
#define	NEW	269
#define	VAR	270
#define	CONTINUE	271
#define	FUNCTION	272
#define	RETURN	273
#define	VOID	274
#define	DELETE	275
#define	IF	276
#define	THIS	277
#define	DO	278
#define	WHILE	279
#define	ELSE	280
#define	IN	281
#define	INSTANCEOF	282
#define	TYPEOF	283
#define	SWITCH	284
#define	WITH	285
#define	RESERVED	286
#define	THROW	287
#define	TRY	288
#define	CATCH	289
#define	FINALLY	290
#define	DEBUG	291
#define	EQEQ	292
#define	NE	293
#define	STREQ	294
#define	STRNEQ	295
#define	LE	296
#define	GE	297
#define	OR	298
#define	AND	299
#define	PLUSPLUS	300
#define	MINUSMINUS	301
#define	LSHIFT	302
#define	RSHIFT	303
#define	URSHIFT	304
#define	PLUSEQUAL	305
#define	MINUSEQUAL	306
#define	MULTEQUAL	307
#define	DIVEQUAL	308
#define	LSHIFTEQUAL	309
#define	RSHIFTEQUAL	310
#define	URSHIFTEQUAL	311
#define	ANDEQUAL	312
#define	MODEQUAL	313
#define	XOREQUAL	314
#define	OREQUAL	315
#define	DOUBLE	316
#define	IDENT	317
#define	AUTO	318


extern YYSTYPE kjsyylval;
