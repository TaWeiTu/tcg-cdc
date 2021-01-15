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
  assert(capturee != NO_PIECE);
  capturer = GetChessPieceType(capturer);
  capturee = GetChessPieceType(capturee);
  if (capturee == COVERED_PIECE) return false;
  assert(GetChessPieceColor(capturer) != GetChessPieceColor(capturee));
  if (capturer == GENERAL && capturee == SOLDIER) return false;
  if (capturer == SOLDIER && capturee == GENERAL) return true;
  if (capturer == CANNON) return true;
  return capturer >= capturee;
}

ChessBoard::ChessBoard() {
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
  // TODO
}

void ChessBoard::MakeMove(const ChessMove &mv) {
  if (mv.IsFlip()) {
    // TODO
  }
  assert(board_[mv.src] != COVERED_PIECE && board_[mv.src] != NO_PIECE);
  assert(board_[mv.dst] != COVERED_PIECE);
  assert(board_[mv.dst] == NO_PIECE ||
         CanCapture(board_[mv.src], board_[mv.dst]));
  board_[mv.dst] = board_[mv.src];
  board_[mv.src] = NO_PIECE;
}
