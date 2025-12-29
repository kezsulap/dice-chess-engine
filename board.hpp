#include <climits>
#include <cstdint>
#include <ostream>
#include <vector>
#include <string>
const int PIECES_TYPES_COUNT = 6, DICE_COUNT = 3;
const int BOARD_WIDTH = 8, BOARD_HEIGHT = 8;
const uint8_t EMPTY = 0, WHITE = 0, BLACK = 1, PAWN = 2, KNIGHT = 4, BISHOP = 6, ROOK = 8, QUEEN = 10, KING = 12;

const uint8_t WHITE_KINGSIDE_CASTLE = 1, WHITE_QUEENSIDE_CASTLE = 2, BLACK_KINGSIDE_CASTLE = 4, BLACK_QUEENSIDE_CASTLE = 8;

const std::string piece_strings[] = {"♟", "♞", "♝", "♜", "♛", "♚"};

const std::string piece_colors[2] = {"\033[38:2:0:0:0m", "\033[38:2:255:255:255m"};
const std::string square_colors[2] = {"\033[48:2:155:129:84m", "\033[48:2:128:103:56m"};
const std::string reset_colors = "\033[0m";
using hash_type = uint64_t;
class move {
	
};
class dice_roll {
	
};
class movelist {
	const std::vector<move>& get_moves(const dice_roll &x); 
};

using square_t = uint8_t;
inline bool is_empty(square_t x) {return x == EMPTY;}
inline bool is_players(square_t x, uint8_t player) {return (x & 1) == player;}
inline uint8_t to_raw_piece(square_t x) {return x &~1;}
class board;
board parse_fen(const std::string &x);
class board {
	uint8_t squares[BOARD_WIDTH][BOARD_HEIGHT]; //TODO: consider making this into a bitfield as well
	uint8_t castling_mask : 4;
	uint8_t to_move : 1;
	uint8_t en_passant_mask : 8;
public:
	movelist generate_moves() const;
	void dump(std::ostream &o) const;
	friend board parse_fen(const std::string &x);
	bool operator==(const board &oth) const = default;
};


