#include "test_utils.hpp"
#include "../board.hpp"
#include <bits/stdc++.h>

void ASSERT_MOVES_EQUAL(const std::string &fen, const std::string &dice_string, const std::vector<std::string> &expected_fens) {
	dice_roll dice = parse_dice_roll(dice_string);
	board b = parse_fen(fen);
	auto got_moves = b.generate_moves().get_moves(dice);
	std::set<board> expected_boards;
	for (auto &expected_fen : expected_fens)
		expected_boards.insert(parse_fen(expected_fen));
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
		mark_test_failure();
		std::cerr << RED << "TEST generating boards for " << dice << " from position:\n" << CLEAR_COLOURS;
		b.dump(std::cerr);
		std::cerr << "Failed\n";
		if (!missing_boards.empty()) {
			std::cerr << RED << "Output missing the following boards (" << missing_boards.size() << "):\n" << CLEAR_COLOURS;
			bulk_dump_boards(missing_boards, std::cerr);
		}
		if (!extranous_boards.empty()) {
			std::cerr << RED << "Output containing unexpected boards (" << extranous_boards.size() << "):\n" << CLEAR_COLOURS;
			bulk_dump_boards(extranous_boards, std::cerr);
		}
		if (!duplicated_boards.empty()) {
			std::cerr << RED << "Output containing duplicated boards:(" << duplicated_boards.size() << ")\n" << CLEAR_COLOURS;
			bulk_dump_boards(duplicated_boards, std::cerr);
		}
		std::cerr << RED << "Output containing " << got_moves.size() << " positions, expected " << expected_fens.size() << "\n" << CLEAR_COLOURS;
		std::cerr << RED << "Actual output: {";
		bool first = true;
		for (auto &x : got_moves) {
			if (!first) std::cerr << ", ";
			first = false;
			std::cerr << "\"" << x.fen() << "\"";
		}
		std::cerr << "}\n";
		std::cerr << RED << "----------------------------------------------\n" << CLEAR_COLOURS;
	}
	else {
		std::cerr << GREEN << "TEST generating boards for " << dice << " from position:\n" << CLEAR_COLOURS;
		b.dump(std::cerr);
		std::cerr << "Passed\n";
		std::cerr << GREEN << "----------------------------------------------\n" << CLEAR_COLOURS;
	}
}

int main() {
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "BRK", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1", "BRK", {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1", "BBB", {});
	ASSERT_MOVES_EQUAL("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1", "QQQ", {});
	ASSERT_MOVES_EQUAL("rn3b2/pk2p3/b7/P7/2q5/8/8/4K3 b - - 0 1", "BPR", {"1n3b2/rk2p3/p7/Pb6/2q5/8/8/4K3 w - - 0 1"});
	ASSERT_MOVES_EQUAL("rn3b2/pk2p3/b7/P7/2q5/K7/8/8 b - - 0 1", "BPR", {});
	ASSERT_MOVES_EQUAL("k7/8/8/4p3/8/8/4P3/K7 w - - 0 1", "PPB", {"k7/8/8/4p3/4P3/8/8/K7 b - - 0 1"});
	ASSERT_MOVES_EQUAL("k7/8/8/4p3/8/8/4P3/K7 w - - 0 1", "PBB", {"k7/8/8/4p3/4P3/8/8/K7 b - e3 0 1", "k7/8/8/4p3/8/4P3/8/K7 b - - 0 1"});
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
	ASSERT_MOVES_EQUAL("k7/8/5p2/8/4P3/8/8/K7 b - e3 0 1", "PPP", {"k7/8/8/8/4P3/5p2/8/K7 w - -", "k7/8/8/8/8/4p3/8/K7 w - -"});
	ASSERT_MOVES_EQUAL("k7/8/5p2/8/4P3/8/8/K7 b - - 0 1", "PPP", {"k7/8/8/8/4P3/5p2/8/K7 w - -", "k7/8/8/8/8/4p3/8/K7 w - -"});
	ASSERT_MOVES_EQUAL("k7/8/4p3/8/4PP2/8/8/K7 b - e3 0 1", "PPP", {"k7/8/8/8/4P3/5p2/8/K7 w - -", "k7/8/8/8/8/4p3/8/K7 w - -"});
	ASSERT_MOVES_EQUAL("k7/8/5pP1/8/6P1/8/8/K7 b - g3 0 1", "PNN", {"k7/8/6P1/5p2/6P1/8/8/K7 w - - 0 1"});
	ASSERT_MOVES_EQUAL("7k/8/8/3Pp3/8/8/8/7K w - e6 0 1", "PBB", {"7k/8/3P4/4p3/8/8/8/7K b - - 0 1", "7k/8/4P3/8/8/8/8/7K b - - 0 1"});
	ASSERT_MOVES_EQUAL("7k/8/4n3/3Pp3/8/8/8/7K w - - 0 1", "PQQ", {"7k/8/3Pn3/4p3/8/8/8/7K b - - 0 1", "7k/8/4P3/4p3/8/8/8/7K b - - 0 1"});
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
	});
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
	});
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
}
