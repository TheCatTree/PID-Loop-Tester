/* simplest version of calculator */
%define api.push-pull push

%{
#include <stdio.h>
#include "loopcommands.h"
%}

%union {
  int id;
  float f;
  int xk;
}



/* declare tokens */
%token <f> NUMBER
%token LOOPID
%token ON
%token OFF
%token <xk> XK 
%token EOL
%token SPEED
%token POSITION

%%

command: /* nothing */                       
 | xkseq EOL
 | onoff EOL
 | onoff xkseq EOL
 | EOL
 | parameterList EOL
 | xkseq parameterList EOL
 | onoff xkseq parameterList EOL
 | onoff parameterList xkseq EOL
 ;

onoff: ON {allOn();}
 | OFF  {allOff();}
 ;

xkseq: xkset
 | xkseq xkset
;

parameterList: parameter
 | parameterList parameter
;

parameter: SPEED NUMBER { updateWantedSpeed($2);}  
 | POSITION NUMBER { updateWantedPosition($2);}
;
xkset: LOOPID NUMBER XK NUMBER { updateCommandXk( $2, $3, $4);} 
 ;
%%

