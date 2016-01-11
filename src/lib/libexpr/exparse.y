/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2013 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                 Eclipse Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*          http://www.eclipse.org/org/documents/epl-v10.html           *
*         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
%{

/*
 * Glenn Fowler
 * AT&T Research
 *
 * expression library grammar and compiler
 */

#include <ast.h>

#undef	RS	/* hp.pa <signal.h> grabs this!! */

%}

%union
{
	struct Exnode_s*expr;
	double		floating;
	struct Exref_s*	reference;
	struct Exid_s*	id;
	Sflong_t	integer;
	int		op;
	char*		string;
	struct Exbuf_s*	buffer;
}

%start	program

%token	MINTOKEN

%token	CHAR
%token	INT
%token	INTEGER
%token	UNSIGNED
%token	FLOATING
%token	STRING
%token	VOID
%token	STATIC

%token	ADDRESS
%token	BREAK
%token	CALL
%token	CASE
%token	CONSTANT
%token	CONTINUE
%token	DECLARE
%token	DEFAULT
%token	DYNAMIC
%token	ELSE
%token	EXIT
%token	FOR
%token	FUNCTION
%token	ITERATE
%token	ID
%token	IF
%token	LABEL
%token	MEMBER
%token	NAME
%token	POS
%token	PRAGMA
%token	PRE
%token	PRINTF
%token	PROCEDURE
%token	QUERY
%token	RETURN
%token	SCANF
%token	SPRINTF
%token	SSCANF
%token	SWITCH
%token	WHILE

%token	F2I
%token	F2S
%token	I2F
%token	I2S
%token	S2B
%token	S2F
%token	S2I

%token	F2X
%token	I2X
%token	S2X
%token	X2F
%token	X2I
%token	X2S

%left	<op>	','
%right	<op>	'='
%right	<op>	'?'	':'
%left	<op>	OR
%left	<op>	AND
%left	<op>	'|'
%left	<op>	'^'
%left	<op>	'&'
%binary	<op>	EQ	NE
%binary	<op>	'<'	'>'	LE	GE
%left	<op>	LS	RS
%left	<op>	'+'	'-'
%left	<op>	'*'	'/'	'%'
%right	<op>	'!'	'~'	UNARY
%right	<op>	INC	DEC
%right	<op>	CAST
%left	<op>	'('

%type <expr>		statement	statement_list	arg_list
%type <expr>		else_opt	expr_opt	expr
%type <expr>		args		variable	assign
%type <expr>		dcl_list	dcl_item	index
%type <expr>		initialize	switch_item	constant
%type <expr>		formals		formal_list	formal_item
%type <reference>	reference
%type <id>		ID		LABEL		NAME
%type <id>		CONSTANT	FUNCTION	DECLARE
%type <id>		EXIT		PRINTF		QUERY
%type <id>		SPRINTF		PROCEDURE	name
%type <id>		IF		WHILE		FOR
%type <id>		BREAK		CONTINUE	print
%type <id>		RETURN		DYNAMIC		SWITCH
%type <id>		SCANF		SSCANF		scan
%type <floating>	FLOATING
%type <integer>		INTEGER		UNSIGNED	array
%type <integer>		static
%type <string>		STRING

%token	MAXTOKEN

%{

#include "exgram.h"

%}

%%

program		:	statement_list action_list
		{
			if ($1 && !(expr.program->disc->flags & EX_STRICT))
			{
				if (expr.program->main.value && !(expr.program->disc->flags & EX_RETAIN))
					exfreenode(expr.program, expr.program->main.value);
				if ($1->op == S2B)
				{
					Exnode_t*	x;

					x = $1;
					$1 = x->data.operand.left;
					x->data.operand.left = 0;
					exfreenode(expr.program, x);
				}
				expr.program->main.lex = PROCEDURE;
				expr.program->main.value = exnewnode(expr.program, PROCEDURE, 1, $1->type, NiL, $1);
			}
		}
		;

action_list	:	/* empty */
		|	action_list action
		;

action		:	LABEL ':' {
				register Dtdisc_t*	disc;

				if (expr.procedure)
					exerror("no nested function definitions");
				$1->lex = PROCEDURE;
				expr.procedure = $1->value = exnewnode(expr.program, PROCEDURE, 1, $1->type, NiL, NiL);
				expr.procedure->type = INTEGER;
				if (!(disc = newof(0, Dtdisc_t, 1, 0)))
					exnospace();
				disc->key = offsetof(Exid_t, name);
				if (expr.assigned && !streq($1->name, "begin"))
				{
					if (!(expr.procedure->data.procedure.frame = dtopen(disc, Dtset)) || !dtview(expr.procedure->data.procedure.frame, expr.program->symbols))
						exnospace();
					expr.program->symbols = expr.program->frame = expr.procedure->data.procedure.frame;
				}
			} statement_list
		{
			expr.procedure = 0;
			if (expr.program->frame)
			{
				expr.program->symbols = expr.program->frame->view;
				dtview(expr.program->frame, NiL);
				expr.program->frame = 0;
			}
			if ($4 && $4->op == S2B)
			{
				Exnode_t*	x;

				x = $4;
				$4 = x->data.operand.left;
				x->data.operand.left = 0;
				exfreenode(expr.program, x);
			}
			$1->value->data.operand.right = excast(expr.program, $4, $1->type, NiL, 0);
		}
		;

statement_list	:	/* empty */
		{
			$$ = 0;
		}
		|	statement_list statement
		{
			if (!$1)
				$$ = $2;
			else if (!$2)
				$$ = $1;
			else if ($1->op == CONSTANT)
			{
				exfreenode(expr.program, $1);
				$$ = $2;
			}
			else if ($1->op == ';')
			{
				$$ = $1;
				$1->data.operand.last = $1->data.operand.last->data.operand.right = exnewnode(expr.program, ';', 1, $2->type, $2, NiL);
			}
			else
			{
				$$ = exnewnode(expr.program, ';', 1, $1->type, $1, NiL);
				$$->data.operand.last = $$->data.operand.right = exnewnode(expr.program, ';', 1, $2->type, $2, NiL);
			}
		}
		;

statement	:	'{' statement_list '}'
		{
			$$ = $2;
		}
		|	expr_opt ';'
		{
			$$ = ($1 && $1->type == STRING) ? exnewnode(expr.program, S2B, 1, INTEGER, $1, NiL) : $1;
		}
		|	static {expr.instatic=$1;} DECLARE {expr.declare=$3->type;} dcl_list ';'
		{
			$$ = $5;
			expr.declare = 0;
		}
		|	IF '(' expr ')' statement else_opt
		{
			if ($3->type == STRING)
				$3 = exnewnode(expr.program, S2B, 1, INTEGER, $3, NiL);
			else if (!INTEGRAL($3->type))
				$3 = excast(expr.program, $3, INTEGER, NiL, 0);
			$$ = exnewnode(expr.program, $1->index, 1, INTEGER, $3, exnewnode(expr.program, ':', 1, $5 ? $5->type : 0, $5, $6));
		}
		|	FOR '(' variable ')' statement
		{
			$$ = exnewnode(expr.program, ITERATE, 0, INTEGER, NiL, NiL);
			$$->data.generate.array = $3;
			if (!$3->data.variable.index || $3->data.variable.index->op != DYNAMIC)
				exerror("simple index variable expected");
			$$->data.generate.index = $3->data.variable.index->data.variable.symbol;
			if ($3->op == ID && $$->data.generate.index->type != INTEGER)
				exerror("integer index variable expected");
			exfreenode(expr.program, $3->data.variable.index);
			$3->data.variable.index = 0;
			$$->data.generate.statement = $5;
		}
		|	FOR '(' expr_opt ';' expr_opt ';' expr_opt ')' statement
		{
			if (!$5)
			{
				$5 = exnewnode(expr.program, CONSTANT, 0, INTEGER, NiL, NiL);
				$5->data.constant.value.integer = 1;
			}
			else if ($5->type == STRING)
				$5 = exnewnode(expr.program, S2B, 1, INTEGER, $5, NiL);
			else if (!INTEGRAL($5->type))
				$5 = excast(expr.program, $5, INTEGER, NiL, 0);
			$$ = exnewnode(expr.program, $1->index, 1, INTEGER, $5, exnewnode(expr.program, ';', 1, 0, $7, $9));
			if ($3)
				$$ = exnewnode(expr.program, ';', 1, INTEGER, $3, $$);
		}
		|	WHILE '(' expr ')' statement
		{
			if ($3->type == STRING)
				$3 = exnewnode(expr.program, S2B, 1, INTEGER, $3, NiL);
			else if (!INTEGRAL($3->type))
				$3 = excast(expr.program, $3, INTEGER, NiL, 0);
			$$ = exnewnode(expr.program, $1->index, 1, INTEGER, $3, exnewnode(expr.program, ';', 1, 0, NiL, $5));
		}
		|	SWITCH '(' expr {expr.declare=$3->type;} ')' '{' switch_list '}'
		{
			register Switch_t*	sw = expr.swstate;

			$$ = exnewnode(expr.program, $1->index, 1, INTEGER, $3, exnewnode(expr.program, DEFAULT, 1, 0, sw->defcase, sw->firstcase));
			expr.swstate = expr.swstate->prev;
			if (sw->base)
				free(sw->base);
			if (sw != &swstate)
				free(sw);
			expr.declare = 0;
		}
		|	BREAK expr_opt ';'
		{
		loopop:
			if (!$2)
			{
				$2 = exnewnode(expr.program, CONSTANT, 0, INTEGER, NiL, NiL);
				$2->data.constant.value.integer = 1;
			}
			else if (!INTEGRAL($2->type))
				$2 = excast(expr.program, $2, INTEGER, NiL, 0);
			$$ = exnewnode(expr.program, $1->index, 1, INTEGER, $2, NiL);
		}
		|	CONTINUE expr_opt ';'
		{
			goto loopop;
		}
		|	RETURN expr_opt ';'
		{
			if ($2)
			{
				if (expr.procedure && !expr.procedure->type)
					exerror("return in void function");
				$2 = excast(expr.program, $2, expr.procedure ? expr.procedure->type : INTEGER, NiL, 0);
			}
			$$ = exnewnode(expr.program, RETURN, 1, $2 ? $2->type : 0, $2, NiL);
		}
		;

switch_list	:	/* empty */
		{
			register Switch_t*		sw;
			int				n;

			if (expr.swstate)
			{
				if (!(sw = newof(0, Switch_t, 1, 0)))
				{
					exnospace();
					sw = &swstate;
				}
				sw->prev = expr.swstate;
			}
			else
				sw = &swstate;
			expr.swstate = sw;
			sw->type = expr.declare;
			sw->firstcase = 0;
			sw->lastcase = 0;
			sw->defcase = 0;
			sw->def = 0;
			n = 8;
			if (!(sw->base = newof(0, Extype_t*, n, 0)))
			{
				exnospace();
				n = 0;
			}
			sw->cur = sw->base;
			sw->last = sw->base + n;
		}
		|	switch_list switch_item
		;

switch_item	:	case_list statement_list
		{
			register Switch_t*	sw = expr.swstate;
			int			n;

			$$ = exnewnode(expr.program, CASE, 1, 0, $2, NiL);
			if (sw->cur > sw->base)
			{
				if (sw->lastcase)
					sw->lastcase->data.select.next = $$;
				else
					sw->firstcase = $$;
				sw->lastcase = $$;
				n = sw->cur - sw->base;
				sw->cur = sw->base;
				$$->data.select.constant = (Extype_t**)exalloc(expr.program, (n + 1) * sizeof(Extype_t*));
				memcpy($$->data.select.constant, sw->base, n * sizeof(Extype_t*));
				$$->data.select.constant[n] = 0;
			}
			else
				$$->data.select.constant = 0;
			if (sw->def)
			{
				sw->def = 0;
				if (sw->defcase)
					exerror("duplicate default in switch");
				else
					sw->defcase = $2;
			}
		}
		;

case_list	:	case_item
		|	case_list case_item
		;

case_item	:	CASE constant ':'
		{
			int	n;

			if (expr.swstate->cur >= expr.swstate->last)
			{
				n = expr.swstate->cur - expr.swstate->base;
				if (!(expr.swstate->base = newof(expr.swstate->base, Extype_t*, 2 * n, 0)))
				{
					exerror("too many case labels for switch");
					n = 0;
				}
				expr.swstate->cur = expr.swstate->base + n;
				expr.swstate->last = expr.swstate->base + 2 * n;
			}
			if (expr.swstate->cur)
			{
				$2 = excast(expr.program, $2, expr.swstate->type, NiL, 0);
				*expr.swstate->cur++ = &($2->data.constant.value);
			}
		}
		|	DEFAULT ':'
		{
			expr.swstate->def = 1;
		}
		;

static	:	/* empty */
		{
			$$ = 0;
		}
		|	STATIC
		{
			$$ = 1;
		}
		;

dcl_list	:	dcl_item
		|	dcl_list ',' dcl_item
		{
			if ($3)
				$$ = $1 ? exnewnode(expr.program, ',', 1, $3->type, $1, $3) : $3;
		}
		;

dcl_item	:	reference NAME {expr.id=$2;} array initialize
		{
			$$ = 0;
			if (!$2->type || expr.declare)
				$2->type = expr.declare;
			if ($1)
			{
				$2->index = MEMBER;
				if (!expr.program->disc->getf || !expr.program->symbols)
					exerror("%s: member references not supported", $1);
				else if ($5)
					exerror("%s: member references cannot be initialized", $2);
				else if (expr.program->disc->reff)
					(*expr.program->disc->reff)(expr.program, $$, $2, $1, NiL, EX_SCALAR, expr.program->disc);
			}
			else if ($5 && $5->op == PROCEDURE)
			{
				$2->lex = PROCEDURE;
				$2->value = $5;
			}
			else
			{
				$2->lex = DYNAMIC;
				$2->value = exnewnode(expr.program, 0, 0, 0, NiL, NiL);
				if ($4 && !$2->local.pointer)
				{
					Dtdisc_t*	disc;

					if (!(disc = newof(0, Dtdisc_t, 1, 0)))
						exnospace();
					disc->key = offsetof(Exassoc_t, name);
					if (!($2->local.pointer = (char*)dtopen(disc, Dtoset)))
						exerror("%s: cannot initialize associative array", $2->name);
				}
				if ($5)
				{
					if ($5->type != $2->type)
					{
						$5->type = $2->type;
						$5->data.operand.right = excast(expr.program, $5->data.operand.right, $2->type, NiL, 0);
					}
					$5->data.operand.left = exnewnode(expr.program, DYNAMIC, 0, $2->type, NiL, NiL);
					$5->data.operand.left->data.variable.symbol = $2;
					$$ = $5;
					if (!expr.program->frame && !expr.program->errors)
					{
						expr.assigned++;
						exeval(expr.program, $$, NiL);
					}
				}
				else if (!$4)
					$2->value->data.value = exzero($2->type);
			}
		}
		;

name		:	NAME
		|	DYNAMIC
		;

else_opt	:	/* empty */
		{
			$$ = 0;
		}
		|	ELSE statement
		{
			$$ = $2;
		}
		;

expr_opt	:	/* empty */
		{
			$$ = 0;
		}
		|	expr
		;

expr		:	'(' expr ')'
		{
			$$ = $2;
		}
		|	'(' DECLARE ')' expr	%prec CAST
		{
			$$ = ($4->type == $2->type) ? $4 : excast(expr.program, $4, $2->type, NiL, 0);
		}
		|	expr '<' expr
		{
			int	rel;

		relational:
			rel = INTEGER;
			goto coerce;
		binary:
			rel = 0;
		coerce:
			if (!$1->type)
			{
				if (!$3->type)
					$1->type = $3->type = rel ? STRING : INTEGER;
				else
					$1->type = $3->type;
			}
			else if (!$3->type)
				$3->type = $1->type;
			if ($1->type != $3->type)
			{
				if ($1->type == STRING)
					$1 = excast(expr.program, $1, $3->type, $3, 0);
				else if ($3->type == STRING)
					$3 = excast(expr.program, $3, $1->type, $1, 0);
				else if ($1->type == FLOATING)
					$3 = excast(expr.program, $3, FLOATING, $1, 0);
				else if ($3->type == FLOATING)
					$1 = excast(expr.program, $1, FLOATING, $3, 0);
			}
			if (!rel)
				rel = ($1->type == STRING) ? STRING : (($1->type == UNSIGNED) ? UNSIGNED : $3->type);
			$$ = exnewnode(expr.program, $2, 1, rel, $1, $3);
			if (!expr.program->errors && $1->op == CONSTANT && $3->op == CONSTANT)
			{
				$$->data.constant.value = exeval(expr.program, $$, NiL);
				$$->binary = 0;
				$$->op = CONSTANT;
				exfreenode(expr.program, $1);
				exfreenode(expr.program, $3);
			}
		}
		|	expr '-' expr
		{
			goto binary;
		}
		|	expr '*' expr
		{
			goto binary;
		}
		|	expr '/' expr
		{
			goto binary;
		}
		|	expr '%' expr
		{
			goto binary;
		}
		|	expr LS expr
		{
			goto binary;
		}
		|	expr RS expr
		{
			goto binary;
		}
		|	expr '>' expr
		{
			goto relational;
		}
		|	expr LE expr
		{
			goto relational;
		}
		|	expr GE expr
		{
			goto relational;
		}
		|	expr EQ expr
		{
			goto relational;
		}
		|	expr NE expr
		{
			goto relational;
		}
		|	expr '&' expr
		{
			goto binary;
		}
		|	expr '|' expr
		{
			goto binary;
		}
		|	expr '^' expr
		{
			goto binary;
		}
		|	expr '+' expr
		{
			goto binary;
		}
		|	expr AND expr
		{
		logical:
			if ($1->type == STRING)
				$1 = exnewnode(expr.program, S2B, 1, INTEGER, $1, NiL);
			if ($3->type == STRING)
				$3 = exnewnode(expr.program, S2B, 1, INTEGER, $3, NiL);
			goto binary;
		}
		|	expr OR expr
		{
			goto logical;
		}
		|	expr ',' expr
		{
			if ($1->op == CONSTANT)
			{
				exfreenode(expr.program, $1);
				$$ = $3;
			}
			else
				$$ = exnewnode(expr.program, ',', 1, $3->type, $1, $3);
		}
		|	expr '?' {expr.nolabel=1;} expr ':' {expr.nolabel=0;} expr
		{
			if (!$4->type)
			{
				if (!$7->type)
					$4->type = $7->type = INTEGER;
				else
					$4->type = $7->type;
			}
			else if (!$7->type)
				$7->type = $4->type;
			if ($1->type == STRING)
				$1 = exnewnode(expr.program, S2B, 1, INTEGER, $1, NiL);
			else if (!INTEGRAL($1->type))
				$1 = excast(expr.program, $1, INTEGER, NiL, 0);
			if ($4->type != $7->type)
			{
				if ($4->type == STRING || $7->type == STRING)
					exerror("if statement string type mismatch");
				else if ($4->type == FLOATING)
					$7 = excast(expr.program, $7, FLOATING, NiL, 0);
				else if ($7->type == FLOATING)
					$4 = excast(expr.program, $4, FLOATING, NiL, 0);
			}
			if ($1->op == CONSTANT)
			{
				if ($1->data.constant.value.integer)
				{
					$$ = $4;
					exfreenode(expr.program, $7);
				}
				else
				{
					$$ = $7;
					exfreenode(expr.program, $4);
				}
				exfreenode(expr.program, $1);
			}
			else
				$$ = exnewnode(expr.program, '?', 1, $4->type, $1, exnewnode(expr.program, ':', 1, $4->type, $4, $7));
		}
		|	'!' expr
		{
		iunary:
			if ($2->type == STRING)
				$2 = exnewnode(expr.program, S2B, 1, INTEGER, $2, NiL);
			else if (!INTEGRAL($2->type))
				$2 = excast(expr.program, $2, INTEGER, NiL, 0);
		unary:
			$$ = exnewnode(expr.program, $1, 1, $2->type == UNSIGNED ? INTEGER : $2->type, $2, NiL);
			if ($2->op == CONSTANT)
			{
				$$->data.constant.value = exeval(expr.program, $$, NiL);
				$$->binary = 0;
				$$->op = CONSTANT;
				exfreenode(expr.program, $2);
			}
		}
		|	'~' expr
		{
			goto iunary;
		}
		|	'-' expr	%prec UNARY
		{
			goto unary;
		}
		|	'+' expr	%prec UNARY
		{
			$$ = $2;
		}
		|	'&' variable	%prec UNARY
		{
			$$ = exnewnode(expr.program, ADDRESS, 0, T($2->type), $2, NiL);
		}
		|	reference FUNCTION '(' args ')'
		{
			$$ = exnewnode(expr.program, FUNCTION, 1, T($2->type), call($1, $2, $4), $4);
			if (!expr.program->disc->getf)
				exerror("%s: function references not supported", $$->data.operand.left->data.variable.symbol->name);
			else if (expr.program->disc->reff)
				(*expr.program->disc->reff)(expr.program, $$->data.operand.left, $$->data.operand.left->data.variable.symbol, $1, NiL, EX_CALL, expr.program->disc);
		}
		|	EXIT '(' expr ')'
		{
			if (!INTEGRAL($3->type))
				$3 = excast(expr.program, $3, INTEGER, NiL, 0);
			$$ = exnewnode(expr.program, EXIT, 1, INTEGER, $3, NiL);
		}
		|	PROCEDURE '(' args ')'
		{
			$$ = exnewnode(expr.program, CALL, 1, $1->type, NiL, $3);
			$$->data.call.procedure = $1;
		}
		|	print '(' args ')'
		{
			$$ = exnewnode(expr.program, $1->index, 0, $1->type, NiL, NiL);
			if ($3 && $3->data.operand.left->type == INTEGER)
			{
				$$->data.print.descriptor = $3->data.operand.left;
				$3 = $3->data.operand.right;
			}
			else
				switch ($1->index)
				{
				case QUERY:
					$$->data.print.descriptor = exnewnode(expr.program, CONSTANT, 0, INTEGER, NiL, NiL);
					$$->data.print.descriptor->data.constant.value.integer = 2;
					break;
				case PRINTF:
					$$->data.print.descriptor = exnewnode(expr.program, CONSTANT, 0, INTEGER, NiL, NiL);
					$$->data.print.descriptor->data.constant.value.integer = 1;
					break;
				case SPRINTF:
					$$->data.print.descriptor = 0;
					break;
				}
			$$->data.print.args = preprint($3);
		}
		|	scan '(' args ')'
		{
			register Exnode_t*	x;

			$$ = exnewnode(expr.program, $1->index, 0, $1->type, NiL, NiL);
			if ($3 && $3->data.operand.left->type == INTEGER)
			{
				$$->data.scan.descriptor = $3->data.operand.left;
				$3 = $3->data.operand.right;
			}
			else
				switch ($1->index)
				{
				case SCANF:
					$$->data.scan.descriptor = 0;
					break;
				case SSCANF:
					if ($3 && $3->data.operand.left->type == STRING)
					{
						$$->data.scan.descriptor = $3->data.operand.left;
						$3 = $3->data.operand.right;
					}
					else
						exerror("%s: string argument expected", $1->name);
					break;
				}
			if (!$3 || !$3->data.operand.left || $3->data.operand.left->type != STRING)
				exerror("%s: format argument expected", $1->name);
			$$->data.scan.format = $3->data.operand.left;
			for (x = $$->data.scan.args = $3->data.operand.right; x; x = x->data.operand.right)
			{
				if (x->data.operand.left->op != ADDRESS)
					exerror("%s: address argument expected", $1->name);
				x->data.operand.left = x->data.operand.left->data.operand.left;
			}
		}
		|	STRING '.' ID
		{
			$$ = exnewnode(expr.program, CONSTANT, 0, $3->type, NiL, NiL);
			if (!expr.program->disc->reff)
				exerror("%s: qualified identifier references not supported", $3->name);
			else
			{
				$$->data.constant.value = (*expr.program->disc->reff)(expr.program, $$, $3, NiL, $1, EX_SCALAR, expr.program->disc);
				$$->data.constant.reference = $3;
			}
		}
		|	variable assign
		{
			if ($2)
			{
				if ($1->op == ID && !expr.program->disc->setf)
					exerror("%s: variable assignment not supported", $1->data.variable.symbol->name);
				else
				{
					if (!$1->type)
						$1->type = $2->type;
#if 0
					else if ($2->type != $1->type && $1->type >= 0200)
#else
					else if ($2->type != $1->type)
#endif
					{
						$2->type = $1->type;
						$2->data.operand.right = excast(expr.program, $2->data.operand.right, $1->type, NiL, 0);
					}
					$2->data.operand.left = $1;
					$$ = $2;
				}
			}
		}
		|	INC variable
		{
		pre:
			if ($2->type == STRING)
				exerror("++ and -- invalid for string variables");
			$$ = exnewnode(expr.program, $1, 0, $2->type, $2, NiL);
			$$->subop = PRE;
		}
		|	variable INC
		{
		pos:
			if ($1->type == STRING)
				exerror("++ and -- invalid for string variables");
			$$ = exnewnode(expr.program, $2, 0, $1->type, $1, NiL);
			$$->subop = POS;
		}
		|	DEC variable
		{
			goto pre;
		}
		|	variable DEC
		{
			goto pos;
		}
		|	constant
		;

constant	:	CONSTANT
		{
			$$ = exnewnode(expr.program, CONSTANT, 0, $1->type, NiL, NiL);
			if (!expr.program->disc->reff)
				exerror("%s: identifier references not supported", $1->name);
			else
				$$->data.constant.value = (*expr.program->disc->reff)(expr.program, $$, $1, NiL, NiL, EX_SCALAR, expr.program->disc);
		}
		|	FLOATING
		{
			$$ = exnewnode(expr.program, CONSTANT, 0, FLOATING, NiL, NiL);
			$$->data.constant.value.floating = $1;
		}
		|	INTEGER
		{
			$$ = exnewnode(expr.program, CONSTANT, 0, INTEGER, NiL, NiL);
			$$->data.constant.value.integer = $1;
		}
		|	STRING
		{
			$$ = exnewnode(expr.program, CONSTANT, 0, STRING, NiL, NiL);
			$$->data.constant.value.string = $1;
		}
		|	UNSIGNED
		{
			$$ = exnewnode(expr.program, CONSTANT, 0, UNSIGNED, NiL, NiL);
			$$->data.constant.value.integer = $1;
		}
		;

print		:	PRINTF
		|	QUERY
		|	SPRINTF
		;

scan		:	SCANF
		|	SSCANF
		;

variable	:	reference ID index
		{
			$$ = exnewnode(expr.program, ID, 0, $2->type, NiL, NiL);
			$$->data.variable.symbol = QUALIFY($1, $2);
			$$->data.variable.reference = $1;
			if ($3 && !INTEGRAL($3->type))
				$3 = excast(expr.program, $3, INTEGER, NiL, 0);
			$$->data.variable.index = $3;
			if (!expr.program->disc->getf)
				exerror("%s: identifier references not supported", $2->name);
			else if (expr.program->disc->reff)
				(*expr.program->disc->reff)(expr.program, $$, $$->data.variable.symbol, $1, NiL, $3 ? 0 : EX_SCALAR, expr.program->disc);
			$$->type = $$->data.variable.symbol->type;
		}
		|	DYNAMIC index
		{
			$$ = exnewnode(expr.program, DYNAMIC, 0, $1->type, NiL, NiL);
			$$->data.variable.symbol = $1;
			$$->data.variable.reference = 0;
			if ((($$->data.variable.index = $2) == 0) != ($1->local.pointer == 0))
				exerror("%s: is%s an array", $1->name, $1->local.pointer ? "" : " not");
		}
		|	NAME
		{
			$$ = exnewnode(expr.program, ID, 0, 0, NiL, NiL);
			$$->data.variable.symbol = $1;
			$$->data.variable.reference = 0;
			$$->data.variable.index = 0;
			if (!(expr.program->disc->flags & EX_UNDECLARED))
				exerror("unknown identifier");
		}
		;

array		:	/* empty */
		{
			$$ = 0;
		}
		|	'[' ']'
		{
			$$ = 1;
		}
		;

index		:	/* empty */
		{
			$$ = 0;
		}
		|	'[' expr ']'
		{
			$$ = $2;
		}
		;

args		:	/* empty */
		{
			$$ = 0;
		}
		|	arg_list
		{
			$$ = $1->data.operand.left;
			$1->data.operand.left = $1->data.operand.right = 0;
			exfreenode(expr.program, $1);
		}
		;

arg_list	:	expr		%prec ','
		{
			$$ = exnewnode(expr.program, ';', 1, 0, exnewnode(expr.program, ';', 1, $1->type, $1, NiL), NiL);
			$$->data.operand.right = $$->data.operand.left;
		}
		|	arg_list ',' expr
		{
			$1->data.operand.right = $1->data.operand.right->data.operand.right = exnewnode(expr.program, ',', 1, $1->type, $3, NiL);
		}
		;

formals		:	/* empty */
		{
			$$ = 0;
		}
		|	DECLARE
		{
			$$ = 0;
			if ($1->type)
				exerror("(void) expected");
		}
		|	formal_list
		;

formal_list	:	formal_item
		{
			$$ = exnewnode(expr.program, ',', 1, $1->type, $1, NiL);
		}
		|	formal_list ',' formal_item
		{
			register Exnode_t*	x;
			register Exnode_t*	y;

			$$ = $1;
			for (x = $1; y = x->data.operand.right; x = y);
			x->data.operand.right = exnewnode(expr.program, ',', 1, $3->type, $3, NiL);
		}
		;

formal_item	:	DECLARE {expr.declare=$1->type;} name
		{
			$$ = exnewnode(expr.program, ID, 0, $3->type, NiL, NiL);
			$$->data.variable.symbol = $3;
			$3->lex = DYNAMIC;
			$3->value = exnewnode(expr.program, 0, 0, 0, NiL, NiL);
			expr.procedure->data.procedure.arity++;
			expr.declare = 0;
		}
		;

reference	:	/* empty */
		{
			$$ = expr.refs = expr.lastref = 0;
		}
		|	reference ID index '.'
		{
			Exref_t*	r;

			r = ALLOCATE(expr.program, Exref_t);
			if (expr.lastref)
			{
				r->symbol = QUALIFY(expr.lastref, $2);
				expr.lastref->next = r;
			}
			else
			{
				r->symbol = $2;
				expr.refs = r;
			}
			expr.lastref = r;
			r->next = 0;
			r->index = $3;
			$$ = expr.refs;
		}
		;

assign		:	/* empty */
		{
			$$ = 0;
		}
		|	'=' expr
		{
			$$ = exnewnode(expr.program, '=', 1, $2->type, NiL, $2);
			$$->subop = $1;
		}
		;

initialize	:	assign
		|	'(' {
				register Dtdisc_t*	disc;

				if (expr.procedure)
					exerror("%s: nested function definitions not supported", expr.id->name);
				expr.procedure = exnewnode(expr.program, PROCEDURE, 1, expr.declare, NiL, NiL);
				if (!(disc = newof(0, Dtdisc_t, 1, 0)))
					exnospace();
				disc->key = offsetof(Exid_t, name);
				if (!streq(expr.id->name, "begin"))
				{
					if (!(expr.procedure->data.procedure.frame = dtopen(disc, Dtset)) || !dtview(expr.procedure->data.procedure.frame, expr.program->symbols))
						exnospace();
					expr.program->symbols = expr.program->frame = expr.procedure->data.procedure.frame;
					expr.program->formals = 1;
				}
				expr.declare = 0;
			} formals {
				expr.id->lex = PROCEDURE;
				expr.id->type = expr.procedure->type;
				expr.program->formals = 0;
				expr.declare = 0;
			} ')' '{' statement_list '}'
		{
			$$ = expr.procedure;
			expr.procedure = 0;
			if (expr.program->frame)
			{
				expr.program->symbols = expr.program->frame->view;
				dtview(expr.program->frame, NiL);
				expr.program->frame = 0;
			}
			$$->data.operand.left = $3;
			$$->data.operand.right = excast(expr.program, $7, $$->type, NiL, 0);

			/*
			 * NOTE: procedure definition was slipped into the
			 *	 declaration initializer statement production,
			 *	 therefore requiring the statement terminator
			 */

			exunlex(expr.program, ';');
		}
		;

%%

#include "exgram.h"
