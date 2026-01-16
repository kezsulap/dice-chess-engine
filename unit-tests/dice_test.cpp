#include "../board.hpp"
#include "test_utils.hpp"
#include <cassert>
int main() {
	std::vector<dice_roll> dices;
	for (int i = 0; i < (int)DICE_ROLL_LENGTH; ++i) {
		dice_roll decoded = dice_roll::decode(i);
		ASSERT_EQUAL(decoded.encode(), i);
		dices.push_back(decoded);
	}
	for (size_t i = 0; i + 1 < dices.size(); ++i)
		assert(dices[i].total_rolls() <= dices[i + 1].total_rolls());
}
