#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include "common.hpp"
#include "moves.hpp"
#include "table.hpp"

TEST_CASE("Stock-waste cycling", "[moves]") {
    Table table;
    table.m_stock_index = CARD("A♠");
    table.m_deck[CARD("A♠")] = CARD("2♣");
    table.m_deck[CARD("2♣")] = CARD("3♦");
    table.m_deck[CARD("3♦")] = c_null_index;
    stock_to_waste(table);
    REQUIRE(table.header_to_string() ==
            "Stock: ?  Waste: A♠ Foundations:                 \n");
    stock_to_waste(table);
    REQUIRE(table.header_to_string() ==
            "Stock: ?  Waste: 2♣ Foundations:                 \n");
    stock_to_waste(table);
    REQUIRE(table.header_to_string() ==
            "Stock:    Waste: 3♦ Foundations:                 \n");
    stock_to_waste(table);
    REQUIRE(table.header_to_string() ==
            "Stock: ?  Waste:    Foundations:                 \n");
}

TEST_CASE("Tableau to tableau", "[moves]") {
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

TEST_CASE("Table movement", "[table]") {
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

    SECTION("Waste to Tableau") {
        table.m_waste_index = CARD("K♠");
        table.m_deck[CARD("K♠")] = CARD("Q♥");
        REQUIRE(table.header_to_string() ==
                "Stock:    Waste: K♠ Foundations:                 \n");
        waste_to_tableau(table, 0);
        REQUIRE(table.tableau_to_string() ==
                " K♠                                \n");
        REQUIRE(table.header_to_string() ==
                "Stock:    Waste: Q♥ Foundations:                 \n");
        waste_to_tableau(table, 0);
        REQUIRE(table.tableau_to_string() ==
                " K♠                                \n"
                " Q♥                                \n");
    }

    SECTION("Foundation to Tableau") {
        table.m_foundation_indices[static_cast<size_t>(Suit::spades)] =
            CARD("3♠");
        table.m_tableau_visible_indices[2] = CARD("4♦");
        REQUIRE(table.header_to_string() ==
                "Stock:    Waste:    Foundations: 3♠              \n");
        REQUIRE(table.tableau_to_string() ==
                "           4♦                      \n");
        foundation_to_tableau(table, Suit::spades, 2);
        REQUIRE(table.header_to_string() ==
                "Stock:    Waste:    Foundations:                 \n");
        REQUIRE(table.tableau_to_string() ==
                "           4♦                      \n"
                "           3♠                      \n");
    }
}

TEST_CASE("Generate moves", "[moves]") {
    SECTION("Basic moves") {
        std::optional<std::mt19937> rng = std::make_optional<std::mt19937>(0);
        auto deck = random_deck(rng);
        Table table(deck);
        auto moves = generate_moves(table);
        REQUIRE(moves.size() == 4);
        REQUIRE(table.header_to_string() ==
                "Stock: ?  Waste:    Foundations:                 \n");
        REQUIRE(table.tableau_to_string() ==
                " Q♠    ?    ?    ?    ?    ?    ?  \n"
                "      A♦    ?    ?    ?    ?    ?  \n"
                "           A♠    ?    ?    ?    ?  \n"
                "                Q♥    ?    ?    ?  \n"
                "                     2♠    ?    ?  \n"
                "                          3♦    ?  \n"
                "                               9♠  \n");
        REQUIRE(moves[0].to_string() == "SW");
        REQUIRE(moves[1].to_string() == "TF 1");
        REQUIRE(moves[2].to_string() == "TF 2");
        REQUIRE(moves[3].to_string() == "TT 4 5 1");
        stock_to_waste(table);
        stock_to_waste(table);
        stock_to_waste(table);
        REQUIRE(table.header_to_string() ==
                "Stock: ?  Waste: 8♦ Foundations:                 \n");
        moves = generate_moves(table);
        REQUIRE(moves.size() == 5);
        REQUIRE(moves[0].to_string() == "SW");
        REQUIRE(moves[1].to_string() == "WT 6");
        REQUIRE(moves[2].to_string() == "TF 1");
        REQUIRE(moves[3].to_string() == "TF 2");
        REQUIRE(moves[4].to_string() == "TT 4 5 1");
    }

    SECTION("No repeat opposite moves") {
        Table table;
        table.m_tableau_visible_indices[0] = CARD("3♠");
        table.m_deck[CARD("3♠")] = CARD("4♥");
        table.m_foundation_indices[static_cast<size_t>(Suit::spades)] =
            CARD("2♠");
        auto moves = generate_moves(table, std::nullopt);
        REQUIRE(moves.size() == 1);
        REQUIRE(moves[0].to_string() == "TF 0");
        tableau_to_foundation(table, 0);
        moves = generate_moves(table, moves[0]);
        REQUIRE(moves.size() == 0);
    }

    SECTION("Don't move kings between empty tableau columns") {
        Table table;
        table.m_tableau_visible_indices[0] = CARD("K♠");
        auto moves = generate_moves(table, std::nullopt);
        REQUIRE(moves.size() == 0);
    }
}
