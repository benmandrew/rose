#include <catch2/catch_test_macros.hpp>

#include "common.h"
#include "table.h"

TEST_CASE("Table default construction", "[table]") {
    Table table;
    for (size_t col = 0; col < c_tableau_columns; col++) {
        REQUIRE(table.n_cards_in_tableau_column(col) == 0);
    }
}

TEST_CASE("Tableau", "[table]") {
    Table table;
    table.m_tableau_visible_indices = {0, 1, 2, 3, 4, 5, 6};
    table.m_tableau_hidden_indices = {7, 8, 9, 10, 11, 12, 13};
    table.m_deck[3] = 14;
    table.m_deck[4] = 15;

    SECTION("Set state") {
        REQUIRE(table.tableau_to_string() ==
            "  ?    ?    ?    ?    ?    ?    ?  \n"
            " A♠   A♥   A♦   4♦   4♣   2♥   2♦  \n"
            "                A♣   2♠            \n");
    }

    SECTION("Add to visible column") {
        table.add_to_visible_tableau_column(3, 51);
        REQUIRE(table.tableau_to_string() ==
            "  ?    ?    ?    ?    ?    ?    ?  \n"
            " A♠   A♥   A♦   4♦   4♣   2♥   2♦  \n"
            "                A♣   2♠            \n"
            "                K♣                 \n");
    }

    SECTION("Seeded random state") {
        std::optional<std::mt19937> rng =
            std::make_optional<std::mt19937>(0);
        auto deck = random_deck(rng);
        table = Table(deck);
        REQUIRE(table.tableau_to_string() ==
            " Q♠    ?    ?    ?    ?    ?    ?  \n"
            "      A♦    ?    ?    ?    ?    ?  \n"
            "           A♠    ?    ?    ?    ?  \n"
            "                Q♥    ?    ?    ?  \n"
            "                     2♠    ?    ?  \n"
            "                          3♦    ?  \n"
            "                               9♠  \n");

        SECTION("Move from hidden to visible") {
            table.m_tableau_visible_indices[6] = c_null_index;
            table.move_from_hidden_to_visible(6);
            table.m_tableau_visible_indices[4] = c_null_index;
            table.move_from_hidden_to_visible(4);
            REQUIRE(table.tableau_to_string() ==
            " Q♠    ?    ?    ?    ?    ?    ?  \n"
            "      A♦    ?    ?    ?    ?    ?  \n"
            "           A♠    ?    ?    ?    ?  \n"
            "                Q♥   10♥   ?    ?  \n"
            "                           ?    ?  \n"
            "                          3♦   3♥  \n");
        }
    }
}

TEST_CASE("Header", "[table]") {
    Table table;
    table.m_stock_index = 0;
    table.m_waste_index = 20;
    table.m_foundation_indices = {10, 33, c_null_index, 39};
    REQUIRE(table.header_to_string() ==
        "Stock: ?  Waste: 6♠ Foundations: 3♦  9♥      10♣ \n");
}

TEST_CASE("Stock to Waste", "[table]") {
    Table table;
    table.m_stock_index = 0;
    table.m_deck[0] = 9;
    table.m_deck[9] = 14;
    table.m_deck[14] = c_null_index;
    table.m_waste_index = c_null_index;
    REQUIRE(table.header_to_string() ==
        "Stock: ?  Waste:    Foundations:                 \n");
    table.move_from_stock_to_waste();
    REQUIRE(table.header_to_string() ==
        "Stock: ?  Waste: A♠ Foundations:                 \n");
    table.move_from_stock_to_waste();
    REQUIRE(table.header_to_string() ==
        "Stock: ?  Waste: 3♥ Foundations:                 \n");
    table.move_from_stock_to_waste();
    REQUIRE(table.header_to_string() ==
        "Stock:    Waste: 4♦ Foundations:                 \n");
    table.move_from_stock_to_waste();
    REQUIRE(table.header_to_string() ==
        "Stock:    Waste: 4♦ Foundations:                 \n");
    table.reset_stock_from_waste();
    REQUIRE(table.header_to_string() ==
        "Stock: ?  Waste:    Foundations:                 \n");
    table.reset_stock_from_waste();
    REQUIRE(table.header_to_string() ==
        "Stock: ?  Waste:    Foundations:                 \n");
}
