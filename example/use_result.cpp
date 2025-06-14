#include "result.hpp"

#include <string>
#include <iostream>
#include <cassert>

using namespace cmdkit;
using R = Result<int, std::string>;

const std::string example_err_str("y shouldn't be zero!");

R div_function(int x, int y)
{
	if (y == 0) return R::err(example_err_str);
	else return R::ok(x / y);
}

int main()
{
	// Result::ok(), Result::err()
	R result1 = div_function(2, 1);
	R result2 = div_function(10, 0);

	// is_ok(), is_err()
	assert(result1.is_ok());
	assert(!result1.is_err());
	assert(!result2.is_ok());
	assert(result2.is_err());

	// ok.unwrap(), ok.unwrap_err()
	assert(result1.unwrap() == 2);
	try { result1.unwrap_err(); } catch (bad_result_access e) {}

	// err.unwrap(), err.unwrap_err()
	try { result2.unwrap(); } catch (bad_result_access e) {}
	assert(result2.unwrap_err() == example_err_str);
	
	// map()
	{
		auto func = [](int x){ return x + 1; };
		auto mapped_result1 = result1.map(func);
		assert(mapped_result1.unwrap() == 3);
	}

	// map_err()
	{
		auto func = [](const std::string& str) { return str.size(); };
		auto mapped_result2 = result2.map_err(func);
		assert(mapped_result2.unwrap_err() == example_err_str.size());
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
		auto and_then_result1 = result1.and_then(func);
		assert(and_then_result1.is_ok());
		assert(and_then_result1.unwrap() == 1.0);

		auto and_then_result2 = result2.and_then(func);
		assert(and_then_result2.is_err());
		assert(and_then_result2.unwrap_err() == example_err_str);
	}

	// match()
	{
		auto func1 = [](int x) { return true; };
		auto func2 = [](const std::string&) {return false; };

		auto match_result1 = result1.match(func1, func2);
		assert(match_result1);

		auto match_result2 = result2.match(func1, func2);
		assert(!match_result2);
	}

	std::cout << "Result examples all passed!" << std::endl;
	getchar();
}