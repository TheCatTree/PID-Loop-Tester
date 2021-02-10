#define NUMLOOPS 3
#include <stdbool.h>

/* interface to the lexer */
extern int yylineno; /* from lexer */
void yyerror(char *s);

typedef enum command_flag {
  F_ON,
  F_OFF,
  F_TOGGLE,
  C_PK,
  C_IK,
  C_DK,
} command_flag_t;

typedef enum xk_flag {
  F_PK,
  F_IK,
  F_DK,
} xk_flag_t;

typedef enum loop_id {
  LI_update,
  LI_position,
  LI_speed,
} loop_id_t;



typedef struct cmnd {
  loop_id_t id;
  unsigned int flags;
  float pk;
  float ik;
  float dk;
} cmnd_t;


cmnd_t cmd_array[NUMLOOPS];


extern int yychar;

void loopsToTest(int loop);
void flagedLoopsOn( void );
void clearLoopTestFlags( void );
void setUpCommands(cmnd_t * xks_a, int number_loops);
void allOn( void );
void allOff( void );
void updateCommandXk(int lId, xk_flag_t xk, float value);
void sloppy_print(const char *s, ...);
void updateWantedSpeed( int x);
void updateWantedPosition( int x);