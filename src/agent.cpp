#include "agent.h"

#include <algorithm>
#include <cassert>
#include <iostream>

#include "chess.h"

Agent::Agent() : color_(UNKNOWN) {}

void Agent::OpponentMove(uint8_t src, uint8_t dst) {
  board_.MakeMove(Move(src, dst));
}

void Agent::OpponentFlip(uint8_t pos, ChessPiece result) {
  board_.MakeMove(Flip(pos, result));
}

ChessMove Agent::GenerateMove() {
  if (color_ == UNKNOWN) return Flip(0);
  if (auto mask = board_.GetCoveredSquares(); mask > 0)
    return Flip(__builtin_ctz(mask & -mask));

  auto mv = board_.ListMoves(color_);
  assert(!mv.empty());
  int p = rand() % mv.size();
  return mv[p];
}
