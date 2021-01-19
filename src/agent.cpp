#include "agent.h"

#include <algorithm>
#include <cassert>
#include <iostream>

#include "chess.h"

Agent::Agent() : color_(UNKNOWN), table_(), depth_limit_(3), num_flip_(0) {}
Agent::Agent(const ChessBoard &board, ChessColor color)
    : board_(board), color_(color), table_() {}

void Agent::MakeMove(uint8_t src, uint8_t dst) {
  board_.MakeMove(Move(src, dst));
}

void Agent::MakeFlip(uint8_t pos, ChessPiece result) {
  if (++num_flip_ == 8) {
    depth_limit_ = std::min(kDepthLimit, depth_limit_ + 1);
    num_flip_ = 0;
  }
  board_.MakeMove(Flip(pos, result));
}

float Agent::ChanceNodeSearch(float alpha, float beta, int depth,
                              ChessColor color, uint8_t pos,
                              BoardUpdater &updater) {
  const auto covered = board_.GetCoveredPieces();
  float sum = 0;
  int total = 0;
  for (uint8_t i = 0; i < covered.size(); ++i) {
    if (covered[i] > 0) {
      total += covered[i];
      updater.MakeMove(Flip(pos, ChessPiece(i)));
      sum += covered[i] *
             -NegaScout(-beta, -alpha, depth - 1, color ^ 1, false, updater);
      updater.Rewind();
    }
  }
  return sum / total;
}

float Agent::NegaScout(float alpha, float beta, int depth, ChessColor color,
                       bool save_move, BoardUpdater &updater) {
  if (depth == 0) return board_.Evaluate(color);
  if (board_.Terminate()) {
    ChessColor winner = board_.GetWinner();
    if (winner == DRAW) return 0;
    return winner == color ? 2000 : -2000;
  }
  float score = -kInf;  // fail soft
  const uint128_t hv = board_.GetHashValue();
  auto &entry = table_.GetEntry(hv);
  if (entry.flag != NO_VALUE && entry.hash_value == hv &&
      board_.Playable(entry.best_move)) {
    if (entry.depth < depth) {
      if (entry.flag == EXACT_VALUE) {
        score = entry.score;
        if (save_move) best_move_ = entry.best_move;
      }
    } else {
      if (entry.flag == EXACT_VALUE) {
        if (save_move) best_move_ = entry.best_move;
        return entry.score;
      }
      if (entry.flag == LOWER_BOUND) {
        if (entry.score >= beta) {
          if (save_move) best_move_ = entry.best_move;
          return entry.score;
        }
        alpha = std::max(alpha, entry.score);
      } else {
        if (entry.score <= alpha) {
          if (save_move) best_move_ = entry.best_move;
          return entry.score;
        }
        beta = std::min(beta, entry.score);
      }
    }
  }
  ChessMove opt;
  auto moves = board_.ListMoves(color);
  float upper_bound = beta;
  for (auto &v : moves) {
    updater.MakeMove(v);
    float t = -NegaScout(-upper_bound, -std::max(alpha, score), depth - 1,
                         color ^ 1, false, updater);
    if (t > score) {  // failed-high
      score = t;
      opt = v;
      if (save_move) best_move_ = v;
      if (upper_bound != beta && depth >= 3 && t < beta)
        score = -NegaScout(-beta, -t, depth - 1, color ^ 1, false, updater);
    }
    updater.Rewind();
    if (score >= beta) {
      entry = Entry<ChessMove>(LOWER_BOUND, hv, score, depth, v);
      return score;
    }
    upper_bound = std::max(score, alpha) + 1;
  }
  if (auto mask = board_.GetCoveredSquares(); mask > 0) {
    while (mask > 0) {
      int p = __builtin_ctz(mask & -mask);
      float t = ChanceNodeSearch(std::max(alpha, score), beta, depth, color, p,
                                 updater);
      if (t > score) {
        score = t;
        opt = Flip(p);
        if (save_move) best_move_ = Flip(p);
      }
      if (score >= beta) {
        entry = Entry<ChessMove>(LOWER_BOUND, hv, score, depth, Flip(p));
        return score;
      }
      mask ^= (1U << p);
    }
  }
  Status flag = (score > alpha) ? EXACT_VALUE : UPPER_BOUND;
  entry = Entry<ChessMove>(flag, hv, score, depth, opt);
  return score;
}

ChessMove Agent::GenerateMove() {
  if (color_ == UNKNOWN) return Flip(0);
  std::cerr << "before: " << board_ << "\n";
  best_move_ = Flip(255, NO_PIECE);
  BoardUpdater updater(board_);
  std::cerr << "depth limit = " << depth_limit_ << "\n";
  float score = NegaScout(-kInf, kInf, depth_limit_, color_, true, updater);
  std::cerr << "NegaScout score = " << score << "\n";
  std::cerr << "board: "
            << "\n";
  std::cerr << board_ << "\n";
  return best_move_;
}

void Agent::TraceMoves() {
  ChessColor color = color_;
  BoardUpdater updater(board_);
  for (size_t i = 0; i < kDepthLimit; ++i) {
    std::cout << "NegaScout score = "
              << NegaScout(-kInf, kInf, kDepthLimit - i, color, true, updater)
              << "\n";
    std::cout << "best_move = " << best_move_ << "\n";
    board_.MakeMove(best_move_);
    std::cout << "board = " << board_ << "\n";
  }
}
