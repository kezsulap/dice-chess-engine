#include <bits/stdc++.h>
#include "board.hpp"
using namespace std;
int main(int argc, char **argv) {
	assert(argc == 2);
	string fen = argv[1];
	board b = parse_fen(fen);
	b.dump(cout);
}
