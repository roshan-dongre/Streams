#pragma once

#include "DelayStreamImpl.h"
#include "EmptyStreamImpl.h"
#include "StreamImpl.h"
#include "OtherStreams.h"
#include "assert.h"

#include <memory>
#include <cmath>

namespace stream
{

template <typename T>
class Stream {
private:

	std::shared_ptr<StreamImpl<T>> impl;

	Stream(const std::shared_ptr<StreamImpl<T>>& i): impl(i) {}

	// Make those functions friend of Stream<T>.
	// They are't static members of Stream<T> since we want to avoid the users having to explicitly specify T
	template<typename U> friend Stream<U> empty();
	template<typename U> friend Stream<U> once(U val);
	template<typename U> friend Stream<U> chain(const Stream<U>& stream0,
			const Stream<U>& stream1);
	template<typename U> friend Stream<U> take(const Stream<U>& stream,
			size_t n);
	template<typename U, typename F> friend Stream<U> filter(
			const Stream<U>& stream, F&& f);

	template<typename F> friend decltype(std::declval<F>()()) delay(F&&);

	template<typename S, typename F> friend Stream<
			decltype(std::declval<F>()(std::declval<S>()) )> map(
			const Stream<S>& stream, F&& f);


public:

	using value_type = T;

	Optional<T> next() {
		return impl->next();
	}
};

template <typename T>
Stream<T> empty() {
	return Stream<T>(std::make_shared<EmptyStreamImpl<T>>());
}

template <typename F>
decltype(std::declval<F>()()) delay(F&& func) {
	using V = decltype(std::declval<F>()());
	using T = typename V::value_type;
	return V(std::make_shared<DelayStreamImpl<T, F>>(std::forward<F>(func)));
}

// You need to implement all the funtions listed below. Read the handout for more information
// Here is a checklist: 
// - once() Finished
// - chain() Finished
// - take() Finished
// - filter() Finished
// - map() Finished
// - prime() Finished
// - hamming() Finished
// - pi() Finished

template <typename T>
Stream<T> once(T val) {
	return Stream<T>(std::make_shared<Singleton<T>>(val));
}

Stream <size_t> repeat() {
	size_t value = 1;
	return chain(once(value), delay(repeat));
}

Stream <size_t> counter(size_t a) {
	return chain(once(a), delay([a] () {return counter(a+1);}));
}

Stream <size_t> fibgen(size_t a, size_t b) {
	return chain(once(a), delay([a,b] () { return fibgen(b, a + b);}));
}

Stream <size_t> fib() { return fibgen(0,1); }

template <typename T>
Stream<T> chain(const Stream<T>& stream0, const Stream<T>& stream1) {
	return Stream<T>(std::make_shared<Chain<T>>(stream0.impl,stream1.impl));
}

template <typename T>
Stream<T> take(const Stream<T>& stream, size_t n) {
	return Stream<T>(std::make_shared<Take<T>>(stream.impl, n));
}

template <typename T, typename F>
Stream<T> filter(const Stream<T>& stream, F&& f) {
	return Stream<T>(std::make_shared<Filter<T,F>>(stream.impl,std::forward<F>(f)));
}

template <typename S, typename F>
Stream<decltype(std::declval<F>()(std::declval<S>()))> map(const Stream<S>& stream, F&& f) {
	using V = decltype(std::declval<F>()(std::declval<S>()));
	return Stream<V>(std::make_shared<Map<S,F,V>>(stream.impl,std::forward<F>(f)));
}

Stream<size_t> sieve(Stream<size_t> s) {
	size_t value = *(s.next());

	auto filterFunction = [=] (int number) {return (number % value != 0);};

	return chain(once(value), delay([=] () {return filter(sieve(s), filterFunction);}));
}

// The prime() function returns a stream of prime numbers, in increasing order
// A prime number is a natural number greater than 1 that has no positive divisors other than 1 and itself
// The first 20 prime numbers are { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71 }
Stream<size_t> prime() {
	// Fill in the implementation here
	return sieve(counter(2));
}

bool isPrimeHamming(size_t n) {
	for(size_t i = 2; i <= n / 2; ++i)
  	{
      if(n % i == 0 && i % 2 != 0 && i % 3 != 0 && i % 5 != 0)
      	return false;
  	}
  	return true;
}

// (THIS IS A HELPER FUNCTION FOR hamming())
// The mergeUnique function takes two streams s0 and s1 as parameters
// Both s0 and s1 are sequences of increasing numbers (potentially infinite)
// Return a new stream of increasing numbers that contains all numbers from s0 and s1, removing duplicates
// e.g. if s0 = { 4, 6, 8, 10 } and s1 = { 3, 6, 9, 12, 15 }, the returning stream should be { 3, 4, 6, 8, 9, 10, 12, 15 }
Stream<size_t> mergeUnique(Stream<size_t> s0, Stream<size_t> s1) {
	// Fill in the implementation here
	size_t value0 = *(s0.next());
	size_t value1 = *(s1.next());
	
	/*Filter out the numbers that have prime factors other than 2, 3, or 5 */
	/*while (!isPrimeHamming(value0))
		value0 = *(s0.next());
	while (!isPrimeHamming(value1))
		value1 = *(s1.next());*/

	if (value0 > value1)
		return chain(once(value1), delay([=] () {return mergeUnique(chain(once(value0), s0), s1);}));
	else if (value1 > value0)
		return chain(once(value0), delay([=] () {return mergeUnique(s0, chain(once(value1), s1));}));
	else
		return chain(once(value0), delay([=] () {return mergeUnique(s0, s1);}));
}

// The hamming() function returns a stream of hamming numbers, in increasing order
// A hamming sequence enumerate, in ascending order with no repetitions, all positive integers with no prime factors other than 2, 3, or 5
// The first 20 hamming numbers are { 1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 15, 16, 18, 20, 24, 25, 27, 30, 32, 36 }
Stream<size_t> hamming() {
	// Fill in the implementation here
	auto two = [] (size_t num) {return num * 2;};
	auto three = [] (size_t num) {return num * 3;};
	auto five = [] (size_t num) {return num * 5;};
	size_t seed = 1;
	return chain(once(seed), delay([=] () {
		return mergeUnique(filter(map(counter(seed), two), isPrimeHamming), 
			mergeUnique(filter(map(counter(seed), three), isPrimeHamming), 
				filter(map(counter(seed), five), isPrimeHamming)));}));
}

// (THIS IS A HELPER FUNCTION FOR pi())
// The partialSum function takes a streams s of doubles (potentially infinite) and returns a new stream representing the partial sum of the input stream
// e.g. if s = { 1, 2, 3, 4, 5 }, the returning stream should be { 1, 3, 6, 10, 15 }
Stream<double> partialSum(Stream<double> s, double previousVal) {
	// Fill in the implementation here
	double nextVal = (*s.next());
	return chain(once(previousVal + nextVal), delay([=](){return partialSum(s, previousVal + nextVal);}));
}

// The pi() function returns a stream of increasingly accurate approximations of pi.
// Use the expansion series for pi/4 for the approximation: 
// pi/4 = 1 - 1/3 + 1/5 - 1/7 + 1/9 - ...
Stream<double> pi() {
	// Fill in the implementation here
	auto function = [] (double num) {return 4 * (pow(-1.0, num-1)/(2 * num - 1));};
	Stream<double> seed = map(counter(1.0), function);
	return partialSum(seed, 0.0);
}

}
