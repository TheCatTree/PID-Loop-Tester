/* just like Unix wc */
%{
#include "loopcommands.tab.h"
#include "loopcommands.h"
%}

%%

[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?  { yylval.f= atof(yytext); return NUMBER;}
[pPiIdD][kK] {  yylval.xk = yytext[1];return XK;}
(L|l)(oop)(_)[0-9] { yylval.xk = atof(yytext+5); return LOOPID;}
\n         { return EOL;}
.          { printf("Mystery character %c\n", *yytext); }

%%

