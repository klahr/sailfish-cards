// Tests for the shared card code:
//   make -C tests

#include "../src/cards/card.h"
#include "../src/cards/testing.h"

#include <stdexcept>

using namespace cards;

namespace {

TEST(ids_cover_the_pack)
{
    const Cards deck = fullDeck();
    CHECK_EQ(deck.size(), 52u);
    for (int id = 0; id < 52; ++id) {
        CHECK_EQ(deck[id].id(), id);
        CHECK(Card::fromId(id) == deck[id]);
        CHECK(deck[id].valid());
    }
    CHECK(!Card().valid());
}

TEST(parses_compact_notation)
{
    CHECK(parseCard("AC") == Card(1, Clubs));
    CHECK(parseCard("10H") == Card(10, Hearts));
    CHECK(parseCard("TD") == Card(10, Diamonds));
    CHECK(parseCard("qs") == Card(12, Spades));
    CHECK(parseCards("5H, JD KC") == Cards({Card(5, Hearts), Card(11, Diamonds), Card(13, Clubs)}));
    bool threw = false;
    try {
        parseCard("1X");
    } catch (const std::invalid_argument &) {
        threw = true;
    }
    CHECK(threw);
}

TEST(aces_are_high_when_asked)
{
    CHECK_EQ(parseCard("AS").high(), 14);
    CHECK_EQ(parseCard("KS").high(), 13);
    CHECK_EQ(parseCard("2S").high(), 2);
}

} // namespace

int main()
{
    return testing::run();
}
