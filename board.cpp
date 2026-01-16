#include "board.hpp"
#include <cassert>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include "output_operators.hpp"

std::string get_piece_image(uint8_t x) {
	return piece_images[x / 2 - 1];
}

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


std::vector <dice_roll> dice_roll::strict_subsets() const {
	std::vector<dice_roll> ret;
	dice_roll oth = *this;
	bool found_dec = true;
	while (true) {
		found_dec = false;
		for (size_t i = 0; i < PIECES_TYPES_COUNT; ++i) {
			if (oth.count[i]) {
				oth.count[i]--;
				found_dec = true;
				break;
			}
			else {
				oth.count[i] = this->count[i];
			}
		}
		if (found_dec) ret.push_back(oth);
		else break;
	}
	return ret;
}

movelist::movelist(const std::array<std::vector<board>, DICE_ROLL_LENGTH> &moves_) : moves(moves_) {}
movelist::movelist(std::array<std::vector<board>, DICE_ROLL_LENGTH> &&moves_) : moves(std::move(moves_)) {}

int movelist::count_winning_on_the_spot() const {
	int ret = 0;
	for (auto &dice : full_dice_rolls) {
		if (this->get_moves(dice).empty()) {
			ret += dice.combinations();
		}
	}
	return ret;
}

uint8_t board::get_to_move() const {
	return this->to_move;
}

dice_roll dice_roll::roll(std::mt19937 &rng) {
	dice_roll ret = {};
	for (int _ = 0; _ < DICE_COUNT; ++_) {
		ret.count[std::uniform_int_distribution(0, PIECES_TYPES_COUNT - 1)(rng)]++;
	}
	return ret;
}

dice_roll parse_dice_roll(const std::string &s) {
	dice_roll ret = {};
	for (char x : s) {
		bool found = false;
		for (size_t i = 0; i < PIECES_TYPES_COUNT; ++i) {
			if (std::string(1, x) == piece_strings[i]) {
				ret.count[i]++;
				found = true;
			}
		}
		assert(found);
	}
	return ret;
}


void board::flip_in_place() {
	reverse(squares.begin(), squares.end());
	to_move ^= 1;
	for (auto &row : squares) for (auto &square : row) if (!is_empty(square)) square ^= 1;
	castling_mask = (castling_mask >> 2 | castling_mask << 2) & 0xf;
}

board board::flip() const {
	board ret = *this;
	ret.flip_in_place();
	return ret;
}

void board::flip_horizontally_in_place() {
	assert(castling_mask == 0);
	for (auto &row : squares) std::reverse(row.begin(), row.end());
	uint8_t new_enpassant_mask = 0;
	for (int i = 0; i < BOARD_WIDTH; ++i) {
		if (this->en_passant_mask >> i & 1) {
			new_enpassant_mask |= (1 << (BOARD_WIDTH - i - 1));
		}
	}
	this->en_passant_mask = new_enpassant_mask;
}

uint8_t board::get_castling_mask() const {return this->castling_mask;}
uint8_t board::get_en_passant_mask() const {return this->en_passant_mask;}

std::pair <int, int> board::get_king_position(uint8_t player) const {
	for (int i = 0; i < BOARD_HEIGHT; ++i)
		for (int j = 0; j < BOARD_WIDTH; ++j)
			if (this->squares[i][j] == make_piece(KING, player))
				return {i, j};
	__builtin_unreachable();
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
					case 'k': ret.squares[i][at] = KING | BLACK; break;
					case 'K': ret.squares[i][at] = KING | WHITE; break;
					case 'q': ret.squares[i][at] = QUEEN | BLACK; break;
					case 'Q': ret.squares[i][at] = QUEEN | WHITE; break;
					case 'r': ret.squares[i][at] = ROOK | BLACK; break;
					case 'R': ret.squares[i][at] = ROOK | WHITE; break;
					case 'b': ret.squares[i][at] = BISHOP | BLACK; break;
					case 'B': ret.squares[i][at] = BISHOP | WHITE; break;
					case 'n': ret.squares[i][at] = KNIGHT | BLACK; break;
					case 'N': ret.squares[i][at] = KNIGHT | WHITE; break;
					case 'p': ret.squares[i][at] = PAWN | BLACK; break;
					case 'P': ret.squares[i][at] = PAWN | WHITE; break;
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

std::string board::fen() const {
	std::stringstream ret;
	for (int i = BOARD_HEIGHT - 1; i >= 0; --i) {
		int current_blank = 0;
		auto push_char = [&](char x){
			if (current_blank) ret << current_blank;
			current_blank = 0;
			ret << x;
		};
		for (int j = 0; j < BOARD_WIDTH; ++j) {
			switch(this->squares[i][j]) {
				case make_piece(KING, WHITE): push_char('K'); break;
				case make_piece(QUEEN, WHITE): push_char('Q'); break;
				case make_piece(ROOK, WHITE): push_char('R'); break;
				case make_piece(BISHOP, WHITE): push_char('B'); break;
				case make_piece(KNIGHT, WHITE): push_char('N'); break;
				case make_piece(PAWN, WHITE): push_char('P'); break;
				case make_piece(KING, BLACK): push_char('k'); break;
				case make_piece(QUEEN, BLACK): push_char('q'); break;
				case make_piece(ROOK, BLACK): push_char('r'); break;
				case make_piece(BISHOP, BLACK): push_char('b'); break;
				case make_piece(KNIGHT, BLACK): push_char('n'); break;
				case make_piece(PAWN, BLACK): push_char('p'); break;
				case EMPTY: current_blank++; break;
				default: assert(false); break; 
			}
		}
		if (current_blank) ret << current_blank;
		if (i) ret << "/";
	}
	ret << " " << (this->to_move == WHITE ? 'w' : 'b') << " ";
	if (this->castling_mask & WHITE_KINGSIDE_CASTLE) ret << "K";
	if (this->castling_mask & WHITE_QUEENSIDE_CASTLE) ret << "Q";
	if (this->castling_mask & BLACK_KINGSIDE_CASTLE) ret << "k";
	if (this->castling_mask & BLACK_QUEENSIDE_CASTLE) ret << "q";
	if (!this->castling_mask) ret << "-";
	ret << " ";
	bool first = true;
	for (int i = 0; i < BOARD_WIDTH; ++i) {
		if (this->en_passant_mask >> i & 1) {
			if (!first) ret << ",";
			first = false;
			ret << (char)('a' + i) << (this->to_move == BLACK ? '3' : '6');
		}
	}
	if (!this->en_passant_mask) ret << "-";
	return ret.str();
}

int dice_roll::encode() const {
	int ret = 0;
	int sum = this->total_rolls();
	for (int i = 0; i < sum; ++i) ret += pascal[PIECES_TYPES_COUNT + i - 1][i];
	for (size_t i = 0; i < PIECES_TYPES_COUNT; ++i) {
		for (int _ = 0; _ < count[i]; ++_) {
			int len = PIECES_TYPES_COUNT - i - 1;
			ret += pascal[sum + len - 1][sum];
			sum--;
		}
	}
	assert(ret >= 0 && ret < (int)DICE_ROLL_LENGTH);
	return ret;
}

dice_roll dice_roll::decode(int x) {
	assert(x >= 0 && x < (int)DICE_ROLL_LENGTH);
	dice_roll ret = {};
	int sum = 0;
	while (x >= pascal[PIECES_TYPES_COUNT + sum - 1][sum]) {
		x -= pascal[PIECES_TYPES_COUNT + sum - 1][sum];
		sum++;
	}
	assert(sum <= DICE_COUNT);
	for (size_t i = 0; i < PIECES_TYPES_COUNT; ++i) {
		int len = PIECES_TYPES_COUNT - i - 1;
		while (sum && x >= pascal[len + sum - 1][sum]) {
			ret.count[i]++;
			x -= pascal[len + sum - 1][sum];
			sum--;
		}
	}
	assert(x == 0);
	return ret;
}

int dice_roll::total_rolls() const {
	int ans = 0;
	for (int x : this->count) ans += x;
	return ans;
}

int dice_roll::combinations() const {
	int denominator = 1, total = 0;
	for (int x : this->count) {
		for (int i = 2; i <= x; ++i) denominator *= i;
		total += x;
	}
	int numerator = 1;
	for (int i = 2; i <= total; ++i) numerator *= i;
	return numerator / denominator;
}

const std::vector <board> &movelist::get_moves(const dice_roll &x) const {
	return this->moves[x.encode()];
}

std::ostream &operator<<(std::ostream &o, const dice_roll &dice) {
	bool any = false;
	for (size_t i = 0; i < PIECES_TYPES_COUNT; ++i) {
		for (size_t j = 0; j < dice.count[i]; ++j) {
			any = true;
			o << piece_strings[i];
		}
	}
	if (!any) o << "---";
	return o;
}

dice_roll dice_roll::append(uint8_t piece) const {
	dice_roll ret = *this;
	ret.count[piece / 2 - 1]++;
	return ret;
}

uint8_t board::get_reachable_en_passant_first_heuristic(uint8_t player) const {
	static_assert(BOARD_HEIGHT == 8 && DICE_COUNT == 3);
	uint8_t ret = 0;
	if (player == WHITE) {
		for (int rank = 1; rank <= 4; ++rank) {
			int range = rank == 1 ? 2 : 5 - rank;
			bool except_this = rank == 4;
			for (int file = 0; file < BOARD_WIDTH; ++file) {
				if (this->squares[rank][file] == make_piece(PAWN, WHITE)) {
					for (int x = file - range; x <= file + range; ++x) { //TODO: Anything less dumb
						if (x >= 0 && x < BOARD_WIDTH && (except_this ? x != file : true)) {
							ret |= (1 << x);
						}
					}
				}
			}
		}
	}
	else {
		for (int rank = 3; rank <= 6; ++rank) {
			int range = rank == 6 ? 2 : rank - 2;
			bool except_this = rank == 3;
			for (int file = 0; file < BOARD_WIDTH; ++file) {
				if (this->squares[rank][file] == make_piece(PAWN, BLACK)) {
					for (int x = file - range; x <= file + range; ++x) { //TODO: Anything less dumb
						if (x >= 0 && x < BOARD_WIDTH && (except_this ? x != file : true)) {
							ret |= (1 << x);
						}
					}
				}
			}
		}
	}
	return ret;
}

#include <bitset>
#define MASK(x) #x " = " << std::bitset<8>(x) << ""

int board::min_moves_to_capture_king_with_pawns(uint8_t player) const {
	auto [i, j] = this->get_king_position(opponent(player));
	int move_dir = player == WHITE ? -1 : 1;
	if (j > 0 && i + move_dir >= 0 && i + move_dir < BOARD_HEIGHT && this->squares[i + move_dir][j - 1] == make_piece(PAWN, player)) return 1;
	if (j + 1 < BOARD_WIDTH && i + move_dir >= 0 && i + move_dir < BOARD_HEIGHT && this->squares[i + move_dir][j + 1] == make_piece(PAWN, player)) return 1;
	return 4;
}

void board::finalize_en_passant() {
	static_assert(BOARD_HEIGHT == 8 && DICE_COUNT == 3);
	if (!this->en_passant_mask) return;
	int en_passant_rank = (this->to_move == BLACK ? 3 : 4);
	int square_hopped_rank = (this->to_move == BLACK ? 2 : 5); //TODO: better name
	int capture_king_dist = this->min_moves_to_capture_king_with_pawns(this->to_move);
	if (capture_king_dist == 1) {
		this->en_passant_mask = 0;
		return;
	}
	for (size_t i = 0; i < BOARD_WIDTH; ++i) { //TODO: how much is there to gain from using __ctz to iterate through bits in all contexts like this?
		if (this->en_passant_mask >> i & 1) {
			if (!is_empty(this->squares[square_hopped_rank][i])) {
				assert(is_players(this->squares[square_hopped_rank][i], opponent(this->to_move)));
				if (this->squares[square_hopped_rank][i] == make_piece(opponent(this->to_move), KING))
					this->en_passant_mask &= ~(1 << i);
				else if ((i == 0 || this->squares[en_passant_rank][i - 1] != make_piece(PAWN, this->to_move)) && (i == BOARD_WIDTH - 1 || this->squares[en_passant_rank][i + 1] != make_piece(PAWN, this->to_move)))
					this->en_passant_mask &= ~(1 << i);
				//TODO: if !is_attacked_by_anything_other_than_the_pawn(square_hopped_rank, i, this->to_move) reset i-th bit (other than that pawn => possibly 2 pawns)
			}
		}
	}
	if (!this->en_passant_mask) return;

	
	uint8_t reachable = 0;

	int dir = (this->to_move == BLACK ? 1 : -1);

	if (capture_king_dist > 2) {

		for (int i = 0; i < BOARD_WIDTH; ++i) {
			if (this->squares[en_passant_rank + dir][i] == make_piece(PAWN, this->to_move)) {
				//It's OK to omit pushing forward from here as forward + en passant is always equivalent to normal capture + forward
				// if (this->squares[en_passant_rank][i] == EMPTY || is_players(this->squares[en_passant_rank][i], this->to_move))
					// reachable |= (1 << i);
				if (i > 0 && this->squares[en_passant_rank][i - 1] != EMPTY && is_players(this->squares[en_passant_rank][i - 1], opponent(this->to_move)))
					reachable |= (1 << (i - 1));
				if (i + 1 < BOARD_WIDTH && this->squares[en_passant_rank][i + 1] != EMPTY && is_players(this->squares[en_passant_rank][i + 1], opponent(this->to_move)))
					reachable |= (1 << (i + 1));
			}
		}
	}

	if (capture_king_dist > 3) {

		uint8_t mask_5th_rank = 0;

		for (int i = 0; i < BOARD_WIDTH; ++i) {
			if (this->squares[en_passant_rank + 2 * dir][i] == make_piece(PAWN, this->to_move)) {
				if (is_empty(this->squares[en_passant_rank + dir][i]))
					mask_5th_rank |= (1 << i);
				if (i > 0 && this->squares[en_passant_rank + dir][i - 1] != EMPTY && is_players(this->squares[en_passant_rank + dir][i - 1], opponent(this->to_move)))
					mask_5th_rank |= (1 << (i - 1));
				if (i + 1 < BOARD_WIDTH && this->squares[en_passant_rank + dir][i + 1] != EMPTY && is_players(this->squares[en_passant_rank + dir][i + 1], opponent(this->to_move)))
					mask_5th_rank |= (1 << (i + 1));
			}
		}

		for (int i = 0; i < BOARD_WIDTH; ++i) {
			if (this->squares[en_passant_rank + 3 * dir][i] == make_piece(PAWN, this->to_move) && is_empty(this->squares[en_passant_rank + 2 * dir][i]) && is_empty(this->squares[en_passant_rank + dir][i])) {
				mask_5th_rank |= (1 << i);
			}
		}

		for (int i = 0; i < BOARD_WIDTH; ++i) {
			if (mask_5th_rank >> i & 1) {
				//It's OK to omit pushing forward from here as forward + en passant is always equivalent to normal capture + forward
				// Therefore no: 
				// if (is_empty(this->squares[en_passant_rank][i]))
					// reachable |= (1 << i);
				if (i > 0 && !is_empty(this->squares[en_passant_rank][i - 1]) && is_players(this->squares[en_passant_rank][i - 1], opponent(this->to_move)))
					reachable |= (1 << (i - 1));
				if (i + 1 < BOARD_WIDTH && !is_empty(this->squares[en_passant_rank][i + 1]) && is_players(this->squares[en_passant_rank][i + 1], opponent(this->to_move)))
					reachable |= (1 << (i + 1));
			}
		}

	}
	for (int i = 0; i < BOARD_WIDTH; ++i) {
		if (this->squares[en_passant_rank][i] == make_piece(PAWN, this->to_move))
			reachable |= (1 << i);
	}
		
		// std::cerr << MASK(mask_5th_rank) << "\n";
		// std::cerr << MASK(reachable) << "\n";

	this->en_passant_mask &= (reachable << 1) | (reachable >> 1);

}

movelist board::generate_moves() const {
	uint8_t current_enpassant_mask = this->en_passant_mask, current_to_move = this->to_move;
	std::array<std::vector<board>, DICE_ROLL_LENGTH> moves;
	std::array<bool, DICE_ROLL_LENGTH> king_capture_found = {};
	moves[0].push_back(*this);
	moves[0][0].en_passant_mask = 0;
	moves[0][0].to_move ^= 1;

	uint8_t reachable_en_passant = this->get_reachable_en_passant_first_heuristic(opponent(this->to_move));
	

	auto mark_king_capture_rec = [&](const dice_roll &x, auto &self) -> void {
		assert(x.total_rolls() >= 1);
		if (king_capture_found[x.encode()]) return;
		king_capture_found[x.encode()] = true;
		moves[x.encode()].clear();
		if (x.total_rolls() < DICE_COUNT)
			for (uint8_t piece : PIECE_TYPES)
				self(x.append(piece), self);
	};

	auto mark_king_capture = [&](const dice_roll &x) {
		mark_king_capture_rec(x, mark_king_capture_rec);
	};

	

	auto go_line = [&](int start_x, int start_y, int dir_x, int dir_y, const board &b, std::vector<board> &destination, const dice_roll &destination_dice_roll) -> bool {
		for (int i = 1; ; ++i) {
			int new_x = start_x + dir_x * i, new_y = start_y + dir_y * i;
			if (new_x < 0 || new_x >= BOARD_WIDTH || new_y < 0 || new_y >= BOARD_HEIGHT) break;
			if (b.squares[new_x][new_y] == make_piece(KING, opponent(current_to_move))) {
				mark_king_capture(destination_dice_roll);
				return true;
			}
			if (is_empty(b.squares[new_x][new_y]) || is_players(b.squares[new_x][new_y], opponent(current_to_move))) {
				board new_board = b;
				new_board.move_piece(start_x, start_y, new_x, new_y);
				destination.push_back(new_board);
			}
			if (!is_empty(b.squares[new_x][new_y])) break;
		}
		return false;
	};

	auto go_one = [&](int start_x, int start_y, int dir_x, int dir_y, const board &b, std::vector<board> &destination, const dice_roll &destination_dice_roll) -> bool {
		int new_x = start_x + dir_x, new_y = start_y + dir_y;
		if (new_x < 0 || new_x >= BOARD_WIDTH || new_y < 0 || new_y >= BOARD_HEIGHT) return false;
		if (b.squares[new_x][new_y] == make_piece(KING, opponent(current_to_move))) {
			mark_king_capture(destination_dice_roll);
			return true;
		}
		if (is_empty(b.squares[new_x][new_y]) || is_players(b.squares[new_x][new_y], opponent(current_to_move))) {
			board new_board = b;
			new_board.move_piece(start_x, start_y, new_x, new_y);
			destination.push_back(new_board);
		}
		return false;
	};

	auto go_one_pawn_forward = [&](int start_x, int start_y, int dir_x, int dir_y, const board &b, std::vector<board> &destination, bool is_double_step, uint8_t promote_to) -> bool {
		int new_x = start_x + dir_x, new_y = start_y + dir_y;
		if (new_x < 0 || new_x >= BOARD_WIDTH || new_y < 0 || new_y >= BOARD_HEIGHT) return false;
		int opp_en_passant_rank = current_to_move == WHITE ? 3 : 4;
		if (is_empty(b.squares[new_x][new_y])) {
			board new_board = b;
			new_board.clear_square(start_x, start_y);
			new_board.put_piece(new_x, new_y, make_piece(promote_to, current_to_move));
			if (is_double_step) new_board.add_en_passant(start_y, reachable_en_passant);
			else if (start_x == opp_en_passant_rank) new_board.remove_en_passant(start_y);
			destination.push_back(new_board);
		}
		return false;
	};

	auto go_one_pawn_diagonally = [&](int start_x, int start_y, int dir_x, int dir_y, const board &b, std::vector<board> &destination, const dice_roll &destination_dice_roll, uint8_t promote_to) -> bool {
		int new_x = start_x + dir_x, new_y = start_y + dir_y;
		int en_passant_rank = current_to_move == WHITE ? 4 : 3;
		int opp_en_passant_rank = current_to_move == WHITE ? 3 : 4;
		if (new_x < 0 || new_x >= BOARD_WIDTH || new_y < 0 || new_y >= BOARD_HEIGHT) return false;
		if (b.squares[new_x][new_y] == make_piece(KING, opponent(current_to_move))) {
			mark_king_capture(destination_dice_roll);
			return true;
		}
		if (!is_empty(b.squares[new_x][new_y]) && is_players(b.squares[new_x][new_y], opponent(current_to_move))) {
			board new_board = b;
			new_board.clear_square(start_x, start_y);
			new_board.put_piece(new_x, new_y, make_piece(promote_to, current_to_move));
			if (start_x == opp_en_passant_rank) new_board.remove_en_passant(start_y);
			destination.push_back(new_board);
		}
		else if (start_x == en_passant_rank && ((current_enpassant_mask >> new_y) & 1) && b.squares[start_x][new_y] == make_piece(PAWN, opponent(current_to_move)) && is_empty(b.squares[new_x][new_y])) {
			board new_board = b;
			new_board.clear_square(start_x, start_y);
			new_board.put_piece(new_x, new_y, make_piece(promote_to, current_to_move));
			new_board.clear_square(start_x, new_y);
			destination.push_back(new_board);
		}
		return false;
	};



	for (size_t dice_roll_id = 0; dice_roll_id < DICE_ROLL_LENGTH; ++dice_roll_id) {
		dice_roll current = dice_roll::decode(dice_roll_id);
		if (current.total_rolls() >= DICE_COUNT) continue;
		std::sort(moves[dice_roll_id].begin(), moves[dice_roll_id].end());
		moves[dice_roll_id].erase(std::unique(moves[dice_roll_id].begin(), moves[dice_roll_id].end()), moves[dice_roll_id].end());
		for (const board &current_board : moves[dice_roll_id]) {
			for (size_t i = 0; i < BOARD_WIDTH ; ++i) for (size_t j = 0; j < BOARD_HEIGHT; ++j) {
				if (is_empty(current_board.squares[i][j]) || !is_players(current_board.squares[i][j], current_to_move)) continue;
				dice_roll destination_dice_roll = current.append(current_board.squares[i][j]);
				size_t new_index = destination_dice_roll.encode();
				std::vector<board> &destination = moves[new_index];
				bool capture_found = king_capture_found[new_index];
				auto process_line_piece = [&](const std::vector<std::pair <int, int> > &dirs) -> void {
					if (capture_found) return;
					for (auto [dx, dy] : dirs) if (go_line(i, j, dx, dy, current_board, destination, destination_dice_roll)) return;
				};
				auto process_hopping_piece = [&](const std::vector<std::pair <int, int> > &dirs) -> void {
					if (capture_found) return;
					for (auto [dx, dy] : dirs) if (go_one(i, j, dx, dy, current_board, destination, destination_dice_roll)) return;
				};
				auto process_pawn = [&]() -> void {
					if (capture_found) return;
					assert(i != 0 && i != 7);
					size_t go_dir = current_to_move == WHITE ? 1 : -1;
					size_t promote_from_rank = current_to_move == WHITE ? 6 : 1;
					size_t double_step_from_rank = current_to_move == WHITE ? 1 : 6;
					if (i == promote_from_rank) {
						for (char promote_to : promotions)
							if (go_one_pawn_forward(i, j, go_dir, 0, current_board, destination, false, promote_to)) return;
					}
					else {
						if (go_one_pawn_forward(i, j, go_dir, 0, current_board, destination, false, PAWN)) return;
						if (i == double_step_from_rank && is_empty(current_board.squares[i + go_dir][j]))
							if (go_one_pawn_forward(i, j, go_dir * 2, 0, current_board, destination, true, PAWN))
								return;
					}
					if (i == promote_from_rank) {
						for (char promote_to : promotions) {
							for (int capture_dir : {-1, 1})
								if (go_one_pawn_diagonally(i, j, go_dir, capture_dir, current_board, destination, destination_dice_roll, promote_to))
									return;
						}
					}
					else {
						for (int capture_dir : {-1, 1})
							if (go_one_pawn_diagonally(i, j, go_dir, capture_dir, current_board, destination, destination_dice_roll, PAWN))
								return;
					}
				};
				if (current_board.squares[i][j] == make_piece(KING, current_to_move)) {
					process_hopping_piece({{0, 1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {-1, -1}, {-1, 0}, {-1, 1}});
				}
				else if (current_board.squares[i][j] == make_piece(QUEEN, current_to_move)) {
					process_line_piece({{0, 1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {-1, -1}, {-1, 0}, {-1, 1}});
				}
				else if (current_board.squares[i][j] == make_piece(ROOK, current_to_move)) {
					process_line_piece({{0, 1}, {0, -1}, {1, 0}, {-1, 0}});
				}
				else if (current_board.squares[i][j] == make_piece(BISHOP, current_to_move)) {
					process_line_piece({{1, 1}, {1, -1}, {-1, 1}, {-1, -1}});
				}
				else if (current_board.squares[i][j] == make_piece(KNIGHT, current_to_move)) {
					process_hopping_piece({{1, 2}, {2, 1}, {1, -2}, {2, -1}, {-1, 2}, {-2, 1}, {-1, -2}, {-2, -1}, });
				}
				else if (current_board.squares[i][j] == make_piece(PAWN, current_to_move)) {
					process_pawn();
				}
			}
			if (current.total_rolls() + 2 <= DICE_COUNT) { //Do castling
				size_t new_index = current.append(KING).append(ROOK).encode();
				std::vector<board> &destination = moves[new_index];
				bool capture_found = king_capture_found[new_index];
				if (!capture_found) {
					if (current_to_move == WHITE) {
						if (current_board.castling_mask & WHITE_KINGSIDE_CASTLE) {
							if (is_empty(current_board.squares[0][5]) && is_empty(current_board.squares[0][6])) {
								board new_board = current_board;
								new_board.move_piece(0, 4, 0, 6);
								new_board.move_piece(0, 7, 0, 5);
								destination.push_back(new_board);
							}
						}
						if (current_board.castling_mask & WHITE_QUEENSIDE_CASTLE) {
							if (is_empty(current_board.squares[0][3]) && is_empty(current_board.squares[0][2]) && is_empty(current_board.squares[0][1])) {
								board new_board = current_board;
								new_board.move_piece(0, 4, 0, 2);
								new_board.move_piece(0, 0, 0, 3);
								destination.push_back(new_board);
							}
						}
					}
					else {
						if (current_board.castling_mask & BLACK_KINGSIDE_CASTLE) {
							if (is_empty(current_board.squares[7][5]) && is_empty(current_board.squares[7][6])) {
								board new_board = current_board;
								new_board.move_piece(7, 4, 7, 6);
								new_board.move_piece(7, 7, 7, 5);
								destination.push_back(new_board);
							}
						}
						if (current_board.castling_mask & BLACK_QUEENSIDE_CASTLE) {
							if (is_empty(current_board.squares[7][3]) && is_empty(current_board.squares[7][2]) && is_empty(current_board.squares[7][1])) {
								board new_board = current_board;
								new_board.move_piece(7, 4, 7, 2);
								new_board.move_piece(7, 0, 7, 3);
								destination.push_back(new_board);
							}
						}
					}
				}
			}
		}
	}
	for (size_t dice_roll_id = 0; dice_roll_id < DICE_ROLL_LENGTH ; ++dice_roll_id) {
		dice_roll current = dice_roll::decode(dice_roll_id);
		if (current.total_rolls() == DICE_COUNT) {
			for (auto &x : moves[dice_roll_id]) x.finalize_en_passant();
			std::sort(moves[dice_roll_id].begin(), moves[dice_roll_id].end());
			moves[dice_roll_id].erase(std::unique(moves[dice_roll_id].begin(), moves[dice_roll_id].end()), moves[dice_roll_id].end());
		}
	}
	std::vector <bool> eliminated_en_passant(DICE_ROLL_LENGTH);
	for (int dice_roll_id = DICE_ROLL_LENGTH - 1; dice_roll_id >= 0; --dice_roll_id) {
		dice_roll current = dice_roll::decode(dice_roll_id);
		if (current.total_rolls() > DICE_COUNT) {
			continue;
		}
		if (king_capture_found[current.encode()]) {
			continue;
		}
		std::vector<board> &current_moves = moves[current.encode()];
		if (!current_moves.empty()) continue;
		size_t current_total = current.total_rolls();
		bool found_any = false;
		std::vector<dice_roll> strict_subsets = current.strict_subsets();
		for (int i = current_total - 1; i >= 0; --i) {
			for (const dice_roll &subset : strict_subsets) {
				if (subset.total_rolls() == i) {
					assert(!king_capture_found[subset.encode()]);
					if (!moves[subset.encode()].empty()) {
						found_any = true;
						if (!eliminated_en_passant[subset.encode()]) {
							for (board &b : moves[subset.encode()]) b.finalize_en_passant();
							eliminated_en_passant[subset.encode()] = true;
						}
						current_moves.insert(current_moves.end(), moves[subset.encode()].begin(), moves[subset.encode()].end()); //TODO: am I 100% sure those subsets are always disjoint(?)
					}
				}
			}
			if (found_any) break;
		}
		assert(!current_moves.empty());
	}

	return moves;
}

std::vector<dice_roll> make_rolls_with(int low, int high) { //TODO: Maybe this can be replaced with just iterating through numbers in range and decoding them on the fly (?)
	std::vector<dice_roll> ret;
	for (int i = 0; i < DICE_ROLL_LENGTH; ++i) {
		dice_roll current = dice_roll::decode(i);
		if (current.total_rolls() >= low && current.total_rolls() <= high)
			ret.push_back(current);
	}
	return ret;
}
	
void board::touch_castling(int x, int y) {
	if (x == 0) {
		if (y == 0) this->castling_mask &=~ WHITE_QUEENSIDE_CASTLE;
		if (y == 4) this->castling_mask &=~ (WHITE_QUEENSIDE_CASTLE | WHITE_KINGSIDE_CASTLE);
		if (y == 7) this->castling_mask &=~ WHITE_KINGSIDE_CASTLE;
	}
	if (x == 7) {
		if (y == 0) this->castling_mask &=~ BLACK_QUEENSIDE_CASTLE;
		if (y == 4) this->castling_mask &=~ (BLACK_QUEENSIDE_CASTLE | BLACK_KINGSIDE_CASTLE);
		if (y == 7) this->castling_mask &=~ BLACK_KINGSIDE_CASTLE;
	}
}

void board::add_en_passant(int x, uint8_t reachable) {
	(this->en_passant_mask |= (1 << x)) &= reachable;
}
void board::remove_en_passant(int x) {
	this->en_passant_mask &=~ (1 << x);
}

void board::move_piece(int from_x, int from_y, int to_x, int to_y) {
	touch_castling(from_x, from_y);
	touch_castling(to_x, to_y);
	this->squares[to_x][to_y] = this->squares[from_x][from_y];
	this->squares[from_x][from_y] = EMPTY;
}
void board::clear_square(int x, int y) {
	touch_castling(x, y); //TODO: this can be optimized sometimes, you're not depriving anyone of castling after an en-passant for example
	this->squares[x][y] = EMPTY;
}
void board::put_piece(int x, int y, uint8_t piece) {
	touch_castling(x, y);
	this->squares[x][y] = piece;
}

void board::dump(std::ostream &o) const {
	for (int i = BOARD_HEIGHT - 1; i >= 0; --i) {
		for (int j = 0; j < BOARD_WIDTH; ++j) {
			o << square_colors[(i + j) % 2];
			if (is_empty(squares[i][j])) o << "  ";
			else o << piece_colors[get_player(squares[i][j])] << get_piece_image(squares[i][j]) << " ";
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
std::string exec(const char* cmd) {
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;
}
int get_screen_width() {
	std::string x = exec("/usr/bin/tput cols");
	return std::stoi(x);
}

void bulk_dump_boards(const std::vector<board> &boards, std::ostream &o) {
	const size_t SPOT_WIDTH = 21, CHUNK = get_screen_width() / SPOT_WIDTH;
	auto go = [&](size_t begin, size_t end) {
		std::vector<std::vector<std::string> > rows;
		for (size_t i = begin; i < end; ++i) {
			std::stringstream s;
			boards[i].dump(s);
			rows.push_back(split(s.str(), '\n'));
			assert(rows.back().size() == (size_t)(4 + BOARD_HEIGHT));
			assert(rows.back().back().empty());
		}
		for (size_t i = 0; i < BOARD_HEIGHT; ++i) {
			for (auto &b : rows) {
				o << b[i] << std::string(SPOT_WIDTH - 2 * BOARD_WIDTH, ' ');
			}
			o << "\n";
		}
		for (size_t i = BOARD_HEIGHT; i < BOARD_HEIGHT + 3; ++i) {
			for (auto &b : rows) {
				o << std::left << std::setw(SPOT_WIDTH) << b[i];
			}
			o << "\n";
		}
	};
	for (size_t i = 0; i < boards.size(); i += CHUNK) {
		go(i, std::min(i + CHUNK, boards.size()));
	}
}

void bulk_dump_boards_with_annotations(const std::vector<board> &boards, const std::vector<std::string> &annotations, std::ostream &o) {
	assert(boards.size() == annotations.size());
	const size_t MIN_WIDTH = 21, SCREEN_WIDTH = get_screen_width();
	std::vector<std::vector<std::string> > rows;
	std::vector<size_t> max_width;
	for (size_t i = 0; i < boards.size(); ++i) {
		std::stringstream s;
		boards[i].dump(s);
		s << annotations[i];
		rows.push_back(split(s.str(), '\n'));
		size_t longest = MIN_WIDTH;
		for (size_t j = BOARD_HEIGHT; j < rows[i].size(); ++j)
			longest = std::max(longest, rows[i][j].size());
		max_width.push_back(longest);
	}
	for (size_t i = 0; i < boards.size(); ) {
		size_t width_sum = 0;
		size_t end = i;
		while (end < boards.size() && width_sum + max_width[end] <= SCREEN_WIDTH) {
			width_sum += max_width[end];
			end++;
		}
		if (end == i) end++;
		size_t max_rows = 0;
		for (size_t j = i; j < end; ++j) max_rows = std::max(max_rows, rows[j].size());
		for (size_t row = 0; row < max_rows; ++row) {
			for (size_t j = i; j < end; ++j) {
				if (row < rows[j].size()) {
					o << rows[j][row];
					if (row < BOARD_HEIGHT) o << std::string(max_width[j] - 2 * BOARD_WIDTH, ' ');
					else o << std::string(max_width[j] - rows[j][row].size(), ' ');
				}
				else {
					o << std::string(max_width[j], ' ');
				}
			}
			o << "\n";
		}
		i = end;
	}
}
void board::shift_in_place(int x) { //square (i, j) -> (i, j + x)
	assert(!this->castling_mask);
	if (x > 0) {
		for (int i = 0; i < BOARD_HEIGHT; ++i)
			for (int j = BOARD_WIDTH - 1; j >= 0; --j)
				this->squares[i][j] = (j - x >= 0 ? this->squares[i][j - x] : EMPTY);
		this->en_passant_mask <<= x;
	}
	else if (x < 0) {
		for (int i = 0; i < BOARD_HEIGHT; ++i)
			for (int j = 0; j < BOARD_WIDTH; ++j)
				this->squares[i][j] = (j - x < BOARD_WIDTH ? this->squares[i][j - x] : EMPTY);
		this->en_passant_mask >>= -x;
	}
	//else pass;
}

std::vector <int> board::get_shift_range() const {
	int leftmost = std::numeric_limits<int>::max(), rightmost = -1;
	for (int i = 0; i < BOARD_HEIGHT; ++i) {
		for (int j = 0; j < BOARD_WIDTH; ++j) {
			if (!is_empty(this->squares[i][j])) {
				leftmost = std::min(leftmost, j);
				rightmost = std::max(rightmost, j);
			}
		}
	}
	assert(leftmost != std::numeric_limits<int>::max());
	assert(rightmost != -1);
	std::vector<int> ret;
	for (int j = -leftmost; j < BOARD_WIDTH - rightmost; ++j) {
		ret.push_back(j);
	}
	return ret;
}
