#include "chess.h"

#include <cassert>
#include <iostream>

constexpr ChessPiece GetChessPieceType(ChessPiece piece) {
  uint8_t p = piece;
  if (p >= kNumChessPieces) p -= kNumChessPieces;
  return ChessPiece(p);
}

constexpr ChessColor GetChessPieceColor(ChessPiece piece) {
  return ChessColor(piece >= kNumChessPieces);
}

bool CanCapture(ChessPiece capturer, ChessPiece capturee) {
  assert(capturer != COVERED_PIECE && capturer != NO_PIECE);
  if (capturee == COVERED_PIECE) return false;
  if (capturee == NO_PIECE) return true;
  if (GetChessPieceColor(capturer) == GetChessPieceColor(capturee))
    return false;

  capturer = GetChessPieceType(capturer);
  capturee = GetChessPieceType(capturee);
  if (capturer == GENERAL && capturee == SOLDIER) return false;
  if (capturer == SOLDIER && capturee == GENERAL) return true;
  if (capturer == CANNON) return true;
  return capturer >= capturee;
}

ChessBoard::ChessBoard()
    : covered_squares_(static_cast<uint32_t>(-1)),
      non_covered_squares_(0),
      current_player_(UNKNOWN),
      hash_value_(0) {
  std::fill(board_.begin(), board_.end(), COVERED_PIECE);
  covered_[RED_GENERAL] = covered_[BLACK_GENERAL] = 1;
  covered_[RED_ADVISOR] = covered_[BLACK_ADVISOR] = 2;
  covered_[RED_ELEPHANT] = covered_[BLACK_ELEPHANT] = 2;
  covered_[RED_CHARIOT] = covered_[BLACK_CHARIOT] = 2;
  covered_[RED_HORSE] = covered_[BLACK_HORSE] = 2;
  covered_[RED_CANNON] = covered_[BLACK_CANNON] = 2;
  covered_[RED_SOLDIER] = covered_[BLACK_SOLDIER] = 5;
}

std::vector<ChessMove> ChessBoard::ListMoves() {
  std::vector<ChessMove> moves;
  for (uint32_t mask = non_covered_squares_; mask > 0;) {
    int p = __builtin_ctz(mask & -mask);
    assert(board_[p] != NO_PIECE);
    if (GetChessPieceType(board_[p]) == CANNON) {
      for (int d : {-4, -1, 1, 4}) {
        bool found = false;
        int x = p;
        while (true) {
          if (x + d < 0 || x + d >= kNumCells) break;
          if ((x % 4) + d < 0 || (x % 4) + d >= 4) break;
          x += d;
          if (non_covered_squares_ >> x & 1) {
            found = true;
            break;
          }
        }
        if (!found) continue;
        while (true) {
          if (x + d < 0 || x + d >= kNumCells) break;
          if ((x % 4) + d < 0 || (x % 4) + d >= 4) break;
          x += d;
          if (non_covered_squares_ >> x & 1) {
            if (CanCapture(board_[p], board_[x])) moves.push_back(Move(p, x));
            break;
          }
        }
      }
    } else {
      for (int d : {-4, -1, 1, 4}) {
        if (p + d < 0 || p + d >= kNumCells) continue;
        if ((p % 4) + d < 0 || (p % 4) + d >= 4) continue;
        if (!CanCapture(board_[p], board_[p + d])) continue;
        moves.push_back(Move(p, p + d));
      }
    }
    mask ^= (1U << p);
  }
  return moves;
}

void ChessBoard::MakeMove(const ChessMove &mv) {
  assert(current_player_ != UNKNOWN || std::holds_alternative<Flip>(mv));
  if (std::holds_alternative<Flip>(mv)) {
    auto &v = std::get<Flip>(mv);
    assert(board_[v.pos] == COVERED_PIECE);
    assert(covered_[v.result] > 0);

    if (current_player_ == UNKNOWN)
      current_player_ = GetChessPieceColor(v.result);

    board_[v.pos] = v.result;
    covered_[v.result]--;
    non_covered_squares_ ^= (1U << v.pos);
    covered_squares_ ^= (1U << v.pos);
  } else {
    auto &v = std::get<Move>(mv);
    assert(board_[v.src] != COVERED_PIECE && board_[v.src] != NO_PIECE);
    assert(board_[v.dst] != COVERED_PIECE);
    assert(CanCapture(board_[v.src], board_[v.dst]));
    board_[v.dst] = board_[v.src];
    board_[v.src] = NO_PIECE;
    non_covered_squares_ ^= (1U << v.src);
  }

  current_player_ ^= 1;
}

namespace {

void PrintSquare(std::ostream &os, uint8_t square) {
  os << static_cast<char>(square % 4 + 'a')
     << static_cast<char>(square / 4 + '1');
}

}  // namespace

std::ostream &operator<<(std::ostream &os, const Move &mv) {
  PrintSquare(os, mv.src);
  os << " ";
  PrintSquare(os, mv.dst);
  return os;
}

std::ostream &operator<<(std::ostream &os, const Flip &fp) {
  PrintSquare(os, fp.pos);
  os << " ";
  PrintSquare(os, fp.pos);
  return os;
}

std::ostream &operator<<(std::ostream &os, const ChessMove &mv) {
  if (std::holds_alternative<Move>(mv)) return os << std::get<Move>(mv);
  return os << std::get<Flip>(mv);
}
