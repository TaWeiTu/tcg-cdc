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
    : num_covered_pieces_{16, 16},
      num_pieces_left_{16, 16},
      uncovered_squares_{0, 0},
      covered_squares_(static_cast<uint32_t>(-1)),
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

std::vector<ChessMove> ChessBoard::ListMoves(ChessColor player) {
  std::vector<ChessMove> moves;
  uint32_t total = uncovered_squares_[RED] | uncovered_squares_[BLACK];
  for (uint32_t mask = uncovered_squares_[player]; mask > 0;) {
    int p = __builtin_ctz(mask & -mask);
    assert(board_[p] != NO_PIECE);
    bool is_cannon = (GetChessPieceType(board_[p]) == CANNON);
    if (is_cannon) {
      for (int d : {-4, -1, 1, 4}) {
        bool found = false;
        int x = p;
        while (true) {
          if (x + d < 0 || x + d >= kNumCells) break;
          if (std::abs(d) == 1 && ((x % 4) + d < 0 || (x % 4) + d >= 4)) break;
          x += d;
          if (total >> x & 1) {
            found = true;
            break;
          }
        }
        if (!found) continue;
        while (true) {
          if (x + d < 0 || x + d >= kNumCells) break;
          if (std::abs(d) == 1 && ((x % 4) + d < 0 || (x % 4) + d >= 4)) break;
          x += d;
          if (total >> x & 1) {
            if (CanCapture(board_[p], board_[x])) moves.push_back(Move(p, x));
            break;
          }
        }
      }
    }
    for (int d : {-4, -1, 1, 4}) {
      if (p + d < 0 || p + d >= kNumCells) continue;
      if (std::abs(d) == 1 && ((p % 4) + d < 0 || (p % 4) + d >= 4)) continue;
      if (is_cannon && board_[p + d] != NO_PIECE) continue;
      if (!CanCapture(board_[p], board_[p + d])) continue;
      moves.push_back(Move(p, p + d));
    }
    mask ^= (1U << p);
  }
  return moves;
}

void ChessBoard::MakeMove(const ChessMove &mv, BoardUpdater *updater) {
  assert(current_player_ != UNKNOWN || std::holds_alternative<Flip>(mv));
  if (updater) updater->SaveMove(mv);
  if (std::holds_alternative<Flip>(mv)) {
    auto &v = std::get<Flip>(mv);
    assert(board_[v.pos] == COVERED_PIECE);
    assert(covered_[v.result] > 0);

    if (current_player_ == UNKNOWN)
      current_player_ = GetChessPieceColor(v.result);

    board_[v.pos] = v.result;
    covered_[v.result]--;
    assert(!(uncovered_squares_[current_player_] >> v.pos & 1));
    uncovered_squares_[GetChessPieceColor(v.result)] ^= (1U << v.pos);
    num_covered_pieces_[GetChessPieceColor(v.result)]--;
    covered_squares_ ^= (1U << v.pos);
  } else {
    auto &v = std::get<Move>(mv);
    assert(board_[v.src] != COVERED_PIECE && board_[v.src] != NO_PIECE);
    assert(board_[v.dst] != COVERED_PIECE);
    assert(CanCapture(board_[v.src], board_[v.dst]));

    if (updater) updater->AddCaptured(board_[v.dst]);
    if (board_[v.dst] != NO_PIECE) {  // capture
      assert(GetChessPieceColor(board_[v.dst]) == (current_player_ ^ 1));
      assert(uncovered_squares_[current_player_ ^ 1] >> v.dst & 1);
      num_pieces_left_[current_player_ ^ 1]--;
      uncovered_squares_[current_player_ ^ 1] ^= (1U << v.dst);
    }

    assert(uncovered_squares_[current_player_] >> v.src & 1);
    assert(!(uncovered_squares_[current_player_] >> v.dst & 1));
    uncovered_squares_[current_player_] ^= (1U << v.src);
    uncovered_squares_[current_player_] ^= (1U << v.dst);
    board_[v.dst] = board_[v.src];
    board_[v.src] = NO_PIECE;
  }

  current_player_ ^= 1;
}

constexpr bool ChessBoard::Terminate() const {
  // TODO: Add other rules to it.
  return num_pieces_left_[RED] == 0 || num_pieces_left_[BLACK] == 0;
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

void BoardUpdater::Rewind() {
  assert(!checkpoints_.empty());
  size_t s = checkpoints_.back();
  checkpoints_.pop_back();
  while (history_.size() > s) {
    ChessMove mv = history_.back();
    history_.pop_back();
    UndoMove(mv);
  }
}

void BoardUpdater::UndoMove(ChessMove mv) {
  auto player = board_.current_player_ ^ 1;
  if (std::holds_alternative<Flip>(mv)) {
    auto &v = std::get<Flip>(mv);
    board_.board_[v.pos] = COVERED_PIECE;
    board_.covered_[v.result]++;
    board_.uncovered_squares_[GetChessPieceColor(v.result)] ^= (1U << v.pos);
    board_.num_covered_pieces_[GetChessPieceColor(v.result)]++;
    board_.covered_squares_ ^= (1U << v.pos);
  } else {
    auto &v = std::get<Move>(mv);
    assert(!captured_.empty());
    ChessPiece capturee = captured_.back();
    captured_.pop_back();
    if (capturee != NO_PIECE) {
      board_.num_pieces_left_[player ^ 1]++;
      board_.uncovered_squares_[player ^ 1] ^= (1U << v.dst);
    }

    board_.uncovered_squares_[player] ^= (1U << v.src);
    board_.uncovered_squares_[player] ^= (1U << v.dst);
    board_.board_[v.src] = board_.board_[v.dst];
    board_.board_[v.dst] = capturee;
  }
}
