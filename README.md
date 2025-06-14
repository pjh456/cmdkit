# cmdkit - Lightweight Result and Command Line Utilities for Morden C++

**cmdkit** is a lightweight, header-only C++17 command-line argument parsing library focused on simplicity, clarity, and extensibility.

It supports only two basic and unambiguous patterns:

- `--flag` (boolean switch)
- `--param value` (named value)

This focused design allows for predictable behavior and easy integration into small tools or large applications alike.

---

## ✨ Features

- 🧩 **Modular**: Use full kit or include only the parts you need ([result](include/result.hpp), [command](include/command.hpp), [terminal](include/terminal.hpp))
- ⚙️ **Simple semantics**: No POSIX-style quirks, just clear `--param` and `--flag` support
- 🎯 **Strong typing**: Uses a modern `Result<T, E>` pattern for error handling
- 🪶 **Header-only**: Easy to include, no linking or setup required
- 🧠 **C++17+**: Clean, modern codebase using `variant`, `invoke`, etc.
- 📂 **CMake-based**: Ready for direct integration into your build system.

---

## 🛠️ Getting Started

### 🔗 Integration

You can either:

- Copy the single header [cmdkit.hpp](include/cmdkit.hpp) to your project, or
- Include individual headers from the modular version in [include](include)

### 📦 CMake

```cmake
add_subdirectory(cmdkit)
target_link_libraries(your_project PRIVATE CMDKIT)
```

Or simply add the header to your include path if using the single-header variant.

### 🧪 Basic Usage

#### Defining Commands

```cpp
#include "cmdkit.hpp"

int main()
{
    cmdkit::Command str_printer(
		"print", // Define the name of command.
		[](const CommandArgs& args) // lambda function when invoked.
		{
			const auto& vec = args.get_positional();
			for (size_t idx = 1; idx < vec.size(); ++idx) std::cout << vec[idx] << " ";
			std::cout << std::endl;
			return R::ok(nullptr);
		}
	);

    str_printer.invoke("print Hello world C++!"); // Invoke str_printer
    // Output: Hello world C++!
}
```

#### Using Result

```cpp
#include "cmdkit.hpp"
#include <string>
#include <cassert>

using R = cmdkit::Result<int, std::string>;

R div_function(int x, int y)
{
	if (y == 0) return R::err(example_err_str);
	else return R::ok(x / y);
}

int main()
{
    R result = div_function(2, 1);

    // ok.unwrap(), ok.unwrap_err()
	assert(result.unwrap() == 2);
	try { result.unwrap_err(); } catch (bad_result_access e) {}

    // map()
	{
		auto func = [](int x){ return x + 1; };
		auto mapped_result = result.map(func);
		assert(mapped_result.unwrap() == 3);
	}

    // and_then()
	{
		const std::string new_err_str = "Error";
		auto func = 
			[&new_err_str](int x)
			{
				using Ret = Result<float, std::string>;
				if (x == 2) return Ret::ok(1.0);
				else return Ret::err(new_err_str);
			};
		auto and_then_result = result.and_then(func);
		assert(and_then_result.is_ok());
		assert(and_then_result.unwrap() == 1.0);
	}

	// match()
	{
		auto func1 = [](int x) { return true; };
		auto func2 = [](const std::string&) {return false; };

		auto match_result = result.match(func1, func2);
		assert(match_result);
	}
}
```

#### Using Terminal

```cpp
#include "cmdkit.hpp"

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
	terminal.invoke("print Hello world C++!", func); // Output: Hello world C++!
	terminal.invoke("help", func); // Output: Can't find command!

	getchar();
}
```

More examples is included in [example](example)

### 📁 Project Structure
```makefile
cmdkit/
├── include/
│   ├── command.hpp
│   ├── result.hpp
│   ├── terminal.hpp
│   └── cmdkit.hpp           # Single-header version (aggregated)
├── example/
│   └── *.cpp                # Usage examples
├── CMakeLists.txt
└── README.md
```

### 📚 Examples

#### Examples are provided in the example/ folder and cover:

- Multiple command dispatch
- Flag parsing
- Parameter value passing
- Command-specific logic
- Result-based error handling

#### To build the examples: it depends on yourself!

### 🧩 Modular Design

You can include just what you need:

- [result.hpp](include/result.hpp): A minimal `Result<T, E>` monadic type for error/value wrapping, not supporting T/E = void in order to minimize it.

- [command.hpp](include/command.hpp): Command abstraction with argument parsing

- [terminal.hpp](include/terminal.hpp): Full CLI dispatcher and entrypoint

Or use the aggregated header [cmdkit.hpp](include/cmdkit.hpp) for everything.

### ⚖ License

This project is licensed under the **MIT License**.

You are free to use it in personal, academic, or commercial projects.

### 💬 Feedback & Contribution

Feel free to open issues, suggest improvements, or contribute via pull requests.

If you find this project useful, a star would be appreciated ⭐

### 🙌 Acknowledgments

Designed with clarity and composability in mind.

cmdkit aims to reduce the mental burden of building CLI tools without sacrificing expressiveness.

---