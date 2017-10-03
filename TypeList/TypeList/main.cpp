#include <iostream>
#include <tuple>
#include <string>
#include "type_list.h"
#include "Parser.h"

namespace gc {
	namespace priv {
		template<size_t I>
		struct _tuple_foreach {
			template<size_t Cur>
			struct inner {
				template<class T, class F>
				static void deal(T const & t, F && f) {
					f(std::get<Cur>(t));
					inner<Cur + 1>::deal(t, std::forward<F>(f));
				}
			};
			template<>
			struct inner<I - 1> {
				template<class T, class F>
				static void deal(T const & t, F && f) {
					f(std::get<I - 1>(t));
				}
			};
			template<class T, class F>
			static void deal(T const & t, F && f) {
				inner<0>::deal(t, std::forward<F>(f));
			}
		};
	}
}

template<class F, class ... Args>
void for_each(std::tuple<Args ...> const &t, F && f) {
	gc::priv::_tuple_foreach<sizeof...(Args)>::deal(t, std::forward<F>(f));
}

struct Parse_settings {
	using lexem_list = gc::type_list<gc::Lexem>;
};

int main() try{
	using list = gc::type_list<int, float, std::string, double, std::iostream, long, gc::type_list<int, double, long>>;
	using namespace std::string_literals;

	auto t = gc::Parser<Parse_settings>().match_string("awd: bsd, 123 ")
		.match_as(gc::Lexem{}, ": ", gc::Lexem{}, ", ", gc::Lexem{});
	
	for_each(t, [](auto & v) {std::cout << v._data << '|'; });

	std::cin.get();
}
catch (std::exception & e) {
	std::cout << e.what() << " :(";
	std::cin.get();
}