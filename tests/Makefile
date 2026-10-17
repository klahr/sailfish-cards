CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
SRC = test_card.cpp ../src/cards/cards.cpp

test: test_card
	./test_card

test_card: $(SRC) $(wildcard ../src/cards/*.h)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC)

clean:
	rm -f test_card

.PHONY: test clean
