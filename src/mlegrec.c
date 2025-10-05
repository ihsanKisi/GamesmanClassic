/************************************************************************
**
** NAME:        mlegrec.c
**
** DESCRIPTION: Le Grec
**
** AUTHOR:      Josh Z, Nahee J, Ihsan K, Shaheer B
**
**************************************************************************/

/*************************************************************************
**
** Everything below here must be in every game file
**
**************************************************************************/

#include "gamesman.h"

#define BOARD_SIZE 16
#define MAX_MOVE_STRING_SIZE (20 * 2 + 2)

#define ENCODE_MOVE(start, end) ((start) << 16 | (end))
#define DECODE_MOVE_START(move) ((move) >> 16)
#define DECODE_MOVE_END(move) (0xffff & (move))
#define NEXT_PLAYER(player) (1 + ((player) % 2))
// X always starts first

POSITION gNumberOfPositions = 0;
POSITION kBadPosition = -1;

char playerPiece[] = " XO"; 

POSITION gInitialPosition = 0;
POSITION gMinimalPosition = 0;

CONST_STRING kAuthorName = "Josh Z, Nahee J, Ihsan K, Shaheer B";
CONST_STRING kGameName = "Le Grec";
BOOLEAN kPartizan = TRUE;
BOOLEAN kDebugMenu = TRUE;
BOOLEAN kGameSpecificMenu = FALSE;
BOOLEAN kTieIsPossible = FALSE;
BOOLEAN kLoopy = TRUE;
BOOLEAN kDebugDetermineValue = FALSE;
void* gGameSpecificTclInit = NULL;

CONST_STRING kHelpGraphicInterface = "On each turn, move a piece (orthogonally or diagonally) to an empty spot one tile away.\nYou may jump over an enemy counter (no jumps over your own counters), landing on an empty spot two tiles away.\nEvery move/jump must be forwards or sideways relative to your side, never backwards.";

CONST_STRING kHelpTextInterface = "On each turn, move a piece (orthogonally or diagonally) to an empty spot one tile away.\nYou may jump over an enemy counter (no jumps over your own counters), landing on an empty spot two tiles away.\nEvery move/jump must be forwards or sideways relative to your side, never backwards.";

CONST_STRING kHelpOnYourTurn = "Make your move using two lowercase letters from a-p, following the legend: <from_index><to_index>";

CONST_STRING kHelpStandardObjective = "Be the first to place all of your pieces in the two rows opposite from where they started.";

CONST_STRING kHelpReverseObjective = "No reverse objective yet.";

CONST_STRING kHelpTieOccursWhen = "Ties are not possible here.";

CONST_STRING kHelpExample = "https://sites.google.com/site/boardandpieces/list-of-games/le-grec";

/*************************************************************************
**
** Everything above here must be in every game file
**
**************************************************************************/

/*************************************************************************
**
** Every variable declared here is only used in this file (game-specific)
**
**************************************************************************/

BOOLEAN kSupportsSymmetries = FALSE; /* Whether we support symmetries */

/************************************************************************
**
** NAME: InitializeDatabases
**
** DESCRIPTION: Initialize the gDatabase, a global variable.
**
************************************************************************/

void InitializeGame()
{
  int hash_data[] = {' ', 2, 2, 'X', 7, 7, 'O', 7, 7, -1};
  gNumberOfPositions = generic_hash_init(BOARD_SIZE, hash_data, NULL, 0);
  char start[] = "XXXX XXXOOO OOOO";
  gInitialPosition = generic_hash_hash(start, 1);
  /* This game is the same game as Blocking and is known to be pure draw. */
  kUsePureDraw = FALSE;
}

/************************************************************************
**
** NAME: DebugMenu
**
** DESCRIPTION: Menu used to debub internal problems. Does nothing if
** kDebugMenu == FALSE
**
************************************************************************/

void DebugMenu()
{
}

/************************************************************************
**
** NAME: GameSpecificMenu
**
** DESCRIPTION: Menu used to change game-specific parmeters, such as
** the side of the board in an nxn Nim board, etc. Does
** nothing if kGameSpecificMenu == FALSE
**
************************************************************************/

void GameSpecificMenu()
{
}

/************************************************************************
**
** NAME: SetTclCGameSpecificOptions
**
** DESCRIPTION: Set the C game-specific options (called from Tcl)
** Ignore if you don't care about Tcl for now.
**
************************************************************************/

void SetTclCGameSpecificOptions(int theOptions[])
{
  (void)theOptions;
}

/************************************************************************
**
** NAME: DoMove
**
** DESCRIPTION: Apply the move to the position.
**
** INPUTS: POSITION position : The old position
** MOVE move : The move to apply.
**
** OUTPUTS: (POSITION) : The position that results after the move.
**
** CALLS: PositionToBlankOX(POSITION,*BlankOX)
** BlankOX WhosTurn(*BlankOX)
**
************************************************************************/

POSITION DoMove(POSITION position, MOVE move)
{
  char board[BOARD_SIZE];
  generic_hash_unhash(position, board);
  int start = DECODE_MOVE_START(move);
  int end = DECODE_MOVE_END(move);
  int player = generic_hash_turn(position);
  assert(board[end] == ' ');
  assert(board[start] != ' ');
  board[end] = board[start];
  board[start] = ' ';
  POSITION out = generic_hash_hash(board, NEXT_PLAYER(player));
  return out;
}

void UndoMove(MOVE move)
{
  (void)move;
  ExitStageRightErrorString("UndoMove not implemented.");
}

/************************************************************************
**
** NAME: PrintComputersMove
**
** DESCRIPTION: Nicely format the computers move.
**
** INPUTS: MOVE *computersMove : The computer's move.
** STRING computersName : The computer's name.
**
************************************************************************/

void PrintComputersMove(MOVE computersMove, STRING computersName) {
  int start = DECODE_MOVE_START(computersMove);
  int end = DECODE_MOVE_END(computersMove);

  char start_char = (char)('a' + start);
  char end_char = (char)('a' + end);

  printf("%s moved: %c %c\n", computersName, start_char, end_char);
}

/************************************************************************
**
** NAME: Primitive
**
** DESCRIPTION: Return the value of a position if it fulfills certain
** 'primitive' constraints. Some examples of this is having
** three-in-a-row with TicTacToe. TicTacToe has two
** primitives it can immediately check for, when the board
** is filled but nobody has one = primitive tie. Three in
** a row is a primitive lose, because the player who faces
** this board has just lost. I.e. the player before him
** created the board and won. Otherwise undecided.
**
** INPUTS: POSITION position : The position to inspect.
**
** OUTPUTS: (VALUE) an enum which is oneof: (win,lose,tie,undecided)
**
** CALLS: BOOLEAN ThreeInARow()
** BOOLEAN AllFilledIn()
** PositionToBlankOX()
**
************************************************************************/

VALUE Primitive(POSITION position)
{
    char board[BOARD_SIZE];
    generic_hash_unhash(position, board);
    char player = playerPiece[generic_hash_turn(position)];

    int x_count = 0;
    for (int i = 8; i < 16; i++) {
        if (board[i] == 'X') {
            x_count++;
        }
    }
    int o_count = 0;
    for (int i = 0; i < 8; i++) {
        if (board[i] == 'O') {
            o_count++;
        }
    }
    
    if (player == 'O' && x_count == 7) {
        return lose;
    } else if (player == 'X' && o_count == 7) {
        return lose;
    } else {
        return undecided;
    }
}

/************************************************************************
**
** NAME: PrintPosition
**
** DESCRIPTION: Print the position in a pretty format, including the
** prediction of the game's outcome.
**
** INPUTS: POSITION position : The position to pretty print.
** STRING playerName : The name of the player.
** BOOLEAN usersTurn : TRUE <==> it's a user's turn.
**
** CALLS: PositionToBlankOX()
** GetValueOfPosition()
** GetPrediction()
**
************************************************************************/

void PrintPosition(POSITION position, STRING playerName, BOOLEAN usersTurn)
{
  char board[BOARD_SIZE];
  generic_hash_unhash(position, board);
  printf("LEGEND: a b c d  |  BOARD: %c %c %c %c %s  |  \n", board[0], board[1], board[2], board[3], GetPrediction(position, playerName, usersTurn));
  printf("        e f g h  |         %c %c %c %c  |\n", board[4], board[5], board[6], board[7]);
  printf("        i j k l  |         %c %c %c %c  |\n", board[8], board[9], board[10], board[11]);
  printf("        m n o p  |         %c %c %c %c  |\n", board[12], board[13], board[14], board[15]);
  printf("\nIt is %s's turn (%c).\n\n", playerName, playerPiece[generic_hash_turn(position)]);
}

/************************************************************************
**
** NAME: GenerateMoves
**
** DESCRIPTION: Create a linked list of every move that can be reached
** from this position. Return a pointer to the head of the
** linked list.
**
** INPUTS: POSITION position : The position to branch off of.
**
** OUTPUTS: (MOVELIST *), a pointer that points to the first item
** in the linked list of moves that can be generated.
**
** CALLS: MOVELIST *CreateMovelistNode(MOVE,MOVELIST *)
**
************************************************************************/

MOVELIST *GenerateMoves(POSITION position)
{
    char board[BOARD_SIZE];
    generic_hash_unhash(position, board);
    
    int player = generic_hash_turn(position);
    MOVELIST *moves = NULL;

    char self;
    char enemy;
    if (playerPiece[player] == 'X') {
        self = 'X';
        enemy = 'O';
    } else {
        self = 'O';
        enemy = 'X';
    }

    // Check all 16 board tiles
    for (int from_index = 0; from_index < 16; from_index++) {
        int from_piece = board[from_index];
        if (from_piece == self) {
            int from_row = from_index / 4;
            int from_col = from_index % 4;

            // Check all 8 directions
            for (int dr = -1; dr <= 1; dr++) {
                for (int dc = -1; dc <= 1; dc++) {
                    if (dr == 0 && dc == 0) {
                        continue;
                    }

                    // Check for normal 1-tile move
                    int to_row = from_row + dr;
                    int to_col = from_col + dc;

                    if (0 <= to_row && to_row < 4 && 0 <= to_col && to_col < 4) {
                        BOOLEAN is_forward;
                        if (self == 'X') {
                            is_forward = (to_row >= from_row);
                        } else {
                            is_forward = (to_row <= from_row);
                        }

                        if (is_forward) {
                            int to_index = to_row * 4 + to_col;
                            int to_piece = board[to_index];

                            if (to_piece == ' ') { // Empty
                                moves = CreateMovelistNode(ENCODE_MOVE(from_index, to_index), moves);
                            }
                        }
                    }

                    // Check for jumping 2-tile move
                    int jump_over_row = from_row + dr;
                    int jump_over_col = from_col + dc;
                    int land_on_row = from_row + 2 * dr;
                    int land_on_col = from_col + 2 * dc;

                    if (0 <= land_on_row && land_on_row < 4 && 0 <= land_on_col && land_on_col < 4) {
                        BOOLEAN is_forward_jump;
                        if (self == 'X') {
                            is_forward_jump = (land_on_row >= from_row);
                        } else {
                            is_forward_jump = (land_on_row <= from_row);
                        }

                        // Check for jumping over enemy piece
                        if (is_forward_jump) {
                            int jump_over_index = jump_over_row * 4 + jump_over_col;
                            int jump_over_piece = board[jump_over_index];

                            int land_on_index = land_on_row * 4 + land_on_col;
                            int land_on_piece = board[land_on_index];

                            if (jump_over_piece == enemy && land_on_piece == ' ') {
                                moves = CreateMovelistNode(ENCODE_MOVE(from_index, land_on_index), moves);
                            }
                        }
                    }
                }
            }
        }
    }
    return moves;
}

/**************************************************/
/**************** SYMMETRY FUN BEGIN **************/
/**************************************************/

/************************************************************************
**
** NAME: GetCanonicalPosition
**
** DESCRIPTION: Go through all of the positions that are symmetrically
** equivalent and return the SMALLEST, which will be used
** as the canonical element for the equivalence set.
**
** INPUTS: POSITION position : The position return the canonical elt. of.
**
** OUTPUTS: POSITION : The canonical element of the set.
**
************************************************************************/

POSITION GetCanonicalPosition(POSITION position)
{
  return position;
}

/************************************************************************
**
** NAME: DoSymmetry
**
** DESCRIPTION: Perform the symmetry operation specified by the input
** on the position specified by the input and return the
** new position, even if it's the same as the input.
**
** INPUTS: POSITION position : The position to branch the symmetry from.
** int symmetry : The number of the symmetry operation.
**
** OUTPUTS: POSITION, The position after the symmetry operation.
**
************************************************************************/

POSITION DoSymmetry(POSITION position, int symmetry) {
  (void)symmetry;
  return position;
}

/**************************************************/
/**************** SYMMETRY FUN END ****************/
/**************************************************/

/************************************************************************
**
** NAME: GetAndPrintPlayersMove
**
** DESCRIPTION: This finds out if the player wanted an undo or abort or not.
** If so, return Undo or Abort and don't change theMove.
** Otherwise get the new theMove and fill the pointer up.
**
** INPUTS: POSITION *thePosition : The position the user is at.
** MOVE *theMove : The move to fill with user's move.
** STRING playerName : The name of the player whose turn it is
**
** OUTPUTS: USERINPUT : Oneof( Undo, Abort, Continue )
**
** CALLS: ValidMove(MOVE, POSITION)
** BOOLEAN PrintPossibleMoves(POSITION) ...Always True!
**
************************************************************************/

USERINPUT GetAndPrintPlayersMove(POSITION thePosition, MOVE *theMove, STRING playerName) {
  USERINPUT ret = Continue;
  do {
    printf("%s's move [<a-p> <a-p>]: ", playerName);

    ret = HandleDefaultTextInput(thePosition, theMove, playerName);
  } while (ret == Continue);
  return ret;
}

/************************************************************************
**
** NAME: ValidTextInput
**
** DESCRIPTION: Return TRUE iff the string input is of the right 'form'.
** For example, if the user is allowed to select one slot
** from the numbers 1-9, and the user chooses 0, it's not
** valid, but anything from 1-9 IS, regardless if the slot
** is filled or not. Whether the slot is filled is left up
** to another routine.
**
** INPUTS: STRING input : The string input the user typed.
**
** OUTPUTS: BOOLEAN : TRUE iff the input is a valid text input.
**
************************************************************************/

BOOLEAN ValidTextInput(STRING input)
{
  int i = 0;
  while (input[i] == ' ') i++;

  // Parse first token: a–p
  while (input[i] && (input[i] >= 'a' && input[i] <= 'p')) {
    i++;
  }
  while (input[i] == ' ') i++;

  // Parse second token: a–p
  while (input[i] && (isdigit(input[i]) || (input[i] >= 'a' && input[i] <= 'p'))) {
    i++;
  }
  while (input[i] == ' ') i++;

  // Must end cleanly (no extra junk)
  return !input[i];
}

/************************************************************************
**
** NAME: ConvertTextInputToMove
**
** DESCRIPTION: Convert the string input to the internal move representation.
**
** INPUTS: STRING input : The string input the user typed.
**
** OUTPUTS: MOVE : The move corresponding to the user's input.
**
************************************************************************/

MOVE ConvertTextInputToMove(STRING input)
{
    const char *c = input;
    int start = -1, end = -1;

    while (*c == ' ') c++;

    // Parse first token: a–p
    if (*c >= 'a' && *c <= 'p') {
        start = *c - 'a';
        c++;
    }
    while (*c == ' ') c++;

    // Parse second token: a-p
    if (*c >= 'a' && *c <= 'p') {
        end = *c - 'a';
        c++;
    }
    while (*c == ' ') c++;

    // (Optional) sanity if you want:
    // assert(0 <= start && start < 16 && 0 <= end && end < 16);

    return ENCODE_MOVE(start, end);
}


/************************************************************************
**
** NAME: PrintMove
**
** DESCRIPTION: Print the move in a nice format.
**
** INPUTS: MOVE *theMove : The move to print.
**
************************************************************************/

void PrintMove(MOVE move)
{
  int start = DECODE_MOVE_START(move);
  int end = DECODE_MOVE_END(move);

  char start_char = (char)('a' + start);
  char end_char = (char)('a' + end);

  printf("%c %c", start_char, end_char);
}


/************************************************************************
**
** NAME: MoveToString
**
** DESCRIPTION: Returns the move as a STRING
**
** INPUTS: MOVE *move : The move to put into a string.
**
************************************************************************/

void MoveToString(MOVE move, char *moveStringBuffer)
{
  int start = DECODE_MOVE_START(move);
  int end = DECODE_MOVE_END(move);

  char start_char = (char)('a' + start);
  char end_char = (char)('a' + end);

  snprintf(moveStringBuffer, MAX_MOVE_STRING_SIZE, "%c %c", start_char, end_char);
}

CONST_STRING kDBName = "legrec";

int NumberOfOptions()
{
  return 1;
}

int getOption()
{
  return 0;
}

void setOption(int option)
{
  (void)option;
}

POSITION ActualNumberOfPositions(int variant)
{
  (void)variant;
  return 0;
}

POSITION StringToPosition(char *positionString) {
	(void) positionString;
	return NULL_POSITION;
}

void PositionToAutoGUIString(POSITION position, char *autoguiPositionStringBuffer) {
	(void) position;
	(void) autoguiPositionStringBuffer;
}

void MoveToAutoGUIString(POSITION position, MOVE move, char *autoguiMoveStringBuffer) {
	(void) position;
	(void) move;
	(void) autoguiMoveStringBuffer;
}
