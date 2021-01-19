#include "chess.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>

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
      no_flip_capture_count_(0),
      current_player_(UNKNOWN),
      hash_value_(0),
      hasher_(0x7122) {
  std::fill(board_.begin(), board_.end(), COVERED_PIECE);
  covered_[RED_GENERAL] = covered_[BLACK_GENERAL] = 1;
  covered_[RED_ADVISOR] = covered_[BLACK_ADVISOR] = 2;
  covered_[RED_ELEPHANT] = covered_[BLACK_ELEPHANT] = 2;
  covered_[RED_CHARIOT] = covered_[BLACK_CHARIOT] = 2;
  covered_[RED_HORSE] = covered_[BLACK_HORSE] = 2;
  covered_[RED_CANNON] = covered_[BLACK_CANNON] = 2;
  covered_[RED_SOLDIER] = covered_[BLACK_SOLDIER] = 5;

  for (size_t i = 0; i < kNumSquares; ++i)
    hash_value_ ^= hasher_.hash_pieces[i][COVERED_PIECE];
  hash_value_ ^= hasher_.hash_player[UNKNOWN];
}

ChessBoard::ChessBoard(const std::array<std::string, 8> &buffer,
                       const std::array<uint8_t, kNumChessPieces * 2> &covered,
                       ChessColor current_player)
    : num_covered_pieces_{},
      num_pieces_left_{},
      uncovered_squares_{},
      covered_(covered),
      current_player_(current_player),
      no_flip_capture_count_(0) {
  for (size_t i = 0; i < 8; ++i) {
    assert(buffer[i].size() == 4);
    for (size_t j = 0; j < 4; ++j)
      board_[i * 4 + j] = kCharPieceMapping[buffer[i][j]];
  }
  for (size_t i = 0; i < covered_.size(); ++i) {
    ChessColor color = GetChessPieceColor(ChessPiece(i));
    num_pieces_left_[color] += covered_[i];
    num_covered_pieces_[color] += covered_[i];
  }
  for (size_t i = 0; i < kNumSquares; ++i) {
    if (board_[i] == NO_PIECE || board_[i] == COVERED_PIECE) continue;
    num_pieces_left_[GetChessPieceColor(board_[i])]++;
    uncovered_squares_[GetChessPieceColor(board_[i])] |= (1U << i);
  }
}

std::vector<ChessMove> ChessBoard::ListMoves(ChessColor player) {
  const uint32_t kNonEmpty =
      covered_squares_ | uncovered_squares_[RED] | uncovered_squares_[BLACK];
  std::vector<ChessMove> moves;
  for (uint32_t mask = uncovered_squares_[player]; mask > 0;) {
    int p = __builtin_ctz(mask & -mask);
    assert(board_[p] != NO_PIECE);
    bool is_cannon = (GetChessPieceType(board_[p]) == CANNON);
    if (is_cannon) {
      for (int d : {-4, -1, 1, 4}) {
        bool found = false;
        int x = p;
        while (true) {
          if (x + d < 0 || x + d >= kNumSquares) break;
          if (std::abs(d) == 1 && ((x % 4) + d < 0 || (x % 4) + d >= 4)) break;
          x += d;
          if (kNonEmpty >> x & 1) {
            found = true;
            break;
          }
        }
        if (!found) continue;
        while (true) {
          if (x + d < 0 || x + d >= kNumSquares) break;
          if (std::abs(d) == 1 && ((x % 4) + d < 0 || (x % 4) + d >= 4)) break;
          x += d;
          if (kNonEmpty >> x & 1) {
            if (uncovered_squares_[player ^ 1] >> x & 1)
              moves.push_back(Move(p, x));
            break;
          }
        }
      }
    }
    for (int d : {-4, -1, 1, 4}) {
      if (p + d < 0 || p + d >= kNumSquares) continue;
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
  no_flip_capture_count_++;
  if (updater) updater->SaveMove(mv);
  bool flip_or_capture = false;
  if (std::holds_alternative<Flip>(mv)) {
    const auto &v = std::get<Flip>(mv);
    assert(board_[v.pos] == COVERED_PIECE);
    assert(covered_[v.result] > 0);
    flip_or_capture = true;

    if (current_player_ == UNKNOWN) {
      current_player_ = GetChessPieceColor(v.result);
      if (updater) updater->SetIsInitial(true);
    }

    board_[v.pos] = v.result;
    covered_[v.result]--;
    assert(!(uncovered_squares_[current_player_] >> v.pos & 1));
    uncovered_squares_[GetChessPieceColor(v.result)] ^= (1U << v.pos);
    num_covered_pieces_[GetChessPieceColor(v.result)]--;
    covered_squares_ ^= (1U << v.pos);
  } else {
    const auto &v = std::get<Move>(mv);
    assert(board_[v.src] != COVERED_PIECE && board_[v.src] != NO_PIECE);
    assert(board_[v.dst] != COVERED_PIECE);
    assert(CanCapture(board_[v.src], board_[v.dst]));

    if (updater) updater->AddCaptured(board_[v.dst]);
    if (board_[v.dst] != NO_PIECE) {  // capture
      flip_or_capture = true;
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
  if (flip_or_capture) {
    if (updater) updater->AddNoFlipCaptureCount(no_flip_capture_count_);
    no_flip_capture_count_ = 0;
  }
}

bool ChessBoard::Terminate() const {
  // TODO: Add other rules to it.
  return num_pieces_left_[RED] == 0 || num_pieces_left_[BLACK] == 0 ||
         no_flip_capture_count_ == kNoFlipCaptureCountLimit;
}

ChessColor ChessBoard::GetWinner() const {
  if (no_flip_capture_count_ == kNoFlipCaptureCountLimit) return DRAW;
  return num_pieces_left_[RED] == 0 ? BLACK : RED;
}

int ChessBoard::Evaluate(ChessColor color) const {
  static constexpr int kValue[7] = {1, 180, 6, 18, 90, 270, 810};
  int score = 0;
  for (size_t i = 0; i < kNumSquares; ++i) {
    if (board_[i] == NO_PIECE || board_[i] == COVERED_PIECE) continue;
    int v = kValue[GetChessPieceType(board_[i])];
    (GetChessPieceColor(board_[i]) == color) ? score += v : score -= v;
  }
  return score;
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

std::ostream &operator<<(std::ostream &os, const ChessBoard &board) {
  os << "num_pieces_left[RED] = " << int(board.num_pieces_left_[RED])
     << " num_pieces_left_[BLACK] = " << int(board.num_pieces_left_[BLACK])
     << "\n";
  os << "no_flip_capture_count = " << board.no_flip_capture_count_ << "\n";
  os << "covered: ";
  for (size_t i = 0; i < board.covered_.size(); ++i)
    os << int(board.covered_[i]) << " ";
  os << "\n";
  os << "evaluation(RED) = " << board.Evaluate(RED)
     << " evaluation(BLACK) = " << board.Evaluate(BLACK) << "\n";
  for (int i = 7; i >= 0; --i) {
    for (int j = 0; j < 4; ++j)
      os << ChessBoard::kPieceCharMapping[board.board_[i * 4 + j]];
    os << "\n";
  }
  return os;
}

ChessBoard::ZobristHash::ZobristHash(uint64_t seed) {
  std::mt19937 rng(seed);
  std::uniform_int_distribution<uint64_t> dist;

  auto Rand128 = [&rng, &dist]() -> uint128_t {
    return (uint128_t(dist(rng)) << 64) | uint128_t(dist(rng));
  };

  for (size_t i = 0; i < hash_pieces.size(); ++i) {
    std::generate(hash_pieces[i].begin(), hash_pieces[i].end(), Rand128);
  }
  for (size_t i = 0; i < 3; ++i) hash_player[i] = Rand128();
}

void BoardUpdater::MakeMove(const ChessMove &mv) { board_.MakeMove(mv, this); }

void BoardUpdater::Rewind() {
  assert(!history_.empty());
  UndoMove(history_.back());
  history_.pop_back();
  if (is_initial_ && history_.empty()) board_.current_player_ = UNKNOWN;
}

void BoardUpdater::UndoMove(ChessMove mv) {
  auto player = board_.current_player_ ^ 1;
  bool flip_or_capture = false;
  if (std::holds_alternative<Flip>(mv)) {
    auto &v = std::get<Flip>(mv);
    board_.board_[v.pos] = COVERED_PIECE;
    board_.covered_[v.result]++;
    board_.uncovered_squares_[GetChessPieceColor(v.result)] ^= (1U << v.pos);
    board_.num_covered_pieces_[GetChessPieceColor(v.result)]++;
    board_.covered_squares_ ^= (1U << v.pos);
    flip_or_capture = true;
  } else {
    auto &v = std::get<Move>(mv);
    assert(!captured_.empty());
    ChessPiece capturee = captured_.back();
    captured_.pop_back();
    if (capturee != NO_PIECE) {
      flip_or_capture = true;
      board_.num_pieces_left_[player ^ 1]++;
      board_.uncovered_squares_[player ^ 1] ^= (1U << v.dst);
    }

    board_.uncovered_squares_[player] ^= (1U << v.src);
    board_.uncovered_squares_[player] ^= (1U << v.dst);
    board_.board_[v.src] = board_.board_[v.dst];
    board_.board_[v.dst] = capturee;
  }
  board_.current_player_ ^= 1;
  if (flip_or_capture) {
    assert(!no_flip_capture_counts_.empty());
    board_.no_flip_capture_count_ = no_flip_capture_counts_.back();
    no_flip_capture_counts_.pop_back();
  }
  assert(board_.no_flip_capture_count_ > 0);
  board_.no_flip_capture_count_--;
}
