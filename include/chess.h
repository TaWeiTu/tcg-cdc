#ifndef CHESS_H_
#define CHESS_H_

#include <array>
#include <cstdint>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

#include "hash.h"

enum ChessColor : uint8_t { RED, BLACK, UNKNOWN, DRAW = UNKNOWN };

inline ChessColor &operator^=(ChessColor &c, int p) {
  return c = ChessColor(int(c) ^ p);
}
inline ChessColor operator^(const ChessColor &c, int p) {
  return ChessColor(int(c) ^ p);
}

constexpr size_t kNumChessPieces = 7;

enum ChessPiece : uint8_t {
  SOLDIER = 0,
  CANNON = 1,
  HORSE = 2,
  CHARIOT = 3,
  ELEPHANT = 4,
  ADVISOR = 5,
  GENERAL = 6,

  RED_SOLDIER = 0,
  RED_CANNON = 1,
  RED_HORSE = 2,
  RED_CHARIOT = 3,
  RED_ELEPHANT = 4,
  RED_ADVISOR = 5,
  RED_GENERAL = 6,

  BLACK_SOLDIER = 7,
  BLACK_CANNON = 8,
  BLACK_HORSE = 9,
  BLACK_CHARIOT = 10,
  BLACK_ELEPHANT = 11,
  BLACK_ADVISOR = 12,
  BLACK_GENERAL = 13,

  NO_PIECE = 14,
  COVERED_PIECE = 15,
};

constexpr ChessPiece GetChessPieceType(ChessPiece piece);
constexpr ChessColor GetChessPieceColor(ChessPiece piece);
bool CanCapture(ChessPiece capturer, ChessPiece capturee);

struct Move {
  uint8_t src;
  uint8_t dst;

  Move() = default;
  Move(uint8_t s, uint8_t d) : src(s), dst(d) {}
};

struct Flip {
  uint8_t pos;
  ChessPiece result;

  Flip() = default;
  Flip(uint8_t p) : pos(p), result(COVERED_PIECE) {}
  Flip(uint8_t p, ChessPiece r) : pos(p), result(r) {}
};

using ChessMove = std::variant<Move, Flip>;

std::ostream &operator<<(std::ostream &os, const ChessMove &mv);
std::ostream &operator<<(std::ostream &os, const Move &mv);
std::ostream &operator<<(std::ostream &os, const Flip &fp);

class BoardUpdater;  // forward declaration

constexpr std::array<ChessPiece, 128> BuildCharPieceMapping() {
  std::array<ChessPiece, 128> mapping{};
  mapping['-'] = NO_PIECE;
  mapping['X'] = COVERED_PIECE;
  mapping['P'] = RED_SOLDIER;
  mapping['C'] = RED_CANNON;
  mapping['N'] = RED_HORSE;
  mapping['R'] = RED_CHARIOT;
  mapping['M'] = RED_ELEPHANT;
  mapping['G'] = RED_ADVISOR;
  mapping['K'] = RED_GENERAL;
  mapping['p'] = BLACK_SOLDIER;
  mapping['c'] = BLACK_CANNON;
  mapping['n'] = BLACK_HORSE;
  mapping['r'] = BLACK_CHARIOT;
  mapping['m'] = BLACK_ELEPHANT;
  mapping['g'] = BLACK_ADVISOR;
  mapping['k'] = BLACK_GENERAL;
  return mapping;
}

constexpr std::array<char, 2 * kNumChessPieces + 2> BuildPieceCharMapping() {
  std::array<char, 2 * kNumChessPieces + 2> mapping{};
  mapping[NO_PIECE] = '-';
  mapping[COVERED_PIECE] = 'X';
  mapping[RED_SOLDIER] = 'P';
  mapping[RED_CANNON] = 'C';
  mapping[RED_HORSE] = 'N';
  mapping[RED_CHARIOT] = 'R';
  mapping[RED_ELEPHANT] = 'M';
  mapping[RED_ADVISOR] = 'G';
  mapping[RED_GENERAL] = 'K';
  mapping[BLACK_SOLDIER] = 'p';
  mapping[BLACK_CANNON] = 'c';
  mapping[BLACK_HORSE] = 'n';
  mapping[BLACK_CHARIOT] = 'r';
  mapping[BLACK_ELEPHANT] = 'm';
  mapping[BLACK_ADVISOR] = 'g';
  mapping[BLACK_GENERAL] = 'k';
  return mapping;
}
class ChessBoard {
  static constexpr size_t kNumSquares = 32;
  std::array<ChessPiece, kNumSquares> board_;
  std::array<uint8_t, kNumChessPieces * 2> covered_;
  std::array<uint8_t, 2> num_covered_pieces_;
  std::array<uint8_t, 2> num_pieces_left_;
  std::array<uint32_t, 2> uncovered_squares_;
  uint32_t covered_squares_;

  static constexpr uint32_t kNoFlipCaptureCountLimit = 60;
  uint32_t no_flip_capture_count_;

  ChessColor current_player_;

  using uint128_t = unsigned __int128;
  uint128_t hash_value_;
  ZobristHash<kNumSquares, kNumChessPieces * 2 + 2, 3> hasher_;

  friend class BoardUpdater;

  static constexpr std::array<ChessPiece, 128> kCharPieceMapping =
      BuildCharPieceMapping();
  static constexpr std::array<char, kNumChessPieces * 2 + 2> kPieceCharMapping =
      BuildPieceCharMapping();

  void UpdateBoard(uint8_t pos, ChessPiece piece);
  void UpdatePlayer(ChessColor new_player);

  uint8_t GetCannonTarget(ChessColor color, uint8_t pos, int d) const;
  uint32_t MarkUnderAttack() const;

 public:
  explicit ChessBoard();
  explicit ChessBoard(const std::array<std::string, 8> &buffer,
                      const std::array<uint8_t, kNumChessPieces * 2> &covered,
                      ChessColor current_player);

  std::vector<ChessMove> ListMoves(ChessColor player);
  void MakeMove(const ChessMove &mv, BoardUpdater *updater = nullptr);

  constexpr uint32_t GetCoveredSquares() const { return covered_squares_; }

  constexpr uint8_t GetNumCoveredPieces(ChessColor c) const {
    return num_covered_pieces_[c];
  }

  constexpr uint128_t GetHashValue() const { return hash_value_; }

  const std::array<uint8_t, kNumChessPieces * 2> &GetCoveredPieces() const {
    return covered_;
  }

  bool Terminate() const;
  ChessColor GetWinner() const;
  float Evaluate(ChessColor color) const;
  bool Playable(const ChessMove &mv) const;

  uint32_t GetNoFlipCaptureCount() const { return no_flip_capture_count_; }

  friend std::ostream &operator<<(std::ostream &os, const ChessBoard &board);
};

class BoardUpdater {
  ChessBoard &board_;
  std::vector<ChessMove> history_;
  std::vector<ChessPiece> captured_;
  std::vector<uint32_t> no_flip_capture_counts_;
  bool is_initial_;

  void UndoMove(ChessMove mv);

 public:
  explicit BoardUpdater(ChessBoard &b);
  void SaveMove(ChessMove v) { history_.push_back(std::move(v)); }
  void SaveCaptured(ChessPiece c) { captured_.push_back(c); }

  void SaveNoFlipCaptureCount(uint32_t c) {
    no_flip_capture_counts_.push_back(c);
  }

  void SetIsInitial(bool v) { is_initial_ = v; }
  void MakeMove(const ChessMove &mv);
  void Rewind();
};

#endif  // CHESS_H_
