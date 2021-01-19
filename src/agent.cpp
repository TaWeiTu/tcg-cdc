#include "agent.h"

#include <algorithm>
#include <cassert>
#include <iostream>

#include "chess.h"

Agent::Agent() : color_(UNKNOWN), round_(0) {}
Agent::Agent(const ChessBoard &board, ChessColor color)
    : board_(board), color_(color), round_(0) {}

void Agent::OpponentMove(uint8_t src, uint8_t dst) {
  board_.MakeMove(Move(src, dst));
}

void Agent::OpponentFlip(uint8_t pos, ChessPiece result) {
  board_.MakeMove(Flip(pos, result));
}

float Agent::ChanceNodeSearch(float alpha, float beta, int depth,
                              ChessColor color, uint8_t pos,
                              BoardUpdater &updater) {
  const auto &covered = board_.GetCoveredPieces();
  float sum = 0;
  int total = 0;
  for (uint8_t i = 0; i < covered.size(); ++i) {
    if (covered[i] > 0) {
      total += covered[i];
      updater.MakeMove(Flip(pos, ChessPiece(i)));
      sum +=
          covered[i] * -NegaScout(-beta, -alpha, depth - 1, color ^ 1, updater);
      updater.Rewind();
    }
  }
  if (total == 0) {
    std::cerr << "total = 0" << std::endl;
    exit(1);
  }
  return sum / total;
}

float Agent::NegaScout(float alpha, float beta, int depth, ChessColor color,
                       BoardUpdater &updater) {
  if (depth == 0) return board_.Evaluate(color);
  if (board_.Terminate()) {
    ChessColor winner = board_.GetWinner();
    // if (winner == DRAW) return 0;
    if (winner == DRAW) {
      std::cerr << "winner = DRAW"
                << "\n";
      std::cerr << "board = " << board_ << "\n";
      exit(1);
    }
    return winner == color ? 2000 : -2000;
  }
  if (alpha > beta) {
    std::cerr << "alpha > beta" << std::endl;
    exit(1);
  }
  auto moves = board_.ListMoves(color);
  float score = -kInf;  // fail soft
  for (auto &v : moves) {
    updater.MakeMove(v);
    float t = -NegaScout(-beta, -std::max(alpha, score), depth - 1, color ^ 1,
                         updater);
    if (t > score) {
      score = t;
      if (depth == kDepthLimit) best_move_ = v;
    }
    updater.Rewind();
    if (score >= beta) return score;
  }
  if (auto mask = board_.GetCoveredSquares(); mask > 0) {
    while (mask > 0) {
      int p = __builtin_ctz(mask & -mask);
      float t = ChanceNodeSearch(std::max(alpha, score), beta, depth, color, p,
                                 updater);
      if (t > score) {
        score = t;
        if (depth == kDepthLimit) best_move_ = Flip(p);
      }
      if (score >= beta) return score;
      mask ^= (1U << p);
    }
  }
  if (round_ > 5 && std::abs(score) < 1E-4) {
    std::cerr << "score = 0"
              << "\n";
    std::cerr << board_ << "\n";
  }
  return score;
}

// float Agent::NegaScout(float alpha, float beta, int depth, ChessColor color,
//                        BoardUpdater &updater) {
//   if (depth == 0) return board_.Evaluate(color);
//   if (board_.Terminate()) {
//     ChessColor winner = board_.GetWinner();
//     if (winner == DRAW) return 0;
//     return winner == color ? 2000 : -2000;
//   }
//   auto moves = board_.ListMoves(color);
//   float score = -kInf;  // fail soft
//   float upper_bound = beta;
//   for (auto &v : moves) {
//     updater.MakeMove(v);
//     float t = -NegaScout(-upper_bound, -std::max(alpha, score), depth - 1,
//                          color ^ 1, updater);
//     if (t > score) {  // failed-high
//       score = t;
//       if (depth == kDepthLimit) best_move_ = v;
//       if (upper_bound != beta && depth >= 3 && t < beta)
//         score = -NegaScout(-beta, -t, depth - 1, color ^ 1, updater);
//     }
//     updater.Rewind();
//     if (score >= beta) return score;
//     upper_bound = std::max(score, alpha) + 1;
//   }
//   if (auto mask = board_.GetCoveredSquares(); mask > 0) {
//     while (mask > 0) {
//       int p = __builtin_ctz(mask & -mask);
//       float t = ChanceNodeSearch(std::max(alpha, score), beta, depth, color,
//       p,
//                                  updater);
//       if (t > score) {
//         score = t;
//         if (depth == kDepthLimit) best_move_ = Flip(p);
//       }
//       if (score >= beta) return score;
//       mask ^= (1U << p);
//     }
//   }
//   return score;
// }

ChessMove Agent::GenerateMove() {
  round_++;
  if (color_ == UNKNOWN) return Flip(0);
  std::cerr << "before: " << board_ << "\n";
  BoardUpdater updater(board_);
  best_move_ = Flip(255, NO_PIECE);
  [[maybe_unused]] float score =
      NegaScout(-kInf, kInf, kDepthLimit, color_, updater);
  std::cerr << "NegaScout score = " << score << "\n";
  std::cerr << "board: "
            << "\n";
  std::cerr << board_ << "\n";
  return best_move_;
}
