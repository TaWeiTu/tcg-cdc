#ifndef CHESS_H_
#define CHESS_H_

#include <array>
#include <vector>

enum ChessPiece : uint8_t {
  NO_PIECE = static_cast<uint8_t>(-1),
  COVERED_PIECE = static_cast<uint8_t>(-2),

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

  kNumChessPieces = 7
};

constexpr ChessPiece GetChessPieceType(ChessPiece piece);
constexpr bool GetChessPieceColor(ChessPiece piece);
bool CanCapture(ChessPiece capturer, ChessPiece capturee);

struct ChessMove {
  uint8_t src;
  uint8_t dst;

  ChessMove() = default;
  ChessMove(uint8_t s, uint8_t d) : src(s), dst(d) {}

  constexpr bool IsFlip() const { return src == static_cast<uint8_t>(-1); }
};

class ChessBoard {
  static constexpr size_t kNumCells = 32;
  std::array<ChessPiece, kNumCells> board_;
  std::array<uint8_t, kNumChessPieces * 2> covered_;

 public:
  explicit ChessBoard();
  std::vector<ChessMove> GenerateMoves();
  void MakeMove(const ChessMove &mv);
};

#endif  // CHESS_H_
