/*Contains all the other streams */

#pragma once

#include "StreamImpl.h"
#include <stdlib.h>
#include <queue>
#include <memory>
#include <iostream>

namespace stream {

	template<typename T>
	class Singleton: public StreamImpl<T> {

	private:
		T val;
		bool seen;

	public:

		Singleton<T>(T t) {
			val = t;
			seen = false;
		}

		Optional<T> next() {

			if (seen)
				return Optional<T>();
			else {
				seen = true;
				return Optional<T>(val);
			}

		}
	};

	template<typename T>
	class Chain: public StreamImpl<T> {

		private:
			std::shared_ptr<StreamImpl<T>> s0;
			std::shared_ptr<StreamImpl<T>> s1;

		public:

			Chain<T>(std::shared_ptr<StreamImpl<T>> stream0, std::shared_ptr<StreamImpl<T>> stream1) {
				s0 = stream0;
				s1 = stream1;
			}

			Optional<T> next() {
				if(Optional<T> result = s0->next())
					return result;
				else
					return s1->next();
			}
	};

	template<typename T>
	class Take: public StreamImpl<T> {

		private:
			std::shared_ptr<StreamImpl<T>> stream;
			int count;

		public:
			Take<T>(std::shared_ptr<StreamImpl<T>> streamPassed, T t) {
				count = t;
				stream = streamPassed;
			}

			Optional<T> next() {
				Optional<T> result;
				if (count > 0) {
					count--;
					result = stream->next();
					if (!result)
						return Optional<T>();
					return result;
				}
				return Optional<T>();
			}
	};


	template<typename T, typename F>
	class Filter: public StreamImpl<T> {

		private:
			std::shared_ptr<StreamImpl<T>> stream;
			F function;

		public:
			Filter(std::shared_ptr<StreamImpl<T>> streamPassed, F&& t): function(std::forward<F>(t)) {
				stream = streamPassed;
			}

			Optional<T> next() {
				Optional<T> result = stream->next();
				while (result && !function(*result)) {
					result = stream->next();
				}
				if (!result) {
					return Optional<T>();
				}
				return result;
			}
	};

	template <typename T, typename F, typename V>
	class Map: public StreamImpl<V> {

		private:
			std::shared_ptr<StreamImpl<T>> stream;
			F function;

		public:
			Map(std::shared_ptr<StreamImpl<T>> streamPointer, F&& t): function(std::forward<F>(t)) {
				stream = streamPointer;
			}

			Optional<V> next() {
				if (Optional<T> result = stream->next()) {
					return Optional<V>(function(*result));
				}
				return Optional<V>();
			}
	};
}