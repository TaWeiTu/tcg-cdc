#ifndef AGENT_H_
#define AGENT_H_

#include "chess.h"

class Agent {
  uint32_t time_limit_;
  uint32_t time_left_;
  ChessBoard board_;

 public:
  void OpponentMove();
  void OpponentFlip();
  void GenerateMove();
  void SetTimeLimit(uint32_t tl) { time_limit_ = tl; }
  void SetTimeLeft(uint32_t tl) { time_left_ = tl; }
};

#endif  // AGENT_H_
