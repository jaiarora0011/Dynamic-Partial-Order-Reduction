D   [0-9]
NZ  [1-9]
L   [a-zA-Z_]
A   [a-zA-Z_0-9]

WS  [ \t\v\n\f]

%{
#include <stdio.h>
#include <string.h>
#include "program.hpp"
#include "parse.tab.hpp"

extern void yyerror(const char *);  /* prints grammar violation message */

static void comment(void);
static int check_type(void);
#define YY_DECL extern "C" int yylex()
%}

%%
"/*"                                    { comment(); }
"//".*                                    { /* consume //-comment */ }

"PROGRAM_ORDER"                              { return PO; }
"release" { return RELEASE; }
"acquire" { return ACQUIRE; }

{L}{A}*					{ yylval.stringVal = strdup(yytext); return IDENTIFIER; }

{NZ}{D}*				{ yylval.intVal = atoi(yytext); return I_CONSTANT; }

";"					{ return ';'; }
("{"|"<%")				{ return '{'; }
("}"|"%>")				{ return '}'; }
","					{ return ','; }
":"					{ return ':'; }
":="					{ return ASSIGN; }
"("					{ return '('; }
")"					{ return ')'; }


{WS}+					{ /* whitespace separates tokens */ }
.					{ /* discard bad characters */ }

%%

int yywrap(void)        /* called at end of input */
{
    return 1;           /* terminate now */
}

static void comment(void)
{
    int c;

    while ((c = yyinput()) != 0)
        if (c == '*')
        {
            while ((c = yyinput()) == '*')
                ;

            if (c == '/')
                return;

            if (c == 0)
                break;
        }
    yyerror("unterminated comment");
}
