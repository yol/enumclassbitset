#include <iostream>

#include "enumclassbitset.hpp"

enum class ConntrackState {
    NEW,
    ESTABLISHED,
    RELATED
};

namespace Util {
	template<>
	struct EnumTraits<ConntrackState> {
	    static constexpr ConntrackState max = ConntrackState::RELATED;
	};
}
typedef Util::EnumClassBitset<ConntrackState> ConntrackStateSet;

void example() {
    ConntrackStateSet states;
    states.set(ConntrackState::NEW);
    states.set(ConntrackState::ESTABLISHED);
    bool result = states.test(ConntrackState::NEW);
    std::cout << result << std::endl;    // 1
    for (auto state : states) {
        switch (state) {
            case ConntrackState::NEW:
                std::cout << "NEW is in the set" << std::endl;         // will be output
                break;
            case ConntrackState::ESTABLISHED:
                std::cout << "ESTABLISHED is in the set" << std::endl; // will be output
                break;
            case ConntrackState::RELATED:
                std::cout << "RELATED is in the set" << std::endl;     // will not be output
                break;
        }
    }
}
int main() {
	example();
}
