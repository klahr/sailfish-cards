#pragma once

// A tiny test runner for the plain C++ rule tests, no framework needed:
//
//   TEST(name) { CHECK(cond); CHECK_EQ(a, b); }
//   int main() { return cards::testing::run(); }

#include <cstdio>
#include <functional>
#include <vector>

namespace cards {
namespace testing {

inline int failures = 0;
inline int checks = 0;
inline const char *current = "";

struct Test {
    const char *name;
    std::function<void()> body;
};

inline std::vector<Test> &tests()
{
    static std::vector<Test> all;
    return all;
}

struct Register {
    Register(const char *name, std::function<void()> body) { tests().push_back({name, body}); }
};

inline int run()
{
    for (const Test &t : tests()) {
        current = t.name;
        const int before = failures;
        t.body();
        std::printf("%s %s\n", failures == before ? "ok  " : "FAIL", t.name);
    }
    std::printf("\n%zu tests, %d checks, %d failures\n", tests().size(), checks, failures);
    return failures ? 1 : 0;
}

} // namespace testing
} // namespace cards

#define CHECK(cond)                                                                      \
    do {                                                                                 \
        ++::cards::testing::checks;                                                      \
        if (!(cond)) {                                                                   \
            ++::cards::testing::failures;                                                \
            std::printf("FAIL %s (%s:%d): %s\n", ::cards::testing::current, __FILE__,    \
                        __LINE__, #cond);                                                \
        }                                                                                \
    } while (0)

#define CHECK_EQ(a, b)                                                                   \
    do {                                                                                 \
        ++::cards::testing::checks;                                                      \
        const auto va = (a);                                                             \
        const auto vb = (b);                                                             \
        if (!(va == vb)) {                                                               \
            ++::cards::testing::failures;                                                \
            std::printf("FAIL %s (%s:%d): %s == %s (%d vs %d)\n",                        \
                        ::cards::testing::current, __FILE__, __LINE__, #a, #b, int(va),  \
                        int(vb));                                                        \
        }                                                                                \
    } while (0)

#define TEST(name)                                                       \
    static void name();                                                  \
    static ::cards::testing::Register register_##name(#name, name);      \
    static void name()
