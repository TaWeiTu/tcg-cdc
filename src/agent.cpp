#include "agent.h"

#include <algorithm>
#include <cassert>
#include <chrono>
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
    if (depth_limit_ < kDepthSoftLimit) depth_limit_++;
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
    return (winner == color ? 2000 : -2000) * (depth + 1);
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
  BoardUpdater updater(board_);
  std::cerr << "depth limit = " << depth_limit_ << "\n";
  int last_search_elapsed = 0;
  float score = NegaScout(-kInf, kInf, 3, color_, true, updater);
  for (int depth_lim = 4; depth_lim <= depth_limit_; ++depth_lim) {
    float alpha = score - kRange, beta = score + kRange;
    auto time_start = std::chrono::system_clock::now();
    best_move_ = Flip(255, NO_PIECE);
    score = NegaScout(alpha, beta, depth_lim, color_, true, updater);
    if (score <= alpha) {
      best_move_ = Flip(255, NO_PIECE);
      score = NegaScout(-kInf, score, depth_lim, color_, true, updater);
    } else if (score >= beta) {
      best_move_ = Flip(255, NO_PIECE);
      score = NegaScout(score, kInf, depth_lim, color_, true, updater);
    }
    auto time_end = std::chrono::system_clock::now();
    last_search_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                              time_end - time_start)
                              .count();
  }
  if (depth_limit_ == kDepthSoftLimit) {
    int depth_lim = depth_limit_;
    while (depth_lim < kDepthHardLimit &&
           last_search_elapsed <= kTimeThreshold) {
      depth_lim++;
      std::cerr << "keep searching depth = " << depth_lim << "\n";
      float alpha = score - kRange, beta = score + kRange;
      auto time_start = std::chrono::system_clock::now();
      best_move_ = Flip(255, NO_PIECE);
      score = NegaScout(alpha, beta, depth_lim, color_, true, updater);
      if (score <= alpha) {
        best_move_ = Flip(255, NO_PIECE);
        score = NegaScout(-kInf, score, depth_lim, color_, true, updater);
      } else if (score >= beta) {
        best_move_ = Flip(255, NO_PIECE);
        score = NegaScout(score, kInf, depth_lim, color_, true, updater);
      }
      auto time_end = std::chrono::system_clock::now();
      last_search_elapsed =
          std::chrono::duration_cast<std::chrono::milliseconds>(time_end -
                                                                time_start)
              .count();
    }
  }
  std::cerr << "NegaScout score = " << score << "\n";
  std::cerr << "board: "
            << "\n";
  std::cerr << board_ << "\n";
  return best_move_;
}

void Agent::TraceMoves() {
  ChessColor color = color_;
  BoardUpdater updater(board_);
  for (size_t i = 0; i < depth_limit_; ++i) {
    std::cout << "NegaScout score = "
              << NegaScout(-kInf, kInf, depth_limit_ - i, color, true, updater)
              << "\n";
    std::cout << "best_move = " << best_move_ << "\n";
    board_.MakeMove(best_move_);
    std::cout << "board = " << board_ << "\n";
  }
}
