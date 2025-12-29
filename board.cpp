#include "board.hpp"
#include <cassert>

std::string get_piece_string(uint8_t x) {
	return piece_strings[x / 2 - 1];
}

uint8_t get_player(uint8_t x) {
	return x & 1;
}


std::vector<std::string> split(const std::string& str, char delimiter) { //TODO: move to some shared location
	std::vector<std::string> tokens;
	size_t start = 0;
	size_t end = str.find(delimiter);

	while (end != std::string::npos) {
		tokens.push_back(str.substr(start, end - start));
		start = end + 1;
		end = str.find(delimiter, start);
	}
	tokens.push_back(str.substr(start));
	return tokens;
}

board parse_fen(const std::string &fen){ //TODO: string_view
	const std::vector<std::string> content = split(fen, ' ');
	assert(content.size() >= 4 && content.size() <= 6);
	const std::string &board_content = content[0], to_move = content[1], castling = content[2], en_passant = content[3];
	const std::vector<std::string> rows = split(board_content, '/');
	assert(rows.size() == BOARD_HEIGHT);
	board ret;
	for (size_t i = 0; i < BOARD_HEIGHT; ++i) {
		const std::string &row = rows[BOARD_HEIGHT - i - 1];
		size_t at = 0;
		for (char x : row) {
			if (x >= '0' && x <= '9') {
				size_t go = x - '0';
				for (size_t j = 0; j < go; ++j) {
					assert(at < BOARD_WIDTH);
					ret.squares[i][at++] = EMPTY;
				}
			}
			else {
				assert(at < BOARD_WIDTH);
				switch(x) {
					case 'k': ret.squares[i][at] = KING | WHITE; break;
					case 'K': ret.squares[i][at] = KING | BLACK; break;
					case 'q': ret.squares[i][at] = QUEEN | WHITE; break;
					case 'Q': ret.squares[i][at] = QUEEN | BLACK; break;
					case 'r': ret.squares[i][at] = ROOK | WHITE; break;
					case 'R': ret.squares[i][at] = ROOK | BLACK; break;
					case 'b': ret.squares[i][at] = BISHOP | WHITE; break;
					case 'B': ret.squares[i][at] = BISHOP | BLACK; break;
					case 'n': ret.squares[i][at] = KNIGHT | WHITE; break;
					case 'N': ret.squares[i][at] = KNIGHT | BLACK; break;
					case 'p': ret.squares[i][at] = PAWN | WHITE; break;
					case 'P': ret.squares[i][at] = PAWN | BLACK; break;
					default: assert(false);
				}
				at++;
			}
		}
		assert(at == BOARD_WIDTH);
	}
	assert(to_move == "b" || to_move == "w");
	ret.to_move = to_move == "b" ? BLACK : WHITE;
	ret.castling_mask = 0;
	for (char x : castling) {
		switch (x) {
			case 'K': ret.castling_mask |= WHITE_KINGSIDE_CASTLE; break;
			case 'k': ret.castling_mask |= BLACK_KINGSIDE_CASTLE; break;
			case 'Q': ret.castling_mask |= WHITE_QUEENSIDE_CASTLE; break;
			case 'q': ret.castling_mask |= BLACK_QUEENSIDE_CASTLE; break;
			case '-': break;
			default: assert(false);	
		}
	}
	ret.en_passant_mask = 0;
	if (en_passant == "-")
		;
	else {
		const std::vector<std::string> en_passant_squares = split(en_passant, ',');
		for (const std::string &x : en_passant_squares) {
			assert(x.size() == 2u);
			assert(x[1] == (ret.to_move == BLACK ? '3' : '6'));
			assert(x[0] >= 'a' && x[0] <= 'h');
			ret.en_passant_mask |= 1 << (x[0] - 'a');
		}
	}
	return ret;
}

void board::dump(std::ostream &o) const {
	for (int i = BOARD_HEIGHT - 1; i >= 0; --i) {
		for (int j = 0; j < BOARD_WIDTH; ++j) {
			o << square_colors[(i + j) % 2];
			if (is_empty(squares[i][j])) o << "  ";
			else o << piece_colors[get_player(squares[i][j])] << get_piece_string(squares[i][j]) << " ";
			o << reset_colors;
		}
		o << "\n";
	}
	o << (to_move == WHITE ? "White" : "Black") << " to move\n";
	o << "Castling: ";
	if (castling_mask == 0) o << "none";
	else {
		if (castling_mask & WHITE_KINGSIDE_CASTLE) o << "K";
		if (castling_mask & WHITE_QUEENSIDE_CASTLE) o << "Q";
		if (castling_mask & BLACK_KINGSIDE_CASTLE) o << "k";
		if (castling_mask & BLACK_QUEENSIDE_CASTLE) o << "q";
	}
	o << "\n";
	o << "En passant: ";
	if (en_passant_mask == 0) o << "none";
	else {
		for (int i = 0; i < BOARD_WIDTH; ++i) if (en_passant_mask >> i & 1) o << (char)('a' + i);
	}
	o << "\n";
}
