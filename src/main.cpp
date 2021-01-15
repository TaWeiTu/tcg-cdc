#include <cassert>
#include <iostream>
#include <string>
#include <string_view>

#include "agent.h"
#include "chess.h"

namespace {

std::string_view Forward(std::string_view &s, size_t k) {
  assert(s.size() >= k);
  std::string_view res = s.substr(k);
  if (!res.empty()) {
    assert(res[0] == ' ');
    res = res.substr(1);
  }
  return res;
}

int ParseInt(std::string_view &cmd) {
  int id = 0;
  size_t ptr;
  for (ptr = 0; ptr < cmd.size() && std::isdigit(cmd[ptr]); ptr++) {
    id = id * 10 + (cmd[ptr] - '0');
  }
  cmd = Forward(cmd, ptr);
  return id;
}

std::string_view ParseString(std::string_view &cmd) {
  size_t ptr = 0;
  while (ptr < cmd.size() && std::isalpha(cmd[ptr])) ptr++;
  std::string_view res = cmd.substr(0, ptr);
  cmd = Forward(cmd, ptr);
  return res;
}

uint8_t ParseSquare(std::string_view &cmd) {
  assert(std::isalpha(cmd[0]));
  assert(std::isdigit(cmd[1]));
  uint8_t col = cmd[0] - 'a';
  uint8_t row = cmd[1] - '1';
  cmd = Forward(cmd, 2);
  return row * 4 + col;
}

ChessPiece ParsePiece(std::string_view &cmd) {
  assert(std::isalpha(cmd[0]));
  char p = cmd[0];
  cmd = Forward(cmd, 1);
  switch (p) {
    case 'K':
      return RED_GENERAL;
    case 'G':
      return RED_ADVISOR;
    case 'M':
      return RED_ELEPHANT;
    case 'R':
      return RED_CHARIOT;
    case 'N':
      return RED_HORSE;
    case 'C':
      return RED_CANNON;
    case 'P':
      return RED_SOLDIER;
    case 'k':
      return BLACK_GENERAL;
    case 'g':
      return BLACK_ADVISOR;
    case 'm':
      return BLACK_ELEPHANT;
    case 'r':
      return BLACK_CHARIOT;
    case 'n':
      return BLACK_HORSE;
    case 'c':
      return BLACK_CANNON;
    case 'p':
      return BLACK_SOLDIER;
  }
  __builtin_unreachable();
}

#ifndef NDEBUG

constexpr char kCmdString[18][20] = {
    "protocol_version",  "name",          "version",
    "known_command",     "list_commands", "quit",
    "boardsize",         "reset_board",   "num_repetition",
    "num_moves_to_draw", "move",          "flip",
    "genmove",           "game_over",     "ready",
    "time_settings",     "time_left",     "showboard"};

#endif

}  // namespace

int main() {
  std::string buffer;
  Agent agent;
  while (true) {
    std::getline(std::cin, buffer);
    std::string_view cmd = buffer;
    int id = ParseInt(cmd);
    auto s = ParseString(cmd);
    assert(s == kCmdString[id]);
    switch (id) {
      case 1:
        std::cout << "=1 AI" << std::endl;
        break;
      case 2:
        std::cout << "=2 1.0.0" << std::endl;
        break;
      case 5:
        std::cout << "=5" << std::endl;
        exit(0);
      case 7:
        std::cout << "=7" << std::endl;
        agent.Reset();
        break;
      case 10: {
        uint8_t src = ParseSquare(cmd);
        uint8_t dst = ParseSquare(cmd);
        agent.OpponentMove(src, dst);
        std::cout << "=10" << std::endl;
        break;
      }
      case 11: {
        uint8_t pos = ParseSquare(cmd);
        ChessPiece result = ParsePiece(cmd);
        agent.OpponentFlip(pos, result);
        std::cout << "=11" << std::endl;
      }
      case 12: {
        auto s = ParseString(cmd);
        if (s == "unknown") {
          assert(agent.GetColor() == UNKNOWN);
        } else {
          ChessColor expected = (s == "red" ? RED : BLACK);
          assert(agent.GetColor() == UNKNOWN || agent.GetColor() == expected);
          agent.SetColor(expected);
        }
        auto mv = agent.GenerateMove();
        std::cout << "=12 " << mv << std::endl;
      }
      case 14:
        std::cout << "=14" << std::endl;
        break;
      case 15: {
        int time_limit = ParseInt(cmd);
        agent.SetTimeLimit(time_limit);
        std::cout << "=15" << std::endl;
        break;
      }
      case 16: {
        int time_left = ParseInt(cmd);
        agent.SetTimeLeft(time_left);
        std::cout << "=16" << std::endl;
        break;
      }
      default:
        std::cerr << "Unsupported MGTP command: " << id << std::endl;
        exit(1);
    }
  }
  return 0;
}
