#ifndef AGENT_H_
#define AGENT_H_

#include "chess.h"

class Agent {
  uint32_t time_limit_;
  uint32_t time_left_;
  ChessBoard board_;
  ChessColor color_;

  int NegaScount();

 public:
  explicit Agent();

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
