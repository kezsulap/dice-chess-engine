#include <bits/stdc++.h>
#include "board.hpp"
using namespace std;
int main(int argc, char **argv) {
	assert(argc == 2 || argc == 3);
	string fen = argv[1];
	board b = parse_fen(fen);
	b.dump(std::cerr);
	b.finalize_en_passant();
	b.dump(std::cerr);
	return 0;
	std::optional<dice_roll> roll;
	if (argc == 3) roll = parse_dice_roll(argv[2]);
	movelist m = b.generate_moves();
	b.dump(cout);
	bool first_empty = true;
	stringstream empty;
	size_t sum = 0;
	std::set<board> all_positions_set;
	int width = get_screen_width();
	for (auto &dice : full_and_partial_dice_rolls) {
		const auto &moves = m.get_moves(dice);
		if (!moves.empty()) {
			all_positions_set.insert(moves.begin(), moves.end());
			sum += moves.size();
			if (!roll.has_value() || *roll == dice) {
				std::cout << dice << " (total of " << moves.size() << " possibilities):\n";
				bulk_dump_boards(m.get_moves(dice), cout);
				std::cout << std::string(width, '-') << "\n";
			}
		}
		else {
			if (!first_empty) empty << ", ";
			first_empty = false;
			empty << dice;
		}
	}
	if (!first_empty) {
		std::cout << "King capture found from: " << empty.str() << "\n";
	}
	std::cout << "Total of " << sum << " different moves, reaching " << all_positions_set.size() << " different positions\n";
}
