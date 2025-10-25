#include <catch2/catch_test_macros.hpp>

#include "common.hpp"
#include "res_config.hpp"

TEST_CASE("Card index roundtrip", "[common]") {
    for (size_t suit = 0; suit < c_num_suits; suit++) {
        for (size_t rank = 0; rank < c_num_cards_in_suit; rank++) {
            auto idx = card_to_index(static_cast<Suit>(suit), rank);
            auto [s, r] = index_to_card(idx);
            REQUIRE(static_cast<size_t>(s) == suit);
            REQUIRE(r == rank);
        }
    }
}

TEST_CASE("Constants sanity", "[common]") {
    REQUIRE(c_num_suits == 4);
    REQUIRE(c_tableau_columns == 7);
    REQUIRE(c_num_cards == c_num_suits * c_num_cards_in_suit);
}

TEST_CASE("Card of string", "[common]") {
    REQUIRE(card_of_string("A♠") == 0);
    REQUIRE(card_of_string("K♠") == 48);
    REQUIRE(card_of_string("A♥") == 1);
    REQUIRE(card_of_string("K♥") == 49);
    REQUIRE(card_of_string("A♦") == 2);
    REQUIRE(card_of_string("K♦") == 50);
    REQUIRE(card_of_string("A♣") == 3);
    REQUIRE(card_of_string("K♣") == 51);
}

TEST_CASE("Import deck", "[common]") {
    auto deck = import_deck(res_dir / "random-deck.txt");
    REQUIRE(deck[0] == CARD("Q♠"));
    REQUIRE(deck[1] == CARD("K♠"));
    REQUIRE(deck[2] == CARD("A♦"));
    REQUIRE(deck[3] == CARD("2♦"));
    REQUIRE(deck[4] == CARD("2♣"));
    REQUIRE(deck[51] == CARD("A♣"));
}
