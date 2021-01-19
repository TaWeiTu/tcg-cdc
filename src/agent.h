#ifndef AGENT_H_
#define AGENT_H_

#include "chess.h"

class Agent {
  uint32_t time_limit_;
  uint32_t time_left_;
  ChessBoard board_;
  ChessColor color_;
  ChessMove best_move_;
  int round_;

  static constexpr float kInf = 1E9;
  static constexpr int kDepthLimit = 4;

  float NegaScout(float alpha, float beta, int depth, ChessColor color,
                  BoardUpdater &updater);

  float ChanceNodeSearch(float alpha, float beta, int depth, ChessColor color,
                         uint8_t pos, BoardUpdater &updater);

 public:
  explicit Agent();
  explicit Agent(const ChessBoard &board, ChessColor color);

  void OpponentMove(uint8_t src, uint8_t dst);
  void OpponentFlip(uint8_t pos, ChessPiece result);
  ChessMove GenerateMove();

  void SetTimeLimit(uint32_t tl) { time_limit_ = tl; }
  void SetTimeLeft(uint32_t tl) { time_left_ = tl; }
  void Reset() { board_ = ChessBoard(); }
  void SetColor(ChessColor c) { color_ = c; }

  constexpr ChessColor GetColor() const { return color_; }
};

#endif  // AGENT_H_
