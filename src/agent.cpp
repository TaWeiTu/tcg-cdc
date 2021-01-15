#include "agent.h"

Agent::Agent() : color_(UNKNOWN) {}

void Agent::OpponentMove(uint8_t src, uint8_t dst) {
  board_.MakeMove(Move(src, dst));
}

void Agent::OpponentFlip(uint8_t pos, ChessPiece result) {
  board_.MakeMove(Flip(pos, result));
}

ChessMove Agent::GenerateMove() {
  if (color_ == UNKNOWN) return Flip(0);
}
