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

int Agent::Evaluate(const ChessBoard &board) const { return 0; }

int Agent::NegaScout(int alpha, int beta, int depth, ChessColor color) {
  if (depth == 0) return Evaluate(board_);
  auto moves = board_.ListMoves(color);
  int score = -kInf;  // fail soft
  for (auto &v : moves) {

  }
}

ChessMove Agent::GenerateMove() {
  if (color_ == UNKNOWN) return Flip(0);
  int score = NegaScout(-kInf, kInf, kDepthLimit, color_);
  if (auto mask = board_.GetCoveredSquares(); mask > 0)
    return Flip(__builtin_ctz(mask & -mask));

  auto mv = board_.ListMoves(color_);
  assert(!mv.empty());
  int p = rand() % mv.size();
  return mv[p];
}
