#include "test_utils.hpp"
#include "../board.hpp"
#include <bits/stdc++.h>

std::vector<board> passed_tests;
std::vector<std::string> passed_tests_annotations;

int passed_orientations = 0;

std::stringstream failed;

std::map<std::pair <std::string, std::string>, int> existing_tests;

const bool DUMP_PASSED_TESTS = false;
// const bool DUMP_PASSED_TESTS = true;

void assert_move_equal_impl(int line, const std::string &fen, const std::string &dice_string, const std::vector<std::string> &expected_fens, bool shift = false) {
	if (DUMP_PASSED_TESTS) {
		std::cerr << "PROCESSING ALL ORIENTATIONS OF " << fen << " WITH DICE " << dice_string << " FROM LINE " << line << "\n";
	}
	auto [it, added] = existing_tests.insert(std::make_pair(std::make_pair(fen, dice_string), line));
	if (!added) {
		std::cerr << fen << ", " << dice_string << " repeated, line " << it->second << " and " << line << "\n";
		assert(false);
	}
	dice_roll dice = parse_dice_roll(dice_string);
	std::vector<bool> passed;
	bool can_flip_horizontally = parse_fen(fen).get_castling_mask() == 0;
	std::vector<bool> possible_flip_horizontally = can_flip_horizontally ? std::vector<bool>{false, true} : std::vector<bool>{false};
	bool all_passed = true;
	int orientations_count = 0;
	for (bool flip_horizontally : possible_flip_horizontally) {
		for (bool flipped : {false, true}) {
			board b0 = parse_fen(fen);
			if (flipped) b0.flip_in_place();
			if (flip_horizontally) b0.flip_horizontally_in_place();
			std::vector<int> shifts;
			if (shift) shifts = b0.get_shift_range();
			else shifts = {0};
			for (int shift_length : shifts) {
				orientations_count++;
				board b = b0;
				if (shift) b.shift_in_place(shift_length);
				auto got_moves = b.generate_moves().get_moves(dice);
				std::set<board> expected_boards;
				for (auto &expected_fen : expected_fens) {
					board expected = parse_fen(expected_fen);
					if (flipped) expected.flip_in_place();
					if (flip_horizontally) expected.flip_horizontally_in_place();
					if (shift) expected.shift_in_place(shift_length);
					expected_boards.insert(expected);
				}
				assert(expected_boards.size() == expected_fens.size());
				std::vector<board> missing_boards, extranous_boards, duplicated_boards;
				std::set<board> got_boards_set;
				for (auto &x : got_moves) {
					if (!expected_boards.count(x)) extranous_boards.push_back(x);
					if (got_boards_set.count(x)) duplicated_boards.push_back(x);
					got_boards_set.insert(x);
				}
				for (auto &x : expected_boards) if (!got_boards_set.count(x)) missing_boards.push_back(x);
				if (!missing_boards.empty() || !extranous_boards.empty() || !duplicated_boards.empty()) {
					passed.push_back(false);
					mark_test_failure();
					failed << RED << "TEST generating boards for " << dice << " at line " << line << ", from position:\n" << CLEAR_COLOURS;
					b.dump(failed);
					failed << "Failed\n";
					if (!missing_boards.empty()) {
						failed << RED << "Output missing the following boards (" << missing_boards.size() << "):\n" << CLEAR_COLOURS;
						bulk_dump_boards(missing_boards, failed);
					}
					if (!extranous_boards.empty()) {
						failed << RED << "Output containing unexpected boards (" << extranous_boards.size() << "):\n" << CLEAR_COLOURS;
						bulk_dump_boards(extranous_boards, failed);
					}
					if (!duplicated_boards.empty()) {
						failed << RED << "Output containing duplicated boards:(" << duplicated_boards.size() << ")\n" << CLEAR_COLOURS;
						bulk_dump_boards(duplicated_boards, failed);
					}
					failed << RED << "Output containing " << got_moves.size() << " positions, expected " << expected_fens.size() << "\n" << CLEAR_COLOURS;
					failed << RED << "ASSERT_MOVES_EQUAL(\"" << (flipped ? b.fen() : fen) << "\", \"" << dice_string << "\", {";
					bool first = true;
					for (auto &x : got_moves) {
						if (!first) failed << ", ";
						first = false;
						failed << "\"" << x.fen() << "\"";
					}
					failed << "});\n";
					failed << RED << "----------------------------------------------\n" << CLEAR_COLOURS;
					all_passed = false;
				} else {
					passed.push_back(true);
					passed_orientations++;
					if (DUMP_PASSED_TESTS) {
						std::cerr << GREEN << "PASSED GENERATING MOVES FROM:\n" << CLEAR_COLOURS;
						b.dump(std::cerr);
						std::cerr << GREEN << "WITH DICE " << dice_string << "\n" << CLEAR_COLOURS;
						bulk_dump_boards(got_moves, std::cerr);
					}
				}
			}
		}
	}
	if (passed != std::vector<bool>(passed.size(), passed[0])) {
		std::cerr << RED << "Some board orientations of " << fen << " passed, while other failed\n" << CLEAR_COLOURS;
		parse_fen(fen).dump(std::cerr);
	}
	if (all_passed) {
		board b = parse_fen(fen);
		passed_tests.push_back(b);
		std::stringstream s;
		s << "Test for dice " << dice << "\nLine = " << line << "\n";
		s << orientations_count << " orientations\n";
		if (expected_fens.empty()) s << "King capture found\n";
		else s << expected_fens.size() << " moves found\n";
		passed_tests_annotations.push_back(s.str());
	}
}

#define ASSERT_MOVES_EQUAL(...) assert_move_equal_impl(__LINE__, __VA_ARGS__)

int main() {
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "BRK", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1", "BRK", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1", "BBB", {});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1", "QQQ", {});
	ASSERT_MOVES_EQUAL("rn3b2/pk2p3/b7/P7/2q5/8/8/4K3 b - - 0 1", "BPR", {"1n3b2/rk2p3/p7/Pb6/2q5/8/8/4K3 w - - 0 1"});
	ASSERT_MOVES_EQUAL("rn3b2/pk2p3/b7/P7/2q5/K7/8/8 b - - 0 1", "BPR", {}, true);
	ASSERT_MOVES_EQUAL("k7/8/8/4p3/8/8/4P3/K7 w - - 0 1", "PPB", {"k7/8/8/4p3/4P3/8/8/K7 b - - 0 1"});
	ASSERT_MOVES_EQUAL("k7/8/8/4p3/8/8/4P3/K7 w - - 0 1", "PBB", {"k7/8/8/4p3/4P3/8/8/K7 b - - 0 1", "k7/8/8/4p3/8/4P3/8/K7 b - - 0 1"});
	ASSERT_MOVES_EQUAL("7k/P7/2N5/8/8/8/8/K7 w - - 0 1", "PPB", {"7k/1B6/2N5/8/8/8/8/K7 b - - 0 1"});
	ASSERT_MOVES_EQUAL("7k/PN6/8/8/8/8/8/K7 w - - 0 1", "PPB", {"N6k/1N6/8/8/8/8/8/K7 b - - 0 1", "B6k/1N6/8/8/8/8/8/K7 b - - 0 1", "R6k/1N6/8/8/8/8/8/K7 b - - 0 1", "Q6k/1N6/8/8/8/8/8/K7 b - - 0 1"});
	ASSERT_MOVES_EQUAL("7k/8/8/8/Pp6/8/8/7K b - a3 0 1", "PRQ", {"7k/8/8/8/P7/1p6/8/7K w - - 0 1", "7k/8/8/8/8/p7/8/7K w - - 0 1"});
	ASSERT_MOVES_EQUAL("7k/8/8/8/Pp6/1P6/8/7K b - a3 0 1", "PBN", {"7k/8/8/8/8/pP6/8/7K w - - 0 1"});
	ASSERT_MOVES_EQUAL("K6k/8/8/8/PpPpP3/8/8/8 b - a3,c3,e3 0 1", "PRQ", {
		"K6k/8/8/8/P1PpP3/1p6/8/8 w - - 0 1",
		"K6k/8/8/8/PpP1P3/3p4/8/8 w - - 0 1",
		"K6k/8/8/8/2PpP3/p7/8/8 w - - 0 1",
		"K6k/8/8/8/P2pP3/2p5/8/8 w - - 0 1",
		"K6k/8/8/8/Pp2P3/2p5/8/8 w - - 0 1",
		"K6k/8/8/8/PpP5/4p3/8/8 w - - 0 1",
	});
	ASSERT_MOVES_EQUAL("K6k/8/8/8/PpPpP3/8/8/8 b - a3,c3,e3 0 1", "PP", {
		"K6k/8/8/8/P7/2p1p3/8/8 w - -",
		"K6k/8/8/8/P3P3/2pp4/8/8 w - -",
		"K6k/8/8/8/P1P5/1p2p3/8/8 w - -",
		"K6k/8/8/8/P1P1P3/1p1p4/8/8 w - -",
		"K6k/8/8/8/P3P3/1pp5/8/8 w - -",
		"K6k/8/8/8/2P5/p3p3/8/8 w - -",
		"K6k/8/8/8/2P1P3/p2p4/8/8 w - -",
		"K6k/8/8/8/4P3/p1p5/8/8 w - -",
		"K6k/8/8/8/PpP5/8/4p3/8 w - -",
		"K6k/8/8/8/PpP1P3/8/3p4/8 w - -",
		"K6k/8/8/8/P2pP3/8/2p5/8 w - -",
		"K6k/8/8/8/Pp2P3/8/2p5/8 w - -",
		"K6k/8/8/8/P1PpP3/8/1p6/8 w - -",
		"K6k/8/8/8/2PpP3/8/p7/8 w - -",
	});
	ASSERT_MOVES_EQUAL("k7/8/5p2/8/4P3/8/8/K7 b - e3 0 1", "PPP", {"k7/8/8/8/4P3/5p2/8/K7 w - -", "k7/8/8/8/8/4p3/8/K7 w - -"}, true);
	ASSERT_MOVES_EQUAL("k7/8/5p2/8/4P3/8/8/K7 b - - 0 1", "PPP", {"k7/8/8/8/4P3/5p2/8/K7 w - -", "k7/8/8/8/8/4p3/8/K7 w - -"}, true);
	ASSERT_MOVES_EQUAL("k7/8/4p3/8/4PP2/8/8/K7 b - e3 0 1", "PPP", {"k7/8/8/8/4P3/5p2/8/K7 w - -", "k7/8/8/8/8/4p3/8/K7 w - -"}, true);
	ASSERT_MOVES_EQUAL("k7/8/5pP1/8/6P1/8/8/K7 b - g3 0 1", "PNN", {"k7/8/6P1/5p2/6P1/8/8/K7 w - - 0 1"}, true);
	ASSERT_MOVES_EQUAL("7k/8/8/3Pp3/8/8/8/7K w - e6 0 1", "PBB", {"7k/8/3P4/4p3/8/8/8/7K b - - 0 1", "7k/8/4P3/8/8/8/8/7K b - - 0 1"}, true);
	ASSERT_MOVES_EQUAL("7k/8/4n3/3Pp3/8/8/8/7K w - - 0 1", "PQQ", {"7k/8/3Pn3/4p3/8/8/8/7K b - - 0 1", "7k/8/4P3/4p3/8/8/8/7K b - - 0 1"}, true);
	ASSERT_MOVES_EQUAL("7k/8/4n3/3PpP2/8/8/8/7K w - e6 0 1", "PPP", {
		"7k/4P3/4P3/8/8/8/8/7K b - -",
		"7k/4P3/5P2/4p3/8/8/8/7K b - -",
		"7k/5P2/4P3/4p3/8/8/8/7K b - -",
		"7k/3P4/4P3/4p3/8/8/8/7K b - -",
		"7k/3P4/4nP2/4p3/8/8/8/7K b - -",
		"7k/4P3/3P4/4p3/8/8/8/7K b - -",
		"7k/5P2/3Pn3/4p3/8/8/8/7K b - -",
		"4N2k/8/8/4pP2/8/8/8/7K b - -",
		"4B2k/8/8/4pP2/8/8/8/7K b - -",
		"4R2k/8/8/4pP2/8/8/8/7K b - -",
		"4Q2k/8/8/4pP2/8/8/8/7K b - -",
		"3N3k/8/4n3/4pP2/8/8/8/7K b - -",
		"3B3k/8/4n3/4pP2/8/8/8/7K b - -",
		"3R3k/8/4n3/4pP2/8/8/8/7K b - -",
		"3Q3k/8/4n3/4pP2/8/8/8/7K b - -",
		"4N2k/8/8/3Pp3/8/8/8/7K b - -",
		"4B2k/8/8/3Pp3/8/8/8/7K b - -",
		"4R2k/8/8/3Pp3/8/8/8/7K b - -",
		"4Q2k/8/8/3Pp3/8/8/8/7K b - -",
		"5N1k/8/4n3/3Pp3/8/8/8/7K b - -",
		"5B1k/8/4n3/3Pp3/8/8/8/7K b - -",
		"5R1k/8/4n3/3Pp3/8/8/8/7K b - -",
		"5Q1k/8/4n3/3Pp3/8/8/8/7K b - -",
	}, true);
	ASSERT_MOVES_EQUAL("2NNNN2/2NK1N2/2N1rN2/2NPp3/8/8/8/7k w - e6 0 1", "KKP", {
		"2NNNN2/2N1KN2/2N1PN2/2N5/8/8/8/7k b - -",
		"2NNNN2/2NK1N2/2N1PN2/2N5/8/8/8/7k b - -",
		"2NNNN2/2N2N2/2NKPN2/2N5/8/8/8/7k b - -",
		"2NNNN2/2N2N2/2N1PN2/2N2K2/8/8/8/7k b - -",
		"2NNNN2/2N1KN2/2N1PN2/2N1p3/8/8/8/7k b - -",
		"2NNNN2/2NK1N2/2N1PN2/2N1p3/8/8/8/7k b - -",
		"2NNNN2/2N1KN2/2NP1N2/2N1p3/8/8/8/7k b - -",
		"2NNNN2/2NK1N2/2NP1N2/2N1p3/8/8/8/7k b - -",
		"2NNNN2/2N1KN2/2NPrN2/2N1p3/8/8/8/7k b - -",
		"2NNNN2/2NK1N2/2NPrN2/2N1p3/8/8/8/7k b - -",
		"2NNNN2/2N2N2/2NPKN2/2N1p3/8/8/8/7k b - -",
		"2NNNN2/2N2N2/2NKPN2/2N1p3/8/8/8/7k b - -",
		"2NNNN2/2N2N2/2NP1N2/2N1pK2/8/8/8/7k b - -",
		"2NNNN2/2N2N2/2N1PN2/2N1K3/8/8/8/7k b - -",
		"2NNNN2/2N2N2/2NP1N2/2N1K3/8/8/8/7k b - -",
		"2NNNN2/2N2N2/2NPrN2/2N1K3/8/8/8/7k b - -",
		"2NNNN2/2N2N2/2N1PN2/2NKp3/8/8/8/7k b - -",
		"2NNNN2/2N2N2/2NP1N2/2NKp3/8/8/8/7k b - -",
	}, true);
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQkq - 0 1", "KRB", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQ1RK1 b kq -", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQ1KR1 b kq -"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w Qkq - 0 1", "KRB", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQ1KR1 b kq -"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQq - 0 1", "KRB", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQ1RK1 b q -", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQ1KR1 b q -"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w Qq - 0 1", "KRB", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQ1KR1 b q -"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w Kkq - 0 1", "KRB", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQ1RK1 b kq -", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQ1KR1 b kq -"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w kq - 0 1", "KRB", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQ1KR1 b kq -"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w Kq - 0 1", "KRB", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQ1RK1 b q -", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQ1KR1 b q -"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w q - 0 1", "KRB", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQ1KR1 b q -"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/R3KBNR w KQkq - 0 1", "KRB", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/2RK1BNR b kq -", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/2KR1BNR b kq -", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/1R1K1BNR b kq -"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/R3KBNR w Qkq - 0 1", "KRB", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/2RK1BNR b kq -", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/2KR1BNR b kq -", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/1R1K1BNR b kq -"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/R3KBNR w Kkq - 0 1", "KRB", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/2RK1BNR b kq -", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/1R1K1BNR b kq -"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/R3KBNR w kq - 0 1", "KRB", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/2RK1BNR b kq -", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/1R1K1BNR b kq -"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/R3KBNR w KQ - 0 1", "KRB", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/2RK1BNR b - -", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/2KR1BNR b - -", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/1R1K1BNR b - -"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/R3KBNR w Q - 0 1", "KRB", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/2RK1BNR b - -", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/2KR1BNR b - -", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/1R1K1BNR b - -"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/R3KBNR w K - 0 1", "KRB", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/2RK1BNR b - -", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/1R1K1BNR b - -"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/R3KBNR w - - 0 1", "KRB", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/2RK1BNR b - -", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/1R1K1BNR b - -"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RN2K3 w Qkq - 0 1", "NRK", {
		"rnbqkbnr/pppppppp/8/8/8/2N5/PPPPPPPP/4RK2 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/N7/PPPPPPPP/4RK2 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/2N5/PPPPPPPP/3R1K2 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/N7/PPPPPPPP/3R1K2 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/2N5/PPPPPPPP/2R2K2 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/N7/PPPPPPPP/2R2K2 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/2N5/PPPPPPPP/2RK4 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/N7/PPPPPPPP/2RK4 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/2N5/PPPPPPPP/2KR4 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/N7/PPPPPPPP/2KR4 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/2N5/PPPPPPPP/1R3K2 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/N7/PPPPPPPP/1R3K2 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/2N5/PPPPPPPP/1R1K4 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/N7/PPPPPPPP/1R1K4 b kq -",
	});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RN2K3 w kq - 0 1", "NRK", {
		"rnbqkbnr/pppppppp/8/8/8/2N5/PPPPPPPP/4RK2 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/N7/PPPPPPPP/4RK2 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/2N5/PPPPPPPP/3R1K2 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/N7/PPPPPPPP/3R1K2 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/2N5/PPPPPPPP/2R2K2 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/N7/PPPPPPPP/2R2K2 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/2N5/PPPPPPPP/2RK4 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/N7/PPPPPPPP/2RK4 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/2N5/PPPPPPPP/1R3K2 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/N7/PPPPPPPP/1R3K2 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/2N5/PPPPPPPP/1R1K4 b kq -",
		"rnbqkbnr/pppppppp/8/8/8/N7/PPPPPPPP/1R1K4 b kq -",
	});
	ASSERT_MOVES_EQUAL("7k/8/8/3p4/2Pp4/8/8/7K w - d6 0 1", "PRQ", {"7k/8/8/3P4/3p4/8/8/7K b - -", "7k/8/8/2Pp4/3p4/8/8/7K b - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/8/3p4/8/2Pp4/8/7K w - d6 0 1", "PRQ", {"7k/8/8/3p4/2P5/3p4/8/7K b - -"}, true);
	ASSERT_MOVES_EQUAL("rnbqkb1r/pppppppp/8/8/8/6n1/PPPPPPPP/RNBQKBNR b KQkq - 0 1", "NBB", {"rnbqkb1r/pppppppp/8/8/8/8/PPPPPPPP/RNBQKnNR w KQkq -", "rnbqkb1r/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNn w Qkq -", "rnbqkb1r/pppppppp/8/7n/8/8/PPPPPPPP/RNBQKBNR w KQkq -", "rnbqkb1r/pppppppp/8/5n2/8/8/PPPPPPPP/RNBQKBNR w KQkq -", "rnbqkb1r/pppppppp/8/8/4n3/8/PPPPPPPP/RNBQKBNR w KQkq -", "r1bqkb1r/pppppppp/2n5/8/8/6n1/PPPPPPPP/RNBQKBNR w KQkq -", "r1bqkb1r/pppppppp/n7/8/8/6n1/PPPPPPPP/RNBQKBNR w KQkq -", "rnbqkb1r/pppppppp/8/8/8/8/PPPPnPPP/RNBQKBNR w KQkq -"});
	ASSERT_MOVES_EQUAL("rnbqkb1r/pppppppp/8/8/8/1n6/PPPPPPPP/RNBQKBNR b KQkq - 0 1", "NBB", {"rnbqkb1r/pppppppp/8/8/8/8/PPPPPPPP/nNBQKBNR w Kkq -", "rnbqkb1r/pppppppp/8/8/8/8/PPPPPPPP/RNnQKBNR w KQkq -", "rnbqkb1r/pppppppp/8/2n5/8/8/PPPPPPPP/RNBQKBNR w KQkq -", "rnbqkb1r/pppppppp/8/n7/8/8/PPPPPPPP/RNBQKBNR w KQkq -", "rnbqkb1r/pppppppp/8/8/3n4/8/PPPPPPPP/RNBQKBNR w KQkq -", "r1bqkb1r/pppppppp/2n5/8/8/1n6/PPPPPPPP/RNBQKBNR w KQkq -", "r1bqkb1r/pppppppp/n7/8/8/1n6/PPPPPPPP/RNBQKBNR w KQkq -", "rnbqkb1r/pppppppp/8/8/8/8/PPPnPPPP/RNBQKBNR w KQkq -"});
	ASSERT_MOVES_EQUAL("8/8/8/8/8/8/6k1/4K2R b K - 0 1", "KBB", {"8/8/8/8/8/7k/8/4K2R w K -", "8/8/8/8/8/6k1/8/4K2R w K -", "8/8/8/8/8/5k2/8/4K2R w K -", "8/8/8/8/8/8/7k/4K2R w K -", "8/8/8/8/8/8/5k2/4K2R w K -", "8/8/8/8/8/8/8/4K2k w - -", "8/8/8/8/8/8/8/4K1kR w K -", "8/8/8/8/8/8/8/4Kk1R w K -"});
	ASSERT_MOVES_EQUAL("b3k3/8/8/8/8/8/8/4K2R b K - 0 1", "BNQ", {"4k3/8/8/8/8/8/8/4K2b w - -", "4k3/1b6/8/8/8/8/8/4K2R w K -", "4k3/8/2b5/8/8/8/8/4K2R w K -", "4k3/8/8/3b4/8/8/8/4K2R w K -", "4k3/8/8/8/4b3/8/8/4K2R w K -", "4k3/8/8/8/8/5b2/8/4K2R w K -", "4k3/8/8/8/8/8/6b1/4K2R w K -"});
	ASSERT_MOVES_EQUAL("qk6/8/8/8/8/8/8/4K2R b K - 0 1", "QRR", {"1k6/1q6/8/8/8/8/8/4K2R w K -", "1k6/q7/8/8/8/8/8/4K2R w K -", "1k6/8/2q5/8/8/8/8/4K2R w K -", "1k6/8/q7/8/8/8/8/4K2R w K -", "1k6/8/8/3q4/8/8/8/4K2R w K -", "1k6/8/8/q7/8/8/8/4K2R w K -", "1k6/8/8/8/4q3/8/8/4K2R w K -", "1k6/8/8/8/q7/8/8/4K2R w K -", "1k6/8/8/8/8/5q2/8/4K2R w K -", "1k6/8/8/8/8/q7/8/4K2R w K -", "1k6/8/8/8/8/8/6q1/4K2R w K -", "1k6/8/8/8/8/8/q7/4K2R w K -", "1k6/8/8/8/8/8/8/4K2q w - -", "1k6/8/8/8/8/8/8/q3K2R w K -"});
	ASSERT_MOVES_EQUAL("4k2r/8/8/8/8/8/8/4K2R b Kk - 0 1", "RQQ", {"4k1r1/8/8/8/8/8/8/4K2R w K -", "4kr2/8/8/8/8/8/8/4K2R w K -", "4k3/7r/8/8/8/8/8/4K2R w K -", "4k3/8/7r/8/8/8/8/4K2R w K -", "4k3/8/8/7r/8/8/8/4K2R w K -", "4k3/8/8/8/7r/8/8/4K2R w K -", "4k3/8/8/8/8/7r/8/4K2R w K -", "4k3/8/8/8/8/8/7r/4K2R w K -", "4k3/8/8/8/8/8/8/4K2r w - -"});
	ASSERT_MOVES_EQUAL("4k3/8/8/8/8/8/6p1/4K2R b K - 0 1", "PPP", {"4k3/8/8/8/8/8/8/4K2n w - -", "4k3/8/8/8/8/8/8/4K2b w - -", "4k3/8/8/8/8/8/8/4K2r w - -", "4k3/8/8/8/8/8/8/4K2q w - -", "4k3/8/8/8/8/8/8/4K1nR w K -", "4k3/8/8/8/8/8/8/4K1bR w K -", "4k3/8/8/8/8/8/8/4K1rR w K -", "4k3/8/8/8/8/8/8/4K1qR w K -"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/5P1P/2P1P3/3B2N1/1PPP1PP1/3QK1RR w Kkq - 0 1", "BNK", {"rnbqkbnr/pppppppp/8/5P1P/2P1P3/6N1/1PPPKPP1/3Q1BRR b kq -", "rnbqkbnr/pppppppp/8/5P1P/2P1P3/6N1/1PPPBPP1/3Q1KRR b kq -", "rnbqkbnr/pppppppp/8/5P1P/2P1P3/3B4/1PPPKPP1/3Q1NRR b kq -", "rnbqkbnr/pppppppp/8/5P1P/2P1P3/3B4/1PPPNPP1/3Q1KRR b kq -", "rnbqkbnr/pppppppp/8/5P1P/2P1P3/8/1PPPBPP1/3QKNRR b Kkq -", "rnbqkbnr/pppppppp/8/5P1P/2P1P3/8/1PPPNPP1/3QKBRR b Kkq -"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/5P1P/2P1P3/3B2N1/1PPP1PP1/3QK1RR w kq - 0 1", "BNK", {"rnbqkbnr/pppppppp/8/5P1P/2P1P3/6N1/1PPPKPP1/3Q1BRR b kq -", "rnbqkbnr/pppppppp/8/5P1P/2P1P3/6N1/1PPPBPP1/3Q1KRR b kq -", "rnbqkbnr/pppppppp/8/5P1P/2P1P3/3B4/1PPPKPP1/3Q1NRR b kq -", "rnbqkbnr/pppppppp/8/5P1P/2P1P3/3B4/1PPPNPP1/3Q1KRR b kq -", "rnbqkbnr/pppppppp/8/5P1P/2P1P3/8/1PPPBPP1/3QKNRR b kq -", "rnbqkbnr/pppppppp/8/5P1P/2P1P3/8/1PPPNPP1/3QKBRR b kq -"});
	ASSERT_MOVES_EQUAL("4k3/8/8/8/8/7P/8/4K2R w K - 0 1", "RKB", {"4k3/8/8/8/8/7P/5K1R/8 b - -", "4k3/8/8/8/8/7P/4K2R/8 b - -", "4k3/8/8/8/8/7P/3K3R/8 b - -", "4k3/8/8/8/8/7P/5K2/6R1 b - -", "4k3/8/8/8/8/7P/4K3/6R1 b - -", "4k3/8/8/8/8/7P/3K4/6R1 b - -", "4k3/8/8/8/8/7P/5K2/5R2 b - -", "4k3/8/8/8/8/7P/4K3/5R2 b - -", "4k3/8/8/8/8/7P/3K4/5R2 b - -", "4k3/8/8/8/8/7P/8/5RK1 b - -", "4k3/8/8/8/8/7P/7R/5K2 b - -", "4k3/8/8/8/8/7P/8/5KR1 b - -", "4k3/8/8/8/8/7P/5K2/4R3 b - -", "4k3/8/8/8/8/7P/4K3/4R3 b - -", "4k3/8/8/8/8/7P/3K4/4R3 b - -", "4k3/8/8/8/8/7P/5K2/3R4 b - -", "4k3/8/8/8/8/7P/4K3/3R4 b - -", "4k3/8/8/8/8/7P/3K4/3R4 b - -", "4k3/8/8/8/8/7P/7R/3K4 b - -", "4k3/8/8/8/8/7P/8/3K2R1 b - -", "4k3/8/8/8/8/7P/8/3K1R2 b - -", "4k3/8/8/8/8/7P/8/3KR3 b - -", "4k3/8/8/8/8/7P/5K2/2R5 b - -", "4k3/8/8/8/8/7P/4K3/2R5 b - -", "4k3/8/8/8/8/7P/3K4/2R5 b - -", "4k3/8/8/8/8/7P/5K2/1R6 b - -", "4k3/8/8/8/8/7P/4K3/1R6 b - -", "4k3/8/8/8/8/7P/3K4/1R6 b - -", "4k3/8/8/8/8/7P/5K2/R7 b - -", "4k3/8/8/8/8/7P/4K3/R7 b - -", "4k3/8/8/8/8/7P/3K4/R7 b - -"});
	ASSERT_MOVES_EQUAL("4k3/8/8/8/8/7P/8/4K2R w - - 0 1", "RKB", {"4k3/8/8/8/8/7P/5K1R/8 b - -", "4k3/8/8/8/8/7P/4K2R/8 b - -", "4k3/8/8/8/8/7P/3K3R/8 b - -", "4k3/8/8/8/8/7P/5K2/6R1 b - -", "4k3/8/8/8/8/7P/4K3/6R1 b - -", "4k3/8/8/8/8/7P/3K4/6R1 b - -", "4k3/8/8/8/8/7P/5K2/5R2 b - -", "4k3/8/8/8/8/7P/4K3/5R2 b - -", "4k3/8/8/8/8/7P/3K4/5R2 b - -", "4k3/8/8/8/8/7P/7R/5K2 b - -", "4k3/8/8/8/8/7P/8/5KR1 b - -", "4k3/8/8/8/8/7P/5K2/4R3 b - -", "4k3/8/8/8/8/7P/4K3/4R3 b - -", "4k3/8/8/8/8/7P/3K4/4R3 b - -", "4k3/8/8/8/8/7P/5K2/3R4 b - -", "4k3/8/8/8/8/7P/4K3/3R4 b - -", "4k3/8/8/8/8/7P/3K4/3R4 b - -", "4k3/8/8/8/8/7P/7R/3K4 b - -", "4k3/8/8/8/8/7P/8/3K2R1 b - -", "4k3/8/8/8/8/7P/8/3K1R2 b - -", "4k3/8/8/8/8/7P/8/3KR3 b - -", "4k3/8/8/8/8/7P/5K2/2R5 b - -", "4k3/8/8/8/8/7P/4K3/2R5 b - -", "4k3/8/8/8/8/7P/3K4/2R5 b - -", "4k3/8/8/8/8/7P/5K2/1R6 b - -", "4k3/8/8/8/8/7P/4K3/1R6 b - -", "4k3/8/8/8/8/7P/3K4/1R6 b - -", "4k3/8/8/8/8/7P/5K2/R7 b - -", "4k3/8/8/8/8/7P/4K3/R7 b - -", "4k3/8/8/8/8/7P/3K4/R7 b - -"});
	ASSERT_MOVES_EQUAL("4k3/8/8/8/8/7P/8/4Kb1R w K - 0 1", "RKB", {"4k3/8/8/8/8/7P/5K1R/5b2 b - -", "4k3/8/8/8/8/7P/4K2R/5b2 b - -", "4k3/8/8/8/8/7P/3K3R/5b2 b - -", "4k3/8/8/8/8/7P/5K2/5bR1 b - -", "4k3/8/8/8/8/7P/4K3/5bR1 b - -", "4k3/8/8/8/8/7P/3K4/5bR1 b - -", "4k3/8/8/8/8/7P/5K2/5R2 b - -", "4k3/8/8/8/8/7P/4K3/5R2 b - -", "4k3/8/8/8/8/7P/3K4/5R2 b - -", "4k3/8/8/8/8/7P/7R/5K2 b - -", "4k3/8/8/8/8/7P/8/5KR1 b - -", "4k3/8/8/8/8/7P/7R/3K1b2 b - -", "4k3/8/8/8/8/7P/8/3K1bR1 b - -", "4k3/8/8/8/8/7P/8/3K1R2 b - -"});
	ASSERT_MOVES_EQUAL("4k3/8/8/8/8/7P/8/4Kb1R w - - 0 1", "RKB", {"4k3/8/8/8/8/7P/5K1R/5b2 b - -", "4k3/8/8/8/8/7P/4K2R/5b2 b - -", "4k3/8/8/8/8/7P/3K3R/5b2 b - -", "4k3/8/8/8/8/7P/5K2/5bR1 b - -", "4k3/8/8/8/8/7P/4K3/5bR1 b - -", "4k3/8/8/8/8/7P/3K4/5bR1 b - -", "4k3/8/8/8/8/7P/5K2/5R2 b - -", "4k3/8/8/8/8/7P/4K3/5R2 b - -", "4k3/8/8/8/8/7P/3K4/5R2 b - -", "4k3/8/8/8/8/7P/7R/5K2 b - -", "4k3/8/8/8/8/7P/8/5KR1 b - -", "4k3/8/8/8/8/7P/7R/3K1b2 b - -", "4k3/8/8/8/8/7P/8/3K1bR1 b - -", "4k3/8/8/8/8/7P/8/3K1R2 b - -"});
	ASSERT_MOVES_EQUAL("4k3/8/8/8/8/7P/8/4K1bR w K - 0 1", "RKB", {"4k3/8/8/8/8/7P/5K1R/6b1 b - -", "4k3/8/8/8/8/7P/4K2R/6b1 b - -", "4k3/8/8/8/8/7P/3K3R/6b1 b - -", "4k3/8/8/8/8/7P/5K2/6R1 b - -", "4k3/8/8/8/8/7P/4K3/6R1 b - -", "4k3/8/8/8/8/7P/3K4/6R1 b - -", "4k3/8/8/8/8/7P/7R/5Kb1 b - -", "4k3/8/8/8/8/7P/8/5KR1 b - -", "4k3/8/8/8/8/7P/7R/3K2b1 b - -", "4k3/8/8/8/8/7P/8/3K2R1 b - -"});
	ASSERT_MOVES_EQUAL("4k3/8/8/8/8/7P/8/4K1bR w - - 0 1", "RKB", {"4k3/8/8/8/8/7P/5K1R/6b1 b - -", "4k3/8/8/8/8/7P/4K2R/6b1 b - -", "4k3/8/8/8/8/7P/3K3R/6b1 b - -", "4k3/8/8/8/8/7P/5K2/6R1 b - -", "4k3/8/8/8/8/7P/4K3/6R1 b - -", "4k3/8/8/8/8/7P/3K4/6R1 b - -", "4k3/8/8/8/8/7P/7R/5Kb1 b - -", "4k3/8/8/8/8/7P/8/5KR1 b - -", "4k3/8/8/8/8/7P/7R/3K2b1 b - -", "4k3/8/8/8/8/7P/8/3K2R1 b - -"});
	ASSERT_MOVES_EQUAL("4k3/8/8/8/8/P7/8/Rb2K3 w Q - 0 1", "RKB", {"4k3/8/8/8/8/P7/R4K2/1b6 b - -", "4k3/8/8/8/8/P7/R3K3/1b6 b - -", "4k3/8/8/8/8/P7/R2K4/1b6 b - -", "4k3/8/8/8/8/P7/R7/1b3K2 b - -", "4k3/8/8/8/8/P7/R7/1b1K4 b - -", "4k3/8/8/8/8/P7/5K2/1R6 b - -", "4k3/8/8/8/8/P7/4K3/1R6 b - -", "4k3/8/8/8/8/P7/3K4/1R6 b - -", "4k3/8/8/8/8/P7/8/1R3K2 b - -", "4k3/8/8/8/8/P7/8/1R1K4 b - -"});
	ASSERT_MOVES_EQUAL("4k3/8/8/8/8/P7/8/Rb2K3 w - - 0 1", "RKB", {"4k3/8/8/8/8/P7/R4K2/1b6 b - -", "4k3/8/8/8/8/P7/R3K3/1b6 b - -", "4k3/8/8/8/8/P7/R2K4/1b6 b - -", "4k3/8/8/8/8/P7/R7/1b3K2 b - -", "4k3/8/8/8/8/P7/R7/1b1K4 b - -", "4k3/8/8/8/8/P7/5K2/1R6 b - -", "4k3/8/8/8/8/P7/4K3/1R6 b - -", "4k3/8/8/8/8/P7/3K4/1R6 b - -", "4k3/8/8/8/8/P7/8/1R3K2 b - -", "4k3/8/8/8/8/P7/8/1R1K4 b - -"});
	ASSERT_MOVES_EQUAL("4k3/8/8/8/8/P7/8/R1b1K3 w Q - 0 1", "RKB", {"4k3/8/8/8/8/P7/R4K2/2b5 b - -", "4k3/8/8/8/8/P7/R3K3/2b5 b - -", "4k3/8/8/8/8/P7/R2K4/2b5 b - -", "4k3/8/8/8/8/P7/R7/2b2K2 b - -", "4k3/8/8/8/8/P7/R7/2bK4 b - -", "4k3/8/8/8/8/P7/5K2/2R5 b - -", "4k3/8/8/8/8/P7/4K3/2R5 b - -", "4k3/8/8/8/8/P7/3K4/2R5 b - -", "4k3/8/8/8/8/P7/8/2R2K2 b - -", "4k3/8/8/8/8/P7/8/2RK4 b - -", "4k3/8/8/8/8/P7/5K2/1Rb5 b - -", "4k3/8/8/8/8/P7/4K3/1Rb5 b - -", "4k3/8/8/8/8/P7/3K4/1Rb5 b - -", "4k3/8/8/8/8/P7/8/1Rb2K2 b - -", "4k3/8/8/8/8/P7/8/1RbK4 b - -"});
	ASSERT_MOVES_EQUAL("4k3/8/8/8/8/P7/8/R1b1K3 w - - 0 1", "RKB", {"4k3/8/8/8/8/P7/R4K2/2b5 b - -", "4k3/8/8/8/8/P7/R3K3/2b5 b - -", "4k3/8/8/8/8/P7/R2K4/2b5 b - -", "4k3/8/8/8/8/P7/R7/2b2K2 b - -", "4k3/8/8/8/8/P7/R7/2bK4 b - -", "4k3/8/8/8/8/P7/5K2/2R5 b - -", "4k3/8/8/8/8/P7/4K3/2R5 b - -", "4k3/8/8/8/8/P7/3K4/2R5 b - -", "4k3/8/8/8/8/P7/8/2R2K2 b - -", "4k3/8/8/8/8/P7/8/2RK4 b - -", "4k3/8/8/8/8/P7/5K2/1Rb5 b - -", "4k3/8/8/8/8/P7/4K3/1Rb5 b - -", "4k3/8/8/8/8/P7/3K4/1Rb5 b - -", "4k3/8/8/8/8/P7/8/1Rb2K2 b - -", "4k3/8/8/8/8/P7/8/1RbK4 b - -"});
	ASSERT_MOVES_EQUAL("4k3/8/8/8/8/P7/8/R2nK3 w Q - 0 1", "RKB", {"4k3/8/8/8/8/P7/R4K2/3n4 b - -", "4k3/8/8/8/8/P7/R3K3/3n4 b - -", "4k3/8/8/8/8/P7/R2K4/3n4 b - -", "4k3/8/8/8/8/P7/R7/3n1K2 b - -", "4k3/8/8/8/8/P7/5K2/3R4 b - -", "4k3/8/8/8/8/P7/4K3/3R4 b - -", "4k3/8/8/8/8/P7/3K4/3R4 b - -", "4k3/8/8/8/8/P7/8/3R1K2 b - -", "4k3/8/8/8/8/P7/R7/3K4 b - -", "4k3/8/8/8/8/P7/5K2/2Rn4 b - -", "4k3/8/8/8/8/P7/4K3/2Rn4 b - -", "4k3/8/8/8/8/P7/3K4/2Rn4 b - -", "4k3/8/8/8/8/P7/8/2Rn1K2 b - -", "4k3/8/8/8/8/P7/8/2RK4 b - -", "4k3/8/8/8/8/P7/5K2/1R1n4 b - -", "4k3/8/8/8/8/P7/4K3/1R1n4 b - -", "4k3/8/8/8/8/P7/3K4/1R1n4 b - -", "4k3/8/8/8/8/P7/8/1R1n1K2 b - -", "4k3/8/8/8/8/P7/8/1R1K4 b - -"});
	ASSERT_MOVES_EQUAL("4k3/8/8/8/8/P7/8/R2nK3 w - - 0 1", "RKB", {"4k3/8/8/8/8/P7/R4K2/3n4 b - -", "4k3/8/8/8/8/P7/R3K3/3n4 b - -", "4k3/8/8/8/8/P7/R2K4/3n4 b - -", "4k3/8/8/8/8/P7/R7/3n1K2 b - -", "4k3/8/8/8/8/P7/5K2/3R4 b - -", "4k3/8/8/8/8/P7/4K3/3R4 b - -", "4k3/8/8/8/8/P7/3K4/3R4 b - -", "4k3/8/8/8/8/P7/8/3R1K2 b - -", "4k3/8/8/8/8/P7/R7/3K4 b - -", "4k3/8/8/8/8/P7/5K2/2Rn4 b - -", "4k3/8/8/8/8/P7/4K3/2Rn4 b - -", "4k3/8/8/8/8/P7/3K4/2Rn4 b - -", "4k3/8/8/8/8/P7/8/2Rn1K2 b - -", "4k3/8/8/8/8/P7/8/2RK4 b - -", "4k3/8/8/8/8/P7/5K2/1R1n4 b - -", "4k3/8/8/8/8/P7/4K3/1R1n4 b - -", "4k3/8/8/8/8/P7/3K4/1R1n4 b - -", "4k3/8/8/8/8/P7/8/1R1n1K2 b - -", "4k3/8/8/8/8/P7/8/1R1K4 b - -"});
	ASSERT_MOVES_EQUAL("8/8/8/8/8/8/3PPP2/R3K2k w Q - 0 1", "RRK", {}, true);
	ASSERT_MOVES_EQUAL("8/8/8/8/8/8/3PPP2/R3K2k w - - 0 1", "RRK", {
		"7R/8/8/8/8/8/3PPP2/5K1k b - -",
		"6R1/8/8/8/8/8/3PPP2/5K1k b - -",
		"5R2/8/8/8/8/8/3PPP2/5K1k b - -",
		"4R3/8/8/8/8/8/3PPP2/5K1k b - -",
		"3R4/8/8/8/8/8/3PPP2/5K1k b - -",
		"2R5/8/8/8/8/8/3PPP2/5K1k b - -",
		"1R6/8/8/8/8/8/3PPP2/5K1k b - -",
		"R7/8/8/8/8/8/3PPP2/5K1k b - -",
		"8/7R/8/8/8/8/3PPP2/5K1k b - -",
		"8/6R1/8/8/8/8/3PPP2/5K1k b - -",
		"8/5R2/8/8/8/8/3PPP2/5K1k b - -",
		"8/4R3/8/8/8/8/3PPP2/5K1k b - -",
		"8/3R4/8/8/8/8/3PPP2/5K1k b - -",
		"8/2R5/8/8/8/8/3PPP2/5K1k b - -",
		"8/1R6/8/8/8/8/3PPP2/5K1k b - -",
		"8/R7/8/8/8/8/3PPP2/5K1k b - -",
		"8/8/7R/8/8/8/3PPP2/5K1k b - -",
		"8/8/6R1/8/8/8/3PPP2/5K1k b - -",
		"8/8/5R2/8/8/8/3PPP2/5K1k b - -",
		"8/8/4R3/8/8/8/3PPP2/5K1k b - -",
		"8/8/3R4/8/8/8/3PPP2/5K1k b - -",
		"8/8/2R5/8/8/8/3PPP2/5K1k b - -",
		"8/8/1R6/8/8/8/3PPP2/5K1k b - -",
		"8/8/R7/8/8/8/3PPP2/5K1k b - -",
		"8/8/8/7R/8/8/3PPP2/5K1k b - -",
		"8/8/8/6R1/8/8/3PPP2/5K1k b - -",
		"8/8/8/5R2/8/8/3PPP2/5K1k b - -",
		"8/8/8/4R3/8/8/3PPP2/5K1k b - -",
		"8/8/8/3R4/8/8/3PPP2/5K1k b - -",
		"8/8/8/2R5/8/8/3PPP2/5K1k b - -",
		"8/8/8/1R6/8/8/3PPP2/5K1k b - -",
		"8/8/8/R7/8/8/3PPP2/5K1k b - -",
		"8/8/8/8/7R/8/3PPP2/5K1k b - -",
		"8/8/8/8/6R1/8/3PPP2/5K1k b - -",
		"8/8/8/8/5R2/8/3PPP2/5K1k b - -",
		"8/8/8/8/4R3/8/3PPP2/5K1k b - -",
		"8/8/8/8/3R4/8/3PPP2/5K1k b - -",
		"8/8/8/8/2R5/8/3PPP2/5K1k b - -",
		"8/8/8/8/1R6/8/3PPP2/5K1k b - -",
		"8/8/8/8/R7/8/3PPP2/5K1k b - -",
		"8/8/8/8/8/7R/3PPP2/5K1k b - -",
		"8/8/8/8/8/6R1/3PPP2/5K1k b - -",
		"8/8/8/8/8/5R2/3PPP2/5K1k b - -",
		"8/8/8/8/8/4R3/3PPP2/5K1k b - -",
		"8/8/8/8/8/3R4/3PPP2/5K1k b - -",
		"8/8/8/8/8/2R5/3PPP2/5K1k b - -",
		"8/8/8/8/8/1R6/3PPP2/5K1k b - -",
		"8/8/8/8/8/R7/3PPP2/5K1k b - -",
		"8/8/8/8/8/8/2RPPP2/5K1k b - -",
		"8/8/8/8/8/8/1R1PPP2/5K1k b - -",
		"8/8/8/8/8/8/R2PPP2/5K1k b - -",
		"8/8/8/8/8/8/3PPP2/4RK1k b - -",
		"8/8/8/8/8/8/3PPP2/3R1K1k b - -",
		"7R/8/8/8/8/8/3PPP2/3K3k b - -",
		"6R1/8/8/8/8/8/3PPP2/3K3k b - -",
		"5R2/8/8/8/8/8/3PPP2/3K3k b - -",
		"4R3/8/8/8/8/8/3PPP2/3K3k b - -",
		"3R4/8/8/8/8/8/3PPP2/3K3k b - -",
		"2R5/8/8/8/8/8/3PPP2/3K3k b - -",
		"1R6/8/8/8/8/8/3PPP2/3K3k b - -",
		"R7/8/8/8/8/8/3PPP2/3K3k b - -",
		"8/7R/8/8/8/8/3PPP2/3K3k b - -",
		"8/6R1/8/8/8/8/3PPP2/3K3k b - -",
		"8/5R2/8/8/8/8/3PPP2/3K3k b - -",
		"8/4R3/8/8/8/8/3PPP2/3K3k b - -",
		"8/3R4/8/8/8/8/3PPP2/3K3k b - -",
		"8/2R5/8/8/8/8/3PPP2/3K3k b - -",
		"8/1R6/8/8/8/8/3PPP2/3K3k b - -",
		"8/R7/8/8/8/8/3PPP2/3K3k b - -",
		"8/8/7R/8/8/8/3PPP2/3K3k b - -",
		"8/8/6R1/8/8/8/3PPP2/3K3k b - -",
		"8/8/5R2/8/8/8/3PPP2/3K3k b - -",
		"8/8/4R3/8/8/8/3PPP2/3K3k b - -",
		"8/8/3R4/8/8/8/3PPP2/3K3k b - -",
		"8/8/2R5/8/8/8/3PPP2/3K3k b - -",
		"8/8/1R6/8/8/8/3PPP2/3K3k b - -",
		"8/8/R7/8/8/8/3PPP2/3K3k b - -",
		"8/8/8/7R/8/8/3PPP2/3K3k b - -",
		"8/8/8/6R1/8/8/3PPP2/3K3k b - -",
		"8/8/8/5R2/8/8/3PPP2/3K3k b - -",
		"8/8/8/4R3/8/8/3PPP2/3K3k b - -",
		"8/8/8/3R4/8/8/3PPP2/3K3k b - -",
		"8/8/8/2R5/8/8/3PPP2/3K3k b - -",
		"8/8/8/1R6/8/8/3PPP2/3K3k b - -",
		"8/8/8/R7/8/8/3PPP2/3K3k b - -",
		"8/8/8/8/7R/8/3PPP2/3K3k b - -",
		"8/8/8/8/6R1/8/3PPP2/3K3k b - -",
		"8/8/8/8/5R2/8/3PPP2/3K3k b - -",
		"8/8/8/8/4R3/8/3PPP2/3K3k b - -",
		"8/8/8/8/3R4/8/3PPP2/3K3k b - -",
		"8/8/8/8/2R5/8/3PPP2/3K3k b - -",
		"8/8/8/8/1R6/8/3PPP2/3K3k b - -",
		"8/8/8/8/R7/8/3PPP2/3K3k b - -",
		"8/8/8/8/8/7R/3PPP2/3K3k b - -",
		"8/8/8/8/8/6R1/3PPP2/3K3k b - -",
		"8/8/8/8/8/5R2/3PPP2/3K3k b - -",
		"8/8/8/8/8/4R3/3PPP2/3K3k b - -",
		"8/8/8/8/8/3R4/3PPP2/3K3k b - -",
		"8/8/8/8/8/2R5/3PPP2/3K3k b - -",
		"8/8/8/8/8/1R6/3PPP2/3K3k b - -",
		"8/8/8/8/8/R7/3PPP2/3K3k b - -",
		"8/8/8/8/8/8/2RPPP2/3K3k b - -",
		"8/8/8/8/8/8/1R1PPP2/3K3k b - -",
		"8/8/8/8/8/8/R2PPP2/3K3k b - -",
		"8/8/8/8/8/8/3PPP2/2R2K1k b - -",
		"8/8/8/8/8/8/3PPP2/2RK3k b - -",
		"8/8/8/8/8/8/3PPP2/1R3K1k b - -",
		"8/8/8/8/8/8/3PPP2/1R1K3k b - -",
		"8/8/8/8/8/8/3PPP2/R4K1k b - -",
		"8/8/8/8/8/8/3PPP2/R2K3k b - -",
	});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/R1BQKB1R w KQkq - 0 1", "RRB", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/R1BQKB1R b Qkq - 0 1", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/R1BQKB1R b Kkq - 0 1", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/1RBQKBR1 b kq - 0 1"});
	ASSERT_MOVES_EQUAL("8/8/8/8/8/8/3PPP2/1k2K2R w K - 0 1", "RRK", {});
	ASSERT_MOVES_EQUAL("8/8/8/8/8/8/3PPP2/1k2K2R w - - 0 1", "RRK", {
		"7R/8/8/8/8/8/3PPP2/1k3K2 b - -",
		"6R1/8/8/8/8/8/3PPP2/1k3K2 b - -",
		"5R2/8/8/8/8/8/3PPP2/1k3K2 b - -",
		"4R3/8/8/8/8/8/3PPP2/1k3K2 b - -",
		"3R4/8/8/8/8/8/3PPP2/1k3K2 b - -",
		"2R5/8/8/8/8/8/3PPP2/1k3K2 b - -",
		"1R6/8/8/8/8/8/3PPP2/1k3K2 b - -",
		"R7/8/8/8/8/8/3PPP2/1k3K2 b - -",
		"8/7R/8/8/8/8/3PPP2/1k3K2 b - -",
		"8/6R1/8/8/8/8/3PPP2/1k3K2 b - -",
		"8/5R2/8/8/8/8/3PPP2/1k3K2 b - -",
		"8/4R3/8/8/8/8/3PPP2/1k3K2 b - -",
		"8/3R4/8/8/8/8/3PPP2/1k3K2 b - -",
		"8/2R5/8/8/8/8/3PPP2/1k3K2 b - -",
		"8/1R6/8/8/8/8/3PPP2/1k3K2 b - -",
		"8/R7/8/8/8/8/3PPP2/1k3K2 b - -",
		"8/8/7R/8/8/8/3PPP2/1k3K2 b - -",
		"8/8/6R1/8/8/8/3PPP2/1k3K2 b - -",
		"8/8/5R2/8/8/8/3PPP2/1k3K2 b - -",
		"8/8/4R3/8/8/8/3PPP2/1k3K2 b - -",
		"8/8/3R4/8/8/8/3PPP2/1k3K2 b - -",
		"8/8/2R5/8/8/8/3PPP2/1k3K2 b - -",
		"8/8/1R6/8/8/8/3PPP2/1k3K2 b - -",
		"8/8/R7/8/8/8/3PPP2/1k3K2 b - -",
		"8/8/8/7R/8/8/3PPP2/1k3K2 b - -",
		"8/8/8/6R1/8/8/3PPP2/1k3K2 b - -",
		"8/8/8/5R2/8/8/3PPP2/1k3K2 b - -",
		"8/8/8/4R3/8/8/3PPP2/1k3K2 b - -",
		"8/8/8/3R4/8/8/3PPP2/1k3K2 b - -",
		"8/8/8/2R5/8/8/3PPP2/1k3K2 b - -",
		"8/8/8/1R6/8/8/3PPP2/1k3K2 b - -",
		"8/8/8/R7/8/8/3PPP2/1k3K2 b - -",
		"8/8/8/8/7R/8/3PPP2/1k3K2 b - -",
		"8/8/8/8/6R1/8/3PPP2/1k3K2 b - -",
		"8/8/8/8/5R2/8/3PPP2/1k3K2 b - -",
		"8/8/8/8/4R3/8/3PPP2/1k3K2 b - -",
		"8/8/8/8/3R4/8/3PPP2/1k3K2 b - -",
		"8/8/8/8/2R5/8/3PPP2/1k3K2 b - -",
		"8/8/8/8/1R6/8/3PPP2/1k3K2 b - -",
		"8/8/8/8/R7/8/3PPP2/1k3K2 b - -",
		"8/8/8/8/8/7R/3PPP2/1k3K2 b - -",
		"8/8/8/8/8/6R1/3PPP2/1k3K2 b - -",
		"8/8/8/8/8/5R2/3PPP2/1k3K2 b - -",
		"8/8/8/8/8/4R3/3PPP2/1k3K2 b - -",
		"8/8/8/8/8/3R4/3PPP2/1k3K2 b - -",
		"8/8/8/8/8/2R5/3PPP2/1k3K2 b - -",
		"8/8/8/8/8/1R6/3PPP2/1k3K2 b - -",
		"8/8/8/8/8/R7/3PPP2/1k3K2 b - -",
		"8/8/8/8/8/8/3PPP1R/1k3K2 b - -",
		"8/8/8/8/8/8/3PPPR1/1k3K2 b - -",
		"8/8/8/8/8/8/3PPP2/1k3K1R b - -",
		"8/8/8/8/8/8/3PPP2/1k3KR1 b - -",
		"7R/8/8/8/8/8/3PPP2/1k1K4 b - -",
		"6R1/8/8/8/8/8/3PPP2/1k1K4 b - -",
		"5R2/8/8/8/8/8/3PPP2/1k1K4 b - -",
		"4R3/8/8/8/8/8/3PPP2/1k1K4 b - -",
		"3R4/8/8/8/8/8/3PPP2/1k1K4 b - -",
		"2R5/8/8/8/8/8/3PPP2/1k1K4 b - -",
		"1R6/8/8/8/8/8/3PPP2/1k1K4 b - -",
		"R7/8/8/8/8/8/3PPP2/1k1K4 b - -",
		"8/7R/8/8/8/8/3PPP2/1k1K4 b - -",
		"8/6R1/8/8/8/8/3PPP2/1k1K4 b - -",
		"8/5R2/8/8/8/8/3PPP2/1k1K4 b - -",
		"8/4R3/8/8/8/8/3PPP2/1k1K4 b - -",
		"8/3R4/8/8/8/8/3PPP2/1k1K4 b - -",
		"8/2R5/8/8/8/8/3PPP2/1k1K4 b - -",
		"8/1R6/8/8/8/8/3PPP2/1k1K4 b - -",
		"8/R7/8/8/8/8/3PPP2/1k1K4 b - -",
		"8/8/7R/8/8/8/3PPP2/1k1K4 b - -",
		"8/8/6R1/8/8/8/3PPP2/1k1K4 b - -",
		"8/8/5R2/8/8/8/3PPP2/1k1K4 b - -",
		"8/8/4R3/8/8/8/3PPP2/1k1K4 b - -",
		"8/8/3R4/8/8/8/3PPP2/1k1K4 b - -",
		"8/8/2R5/8/8/8/3PPP2/1k1K4 b - -",
		"8/8/1R6/8/8/8/3PPP2/1k1K4 b - -",
		"8/8/R7/8/8/8/3PPP2/1k1K4 b - -",
		"8/8/8/7R/8/8/3PPP2/1k1K4 b - -",
		"8/8/8/6R1/8/8/3PPP2/1k1K4 b - -",
		"8/8/8/5R2/8/8/3PPP2/1k1K4 b - -",
		"8/8/8/4R3/8/8/3PPP2/1k1K4 b - -",
		"8/8/8/3R4/8/8/3PPP2/1k1K4 b - -",
		"8/8/8/2R5/8/8/3PPP2/1k1K4 b - -",
		"8/8/8/1R6/8/8/3PPP2/1k1K4 b - -",
		"8/8/8/R7/8/8/3PPP2/1k1K4 b - -",
		"8/8/8/8/7R/8/3PPP2/1k1K4 b - -",
		"8/8/8/8/6R1/8/3PPP2/1k1K4 b - -",
		"8/8/8/8/5R2/8/3PPP2/1k1K4 b - -",
		"8/8/8/8/4R3/8/3PPP2/1k1K4 b - -",
		"8/8/8/8/3R4/8/3PPP2/1k1K4 b - -",
		"8/8/8/8/2R5/8/3PPP2/1k1K4 b - -",
		"8/8/8/8/1R6/8/3PPP2/1k1K4 b - -",
		"8/8/8/8/R7/8/3PPP2/1k1K4 b - -",
		"8/8/8/8/8/7R/3PPP2/1k1K4 b - -",
		"8/8/8/8/8/6R1/3PPP2/1k1K4 b - -",
		"8/8/8/8/8/5R2/3PPP2/1k1K4 b - -",
		"8/8/8/8/8/4R3/3PPP2/1k1K4 b - -",
		"8/8/8/8/8/3R4/3PPP2/1k1K4 b - -",
		"8/8/8/8/8/2R5/3PPP2/1k1K4 b - -",
		"8/8/8/8/8/1R6/3PPP2/1k1K4 b - -",
		"8/8/8/8/8/R7/3PPP2/1k1K4 b - -",
		"8/8/8/8/8/8/3PPP1R/1k1K4 b - -",
		"8/8/8/8/8/8/3PPPR1/1k1K4 b - -",
		"8/8/8/8/8/8/3PPP2/1k1K3R b - -",
		"8/8/8/8/8/8/3PPP2/1k1K2R1 b - -",
		"8/8/8/8/8/8/3PPP2/1k1K1R2 b - -",
		"8/8/8/8/8/8/3PPP2/1k1KR3 b - -"
	});

	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppBpp/8/8/4P3/8/PPPP1PPP/RNBQK1NR w KQkq - 0 1", "BRK", {});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppBpp/8/8/4P3/8/PPPP1PPP/RNBQK1NR w KQkq - 0 1", "BRP", {});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppBpp/8/8/4P3/8/PPPP1PPP/RNBQK1NR w KQkq - 0 1", "BBB", {});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppBpp/8/8/4P3/8/PPPP1PPP/RNBQK1NR w KQkq - 0 1", "BBR", {});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppBpp/8/8/4P3/8/PPPP1PPP/RNBQK1NR w KQkq - 0 1", "BBP", {});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppBpp/8/8/4P3/8/PPPP1PPP/RNBQK1NR w KQkq - 0 1", "BBN", {});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppBpp/8/8/4P3/8/PPPP1PPP/RNBQK1NR w KQkq - 0 1", "BBK", {});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppBpp/8/8/4P3/8/PPPP1PPP/RNBQK1NR w KQkq - 0 1", "BBQ", {});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppBpp/8/8/4P3/8/PPPP1PPP/RNBQK1NR w KQkq - 0 1", "BQQ", {});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppBpp/8/8/4P3/8/PPPP1PPP/RNBQK1NR w KQkq - 0 1", "BNN", {});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/4P3/1B6/PPPP1PPP/RNBQK1NR w KQkq - 0 1", "BBB", {});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/4P3/1B6/PPPP1PPP/RNBQK1NR w KQkq - 0 1", "BBP", {});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/4P3/1B6/PPPP1PPP/RNBQK1NR w KQkq - 0 1", "BBN", {});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/4P3/1B6/PPPP1PPP/RNBQK1NR w KQkq - 0 1", "BBK", {});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/4P3/1B6/PPPP1PPP/RNBQK1NR w KQkq - 0 1", "BBQ", {});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/4P3/1B6/PPPP1PPP/RNBQK1NR w KQkq - 0 1", "BBR", {});
	ASSERT_MOVES_EQUAL("4k3/8/1p6/8/BP6/8/8/K7 w - - 0 1", "BPP", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/1p6/8/BP6/8/8/K7 w - - 0 1", "BPK", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/1p6/8/BP6/8/8/K7 w - - 0 1", "BBP", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/1p6/8/BP6/8/8/K7 w - - 0 1", "BBB", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/1p6/8/BP6/8/8/K7 w - - 0 1", "BBR", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/1p6/8/BP6/8/8/K7 w - - 0 1", "BBN", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/1p6/8/BP6/8/8/K7 w - - 0 1", "BBQ", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/1p6/8/BP6/8/8/K7 w - - 0 1", "BBK", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/5N2/8/8/8/8/4K3 w - - 0 1", "NNN", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/5N2/8/8/8/8/4K3 w - - 0 1", "NNK", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/5N2/8/8/8/8/4K3 w - - 0 1", "NKK", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/5N2/8/8/8/8/4K3 w - - 0 1", "NKB", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/5N2/8/8/8/8/4K3 w - - 0 1", "NKR", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/5N2/8/8/8/8/4K3 w - - 0 1", "NKQ", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/5N2/8/8/8/8/4K3 w - - 0 1", "NKP", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/5N2/8/8/8/8/4K3 w - - 0 1", "NPP", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/5N2/8/8/8/8/4K3 w - - 0 1", "NQQ", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/8/8/6N1/8/8/4K3 w - - 0 1", "NNP", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/8/8/6N1/8/8/4K3 w - - 0 1", "NNN", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/8/8/6N1/8/8/4K3 w - - 0 1", "NNB", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/8/8/6N1/8/8/4K3 w - - 0 1", "NNQ", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/8/8/6N1/8/8/4K3 w - - 0 1", "NNK", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/8/8/6N1/8/8/4K3 w - - 0 1", "NNR", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/8/8/8/8/7N/4K3 w - - 0 1", "NNN", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/8/8/8/8/4K3/4R3 w - - 0 1", "RRR", {}, true);
	ASSERT_MOVES_EQUAL("4k3/8/8/8/8/8/4K3/4Q3 w - - 0 1", "QQQ", {}, true);
	ASSERT_MOVES_EQUAL("8/8/k7/8/8/8/4P3/4KB2 w - - 0 1", "BPK", {}, true);
	ASSERT_MOVES_EQUAL("8/8/k7/8/8/8/4P3/4KB2 w - - 0 1", "BBP", {}, true);
	ASSERT_MOVES_EQUAL("8/8/k7/8/8/8/4P3/4KB2 w - - 0 1", "BPP", {}, true);
	ASSERT_MOVES_EQUAL("8/8/k7/8/8/8/4P3/4KB2 w - - 0 1", "BPQ", {}, true);
	ASSERT_MOVES_EQUAL("8/8/k7/8/8/8/4P3/4KB2 w - - 0 1", "BPR", {}, true);
	ASSERT_MOVES_EQUAL("8/8/k7/8/8/8/4P3/4KB2 w - - 0 1", "BPN", {}, true);

	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/2B1P3/2N5/PPPP1PPP/R1BQK1NR w KQkq - 0 1", "BRR", {
		"rnbqkbnr/pppppppp/8/1B6/4P3/2N5/PPPP1PPP/R1BQK1NR b Kkq - 0 1",
		"rnbqkbnr/pppppppp/8/3B4/4P3/2N5/PPPP1PPP/R1BQK1NR b Kkq - 0 1",
		"rnbqkbnr/pppppppp/B7/8/4P3/2N5/PPPP1PPP/R1BQK1NR b Kkq - 0 1",
		"rnbqkbnr/pppppppp/4B3/8/4P3/2N5/PPPP1PPP/R1BQK1NR b Kkq - 0 1",
		"rnbqkbnr/pppppBpp/8/8/4P3/2N5/PPPP1PPP/R1BQK1NR b Kkq - 0 1",
		"rnbqkbnr/pppppppp/8/8/4P3/1BN5/PPPP1PPP/R1BQK1NR b Kkq - 0 1",
		"rnbqkbnr/pppppppp/8/8/4P3/2NB4/PPPP1PPP/R1BQK1NR b Kkq - 0 1",
		"rnbqkbnr/pppppppp/8/8/4P3/2N5/PPPPBPPP/R1BQK1NR b Kkq - 0 1",
		"rnbqkbnr/pppppppp/8/8/4P3/2N5/PPPP1PPP/R1BQKBNR b Kkq - 0 1",
	});

	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/4P1Q1/8/PPPP1PPP/RNB1KBNR w KQkq - 0 1", "QRR", {"rnbqkbnr/ppppppQp/8/8/4P3/8/PPPP1PPP/RNB1KBNR b KQkq -", "rnbqkbnr/pppQpppp/8/8/4P3/8/PPPP1PPP/RNB1KBNR b KQkq -", "rnbqkbnr/pppppppp/6Q1/8/4P3/8/PPPP1PPP/RNB1KBNR b KQkq -", "rnbqkbnr/pppppppp/4Q3/8/4P3/8/PPPP1PPP/RNB1KBNR b KQkq -", "rnbqkbnr/pppppppp/8/7Q/4P3/8/PPPP1PPP/RNB1KBNR b KQkq -", "rnbqkbnr/pppppppp/8/6Q1/4P3/8/PPPP1PPP/RNB1KBNR b KQkq -", "rnbqkbnr/pppppppp/8/5Q2/4P3/8/PPPP1PPP/RNB1KBNR b KQkq -", "rnbqkbnr/pppppppp/8/8/4P2Q/8/PPPP1PPP/RNB1KBNR b KQkq -", "rnbqkbnr/pppppppp/8/8/4PQ2/8/PPPP1PPP/RNB1KBNR b KQkq -", "rnbqkbnr/pppppppp/8/8/4P3/7Q/PPPP1PPP/RNB1KBNR b KQkq -", "rnbqkbnr/pppppppp/8/8/4P3/6Q1/PPPP1PPP/RNB1KBNR b KQkq -", "rnbqkbnr/pppppppp/8/8/4P3/5Q2/PPPP1PPP/RNB1KBNR b KQkq -", "rnbqkbnr/pppppppp/8/8/4P3/8/PPPPQPPP/RNB1KBNR b KQkq -", "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq -"});

	ASSERT_MOVES_EQUAL("r1bqkbnr/pppppppp/8/8/7P/2n1R2N/PPPPPPP1/RNBQKB2 w Qkq - 0 1", "RBB", {"r1bqkbnr/ppppRppp/8/8/7P/2n4N/PPPPPPP1/RNBQKB2 b Qkq -", "r1bqkbnr/pppppppp/4R3/8/7P/2n4N/PPPPPPP1/RNBQKB2 b Qkq -", "r1bqkbnr/pppppppp/8/4R3/7P/2n4N/PPPPPPP1/RNBQKB2 b Qkq -", "r1bqkbnr/pppppppp/8/8/4R2P/2n4N/PPPPPPP1/RNBQKB2 b Qkq -", "r1bqkbnr/pppppppp/8/8/7P/2n3RN/PPPPPPP1/RNBQKB2 b Qkq -", "r1bqkbnr/pppppppp/8/8/7P/2n2R1N/PPPPPPP1/RNBQKB2 b Qkq -", "r1bqkbnr/pppppppp/8/8/7P/2nR3N/PPPPPPP1/RNBQKB2 b Qkq -", "r1bqkbnr/pppppppp/8/8/7P/2R4N/PPPPPPP1/RNBQKB2 b Qkq -"});

	ASSERT_MOVES_EQUAL("rnbq1bnr/pppp1ppp/k3p3/8/8/8/PPPPPPPP/RNBQKBNR w KQ - 0 1", "RBK", {"rnbq1bnr/pppp1ppp/k3p3/8/8/8/PPPPPPPP/RNBQKBNR b KQ - 0 1"});
	ASSERT_MOVES_EQUAL("rnbq1bnr/pppp1ppp/k3p3/8/8/8/PPPPPPPP/RNBQKBNR w KQ - 0 1", "RRR", {"rnbq1bnr/pppp1ppp/k3p3/8/8/8/PPPPPPPP/RNBQKBNR b KQ - 0 1"});
	ASSERT_MOVES_EQUAL("rnbq1bnr/pppp1ppp/k3p3/8/8/8/PPPPPPPP/RNBQKBNR w KQ - 0 1", "RBB", {"rnbq1bnr/pppp1ppp/k3p3/8/8/8/PPPPPPPP/RNBQKBNR b KQ - 0 1"});
	ASSERT_MOVES_EQUAL("rnbq1bnr/pppp1ppp/k3p3/8/8/8/PPPPPPPP/RNBQKBNR w KQ - 0 1", "RRB", {"rnbq1bnr/pppp1ppp/k3p3/8/8/8/PPPPPPPP/RNBQKBNR b KQ - 0 1"});
	ASSERT_MOVES_EQUAL("rnbq1bnr/pppp1ppp/k3p3/8/8/8/PPPPPPPP/RNBQKBNR w KQ - 0 1", "BBB", {"rnbq1bnr/pppp1ppp/k3p3/8/8/8/PPPPPPPP/RNBQKBNR b KQ - 0 1"});

	ASSERT_MOVES_EQUAL("rnbq1bnr/pppp1ppp/k3p2Q/8/8/8/PPPPPPPP/RNB1KBNR w KQ - 0 1", "BRQ", {"rnbq1bnr/pppp1ppQ/k3p3/8/8/8/PPPPPPPP/RNB1KBNR b KQ -", "rnbq1bnr/pppp1pQp/k3p3/8/8/8/PPPPPPPP/RNB1KBNR b KQ -", "rnbq1bnr/pppp1ppp/k3p1Q1/8/8/8/PPPPPPPP/RNB1KBNR b KQ -", "rnbq1bnr/pppp1ppp/k3pQ2/8/8/8/PPPPPPPP/RNB1KBNR b KQ -", "rnbq1bnr/pppp1ppp/k3Q3/8/8/8/PPPPPPPP/RNB1KBNR b KQ -", "rnbq1bnr/pppp1ppp/k3p3/7Q/8/8/PPPPPPPP/RNB1KBNR b KQ -", "rnbq1bnr/pppp1ppp/k3p3/6Q1/8/8/PPPPPPPP/RNB1KBNR b KQ -", "rnbq1bnr/pppp1ppp/k3p3/8/7Q/8/PPPPPPPP/RNB1KBNR b KQ -", "rnbq1bnr/pppp1ppp/k3p3/8/5Q2/8/PPPPPPPP/RNB1KBNR b KQ -", "rnbq1bnr/pppp1ppp/k3p3/8/8/7Q/PPPPPPPP/RNB1KBNR b KQ -", "rnbq1bnr/pppp1ppp/k3p3/8/8/4Q3/PPPPPPPP/RNB1KBNR b KQ -"});

	ASSERT_MOVES_EQUAL("4k3/8/4P3/8/5p2/8/4P3/4K3 w - - 0 1", "PPB", {"4k3/8/4P3/8/5P2/8/8/4K3 b - -", "4k3/8/4P3/4P3/5p2/8/8/4K3 b - -", "4k3/4P3/8/8/4Pp2/8/8/4K3 b - e3", "4k3/8/4P3/8/4Pp2/8/8/4K3 b - -", "4k3/4P3/8/8/5p2/4P3/8/4K3 b - -"}, true);

	ASSERT_MOVES_EQUAL("K3k3/4p3/8/5P2/8/8/4p3/8 b - -", "PPK", {"K4k2/8/8/5p2/8/8/4p3/8 w - -", "K2k4/8/8/5p2/8/8/4p3/8 w - -", "K7/5k2/8/5p2/8/8/4p3/8 w - -", "K7/4k3/8/5p2/8/8/4p3/8 w - -", "K7/3k4/8/5p2/8/8/4p3/8 w - -", "K4k2/8/8/4pP2/8/8/4p3/8 w - -", "K2k4/8/8/4pP2/8/8/4p3/8 w - -", "K7/5k2/8/4pP2/8/8/4p3/8 w - -", "K7/4k3/8/4pP2/8/8/4p3/8 w - -", "K7/3k4/8/4pP2/8/8/4p3/8 w - -", "K4k2/8/8/5P2/4p3/8/4p3/8 w - -", "K2k4/8/8/5P2/4p3/8/4p3/8 w - -", "K7/5k2/8/5P2/4p3/8/4p3/8 w - -", "K7/4k3/8/5P2/4p3/8/4p3/8 w - -", "K7/3k4/8/5P2/4p3/8/4p3/8 w - -", "K4k2/8/4p3/5P2/8/8/8/4n3 w - -", "K2k4/8/4p3/5P2/8/8/8/4n3 w - -", "K7/5k2/4p3/5P2/8/8/8/4n3 w - -", "K7/4k3/4p3/5P2/8/8/8/4n3 w - -", "K7/3k4/4p3/5P2/8/8/8/4n3 w - -", "K4k2/8/8/4pP2/8/8/8/4n3 w - e6", "K2k4/8/8/4pP2/8/8/8/4n3 w - e6", "K7/5k2/8/4pP2/8/8/8/4n3 w - e6", "K7/4k3/8/4pP2/8/8/8/4n3 w - e6", "K7/3k4/8/4pP2/8/8/8/4n3 w - e6", "K4k2/8/4p3/5P2/8/8/8/4b3 w - -", "K2k4/8/4p3/5P2/8/8/8/4b3 w - -", "K7/5k2/4p3/5P2/8/8/8/4b3 w - -", "K7/4k3/4p3/5P2/8/8/8/4b3 w - -", "K7/3k4/4p3/5P2/8/8/8/4b3 w - -", "K4k2/8/8/4pP2/8/8/8/4b3 w - e6", "K2k4/8/8/4pP2/8/8/8/4b3 w - e6", "K7/5k2/8/4pP2/8/8/8/4b3 w - e6", "K7/4k3/8/4pP2/8/8/8/4b3 w - e6", "K7/3k4/8/4pP2/8/8/8/4b3 w - e6", "K4k2/8/4p3/5P2/8/8/8/4r3 w - -", "K2k4/8/4p3/5P2/8/8/8/4r3 w - -", "K7/5k2/4p3/5P2/8/8/8/4r3 w - -", "K7/4k3/4p3/5P2/8/8/8/4r3 w - -", "K7/3k4/4p3/5P2/8/8/8/4r3 w - -", "K4k2/8/8/4pP2/8/8/8/4r3 w - e6", "K2k4/8/8/4pP2/8/8/8/4r3 w - e6", "K7/5k2/8/4pP2/8/8/8/4r3 w - e6", "K7/4k3/8/4pP2/8/8/8/4r3 w - e6", "K7/3k4/8/4pP2/8/8/8/4r3 w - e6", "K4k2/8/4p3/5P2/8/8/8/4q3 w - -", "K2k4/8/4p3/5P2/8/8/8/4q3 w - -", "K7/5k2/4p3/5P2/8/8/8/4q3 w - -", "K7/4k3/4p3/5P2/8/8/8/4q3 w - -", "K7/3k4/4p3/5P2/8/8/8/4q3 w - -", "K4k2/8/8/4pP2/8/8/8/4q3 w - e6", "K2k4/8/8/4pP2/8/8/8/4q3 w - e6", "K7/5k2/8/4pP2/8/8/8/4q3 w - e6", "K7/4k3/8/4pP2/8/8/8/4q3 w - e6", "K7/3k4/8/4pP2/8/8/8/4q3 w - e6"}, true);
	ASSERT_MOVES_EQUAL("8/4P3/8/8/5p2/8/4P3/k3K3 w - -", "PPN", {"8/6N1/8/8/4Pp2/8/8/k3K3 b - e3", "8/2N5/8/8/4Pp2/8/8/k3K3 b - e3", "8/8/5N2/8/4Pp2/8/8/k3K3 b - e3", "8/8/3N4/8/4Pp2/8/8/k3K3 b - e3", "8/6N1/8/8/5p2/4P3/8/k3K3 b - -", "8/2N5/8/8/5p2/4P3/8/k3K3 b - -", "8/8/5N2/8/5p2/4P3/8/k3K3 b - -", "8/8/3N4/8/5p2/4P3/8/k3K3 b - -"});

	ASSERT_MOVES_EQUAL("7k/8/8/8/3p4/8/2P1P3/7K w - - 0 1", "PPP", {"7k/8/8/8/3PP3/8/8/7K b - -", "7k/8/8/2P5/3pP3/8/8/7K b - e3", "7k/8/8/8/2PP4/8/8/7K b - -", "7k/8/8/4P3/2Pp4/8/8/7K b - c3", "7k/8/8/8/2PpP3/8/8/7K b - c3", "7k/8/8/8/2PpP3/8/8/7K b - e3", "7k/8/8/8/3P4/4P3/8/7K b - -", "7k/8/8/2P5/3p4/4P3/8/7K b - -", "7k/8/8/8/2Pp4/4P3/8/7K b - -", "7k/8/8/8/3P4/2P5/8/7K b - -", "7k/8/8/4P3/3p4/2P5/8/7K b - -", "7k/8/8/8/3pP3/2P5/8/7K b - -", "7k/8/8/3P4/8/8/4P3/7K b - -", "7k/8/2P5/8/3p4/8/4P3/7K b - -", "7k/8/8/2P5/3p4/8/4P3/7K b - -", "7k/8/8/3P4/8/8/2P5/7K b - -", "7k/8/4P3/8/3p4/8/2P5/7K b - -", "7k/8/8/4P3/3p4/8/2P5/7K b - -"}, true);

	ASSERT_MOVES_EQUAL("7k/8/8/8/2PpP3/8/8/7K b - - 0 1", "PNN", {"7k/8/8/8/2P1P3/3p4/8/7K w - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/8/8/2PpP3/8/8/7K b - c3 0 1", "PNN", {"7k/8/8/8/2P1P3/3p4/8/7K w - -", "7k/8/8/8/4P3/2p5/8/7K w - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/8/8/2PpP3/8/8/7K b - e3 0 1", "PNN", {"7k/8/8/8/2P5/4p3/8/7K w - -", "7k/8/8/8/2P1P3/3p4/8/7K w - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/8/8/2PpP3/8/8/7K b - c3,e3 0 1", "PNN", {"7k/8/8/8/2P5/4p3/8/7K w - -", "7k/8/8/8/2P1P3/3p4/8/7K w - -", "7k/8/8/8/4P3/2p5/8/7K w - -"}, true);


	ASSERT_MOVES_EQUAL("7k/8/8/2p5/2PNP3/8/8/7K b - - 0 1", "PPN", {"7k/8/8/8/2P1P3/3p4/8/7K w - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/8/2p5/2PNP3/8/8/7K b - c3 0 1", "PPN", {"7k/8/8/8/2P1P3/3p4/8/7K w - -", "7k/8/8/8/4P3/2p5/8/7K w - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/8/2p5/2PNP3/8/8/7K b - e3 0 1", "PPN", {"7k/8/8/8/2P5/4p3/8/7K w - -", "7k/8/8/8/2P1P3/3p4/8/7K w - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/8/2p5/2PNP3/8/8/7K b - c3,e3 0 1", "PPN", {"7k/8/8/8/2P5/4p3/8/7K w - -", "7k/8/8/8/2P1P3/3p4/8/7K w - -", "7k/8/8/8/4P3/2p5/8/7K w - -"}, true);

	ASSERT_MOVES_EQUAL("4k3/4p3/8/8/8/8/4P3/4K3 w - - 0 1", "PBB", {"4k3/4p3/8/8/4P3/8/8/4K3 b - -", "4k3/4p3/8/8/8/4P3/8/4K3 b - -"}, true);
	
	ASSERT_MOVES_EQUAL("4k3/3p4/8/8/8/8/4P3/4K3 w - - 0 1", "PBB", {"4k3/3p4/8/8/4P3/8/8/4K3 b - -", "4k3/3p4/8/8/8/4P3/8/4K3 b - -"}, true);

	ASSERT_MOVES_EQUAL("4k3/8/8/8/8/8/4P3/4K3 w - - 0 1", "PBB", {"4k3/8/8/8/4P3/8/8/4K3 b - -", "4k3/8/8/8/8/4P3/8/4K3 b - -"}, true);

	ASSERT_MOVES_EQUAL("4k3/8/8/3p4/8/8/4P3/4K3 w - - 0 1", "PBB", {"4k3/8/8/3p4/4P3/8/8/4K3 b - -", "4k3/8/8/3p4/8/4P3/8/4K3 b - -"}, true);

	ASSERT_MOVES_EQUAL("4k3/8/8/4p3/8/8/4P3/4K3 w - - 0 1", "PBB", {"4k3/8/8/4p3/4P3/8/8/4K3 b - -", "4k3/8/8/4p3/8/4P3/8/4K3 b - -"}, true); 

	ASSERT_MOVES_EQUAL("7k/8/8/1p6/8/8/P2K4/1N6 w - - 0 1", "PNQ", {"7k/8/8/1p6/P7/2N5/3K4/8 b - -", "7k/8/8/1p6/8/P1N5/3K4/8 b - -", "7k/8/8/1p6/P7/N7/3K4/8 b - -"}, true);

	ASSERT_MOVES_EQUAL("7k/8/8/8/1p6/8/P2K4/1N6 w - - 0 1", "PNQ", {"7k/8/8/8/Pp6/2N5/3K4/8 b - a3", "7k/8/8/8/1p6/P1N5/3K4/8 b - -", "7k/8/8/8/Pp6/N7/3K4/8 b - a3"}, true);

	ASSERT_MOVES_EQUAL("7k/8/8/p7/1B6/8/P2K4/1N6 w - - 0 1", "PNQ", {"7k/8/8/p7/PB6/2N5/3K4/8 b - a3", "7k/8/8/p7/1B6/P1N5/3K4/8 b - -", "7k/8/8/p7/PB6/N7/3K4/8 b - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/1p6/2N5/3N4/8/4P3/4K3 w - - 0 1", "PRR", {"7k/8/1p6/2N5/3NP3/8/8/4K3 b - e3", "7k/8/1p6/2N5/3N4/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/1p6/2n5/3N4/8/4P3/4K3 w - - 0 1", "PRR", {"7k/8/1p6/2n5/3NP3/8/8/4K3 b - -", "7k/8/1p6/2n5/3N4/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/1p6/8/3N4/8/4P3/4K3 w - - 0 1", "PRR", {"7k/8/1p6/8/3NP3/8/8/4K3 b - -", "7k/8/1p6/8/3N4/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/1p6/2N5/3n4/8/4P3/4K3 w - - 0 1", "PRR", {"7k/8/1p6/2N5/3nP3/8/8/4K3 b - -", "7k/8/1p6/2N5/3n4/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/1p6/2n5/3n4/8/4P3/4K3 w - - 0 1", "PRR", {"7k/8/1p6/2n5/3nP3/8/8/4K3 b - -", "7k/8/1p6/2n5/3n4/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/1p6/8/3n4/8/4P3/4K3 w - - 0 1", "PRR", {"7k/8/1p6/8/3nP3/8/8/4K3 b - -", "7k/8/1p6/8/3n4/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/1p6/2N5/8/8/4P3/4K3 w - - 0 1", "PRR", {"7k/8/1p6/2N5/4P3/8/8/4K3 b - -", "7k/8/1p6/2N5/8/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/1p6/2n5/8/8/4P3/4K3 w - - 0 1", "PRR", {"7k/8/1p6/2n5/4P3/8/8/4K3 b - -", "7k/8/1p6/2n5/8/4P3/8/4K3 b - -"}, true);




	ASSERT_MOVES_EQUAL("k7/8/8/2p5/8/8/1P2K3/2N5 w - - 0 1", "PNQ", {"k7/8/8/2p5/1P6/3N4/4K3/8 b - -", "k7/8/8/2p5/8/1P1N4/4K3/8 b - -", "k7/8/8/2p5/1P6/1N6/4K3/8 b - -", "k7/8/8/2p5/1P6/8/N3K3/8 b - -", "k7/8/8/2p5/8/1P6/N3K3/8 b - -"}, true);

	//This should change to not include en passant rights after doing !is_attacked_by_anything_other_than_the_pawn(square_hopped_rank, i, this->to_move) reset i-th bit (other than that pawn => possibly 2 pawns) TODO in board.cpp
		ASSERT_MOVES_EQUAL("k7/8/8/8/2p5/8/1P2K3/2N5 w - - 0 1", "PNQ", {"k7/8/8/8/1Pp5/3N4/4K3/8 b - b3", "k7/8/8/8/2p5/1P1N4/4K3/8 b - -", "k7/8/8/8/1Pp5/1N6/4K3/8 b - b3", "k7/8/8/8/1Pp5/8/N3K3/8 b - b3", "k7/8/8/8/2p5/1P6/N3K3/8 b - -"}, true);

	ASSERT_MOVES_EQUAL("k7/8/8/1p6/2B5/8/1P2K3/2N5 w - - 0 1", "PNQ", {"k7/8/8/1p6/1PB5/3N4/4K3/8 b - b3", "k7/8/8/1p6/2B5/1P1N4/4K3/8 b - -", "k7/8/8/1p6/1PB5/1N6/4K3/8 b - -", "k7/8/8/1p6/1PB5/8/N3K3/8 b - b3", "k7/8/8/1p6/2B5/1P6/N3K3/8 b - -"}, true);
	ASSERT_MOVES_EQUAL("k7/8/8/3p4/2B5/8/1P2K3/2N5 w - - 0 1", "PNQ", {"k7/8/8/3p4/1PB5/3N4/4K3/8 b - b3", "k7/8/8/3p4/2B5/1P1N4/4K3/8 b - -", "k7/8/8/3p4/1PB5/1N6/4K3/8 b - -", "k7/8/8/3p4/1PB5/8/N3K3/8 b - b3", "k7/8/8/3p4/2B5/1P6/N3K3/8 b - -"}, true);
	ASSERT_MOVES_EQUAL("k7/8/8/2p5/2B5/8/1P2K3/2N5 w - - 0 1", "PNQ", {"k7/8/8/2p5/1PB5/3N4/4K3/8 b - -", "k7/8/8/2p5/2B5/1P1N4/4K3/8 b - -", "k7/8/8/2p5/1PB5/1N6/4K3/8 b - -", "k7/8/8/2p5/1PB5/8/N3K3/8 b - -", "k7/8/8/2p5/2B5/1P6/N3K3/8 b - -"}, true);


	ASSERT_MOVES_EQUAL("k7/8/1p6/2N5/3N4/8/4P3/4K3 w - - 0 1", "PRR", {"k7/8/1p6/2N5/3NP3/8/8/4K3 b - e3", "k7/8/1p6/2N5/3N4/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("k7/8/1p6/2n5/3N4/8/4P3/4K3 w - - 0 1", "PRR", {"k7/8/1p6/2n5/3NP3/8/8/4K3 b - -", "k7/8/1p6/2n5/3N4/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("k7/8/1p6/8/3N4/8/4P3/4K3 w - - 0 1", "PRR", {"k7/8/1p6/8/3NP3/8/8/4K3 b - -", "k7/8/1p6/8/3N4/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("k7/8/1p6/2N5/3n4/8/4P3/4K3 w - - 0 1", "PRR", {"k7/8/1p6/2N5/3nP3/8/8/4K3 b - -", "k7/8/1p6/2N5/3n4/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("k7/8/1p6/2n5/3n4/8/4P3/4K3 w - - 0 1", "PRR", {"k7/8/1p6/2n5/3nP3/8/8/4K3 b - -", "k7/8/1p6/2n5/3n4/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("k7/8/1p6/8/3n4/8/4P3/4K3 w - - 0 1", "PRR", {"k7/8/1p6/8/3nP3/8/8/4K3 b - -", "k7/8/1p6/8/3n4/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("k7/8/1p6/2N5/8/8/4P3/4K3 w - - 0 1", "PRR", {"k7/8/1p6/2N5/4P3/8/8/4K3 b - -", "k7/8/1p6/2N5/8/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("k7/8/1p6/2n5/8/8/4P3/4K3 w - - 0 1", "PRR", {"k7/8/1p6/2n5/4P3/8/8/4K3 b - -", "k7/8/1p6/2n5/8/4P3/8/4K3 b - -"}, true);




	ASSERT_MOVES_EQUAL("7k/8/1p6/8/8/8/4P3/4K3 w - - 0 1", "PRR", {"7k/8/1p6/8/4P3/8/8/4K3 b - -", "7k/8/1p6/8/8/4P3/8/4K3 b - -"}, true);


	ASSERT_MOVES_EQUAL("7k/8/3p4/2N5/3N4/8/4P3/4K3 w - - 0 1", "PRR", {"7k/8/3p4/2N5/3NP3/8/8/4K3 b - e3", "7k/8/3p4/2N5/3N4/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/3p4/2n5/3N4/8/4P3/4K3 w - - 0 1", "PRR", {"7k/8/3p4/2n5/3NP3/8/8/4K3 b - -", "7k/8/3p4/2n5/3N4/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/3p4/8/3N4/8/4P3/4K3 w - - 0 1", "PRR", {"7k/8/3p4/8/3NP3/8/8/4K3 b - -", "7k/8/3p4/8/3N4/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/3p4/2N5/3n4/8/4P3/4K3 w - - 0 1", "PRR", {"7k/8/3p4/2N5/3nP3/8/8/4K3 b - -", "7k/8/3p4/2N5/3n4/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/3p4/2n5/3n4/8/4P3/4K3 w - - 0 1", "PRR", {"7k/8/3p4/2n5/3nP3/8/8/4K3 b - -", "7k/8/3p4/2n5/3n4/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/3p4/8/3n4/8/4P3/4K3 w - - 0 1", "PRR", {"7k/8/3p4/8/3nP3/8/8/4K3 b - -", "7k/8/3p4/8/3n4/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/3p4/2N5/8/8/4P3/4K3 w - - 0 1", "PRR", {"7k/8/3p4/2N5/4P3/8/8/4K3 b - -", "7k/8/3p4/2N5/8/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/3p4/2n5/8/8/4P3/4K3 w - - 0 1", "PRR", {"7k/8/3p4/2n5/4P3/8/8/4K3 b - -", "7k/8/3p4/2n5/8/4P3/8/4K3 b - -"}, true);
	ASSERT_MOVES_EQUAL("7k/8/3p4/8/8/8/4P3/4K3 w - - 0 1", "PRR", {"7k/8/3p4/8/4P3/8/8/4K3 b - -", "7k/8/3p4/8/8/4P3/8/4K3 b - -"}, true);



	ASSERT_MOVES_EQUAL("3k4/3p4/8/8/2N5/8/3P4/3K4 w - - 0 1", "PRR", {"3k4/3p4/8/8/2NP4/8/8/3K4 b - d3", "3k4/3p4/8/8/2N5/3P4/8/3K4 b - -"}, true);
	ASSERT_MOVES_EQUAL("8/3p4/3k4/8/2N5/8/3P4/3K4 w - - 0 1", "PRR", {"8/3p4/3k4/8/2NP4/8/8/3K4 b - -", "8/3p4/3k4/8/2N5/3P4/8/3K4 b - -"}, true);
	ASSERT_MOVES_EQUAL("3k4/3p4/3p4/8/2N5/8/3P4/3K4 w - - 0 1", "PRR", {"3k4/3p4/3p4/8/2NP4/8/8/3K4 b - d3", "3k4/3p4/3p4/8/2N5/3P4/8/3K4 b - -"}, true);

	ASSERT_MOVES_EQUAL("8/8/8/kpPR4/8/8/8/K7 w - b6 0 1", "PRB", {}, true);
	ASSERT_MOVES_EQUAL("8/8/8/kpPR4/8/8/8/K7 w - b6 0 1", "PRR", {}, true);
	ASSERT_MOVES_EQUAL("8/8/8/kpPR4/8/8/8/K7 w - b6 0 1", "PPR", {}, true);
	ASSERT_MOVES_EQUAL("8/8/8/kpPR4/8/8/8/K7 w - b6 0 1", "PRK", {}, true);
	ASSERT_MOVES_EQUAL("8/8/8/kpPR4/8/8/8/K7 w - b6 0 1", "PRQ", {}, true);
	ASSERT_MOVES_EQUAL("8/8/8/kpP5/8/8/8/K2R4 w - b6 0 1", "PRR", {}, true);

	ASSERT_MOVES_EQUAL("8/8/8/kpPR4/8/8/8/K7 w - - 0 1", "PRB", {"3R4/8/2P5/kp6/8/8/8/K7 b - -", "8/3R4/2P5/kp6/8/8/8/K7 b - -", "8/8/2PR4/kp6/8/8/8/K7 b - -", "8/8/2P5/kp5R/8/8/8/K7 b - -", "8/8/2P5/kp4R1/8/8/8/K7 b - -", "8/8/2P5/kp3R2/8/8/8/K7 b - -", "8/8/2P5/kp2R3/8/8/8/K7 b - -", "8/8/2P5/kpR5/8/8/8/K7 b - -", "8/8/2P5/kR6/8/8/8/K7 b - -", "8/8/2P5/kp6/3R4/8/8/K7 b - -", "8/8/2P5/kp6/8/3R4/8/K7 b - -", "8/8/2P5/kp6/8/8/3R4/K7 b - -", "8/8/2P5/kp6/8/8/8/K2R4 b - -"});
	ASSERT_MOVES_EQUAL("8/8/8/kpPR4/8/8/8/K7 w - - 0 1", "PRR", {});
	ASSERT_MOVES_EQUAL("8/8/8/kpPR4/8/8/8/K7 w - - 0 1", "PPR", {"3R4/2P5/8/kp6/8/8/8/K7 b - -", "8/2PR4/8/kp6/8/8/8/K7 b - -", "8/2P5/3R4/kp6/8/8/8/K7 b - -", "8/2P5/8/kp5R/8/8/8/K7 b - -", "8/2P5/8/kp4R1/8/8/8/K7 b - -", "8/2P5/8/kp3R2/8/8/8/K7 b - -", "8/2P5/8/kp2R3/8/8/8/K7 b - -", "8/2P5/8/kpR5/8/8/8/K7 b - -", "8/2P5/8/kR6/8/8/8/K7 b - -", "8/2P5/8/kp6/3R4/8/8/K7 b - -", "8/2P5/8/kp6/8/3R4/8/K7 b - -", "8/2P5/8/kp6/8/8/3R4/K7 b - -", "8/2P5/8/kp6/8/8/8/K2R4 b - -"});
	ASSERT_MOVES_EQUAL("8/8/8/kpPR4/8/8/8/K7 w - - 0 1", "PRK", {"3R4/8/2P5/kp6/8/8/1K6/8 b - -", "8/3R4/2P5/kp6/8/8/1K6/8 b - -", "8/8/2PR4/kp6/8/8/1K6/8 b - -", "8/8/2P5/kp5R/8/8/1K6/8 b - -", "8/8/2P5/kp4R1/8/8/1K6/8 b - -", "8/8/2P5/kp3R2/8/8/1K6/8 b - -", "8/8/2P5/kp2R3/8/8/1K6/8 b - -", "8/8/2P5/kpR5/8/8/1K6/8 b - -", "8/8/2P5/kR6/8/8/1K6/8 b - -", "8/8/2P5/kp6/3R4/8/1K6/8 b - -", "8/8/2P5/kp6/8/3R4/1K6/8 b - -", "8/8/2P5/kp6/8/8/1K1R4/8 b - -", "3R4/8/2P5/kp6/8/8/K7/8 b - -", "8/3R4/2P5/kp6/8/8/K7/8 b - -", "8/8/2PR4/kp6/8/8/K7/8 b - -", "8/8/2P5/kp5R/8/8/K7/8 b - -", "8/8/2P5/kp4R1/8/8/K7/8 b - -", "8/8/2P5/kp3R2/8/8/K7/8 b - -", "8/8/2P5/kp2R3/8/8/K7/8 b - -", "8/8/2P5/kpR5/8/8/K7/8 b - -", "8/8/2P5/kR6/8/8/K7/8 b - -", "8/8/2P5/kp6/3R4/8/K7/8 b - -", "8/8/2P5/kp6/8/3R4/K7/8 b - -", "8/8/2P5/kp6/8/8/K2R4/8 b - -", "8/8/2P5/kp6/8/8/1K6/3R4 b - -", "8/8/2P5/kp6/8/8/K7/3R4 b - -", "3R4/8/2P5/kp6/8/8/8/1K6 b - -", "8/3R4/2P5/kp6/8/8/8/1K6 b - -", "8/8/2PR4/kp6/8/8/8/1K6 b - -", "8/8/2P5/kp5R/8/8/8/1K6 b - -", "8/8/2P5/kp4R1/8/8/8/1K6 b - -", "8/8/2P5/kp3R2/8/8/8/1K6 b - -", "8/8/2P5/kp2R3/8/8/8/1K6 b - -", "8/8/2P5/kpR5/8/8/8/1K6 b - -", "8/8/2P5/kR6/8/8/8/1K6 b - -", "8/8/2P5/kp6/3R4/8/8/1K6 b - -", "8/8/2P5/kp6/8/3R4/8/1K6 b - -", "8/8/2P5/kp6/8/8/3R4/1K6 b - -", "8/8/2P5/kp6/8/8/8/1K1R4 b - -"});
	ASSERT_MOVES_EQUAL("8/8/8/kpPR4/8/8/8/K7 w - - 0 1", "PRQ", {"3R4/8/2P5/kp6/8/8/8/K7 b - -", "8/3R4/2P5/kp6/8/8/8/K7 b - -", "8/8/2PR4/kp6/8/8/8/K7 b - -", "8/8/2P5/kp5R/8/8/8/K7 b - -", "8/8/2P5/kp4R1/8/8/8/K7 b - -", "8/8/2P5/kp3R2/8/8/8/K7 b - -", "8/8/2P5/kp2R3/8/8/8/K7 b - -", "8/8/2P5/kpR5/8/8/8/K7 b - -", "8/8/2P5/kR6/8/8/8/K7 b - -", "8/8/2P5/kp6/3R4/8/8/K7 b - -", "8/8/2P5/kp6/8/3R4/8/K7 b - -", "8/8/2P5/kp6/8/8/3R4/K7 b - -", "8/8/2P5/kp6/8/8/8/K2R4 b - -"});
	ASSERT_MOVES_EQUAL("8/8/8/kpP5/8/8/8/K2R4 w - - 0 1", "PRR", {"7R/8/2P5/kp6/8/8/8/K7 b - -", "6R1/8/2P5/kp6/8/8/8/K7 b - -", "5R2/8/2P5/kp6/8/8/8/K7 b - -", "4R3/8/2P5/kp6/8/8/8/K7 b - -", "3R4/8/2P5/kp6/8/8/8/K7 b - -", "2R5/8/2P5/kp6/8/8/8/K7 b - -", "1R6/8/2P5/kp6/8/8/8/K7 b - -", "R7/8/2P5/kp6/8/8/8/K7 b - -", "8/7R/2P5/kp6/8/8/8/K7 b - -", "8/6R1/2P5/kp6/8/8/8/K7 b - -", "8/5R2/2P5/kp6/8/8/8/K7 b - -", "8/4R3/2P5/kp6/8/8/8/K7 b - -", "8/3R4/2P5/kp6/8/8/8/K7 b - -", "8/2R5/2P5/kp6/8/8/8/K7 b - -", "8/1R6/2P5/kp6/8/8/8/K7 b - -", "8/R7/2P5/kp6/8/8/8/K7 b - -", "8/8/2P4R/kp6/8/8/8/K7 b - -", "8/8/2P3R1/kp6/8/8/8/K7 b - -", "8/8/2P2R2/kp6/8/8/8/K7 b - -", "8/8/2P1R3/kp6/8/8/8/K7 b - -", "8/8/2PR4/kp6/8/8/8/K7 b - -", "8/8/1RP5/kp6/8/8/8/K7 b - -", "8/8/R1P5/kp6/8/8/8/K7 b - -", "8/8/2P5/kp5R/8/8/8/K7 b - -", "8/8/2P5/kp4R1/8/8/8/K7 b - -", "8/8/2P5/kp3R2/8/8/8/K7 b - -", "8/8/2P5/kp2R3/8/8/8/K7 b - -", "8/8/2P5/kp1R4/8/8/8/K7 b - -", "8/8/2P5/kpR5/8/8/8/K7 b - -", "8/8/2P5/kR6/8/8/8/K7 b - -", "8/8/2P5/kp6/7R/8/8/K7 b - -", "8/8/2P5/kp6/6R1/8/8/K7 b - -", "8/8/2P5/kp6/5R2/8/8/K7 b - -", "8/8/2P5/kp6/4R3/8/8/K7 b - -", "8/8/2P5/kp6/3R4/8/8/K7 b - -", "8/8/2P5/kp6/2R5/8/8/K7 b - -", "8/8/2P5/kp6/1R6/8/8/K7 b - -", "8/8/2P5/kp6/R7/8/8/K7 b - -", "8/8/2P5/kp6/8/7R/8/K7 b - -", "8/8/2P5/kp6/8/6R1/8/K7 b - -", "8/8/2P5/kp6/8/5R2/8/K7 b - -", "8/8/2P5/kp6/8/4R3/8/K7 b - -", "8/8/2P5/kp6/8/3R4/8/K7 b - -", "8/8/2P5/kp6/8/2R5/8/K7 b - -", "8/8/2P5/kp6/8/1R6/8/K7 b - -", "8/8/2P5/kp6/8/R7/8/K7 b - -", "8/8/2P5/kp6/8/8/7R/K7 b - -", "8/8/2P5/kp6/8/8/6R1/K7 b - -", "8/8/2P5/kp6/8/8/5R2/K7 b - -", "8/8/2P5/kp6/8/8/4R3/K7 b - -", "8/8/2P5/kp6/8/8/3R4/K7 b - -", "8/8/2P5/kp6/8/8/2R5/K7 b - -", "8/8/2P5/kp6/8/8/1R6/K7 b - -", "8/8/2P5/kp6/8/8/R7/K7 b - -", "8/8/2P5/kp6/8/8/8/K6R b - -", "8/8/2P5/kp6/8/8/8/K5R1 b - -", "8/8/2P5/kp6/8/8/8/K4R2 b - -", "8/8/2P5/kp6/8/8/8/K3R3 b - -", "8/8/2P5/kp6/8/8/8/K2R4 b - -", "8/8/2P5/kp6/8/8/8/K1R5 b - -", "8/8/2P5/kp6/8/8/8/KR6 b - -"});

	ASSERT_MOVES_EQUAL("1k1K4/8/8/2p5/3p4/8/1PP5/8 w - - 0 1", "PPR", {"1k1K4/8/8/2p5/1PPp4/8/8/8 b - c3", "1k1K4/8/8/2p5/1P1p4/2P5/8/8 b - -", "1k1K4/8/8/2p5/2Pp4/1P6/8/8 b - c3", "1k1K4/8/8/2p5/3p4/1PP5/8/8 b - -", "1k1K4/8/8/2P5/3p4/8/2P5/8 b - -", "1k1K4/8/8/1Pp5/3p4/8/2P5/8 b - -", "1k1K4/8/8/2p5/1P1p4/8/2P5/8 b - -", "1k1K4/8/8/2p5/3P4/8/1P6/8 b - -", "1k1K4/8/8/2p5/2Pp4/8/1P6/8 b - -"});

	std::cerr << GREEN << "PASSED TESTS (" << passed_tests.size() << "):\n" << CLEAR_COLOURS;
	bulk_dump_boards_with_annotations(passed_tests, passed_tests_annotations, std::cerr);



	std::cerr << failed.str();
	std::cerr << "Passed total of " << passed_orientations << " including all board orientations\n";
}
