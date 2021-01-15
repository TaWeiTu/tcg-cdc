#ifndef CHESS_H_
#define CHESS_H_

#include <array>
#include <cstdint>
#include <ostream>
#include <variant>
#include <vector>

enum ChessColor : uint8_t { RED, BLACK, UNKNOWN };

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

class ChessBoard {
  static constexpr size_t kNumSquares = 32;
  std::array<ChessPiece, kNumSquares> board_;
  std::array<uint8_t, kNumChessPieces * 2> covered_;
  std::array<uint8_t, 2> num_covered_pieces_;
  std::array<uint8_t, 2> num_pieces_left_;
  std::array<uint32_t, 2> uncovered_squares_;
  uint32_t covered_squares_;
  ChessColor current_player_;

  using uint128_t = unsigned __int128;
  uint128_t hash_value_;

  struct ZobristHash {
    std::array<std::array<uint128_t, kNumChessPieces * 2 + 2>, kNumSquares>
        hash_pieces;
    std::array<uint128_t, 3> hash_player;

    explicit ZobristHash(uint64_t seed = 0);
  };

  ZobristHash hasher_;

  friend class BoardUpdater;

 public:
  explicit ChessBoard();

  // Remove copy constructors to avoid accidently having different hashers.
  ChessBoard(const ChessBoard &board) = delete;

  std::vector<ChessMove> ListMoves(ChessColor player);
  void MakeMove(const ChessMove &mv, BoardUpdater *updater = nullptr);

  constexpr uint32_t GetCoveredSquares() const { return covered_squares_; }

  constexpr uint8_t GetNumCoveredPieces(ChessColor c) const {
    return num_covered_pieces_[c];
  }

  const std::array<uint8_t, kNumChessPieces * 2> &GetCoveredPieces() const {
    return covered_;
  }

  constexpr bool Terminate() const;
  int Evaluate(ChessColor color) const;

  friend std::ostream &operator<<(std::ostream &os, const ChessBoard &board);
};

class BoardUpdater {
  ChessBoard &board_;
  std::vector<ChessMove> history_;
  std::vector<ChessPiece> captured_;
  bool is_initial_;
  // std::vector<size_t> checkpoints_;

  void UndoMove(ChessMove mv);

 public:
  explicit BoardUpdater(ChessBoard &b) : board_(b), is_initial_(false) {}
  void SaveMove(ChessMove v) { history_.push_back(v); }
  void AddCaptured(ChessPiece c) { captured_.push_back(c); }
  void SetIsInitial(bool v) { is_initial_ = v; }
  void Rewind();
};

#endif  // CHESS_H_
