typedef union {
  int                 ival;
  double              dval;
  UString             *ustr;
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
#define	NUMBER	262
#define	BREAK	263
#define	CASE	264
#define	DEFAULT	265
#define	FOR	266
#define	NEW	267
#define	VAR	268
#define	CONTINUE	269
#define	FUNCTION	270
#define	RETURN	271
#define	VOID	272
#define	DELETE	273
#define	IF	274
#define	THIS	275
#define	DO	276
#define	WHILE	277
#define	ELSE	278
#define	IN	279
#define	INSTANCEOF	280
#define	TYPEOF	281
#define	SWITCH	282
#define	WITH	283
#define	RESERVED	284
#define	THROW	285
#define	TRY	286
#define	CATCH	287
#define	FINALLY	288
#define	EQEQ	289
#define	NE	290
#define	STREQ	291
#define	STRNEQ	292
#define	LE	293
#define	GE	294
#define	OR	295
#define	AND	296
#define	PLUSPLUS	297
#define	MINUSMINUS	298
#define	LSHIFT	299
#define	RSHIFT	300
#define	URSHIFT	301
#define	PLUSEQUAL	302
#define	MINUSEQUAL	303
#define	MULTEQUAL	304
#define	DIVEQUAL	305
#define	LSHIFTEQUAL	306
#define	RSHIFTEQUAL	307
#define	URSHIFTEQUAL	308
#define	ANDEQUAL	309
#define	MODEQUAL	310
#define	XOREQUAL	311
#define	OREQUAL	312
#define	IDENT	313
#define	AUTO	314


extern YYSTYPE kjsyylval;
