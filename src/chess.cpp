#include "chess.h"

#include <cassert>

constexpr ChessPiece GetChessPieceType(ChessPiece piece) {
  uint8_t p = piece;
  if (p >= kNumChessPieces) p -= kNumChessPieces;
  return ChessPiece(p);
}

constexpr bool GetChessPieceColor(ChessPiece piece) {
  return piece >= kNumChessPieces;
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
    : covered_squares_(static_cast<uint32_t>(-1)), non_covered_squares_(0) {
  std::fill(board_.begin(), board_.end(), COVERED_PIECE);
  covered_[RED_GENERAL] = covered_[BLACK_GENERAL] = 1;
  covered_[RED_ADVISOR] = covered_[BLACK_ADVISOR] = 2;
  covered_[RED_ELEPHANT] = covered_[BLACK_ELEPHANT] = 2;
  covered_[RED_CHARIOT] = covered_[BLACK_CHARIOT] = 2;
  covered_[RED_HORSE] = covered_[BLACK_HORSE] = 2;
  covered_[RED_CANNON] = covered_[BLACK_CANNON] = 2;
  covered_[RED_SOLDIER] = covered_[BLACK_SOLDIER] = 5;
}

std::vector<ChessMove> ChessBoard::GenerateMoves() {
  std::vector<ChessMove> moves;
  for (uint32_t mask = non_covered_squares_; mask > 0;) {
    int p = __builtin_ctz(mask & -mask);
    assert(board_[p] != NO_PIECE);
    if (GetChessPieceType(board_[p]) == CANNON) {
      // TODO: Generate moves for cannon.
    } else {
      for (int d : {-4, -1, 1, 4}) {
        if (p + d < 0 || p + d >= kNumCells) continue;
        if ((p % 4) + d < 0 || (p % 4) + d >= 4) continue;
        if (!CanCapture(board_[p], board_[p + d])) continue;
        moves.push_back(Move(p, p - 4));
      }
    }
    mask ^= (1U << p);
  }
  return moves;
}

void ChessBoard::MakeMove(const ChessMove &mv) {
  if (std::holds_alternative<Flip>(mv)) {
    auto &v = std::get<Flip>(mv);
    assert(board_[v.pos] == COVERED_PIECE);
    assert(covered_[v.result] > 0);
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
}
