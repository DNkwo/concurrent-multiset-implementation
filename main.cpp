#include "CMSet.hpp"

int main() {
    CMSet_Lock<int> cmset;

    cmset.add(5);
    cmset.add(1);
    cmset.add(5);
    cmset.add(3);

    if (cmset.contains(5)) {
        std::cout << "5 is in the multiset, count: " << cmset.count(5) << std::endl;
    }

    bool removed = cmset.remove(5);
    std::cout << "Attempt to remove 5 was " << (removed ? "successful" : "unsuccessful") << std::endl;


}