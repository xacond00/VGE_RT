#pragma once
// Created by Ondrej Ac (xacond00)
#include "defines.h"
#include <cmath>
#include <type_traits> // for std::enable_if, etc.
#include <utility>	   // for std::index_sequence

template <class T, Uint N>
struct alignas(sizeof(T)) Vec_t {
	Vec_t() {}
	// Helper for variadic constructor
	template <typename U, std::size_t... Is>
	constexpr Vec_t(U t, std::index_sequence<Is...>) : data{((void)Is, static_cast<T>(t))...} {
		static_assert(N > 1, "Vectors can only have size >= 2");
	}

	// Main constructor that uses the helper
	template <typename U, typename = std::enable_if_t<std::is_convertible<U, T>::value>>
	explicit constexpr Vec_t(U t) : Vec_t(t, std::make_index_sequence<N>{}) {
		static_assert(N > 1, "Vectors can only have size >= 2");
	}

	// Variadic constructor for 2+ values
	template <typename... Args, typename = std::enable_if_t<(sizeof...(Args) >= 2 && sizeof...(Args) <= N)>>
	constexpr Vec_t(Args... args) : data{static_cast<T>(args)...} {
		static_assert(N > 1, "Vectors can only have size >= 2");
	}
	Vec_t &operator=(T t){
		return *this = Vec_t(t);
	}
	constexpr T operator[](Uint i) const { return data[i]; }
	constexpr T &operator[](Uint i) { return data[i]; }

	constexpr const T *ptr() const { return data; }
	constexpr T *ptr() { return data; }

	constexpr static Uint size = N;
	constexpr static Uint bytes = N * sizeof(T);

	void print()const{
		std::cout << '(';
		for (Uint i = 0; i < N - 1; i++) {
			std::cout << data[i] << '|';
		}
		std::cout << data[N -1] << ")\n";
	}

	Vec_t<T, N + 1> append(Float t = 0) const {
		Vec_t<T, N + 1> res;
		for(Uint i = 0; i < N; i++) {
			res[i] = data[i];
		}
		res[N] = t;
		return res;
	}
	Vec_t<T, N - 1> shrink() const {
		Vec_t<T, N - 1> res;
		for(Uint i = 0; i < N - 1; i++) {
			res[i] = data[i];
		}
		return res;
	}

	Vec_t lrotate() const {
		Vec_t res;
		for (Uint i = 0; i < N - 1; i++) {
			res[i] = data[i + 1];
		}
		res[N - 1] = data[0];
		return res;
	}

	Vec_t rrotate() const {
		Vec_t res;
		for (Uint i = 1; i < N; i++) {
			res[i] = data[i - 1];
		}
		res[0] = data[N - 1];
		return res;
	}

	T sum() const {
		T res = 0;
		for (Uint i = 0; i < N; i++) {
			res += data[i];
		}
		return res;
	}
	T max() const {
		T res = data[0];
		for (Uint i = 1; i < N; i++) {
			res = max(res, data[i]);
		}
		return res;
	}
	T min() const {
		T res = data[0];
		for (Uint i = 1; i < N; i++) {
			res = min(res, data[i]);
		}
		return res;
	}
	T prod() const {
		T res = 1;
		for (Uint i = 0; i < N; i++) {
			res *= data[i];
		}
		return res;
	}
	T len2() const {
		T res = 0;
		for (Uint i = 0; i < N; i++) {
			res += data[i] * data[i];
		}
		return res;
	}
	T len() const { return std::sqrt(len2()); }
	T norm() const{
		return 1.f / std::sqrt(len2());
	}
	inline T& x(){static_assert(N > 0, "Out of bounds !");return data[0];}
	inline T& y(){static_assert(N > 1, "Out of bounds !");return data[1];}
	inline T& z(){static_assert(N > 2, "Out of bounds !");return data[2];}
	inline T& w(){static_assert(N > 3, "Out of bounds !");return data[3];}
	inline T x()const{static_assert(N > 0, "Out of bounds !");return data[0];}
	inline T y()const{static_assert(N > 1, "Out of bounds !");return data[1];}
	inline T z()const{static_assert(N > 2, "Out of bounds !");return data[2];}
	inline T w()const{static_assert(N > 3, "Out of bounds !");return data[3];}
	T data[N] = {};
};

using Vec2f = Vec_t<Float, 2>;
using Vec3f = Vec_t<Float, 3>;
using Vec4f = Vec_t<Float, 4>;
using Vec2u = Vec_t<Float, 2>;
using Vec3u = Vec_t<Uint, 3>;


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
// Vec X Vec ops
template <class T, class U, Uint N>
inline auto operator+(const Vec_t<T, N> &a, const Vec_t<U, N> &b) {
	return bin_op(a, b, [](T a, T b) { return a + b; });
}
template <class T, class U, Uint N>
inline auto operator-(const Vec_t<T, N> &a, const Vec_t<U, N> &b) {
	return bin_op(a, b, [](T a, T b) { return a - b; });
}
template <class T, class U, Uint N>
inline auto operator*(const Vec_t<T, N> &a, const Vec_t<U, N> &b) {
	return bin_op(a, b, [](T a, T b) { return a * b; });
}
template <class T, class U, Uint N>
inline auto operator/(const Vec_t<T, N> &a, const Vec_t<U, N> &b) {
	return bin_op(a, b, [](T a, T b) { return a / b; });
}
template <class T, class U, Uint N>
inline auto max(const Vec_t<T, N> &a, const Vec_t<U, N> &b) {
	return bin_op(a, b, [](T a, T b) { return std::max(a, b); });
}
template <class T, class U, Uint N>
inline auto min(const Vec_t<T, N> &a, const Vec_t<U, N> &b) {
	return bin_op(a, b, [](T a, T b) { return std::min(a, b); });
}
// Scalar X Vec ops
template <class T, class U, Uint N>
inline auto operator+(const T &a, const Vec_t<U, N> &b) {
	return bin_op(Vec_t<T, N>(a), b, [](T a, T b) { return a + b; });
}
template <class T, class U, Uint N>
inline auto operator-(const T &a, const Vec_t<U, N> &b) {
	return bin_op(Vec_t<T, N>(a), b, [](T a, T b) { return a - b; });
}
template <class T, class U, Uint N>
inline auto operator*(const T &a, const Vec_t<U, N> &b) {
	return bin_op(Vec_t<T, N>(a), b, [](T a, T b) { return a * b; });
}
template <class T, class U, Uint N>
inline auto operator/(const T &a, const Vec_t<U, N> &b) {
	return bin_op(Vec_t<T, N>(a), b, [](T a, T b) { return a / b; });
}
// Vec X Scalar ops
template <class T, class U, Uint N>
inline auto operator+(const Vec_t<T, N> &a, const U&b) {
	return bin_op(a, Vec_t<U, N>(b), [](T a, T b) { return a + b; });
}
template <class T, class U, Uint N>
inline auto operator-(const Vec_t<T, N> &a, const U&b) {
	return bin_op(a, Vec_t<U, N>(b), [](T a, T b) { return a - b; });
}
template <class T, class U, Uint N>
inline auto operator*(const Vec_t<T, N> &a, const U&b) {
	return bin_op(a, Vec_t<U, N>(b), [](T a, T b) { return a * b; });
}
template <class T, class U, Uint N>
inline auto operator/(const Vec_t<T, N> &a, const U&b) {
	return bin_op(a, Vec_t<U, N>(b), [](T a, T b) { return a / b; });
}
// Unary ops
template <class T, Uint N>
inline auto operator-(const Vec_t<T, N> &a) {
	return un_op(a, [](T a) { return -a; });
}
template <class T, Uint N>
inline auto sqrt(const Vec_t<T, N> &a) {
	return un_op(a, [](T a) { return std::sqrt(a); });
}
template <class T, Uint N>
inline auto sin(const Vec_t<T, N> &a) {
	return un_op(a, [](T a) { return std::sin(a); });
}
template <class T, Uint N>
inline auto cos(const Vec_t<T, N> &a) {
	return un_op(a, [](T a) { return std::cos(a); });
}
template <class T, Uint N>
inline auto atan(const Vec_t<T, N> &a) {
	return un_op(a, [](T a) { return std::atan(a); });
}
template <class T, Uint N>
inline auto rcp(const Vec_t<T, N> &a) {
	return un_op(a, [](T a) { return T(1) / a; });
}
template <class T, Uint N>
inline auto log(const Vec_t<T, N> &a) {
	return un_op(a, [](T a) { return std::log(a); });
}
template <class T, Uint N>
inline auto abs(const Vec_t<T, N> &a) {
	return un_op(a, [](T a) { return std::abs(a); });
}
template <class T, Uint N>
inline auto clip(const Vec_t<T, N> &a) {
	return un_op(a, [](T a) { return std::max(std::min(a, T(1)), T(0)); });
}

// Other ops
template <class T, class U, Uint N>
inline auto dot(Vec_t<T, N> a, const Vec_t<U, N> &b) {
	a = a * b;
	return a.sum();
}

template <class T, Uint N>
inline auto norm(const Vec_t<T, N> &a) {
	return a * Vec3f(a.norm());
}

inline Vec3f cross(const Vec3f &a, const Vec3f &b) {
	return Vec3f(a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]);
}

template<typename T>
inline T lerp(const T &a, const T &b, const T& t) {
	return a * (T(1) - t) + b * t;
}

