#ifndef HASH_H_
#define HASH_H_

#include <algorithm>
#include <array>
#include <iostream>
#include <random>

using uint128_t = unsigned __int128;

inline std::ostream &operator<<(std::ostream &os, uint128_t v) {
  if (v == 0) return os << "0";
  char stk[64];
  int ptr = 0;
  while (v > 0) {
    stk[ptr++] = (v % 10) + '0';
    v /= 10;
  }
  while (ptr > 0) os << stk[--ptr];
  return os;
}

template <size_t N, size_t M, size_t K>
class ZobristHash {
  std::array<std::array<uint128_t, M>, N> hash_pieces_;
  std::array<uint128_t, K> hash_player_;

 public:
  explicit ZobristHash(uint64_t seed = 0) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<uint64_t> dist;

    auto Rand128 = [&rng, &dist]() -> uint128_t {
      return (uint128_t(dist(rng)) << 64) | uint128_t(dist(rng));
    };

    for (size_t i = 0; i < N; ++i) {
      std::generate(hash_pieces_[i].begin(), hash_pieces_[i].end(), Rand128);
    }
    for (size_t i = 0; i < K; ++i) hash_player_[i] = Rand128();
  }

  uint128_t GetPieceHash(size_t i, size_t j) const {
    return hash_pieces_[i][j];
  };

  uint128_t GetPlayerHash(size_t idx) const { return hash_player_[idx]; }
};
enum Status : uint8_t { NO_VALUE, EXACT_VALUE, LOWER_BOUND, UPPER_BOUND };

template <class MoveT>
struct Entry {
  Status flag;
  uint128_t hash_value;
  float score;
  int depth;
  MoveT best_move;

  Entry() = default;
  Entry(Status f, uint128_t v, float s, int d, const MoveT &mv)
      : flag(f), hash_value(v), score(s), depth(d), best_move(mv) {}
};

template <size_t B, class MoveT>
class TranspositionTable {
  static constexpr size_t N = (1 << B);
  std::array<Entry<MoveT>, N> table_;

 public:
  explicit TranspositionTable() {
    for (size_t i = 0; i < N; ++i) table_[i].flag = NO_VALUE;
  }

  Entry<MoveT> &GetEntry(uint128_t v) { return table_[v & (N - 1)]; }
};

#endif  // HASH_H_
