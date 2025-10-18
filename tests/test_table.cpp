#include <catch2/catch_test_macros.hpp>

#include "table.h"

TEST_CASE("Table default construction", "[table]") {
    Table t;
    for (size_t col = 0; col < c_tableau_columns; col++) {
        REQUIRE(t.n_cards_in_tableau_column(col) == 0);
    }
}

TEST_CASE("Printing tableau", "[table]") {
    Table t;
    t.m_tableau_visible_indices = {0, 1, 2, 3, 4, 5, 6};
    t.m_tableau_hidden_indices = {7, 8, 9, 10, 11, 12, 13};
    t.m_deck[3] = 14;
    t.m_deck[4] = 15;

    SECTION("Initial state") {
        REQUIRE(t.tableau_to_string() ==
            "  ?    ?    ?    ?    ?    ?    ?  \n"
            " A♠   A♥   A♦   4♦   4♣   2♥   2♦  \n"
            "                A♣   2♠            \n");
    }
}
