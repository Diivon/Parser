#pragma once
#include "Lexem.h"

namespace gc {
	template<class Settings>
	class Parser {
		std::string_view _data;

		static bool is_white_space(char c) {
			switch (c)
			{
			case ' ': return true;
			case '\t': return true;
			case '\n': return true;
			case '\r': return true;
			case '\0': return true;
			case ':': return true;
			}
			return false;
		}

		template<class F>
		static size_t find_first_if(std::string_view s, F && f) {
			for (size_t i = 0; i < s.size(); ++i) {
				if (f(s[i]))
					return i;
			}
			return std::string_view::npos;
		}

		template<bool is_lexem>
		struct _parser_helper {
			template<class T>
			static std::tuple<T> _parse(T && t, std::string_view & data) {
				size_t suffix = find_first_if(data, [](char c) { return is_white_space(c); });

				T lex{ data.substr(0, suffix) };
				data.remove_prefix(lex._data.size());

				std::cout << '|' << lex._data << '|';

				return std::tuple<T>{ lex };
			}
		};
		template<>
		struct _parser_helper<false> {
			static std::tuple<> _parse(std::string const & t, std::string_view & data) {
				std::cout << data << '!';
				for (size_t i = 0; i < t.size(); ++i)
					if (t[i] == data[i])
						data.remove_prefix(1);
					else
						throw std::exception("RAZ");
				return {};
			}
			template<size_t C>
			static std::tuple<> _parse(char (*t)[C], std::string_view & data) {
				std::cout << t << '!';
				for (size_t i = 0; i < C; ++i)
					if (t[i] == data[i])
						data.remove_prefix(1);
					else
						throw std::exception("DVA");
				return {};
			}
			static std::tuple<> _parse(char t, std::string_view & data) {
				std::cout << t << '!';
				if (t == data[0])
					data.remove_prefix(1);
				else
					throw std::exception("TRI");
				return {};
			}
		};
		template<class T, class ... Args>
		auto parse(T && t, Args && ... args) {
			static constexpr bool flag = Settings::lexem_list::is_contain_v<std::decay_t<T>>;
			return std::tuple_cat(
				parse(std::forward<Args>(args)...),
				_parser_helper<flag>::_parse(std::forward<T>(t), _data)
			);
		}
		template<class T>
		auto parse(T && t) {
			static constexpr bool flag = Settings::lexem_list::is_contain_v<std::decay_t<T>>;
			return _parser_helper<flag>::_parse(std::forward<T>(t), _data);
		}
	public:
		template<class T>
		Parser & match_string(T && v) {
			_data = v;
			return *this;
		}
		std::tuple<> match_as() {
			return {};
		}
		template<class ... Args>
		auto match_as(Args && ... args) {
			return parse(std::forward<Args>(args)...);
		}
	};
}