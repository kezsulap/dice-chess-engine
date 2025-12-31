#include <climits>
#include <array>
#include <cstdint>
#include <ostream>
#include <vector>
#include <string>
#include <random>
const int PIECES_TYPES_COUNT = 6, DICE_COUNT = 3;
const int BOARD_WIDTH = 8, BOARD_HEIGHT = 8;
const uint8_t EMPTY = 0, WHITE = 0, BLACK = 1, PAWN = 2, KNIGHT = 4, BISHOP = 6, ROOK = 8, QUEEN = 10, KING = 12;

const uint8_t PIECE_TYPES[] = {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};

const uint8_t promotions[] = {KNIGHT, BISHOP, ROOK, QUEEN};

const uint8_t WHITE_KINGSIDE_CASTLE = 1, WHITE_QUEENSIDE_CASTLE = 2, BLACK_KINGSIDE_CASTLE = 4, BLACK_QUEENSIDE_CASTLE = 8;

const std::string piece_images[] = {"♟", "♞", "♝", "♜", "♛", "♚"};
const std::string piece_strings[] = {"P", "N", "B", "R", "Q", "K"};

const std::string piece_colors[2] = {"\033[38:2:255:255:255m", "\033[38:2:0:0:0m"};
const std::string square_colors[2] = {"\033[48:2:155:129:84m", "\033[48:2:128:103:56m"};
const std::string reset_colors = "\033[0m";
using hash_type = uint64_t;
class board;


constexpr int power(int base, int exponent) {
	return exponent == 0 ? 1 : base * power(base, exponent - 1);
}

const int OMEGA = power(PIECES_TYPES_COUNT, DICE_COUNT);

constexpr uint8_t opponent(uint8_t x) {return x ^ 1;}

int get_screen_width();

class dice_roll {
public:
	std::array<uint8_t, PIECES_TYPES_COUNT> count;
	int encode() const;
	static dice_roll decode(int); 
	int total_rolls() const;
	dice_roll append(uint8_t piece) const;
	std::vector<dice_roll> strict_subsets() const;
	dice_roll& operator=(const dice_roll& other) = default;
	int combinations() const;
	static dice_roll roll(std::mt19937 &rng);
	auto operator<=>(const dice_roll &oth) const = default;
};

dice_roll parse_dice_roll(const std::string &s);

std::vector<dice_roll> make_rolls_with(int low, int high);

const std::vector<dice_roll> full_dice_rolls = make_rolls_with(3, 3);
const std::vector<dice_roll> partial_dice_rolls = make_rolls_with(0, 2);
const std::vector<dice_roll> full_and_partial_dice_rolls = make_rolls_with(0, 3);

std::ostream &operator<<(std::ostream &o, const dice_roll &dice);

class movelist {
	std::array<std::vector<board>, power(DICE_COUNT + 1, PIECES_TYPES_COUNT)> moves;

public:
	movelist(const std::array<std::vector<board>, power(DICE_COUNT + 1, PIECES_TYPES_COUNT)> &moves_);
	movelist(std::array<std::vector<board>, power(DICE_COUNT + 1, PIECES_TYPES_COUNT)> &&moves_);
	const std::vector<board>& get_moves(const dice_roll &x) const; //Keep as a separate class so can be done lazily or anything alike in the future, probably replace to return some wrapper around possibly multiple vectors
																																 //(use when there's no way to move all 3 pieces, but there are some to move a subset and those strict_subsets can be only stored once)
	int count_winning_on_the_spot() const;
};

class partial_movelist {
	
};

using square_t = uint8_t;
constexpr bool is_empty(square_t x) {return x == EMPTY;}
constexpr bool is_players(square_t x, uint8_t player) {return (x & 1) == player;} //TODO: what should this return when x is empty (?), so far just don't use it with this value at all
constexpr uint8_t to_raw_piece(square_t x) {return x &~1;}
constexpr uint8_t make_piece(uint8_t piece, uint8_t player) {return piece | player;}
board parse_fen(const std::string &x);
class board {
	std::array<std::array<uint8_t, BOARD_HEIGHT>, BOARD_WIDTH> squares;
	uint8_t castling_mask;
	uint8_t to_move;
	uint8_t en_passant_mask;
	void touch_castling(int x, int y);
	void add_en_passant(int x);
	void remove_en_passant(int x);
public:
	void move_piece(int from_x, int from_y, int to_x, int to_y);
	void clear_square(int x, int y);
	void put_piece(int x, int y, uint8_t piece);
	movelist generate_moves() const;
	partial_movelist generate_partial_moves() const;
	void dump(std::ostream &o) const;
	friend board parse_fen(const std::string &x);
	auto operator<=>(const board &oth) const = default;
	std::string fen() const;
	uint8_t get_to_move() const;
	void flip_in_place();
	board flip() const;
};


void bulk_dump_boards(const std::vector<board> &, std::ostream &o);

