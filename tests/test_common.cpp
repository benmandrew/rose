#include <catch2/catch_test_macros.hpp>

#include "common.h"

TEST_CASE("card index roundtrip", "[common]") {
    for (std::size_t suit = 0; suit < c_num_suits; suit++) {
        for (std::size_t rank = 0; rank < c_num_cards_in_suit; rank++) {
            auto idx = card_to_index(static_cast<Suit>(suit), rank);
            auto [s, r] = index_to_card(idx);
            REQUIRE(static_cast<std::size_t>(s) == suit);
            REQUIRE(r == rank);
        }
    }
}

TEST_CASE("constants sanity", "[common]") {
    REQUIRE(c_num_suits == 4);
    REQUIRE(c_tableau_columns == 7);
    REQUIRE(c_num_cards == c_num_suits * c_num_cards_in_suit);
}
