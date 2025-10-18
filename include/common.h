#ifndef COMMON_H
#define COMMON_H

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

enum Suit : uint8_t {
    spades = 0,
    hearts = 1,
    diamonds = 2,
    clubs = 3,
};

inline constexpr std::size_t c_num_suits = 4;
inline constexpr std::size_t c_num_cards_in_suit = 13;
inline constexpr std::size_t c_num_cards = c_num_cards_in_suit * c_num_suits;
inline constexpr std::size_t c_tableau_columns = 7;
inline constexpr uint8_t c_null_index = static_cast<uint8_t>(c_num_cards);
inline constexpr uint8_t c_hidden_index = static_cast<uint8_t>(c_num_cards + 1);
const std::array<std::string, c_num_suits> c_suit_strings = {"♠", "♥", "♦",
                                                             "♣"};
const std::array<std::string, c_num_cards_in_suit> c_rank_strings = {
    "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};

inline auto card_to_index(Suit suit, std::size_t rank) -> std::size_t {
    return (rank * c_num_suits) + static_cast<std::size_t>(suit);
}

inline auto index_to_card(std::size_t index) -> std::pair<Suit, std::size_t> {
    assert(index < c_num_cards);
    Suit suit = static_cast<Suit>(index % c_num_suits);
    std::size_t rank = index / c_num_suits;
    return {suit, rank};
}

#endif
