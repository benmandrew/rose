#include <catch2/catch_test_macros.hpp>

#include "common.h"
#include "moves.h"
#include "table.h"

TEST_CASE("Tableau", "[moves]") {
    Table table;
    table.m_tableau_hidden_indices = {CARD("A♠"), CARD("A♥"), CARD("A♦"),
                                      CARD("A♣"), CARD("2♠"), CARD("2♥"),
                                      CARD("2♦")};
    table.m_tableau_visible_indices = {CARD("6♠"),  CARD("9♠"), CARD("10♦"),
                                       CARD("10♣"), CARD("J♠"), CARD("J♥"),
                                       CARD("J♦")};
    table.m_deck[CARD("6♠")] = CARD("7♦");
    table.m_deck[CARD("7♦")] = CARD("8♣");
    table.m_deck[CARD("8♣")] = CARD("9♥");
    table.m_deck[CARD("9♥")] = CARD("10♠");
    table.m_deck[CARD("9♠")] = CARD("10♥");
    REQUIRE(table.tableau_to_string() ==
            "  ?    ?    ?    ?    ?    ?    ?  \n"
            " 10♠  10♥  10♦  10♣  J♠   J♥   J♦  \n"
            " 9♥   9♠                           \n"
            " 8♣                                \n"
            " 7♦                                \n"
            " 6♠                                \n");
    SECTION("Move cards within tableau") {
        tableau_to_tableau(table, 0, 1, 3);
        REQUIRE(table.tableau_to_string() ==
                "  ?    ?    ?    ?    ?    ?    ?  \n"
                " 10♠  10♥  10♦  10♣  J♠   J♥   J♦  \n"
                " 9♥   9♠                           \n"
                "      8♣                           \n"
                "      7♦                           \n"
                "      6♠                           \n");
        tableau_to_tableau(table, 0, 3, 1);
        REQUIRE(table.tableau_to_string() ==
                "  ?    ?    ?    ?    ?    ?    ?  \n"
                " 10♠  10♥  10♦  10♣  J♠   J♥   J♦  \n"
                "      9♠        9♥                 \n"
                "      8♣                           \n"
                "      7♦                           \n"
                "      6♠                           \n");
    }
}

TEST_CASE("Table", "[table]") {
    Table table;

    SECTION("Waste to Foundation") {
        table.m_waste_index = CARD("A♠");
        table.m_deck[CARD("A♠")] = CARD("2♠");
        table.m_deck[CARD("2♠")] = CARD("3♠");
        table.m_deck[CARD("3♠")] = c_null_index;
        REQUIRE(table.header_to_string() ==
                "Stock:    Waste: A♠ Foundations:                 \n");
        waste_to_foundation(table);
        REQUIRE(table.header_to_string() ==
                "Stock:    Waste: 2♠ Foundations: A♠              \n");
        waste_to_foundation(table);
        REQUIRE(table.header_to_string() ==
                "Stock:    Waste: 3♠ Foundations: 2♠              \n");
        waste_to_foundation(table);
        REQUIRE(table.header_to_string() ==
                "Stock:    Waste:    Foundations: 3♠              \n");
    }
}
