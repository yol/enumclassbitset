#include "enumclassbitset.hpp"

#define BOOST_TEST_MODULE EnumClassBitsetTest
#include <boost/test/unit_test.hpp>
#include <boost/test/execution_monitor.hpp>

enum class TestEnum
{
    A,
    B,
    C,
    D,
    E
};

namespace Util {
    template<>
    struct EnumTraits<TestEnum> {
        static constexpr TestEnum max = TestEnum::E;
    };
}

using Util::EnumClassBitset;

BOOST_AUTO_TEST_CASE( Iterator )
{
    EnumClassBitset<TestEnum> e;
    BOOST_CHECK(e.begin() == e.end());
    BOOST_CHECK_EQUAL(e.all(), false);
    BOOST_CHECK_EQUAL(e.any(), false);
    BOOST_CHECK_EQUAL(e.none(), true);
    // Check access to first and last element
    BOOST_CHECK_EQUAL(e.test(TestEnum::A), false);
    BOOST_CHECK_EQUAL(e.test(TestEnum::E), false);
    // Ignore the unused variable warning here
    for (auto el : e) {
        // may not be reached, no element is in the set!
        BOOST_CHECK(false);
    }
    const auto& ce = e;
    e.set(TestEnum::A);
    BOOST_CHECK(e.begin() != e.end());
    BOOST_CHECK(ce.begin() != ce.end());
    BOOST_CHECK_EQUAL(ce.none(), false);
    BOOST_CHECK_EQUAL(ce.any(), true);

    EnumClassBitset<TestEnum> e2;
    BOOST_CHECK(e != e2);
    e2.set(TestEnum::A);
    BOOST_CHECK(e == e2);
    BOOST_CHECK(!(e != e2));

    EnumClassBitset<TestEnum>::reference e2ref = e2[TestEnum::B];
    BOOST_CHECK_EQUAL(e2.test(TestEnum::B), false);
    e2ref = true;
    BOOST_CHECK_EQUAL(e2.test(TestEnum::B), true);

    std::bitset<5> raw = e2.to_raw_bitset();
    BOOST_CHECK_EQUAL(raw.count(), 2);
    raw.set(static_cast<int> (TestEnum::C));
    BOOST_CHECK_EQUAL(e2.test(TestEnum::C), false);

    auto e3 = e;
    e3 &= e2;
    BOOST_CHECK_EQUAL(e3.count(), 1);
    auto e4 = ~e3;
    BOOST_CHECK_EQUAL(e4.count(), 4);
    // Just instantiate to check it compiles correctly
    e3 |= e2;
    e3 ^= e2;
    auto e5 = e & e2;
    BOOST_CHECK_EQUAL(e5.count(), 1);
    e5 = e | e2;
    e5 = e ^ e2;

    unsigned int count = 0;
    for (auto el : e) {
        BOOST_CHECK(el == TestEnum::A);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 1);
    count = 0;
    for (auto el: ce) {
        BOOST_CHECK(el == TestEnum::A);
        count++;
    }
    BOOST_CHECK_EQUAL(count, 1);
    e.set(TestEnum::B).set(TestEnum::C).set(TestEnum::D).set(TestEnum::E);
    BOOST_CHECK_EQUAL(e.all(), true);
    count = 0;
    std::uint8_t bmask = 0;
    for (auto el : e) {
        count++;
        bmask |= (1 << static_cast<unsigned int> (el));
    }
    BOOST_CHECK_EQUAL(count, 5);
    BOOST_CHECK_EQUAL(bmask, 0x1f);
}
