#include "card.h"

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

} // namespace cards
