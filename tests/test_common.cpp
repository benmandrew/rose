#include <catch2/catch_test_macros.hpp>

#include "common.hpp"

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
