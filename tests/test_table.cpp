#include <catch2/catch_test_macros.hpp>

#include "table.h"

TEST_CASE("table default construction", "[table]") {
    Table t;
    for (size_t col = 0; col < c_tableau_columns; col++) {
        REQUIRE(t.n_cards_in_tableau_column(col) == 0);
    }
}
