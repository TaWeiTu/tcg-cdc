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

int Agent::ChanceNodeSearch(int alpha, int beta, int depth, ChessColor color,
                            uint8_t pos, BoardUpdater &updater) {
  const auto &covered = board_.GetCoveredPieces();
  int sum = 0, total = 0;
  for (uint8_t i = 0; i < covered.size(); ++i) {
    if (covered[i] > 0) {
      total += covered[i];
      board_.MakeMove(Flip(pos, ChessPiece(i)), &updater);
      sum +=
          covered[i] * -NegaScout(-beta, -alpha, depth - 1, color ^ 1, updater);
      updater.Rewind();
    }
  }
  assert(total > 0);
  return sum / total;
}

int Agent::NegaScout(int alpha, int beta, int depth, ChessColor color,
                     BoardUpdater &updater) {
  if (depth == 0) return board_.Evaluate(color);
  auto moves = board_.ListMoves(color);
  int score = -kInf;  // fail soft
  int upper_bound = beta;
  for (auto &v : moves) {
    board_.MakeMove(v, &updater);
    int t = -NegaScout(-upper_bound, -std::max(alpha, score), depth - 1,
                       color ^ 1, updater);
    if (t > score) {  // failed-high
      score = t;
      if (depth == kDepthLimit) best_move_ = v;
      if (upper_bound != beta && depth >= 3 && t < beta)
        score = -NegaScout(-beta, -t, depth - 1, color ^ 1, updater);
    }
    updater.Rewind();
    if (score >= beta) return score;
    upper_bound = std::max(score, alpha) + 1;
  }
  if (auto mask = board_.GetCoveredSquares(); mask > 0) {
    while (mask > 0) {
      int p = __builtin_ctz(mask & -mask);
      int t = ChanceNodeSearch(std::max(alpha, score), beta, depth, color, p,
                               updater);
      if (t > score) {
        score = t;
        best_move_ = Flip(p);
      }
      if (score >= beta) return score;
      mask ^= (1U << p);
    }
  }
  return score;
}

ChessMove Agent::GenerateMove() {
  if (color_ == UNKNOWN) return Flip(0);
  BoardUpdater updater(board_);
  [[maybe_unused]] int score =
      NegaScout(-kInf, kInf, kDepthLimit, color_, updater);
  return best_move_;
}
