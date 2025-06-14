#include "terminal.hpp"

#include <iostream>
#include <string>
#include <vector>

using namespace cmdkit;
using R = Result<void*, std::string>;
using C = Command;
using T = Terminal;

int main()
{
	T terminal;

	C str_printer(
		"print",
		[](const CommandArgs& args)
		{
			const auto& vec = args.get_positional();
			for (size_t idx = 1; idx < vec.size(); ++idx) std::cout << vec[idx] << " ";
			std::cout << std::endl;
			return R::ok(nullptr);
		}
	);

	auto func = []() {std::cout << "Can't find command!" << std::endl; };
	terminal.register_command(str_printer);
	terminal.invoke("print Hello world C++!", func);
	terminal.invoke("help", func);

	getchar();
}