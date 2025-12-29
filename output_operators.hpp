#ifndef OUTPUT_OPERATORS_H
#define OUTPUT_OPERATORS_H
#include <utility>
#include <algorithm>
#include <string>
#include <ostream>
#include <type_traits>
#include <optional>
inline std::ostream &operator<<(std::ostream &o, __int128 x) {
	if (x < 0) o << "-";
	std::string r;
	do {
		r.push_back('0' + std::abs(int(x % 10)));
		x /= 10;
	}
	while (x != 0);
	std::reverse(r.begin(), r.end());
	return o << r;
}
template <class c, class d> std::ostream &operator<<(std::ostream &o, const std::pair <c, d> &x);

template <class c> auto operator<<(std::ostream &o, const c&v)
		-> typename std::enable_if<!std::is_same<c, std::string>::value, decltype(v.end(), o)>::type;

template <class c> std::ostream &operator<<(std::ostream &o, const std::optional <c> &x);

template <class ...c> std::ostream &operator<<(std::ostream &o, const std::tuple<c...> &x);

template <class c, class d> std::ostream &operator<<(std::ostream &o, const std::pair <c, d> &x) {
	return o << "(" << x.first << ", " << x.second << ")";
}
template <class c> auto operator<<(std::ostream &o, const c&v)
		-> typename std::enable_if<!std::is_same<c, std::string>::value, decltype(v.end(), o)>::type {
	o << "{";
	int q = 0;
	for (const auto &x : v) o << ", " + 2 * !q++ << x;
	return o << "}";
}
template <class c> std::ostream &operator<<(std::ostream &o, const std::optional <c> &x) {
	if (!x.has_value()) return o << "---";
	return o << "{" << *x << "}";
}
template <class ...c> std::ostream &operator<<(std::ostream &o, const std::tuple<c...> &x) {
	o << "(";
	int q = 0;
	apply([&](c...y){
		((o << ", " + 2 * !q++ << y), ...);
	}, x);
	return o << ")";
}
#endif
