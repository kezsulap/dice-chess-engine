#include <bits/stdc++.h>
#include "board.hpp"
using namespace std;
void output_summary(const std::vector<long double> &data) {
	assert(!data.empty());
	long double sum = 0;
	for (long double x : data) sum += x;
	long double std_dev = 0;
	long double mean = sum / data.size();
	for (long double x : data) std_dev += (x - mean) * (x - mean);
	std_dev /= data.size() - 1;
	std_dev = sqrt(std_dev);
	long double error = std_dev / sqrt(data.size());
	std::cerr << "mean = " << mean << ", std_dev = " << std_dev << " (error ≈ " << error << ")" << "\n";
}
template <class T> const T& random_choice(const std::vector<T> &content, mt19937 &rng) {
	assert(!content.empty());
	return content[uniform_int_distribution<size_t>(0, content.size() - 1)(rng)];
}
int main(int argc, char **argv) {
	mt19937 rng(10);
	assert(argc == 2);
	string fen = argv[1];
	board starting_position = parse_fen(fen);
	int count = 0;
	int next_show = 1;
	starting_position.dump(cerr);
	vector <long double> white_win_total, black_win_total, still_playing_total;
	while (true) {
		board b = starting_position;
		count++;
		long double white_won = 0, black_won = 0, still_playing = 1;
		for (int _ = 0; _ < 1000 && still_playing > 1e-5; ++_) {
			auto moves = b.generate_moves();
			int wins_here = moves.count_winning_on_the_spot();
			long double p_wins_here = wins_here / (long double)OMEGA;
			if (b.get_to_move() == WHITE) white_won += p_wins_here * still_playing;
			else black_won += p_wins_here * still_playing;
			still_playing *= (1 - p_wins_here);
			if (wins_here == OMEGA) {
				// cerr << "Found a board where every dice roll wins\n";
				// b.dump(std::cerr);
				break;
			}
			dice_roll roll;
			do roll = dice_roll::roll(rng);
			while (moves.get_moves(roll).empty());
			b = random_choice(moves.get_moves(roll), rng);
		}
		white_win_total.push_back(white_won);
		black_win_total.push_back(black_won);
		still_playing_total.push_back(still_playing);
		if (count >= next_show) {
			cerr << "count = " << count << "\n";
			cerr << "white won: ";
			output_summary(white_win_total);
			cerr << "black won: ";
			output_summary(black_win_total);
			cerr << "still playing: ";
			output_summary(still_playing_total);
			next_show = max<int>(count + 1, next_show * 1.05);
		}
	}
}
