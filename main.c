#include <mem.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// List-Format:
//  Every List is a char[64] where the first element tells how long the list is.
//  For example, the first list generated will be: {4, 19, 29, 34, 44, ... }, where the rest of the list is irrelevant.

// Player-Format & The HeuristicScores-values:
//  The char's named Player can only be 1 or -1. To switch player I use a unary minus.
//  The HeuristicScores-values describes how valuable the pieces on theese positions are.
//  You might want to change theese to tweak the AI.

// For my convienience:
#define bool char
#define true 1
#define false 0

// Definies the "smartness" of the AI. 0 = Easy, 1 = Normal, 3 = Hard, 4 = Very Hard.
#define Smartness 4
char PreAllocatedBoards[Smartness][64]; // Pre-allocate intensively used memmory.
char PreAllocatedMovesPlayer[Smartness][64]; // Pre-allocate intensively used memmory.
char PreAllocatedMovesOpponent[Smartness][64]; // Pre-allocate intensively used memmory.

char GetBestMove(char Board[], char Player);
void GetLegalMoves(char List[], char Board[], char Player);
bool MoveIsLegal(char Position, char Board[], char Player);
bool StepIsLegal(char Position, char OffSet);

bool FoundRow(char Position, char Direction, char Board[]);

void Move(char Position, char Board[], char Player);
void FlipRow(char Position, char Direction, char Board[], char Player);
int MiniMax(char Board[], char Player, char SearchDepth);


// Defining the heuristic score for each square. Thanx to Jesper Antonsson, jesan733@student.liu.se for the idea.
char HeuristicScores[64]={
  8,-4, 6, 4, 4, 6,-4, 8,
 -4,-4, 0, 0, 0, 0,-4,-4,
  6, 0, 2, 2, 2, 2, 0, 6,
  4, 0, 2, 1, 1, 2, 0, 4,
  4, 0, 2, 1, 1, 2, 0, 4,
  6, 0, 2, 2, 2, 2, 0, 6,
 -4,-4, 0, 0, 0, 0,-4,-4,
  8,-4, 6, 4, 4, 6,-4, 8
};

// Definying offsets for the 8 directions. Upp-left, Upp, Upp-right, ..., Down-right. The order doesn't really matter.
char OffSet[8]={
 -9,-8,-7,
 -1,    1,
  7, 8, 9
};
  
void GetLegalMoves(char List[], char Board[], char Player){
  int i;
  int j=0;

  for(i=0; i<=63; i++) // Loop through all squares to find legal moves and add them to the list.
    if(MoveIsLegal(i, Board, Player)){
      j++;
      List[j] = i;
    }

  List[0] = j; // The first char in the list tells how many moves the list contains. (Even though the size of the list is allways 64 bytes.)
}

bool MoveIsLegal(char Position, char Board[], char Player){
  char CurrentPosition;
  char Direction;
  char StepsMoved;

  if(Board[Position]) return false; // We may only put pieces in empty squares.

  for(Direction=0; Direction<8; Direction++){ // Test every direction.
    if(!StepIsLegal(Position, OffSet[Direction])) continue; // Skip this direction if one may not step there.
    CurrentPosition = Position + OffSet[Direction]; // Start steping one square from Position.
    StepsMoved = 0;  

    while(Board[CurrentPosition] == -Player && StepIsLegal(CurrentPosition, OffSet[Direction]) ){ // Take a step in Direction as long as it is legal (we may not step out of the board) and the pices belongs to Opponent (-Player).
      CurrentPosition += OffSet[Direction]; // Step to the next square in Direction.
      StepsMoved++;
    }
    if(StepsMoved > 0 && Board[CurrentPosition] == Player ) return true; // If this is true, we have found a comlete row.
  }
  
  return false; // If no legal move is found in either direction, this move is illegal.
}

bool StepIsLegal(char Position, char OffSet){
  // Take care of left, ...
  if(Position%8 == 0 && (OffSet == -9 || OffSet == -1 || OffSet == 7)) return false;
  // ... right, ... 
  if(Position%8 == 7 && (OffSet == -7 || OffSet == 1 || OffSet == 9)) return false;
  // ... upper, ... 
  if(Position/8 == 0 && OffSet < 0 && OffSet != -1) return false;
  // ... and lower edge.
  if(Position/8 == 7 && OffSet > 0 && OffSet != 1) return false;

  // The step is not illegal, return true.
  return true;
}

// Make shure there IS a legal move before you call this function.
char GetBestMove(char Board[], char Player){
  char LegalMoves[64];
  char NewBoard[64];
  char i;
  int Score, BestScore;
  char BestMove;

  GetLegalMoves(LegalMoves, Board, Player);

  Score = -65000;
  BestScore = -65000;
  BestMove = LegalMoves[1];

  for(i=1; i<=LegalMoves[0]; i++){
    memcpy(NewBoard, Board, 64);

    Move(LegalMoves[i], NewBoard, Player);

    Score = -MiniMax(NewBoard, -Player, Smartness);
    
    if(Score > BestScore){
      BestScore = Score;
      BestMove = LegalMoves[i];
    }
  }

  return BestMove;
}

int MiniMax(char Board[], char Player, char SearchDepth){
  int Score, BestScore;
  char i;
  char* NewBoard;
  char* MoveListPlayer;
  char* MoveListOpponent;
  char* TempList;
  char PlayerCount;
  char OpponentCount;

  NewBoard = PreAllocatedBoards[SearchDepth-1];
  MoveListPlayer = PreAllocatedMovesPlayer[SearchDepth-1];
  MoveListOpponent = PreAllocatedMovesOpponent[SearchDepth-1];

  // Check for Game Over.
  GetLegalMoves(MoveListPlayer, Board, Player); // The Player's moves.
  GetLegalMoves(MoveListOpponent, Board, -Player); // The opponent's moves.
  if(!MoveListPlayer[0] && !MoveListOpponent[0]){
    // Count the pieces.
    for(i=0; i<64; i++)
      if(Board[i] == Player)
        PlayerCount++;
      else if(Board[i] == -Player)
        OpponentCount++;
    
    // Reward the winner.
    if(PlayerCount > OpponentCount)
      return 65000;
    else if(PlayerCount < OpponentCount)
      return -65000;
    else
      return 0;
  }
    
  // Switch player if Player has no moves.
  if(!MoveListPlayer[0]){
    Player = -Player;
    TempList = MoveListPlayer;
    MoveListPlayer = MoveListOpponent;
    MoveListOpponent = TempList;
  }

  // Try the moves and return the best score.
  Score = -65000;
  BestScore = -65000;
  for(i=1; i<=MoveListPlayer[0]; i++){
    memcpy(NewBoard, Board, 64);
    
    Move(MoveListPlayer[i], NewBoard, Player);

    if(SearchDepth > 0)
      Score = -MiniMax(NewBoard, -Player, SearchDepth-1);
    else{
      // Heuristics
    
      Score=0;

      // Reward the player if he has more legal moves to choose from than the opponent.
      Score += MoveListPlayer[0];
      Score -= MoveListOpponent[0];

      // Reward the player if he has more (weighted) pieces than the opponent.
      for(i=0; i<64; i++)
        Score += HeuristicScores[i]*Player*Board[i];

    }
    if(Score > BestScore)
      BestScore = Score;
  }

  return BestScore;
}

// Make shure you MAY move before you call this function.
void Move(char Position, char Board[], char Player){
  char Direction;
  char CurrentPosition;
  char StepsMoved;
  
  Board[Position] = Player;

  for(Direction=0; Direction<8; Direction++){
    if(!StepIsLegal(Position, OffSet[Direction])) continue; // Skip this direction if one may not step there.
    CurrentPosition = Position + OffSet[Direction]; // Start steping one square from Position.
    StepsMoved = 0;

    while(Board[CurrentPosition] == -Player && StepIsLegal(CurrentPosition, OffSet[Direction])){
      CurrentPosition += OffSet[Direction];
      StepsMoved++;
    }
    // If we found a row:
    if(StepsMoved > 0 && Board[CurrentPosition] == Player ){
      // Flip
      for(; StepsMoved>0; StepsMoved--){
        CurrentPosition -= OffSet[Direction];
        Board[CurrentPosition] = Player;
      }
    }
  }
}














// Text-mode interface. Sucks but works.

void PrintMoves(char Moves[]){
  char i;

  printf("%i legal moves: ", Moves[0]);

  for(i=1; i<=Moves[0]; i++)
    printf("%i, ", Moves[i]);
    
  printf("\n");
}
void PrintBoard(char Board[], char MarkedPosition, char Player){
  char Xpos=MarkedPosition%8;
  char Ypos=MarkedPosition/8;

  char x;
  char y;
  
  char pl1Count=0;
  char pl2Count=0;
  char i;
  for(i=0; i<64; i++)
    if(Board[i] == 1)
      pl1Count++;
    else if(Board[i] == -1)
      pl2Count++;
  
  
  system("CLS");
  printf("\n  Player: "); if(Player == 1) printf("X"); else printf("O"); printf("   X:%i O:%i", pl1Count, pl2Count);
  printf("\n  +-+-+-+-+-+-+-+-+\n");
  for(y=0; y<8; y++){
    printf("  |");
    for(x=0; x<8; x++){
      if(Board[x+y*8] == 1)
        printf("X|");
      else if(Board[x+y*8] == -1)
        printf("O|");
      else
        printf(" |");
    }
    if(y==Ypos)
      printf(" <-");
    printf("\n  +-+-+-+-+-+-+-+-+\n");
  }

  for(x=0; x<=Xpos; x++)
    printf("  ");
  printf(" ^\n");
  for(x=0; x<=Xpos; x++)
    printf("  ");
  printf(" |\n");
}

// Let the user pick a move. Returns -1 if he/she wants to quit.
char GetUserMove(char Board[], char Player, char MarkedPosition){
  char Key;

  PrintBoard(Board, MarkedPosition, Player);

  do{
    Key = getch();

    if(Key == 27) return -1; // [ESC]-key
    else if(Key == 'K' && StepIsLegal(MarkedPosition, -1))
      MarkedPosition -= 1;
    else if(Key == 'M' && StepIsLegal(MarkedPosition, +1))
      MarkedPosition += 1;
    else if(Key == 'H' && StepIsLegal(MarkedPosition, -8))
      MarkedPosition -= 8;
    else if(Key == 'P' && StepIsLegal(MarkedPosition, +8))
      MarkedPosition += 8;
    
    PrintBoard(Board, MarkedPosition, Player);

  }while(Key != ' ');

  return MarkedPosition;
}






int main(){

  char Player = 1;
  char MarkedPosition = 0;
  char UserMove;
  char MoveList[64];
  char Board[64]={
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0,-1, 1, 0, 0, 0,
    0, 0, 0, 1,-1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
  };
  
  

  GetLegalMoves(MoveList, Board, Player);
  MarkedPosition = MoveList[1];

  do{
    GetLegalMoves(MoveList, Board, Player);
    if(MoveList[0]){
      if(Player == 1){
        // User input.
//        MarkedPosition = GetBestMove(Board, Player);
//        MarkedPosition = MoveList[1];
        UserMove = GetUserMove(Board, Player, MarkedPosition);

        if(MoveIsLegal(UserMove, Board, Player)){
          Move(UserMove, Board, Player);
          PrintBoard(Board, UserMove, Player);
          Player = -Player;
        }
      }
      else{
        // AI
        MarkedPosition = GetBestMove(Board, Player);
        Move(MarkedPosition, Board, Player);
        Player = -Player;
      }

    }
    else{
      Player = -Player;
      GetLegalMoves(MoveList, Board, Player);
      if(!MoveList[0]){
        UserMove = -1; // Quit
        PrintBoard(Board, MarkedPosition, Player);
        printf("  Game Over\n\n");
        system("PAUSE");
      }
    }
  }while(UserMove >= 0);





  return 0;
}


