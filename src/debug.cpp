#include <iostream>
#include <string>
#include <vector>

#include "agent.h"
#include "chess.h"

int main() {
  std::array<std::string, 8> buffer;
  for (int i = 7; i >= 0; --i) std::cin >> buffer[i];
  std::array<uint8_t, 14> covered;
  for (int i = 0; i < 14; ++i) {
    int v;
    std::cin >> v;
    covered[i] = v;
  }
  std::string player;
  std::cin >> player;
  ChessBoard board(buffer, covered, (player == "RED" ? RED : BLACK));
  Agent agent(board, (player == "RED" ? RED : BLACK));
  agent.GenerateMove();
  agent.TraceMoves();
}
