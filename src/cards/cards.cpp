#include "card.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace cards {

Card parseCard(const std::string &text)
{
    if (text.size() < 2)
        throw std::invalid_argument("bad card: " + text);
    std::string r = text.substr(0, text.size() - 1);
    char s = static_cast<char>(std::toupper(static_cast<unsigned char>(text.back())));
    int rank = 0;
    if (r == "A" || r == "a") rank = 1;
    else if (r == "J" || r == "j") rank = 11;
    else if (r == "Q" || r == "q") rank = 12;
    else if (r == "K" || r == "k") rank = 13;
    else if (r == "T" || r == "t") rank = 10;
    else rank = std::stoi(r);
    int suit = s == 'C' ? Clubs : s == 'D' ? Diamonds : s == 'H' ? Hearts : s == 'S' ? Spades : -1;
    Card c(rank, suit);
    if (!c.valid())
        throw std::invalid_argument("bad card: " + text);
    return c;
}

Cards parseCards(const std::string &text)
{
    Cards out;
    std::string token;
    for (char ch : text + " ") {
        if (ch == ' ' || ch == ',') {
            if (!token.empty())
                out.push_back(parseCard(token));
            token.clear();
        } else {
            token += ch;
        }
    }
    return out;
}

Cards fullDeck()
{
    Cards deck;
    for (int s = 0; s < 4; ++s)
        for (int r = 1; r <= 13; ++r)
            deck.emplace_back(r, s);
    return deck;
}

bool contains(const Cards &cards, const Card &card)
{
    return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void removeCard(Cards &cards, const Card &card)
{
    auto it = std::find(cards.begin(), cards.end(), card);
    if (it != cards.end())
        cards.erase(it);
}

int countSuit(const Cards &cards, int suit)
{
    return static_cast<int>(std::count_if(cards.begin(), cards.end(), [suit](const Card &c) { return c.suit == suit; }));
}

void sortAlternating(Cards &cards)
{
    static const int order[] = {0, 1, 3, 2};
    std::sort(cards.begin(), cards.end(), [](const Card &a, const Card &b) {
        if (a.suit != b.suit)
            return order[a.suit] < order[b.suit];
        return a.high() < b.high();
    });
}

} // namespace cards
