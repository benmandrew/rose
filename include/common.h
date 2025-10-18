#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <string>
#include <utility>

enum Suit : uint8_t {
    spades = 0,
    hearts = 1,
    diamonds = 2,
    clubs = 3,
};

inline constexpr size_t c_num_suits = 4;
inline constexpr size_t c_num_cards_in_suit = 13;
inline constexpr size_t c_num_cards = c_num_cards_in_suit * c_num_suits;
inline constexpr size_t c_tableau_columns = 7;
inline constexpr uint8_t c_null_index = static_cast<uint8_t>(c_num_cards);
constexpr std::string c_no_card_string = " ";
inline constexpr uint8_t c_hidden_index = static_cast<uint8_t>(c_num_cards + 1);
constexpr std::string c_hidden_card_string = "?";
constexpr std::array<std::string, c_num_suits> c_suit_strings = {"♠", "♥", "♦",
                                                                 "♣"};
constexpr std::array<std::string, c_num_cards_in_suit> c_rank_strings = {
    "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};

inline auto card_to_index(Suit suit, size_t rank) -> size_t {
    assert(rank < c_num_cards_in_suit);
    return (rank * c_num_suits) + static_cast<size_t>(suit);
}

inline auto index_to_card(size_t index) -> std::pair<Suit, size_t> {
    assert(index < c_num_cards);
    Suit suit = static_cast<Suit>(index % c_num_suits);
    size_t rank = index / c_num_suits;
    return {suit, rank};
}

inline auto random_deck(std::optional<std::mt19937>& rng)
    -> std::array<uint8_t, c_num_cards> {
    std::array<uint8_t, c_num_cards> cards{};
    for (size_t i = 0; i < c_num_cards; i++) {
        cards[i] = static_cast<uint8_t>(i);
    }
    if (rng) {
        std::shuffle(cards.begin(), cards.end(), *rng);
    } else {
        std::shuffle(cards.begin(), cards.end(),
                     std::mt19937(std::random_device{}()));
    }
    return cards;
}
