#include <catch2/catch_test_macros.hpp>

#include "table.h"
#include "moves.h"

TEST_CASE("Tableau", "[moves]") {
    Table table;
    table.m_tableau_hidden_indices = {0, 1, 2, 3, 4, 5, 6};
    table.m_tableau_visible_indices = {20, 32, 38, 39, 40, 41, 42};
    table.m_deck[20] = 26;
    table.m_deck[26] = 31;
    table.m_deck[31] = 33;
    table.m_deck[33] = 36;
    table.m_deck[32] = 37;

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
