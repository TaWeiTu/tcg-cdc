#ifndef AGENT_H_
#define AGENT_H_

#include <utility>

#include "chess.h"
#include "hash.h"

class Agent {
  uint32_t time_limit_;
  uint32_t time_left_;
  ChessBoard board_;
  ChessColor color_;
  ChessMove best_move_;
  TranspositionTable<20, ChessMove> table_;
  int depth_limit_, num_flip_;

  static constexpr float kInf = 1E9;
  static constexpr int kDepthLimit = 15;
  static constexpr float kRange = 5;
  static constexpr int kTimeThreshold = 100;

  float NegaScout(float alpha, float beta, int depth, ChessColor color,
                  bool save_move, BoardUpdater &updater);

  float ChanceNodeSearch(float alpha, float beta, int depth, ChessColor color,
                         uint8_t pos, BoardUpdater &updater);

  std::pair<float, int> SearchSingleDepth(float alpha, float beta, int depth,
                                          BoardUpdater &updater);

 public:
  explicit Agent();
  explicit Agent(const ChessBoard &board, ChessColor color);

  void MakeMove(uint8_t src, uint8_t dst);
  void MakeFlip(uint8_t pos, ChessPiece result);
  ChessMove GenerateMove();
  void TraceMoves();

  void SetTimeLimit(uint32_t tl) { time_limit_ = tl; }
  void SetTimeLeft(uint32_t tl) { time_left_ = tl; }
  void Reset() { board_ = ChessBoard(); }
  void SetColor(ChessColor c) { color_ = c; }

  constexpr ChessColor GetColor() const { return color_; }
};

#endif  // AGENT_H_
