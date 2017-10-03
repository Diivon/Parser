#pragma once
#include <string_view>

namespace gc {
	struct Lexem {
		std::string_view _data;
		Lexem(std::string_view d) :
			_data{ d }
		{}
		Lexem() {}
	};
}