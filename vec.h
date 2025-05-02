#pragma once
#include "defines.h"
#include <type_traits> // for std::enable_if, etc.
#include <utility>	   // for std::index_sequence
#include <cmath>

template <class T, Uint N>
struct alignas(sizeof(T)) Vec_t {
	Vec_t() {}
	// Helper for variadic constructor
	template <typename U, std::size_t... Is>
	constexpr Vec_t(U t, std::index_sequence<Is...>) : data{((void)Is, static_cast<T>(t))...} {}

	// Main constructor that uses the helper
	template <typename U, typename = std::enable_if_t<std::is_convertible<U, T>::value>>
	explicit constexpr Vec_t(U t) : Vec_t(t, std::make_index_sequence<N>{}) {}

	// Variadic constructor for 2+ values
	template <typename... Args, typename = std::enable_if_t<(sizeof...(Args) >= 2 && sizeof...(Args) <= N)>>
	constexpr Vec_t(Args... args) : data{static_cast<T>(args)...} {}

	constexpr T operator[](Uint i) const { return data[i]; }
	constexpr T &operator[](Uint i) { return data[i]; }

	constexpr const T *ptr() const { return data; }
	constexpr T *ptr() { return data; }

	constexpr static Uint size = N;
	constexpr static Uint bytes = N * sizeof(T);

	T data[N] = {};
};

template <class T, class OP, Uint N>
inline auto un_op(Vec_t<T, N> a, const OP &op) {
	for (Uint i = 0; i < N; i++) {
		a[i] = op(a[i]);
	}
	return a;
}

template <class T, class U, class OP, Uint N>
inline auto bin_op(Vec_t<T, N> a, const Vec_t<U, N> &b, const OP &op) {
    #pragma unroll(N)
	for (Uint i = 0; i < N; i++) {
		a[i] = op(a[i], b[i]);
	}
	return a;
}

template <class T, class U, Uint N>
inline auto operator+(const Vec_t<T, N>& a, const Vec_t<U, N>& b) {
	return bin_op(a, b, [](T a, T b){return a + b;});
}
template <class T, class U, Uint N>
inline auto operator-(const Vec_t<T, N>& a, const Vec_t<U, N>& b) {
	return bin_op(a, b, [](T a, T b){return a - b;});
}
template <class T, class U, Uint N>
inline auto operator*(const Vec_t<T, N>& a, const Vec_t<U, N>& b) {
	return bin_op(a, b, [](T a, T b){return a * b;});
}
template <class T, class U, Uint N>
inline auto operator/(const Vec_t<T, N>& a, const Vec_t<U, N>& b) {
	return bin_op(a, b, [](T a, T b){return a / b;});
}

template <class T, Uint N>
inline auto operator-(const Vec_t<T, N>& a) {
    return un_op(a, [](T a){return -a;});
}
template <class T, Uint N>
inline auto sqrt(const Vec_t<T, N>& a) {
    return un_op(a, [](T a){return std::sqrt(a);});
}
template <class T, Uint N>
inline auto rcp(const Vec_t<T, N>& a) {
    return un_op(a, [](T a){return T(1) / a;});
}
template <class T, Uint N>
inline auto log(const Vec_t<T, N>& a) {
    return un_op(a, [](T a){return std::log(a);});
}


using Vec2f = Vec_t<Float, 2>;
using Vec3f = Vec_t<Float, 3>;
using Vec4f = Vec_t<Float, 4>;
using Vec3u = Vec_t<Uint, 3>;
