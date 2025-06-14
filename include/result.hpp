#ifndef INCLUDE_CMDKIT_RESULT
#define INCLUDE_CMDKIT_RESULT

#include <type_traits>
#include <stdexcept>
#include <variant>

namespace cmdkit
{
	template<typename T, typename E>
	class Result;

	template<typename>
	struct is_result : std::false_type {};

	template<typename T, typename E>
	struct is_result<Result<T, E>> : std::true_type {};

	class bad_result_access : public std::logic_error
	{
	public:
		explicit bad_result_access(const std::string& msg) : std::logic_error(msg) {}
		explicit bad_result_access(const char* msg) : std::logic_error(msg) {}
	};

	template<typename T, typename E>
	class Result
	{
		static_assert(!std::is_same_v<T, E>, "Result<T, E>: T and E must not be the same type");
	public:
		using OkType = T;
		using ErrType = E;

	private:
		bool ok_flag;
		std::variant<T, E> data;

		Result(const T& val) : data(val), ok_flag(true) {}
		Result(T&& val) noexcept : data(std::move(val)), ok_flag(true) {}

		Result(const E& val) : data(val), ok_flag(false) {}
		Result(E&& val) noexcept : data(std::move(val)), ok_flag(false) {}

	public:
		static Result<T, E> ok(const T& val) { return Result<T, E>(val); }
		static Result<T, E> ok(T&& val) noexcept { return Result<T, E>(std::move(val)); }

		static Result<T, E> err(const E& val) { return Result<T, E>(val); }
		static Result<T, E> err(E&& val) noexcept { return Result<T, E>(std::move(val)); }

		~Result() = default;

	public:
		Result& operator=(const Result& other) { ok_flag = other.ok_flag, data = other.data; return *this; }

		Result& operator=(Result&& other) noexcept { ok_flag = other.ok_flag, data = std::move(other.data); return *this; }

	public:
		bool is_ok() const noexcept { return ok_flag; }
		bool is_err() const noexcept { return !ok_flag; }
		operator bool() const { return is_ok(); }

	public:
		T& unwrap()& { if (is_ok()) return std::get<T>(data); else throw bad_result_access("unwrap called on an error Result"); }
		const T& unwrap() const& { if (is_ok()) return std::get<T>(data); else throw bad_result_access("unwrap called on an error Result"); }
		T unwrap()&& { if (is_ok()) return std::move(std::get<T>(data)); else throw bad_result_access("unwrap called on an error Result"); }

		E& unwrap_err()& { if (is_err()) return std::get<E>(data); else throw bad_result_access("unwrap_err called on an okay Result"); }
		const E& unwrap_err() const& { if (is_err()) return std::get<E>(data); else throw bad_result_access("unwrap_err called on an okay Result"); }
		E unwrap_err()&& { if (is_err()) return std::move(std::get<E>(data)); else throw bad_result_access("unwrap_err called on an okay Result"); }

	public:
		T& unwrap_or(const T& default_val) & noexcept { return is_ok() ? unwrap() : default_val; }
		const T& unwrap_or(const T& default_val) const& noexcept { return is_ok() ? unwrap() : default_val; }
		T unwrap_or(T default_val) && noexcept { return is_ok() ? std::move(unwrap()) : default_val; }

		E& unwrap_err_or(const E& default_val) & noexcept { return is_err() ? unwrap_err() : default_val; }
		const E& unwrap_err_or(const E& default_val) const& noexcept { return is_err() ? unwrap_err() : default_val; }
		E unwrap_err_or(E default_val) && noexcept { return is_err() ? std::move(unwrap_err()) : default_val; }

	public:
		template<typename F>
		auto map(F&& f) const& ->
			std::enable_if_t<
			std::is_invocable_v<F, const T&>,
			Result<decltype(f(std::declval<const T&>())), E>
			>
		{
			using U = decltype(f(std::declval<const T&&>()));
			if (is_ok()) return Result<U, E>::ok(f(unwrap()));
			else return Result<U, E>::err(std::move(unwrap_err()));
		}

		template<typename F>
		auto map(F&& f) && ->
			std::enable_if_t<
			std::is_invocable_v<F, T&&>,
			Result<decltype(f(std::declval<T&&>())), E>
			>
		{
			using U = decltype(f(std::declval<T&&>()));
			if (is_ok()) return Result<U, E>::ok(f(std::move(unwrap())));
			else return Result<U, E>::err(std::move(unwrap_err()));
		}

	public:
		template<typename F>
		auto map_err(F&& f) const& ->
			std::enable_if_t<
			std::is_invocable_v<F, const E&>,
			Result<T, decltype(f(std::declval<const E&>()))>
			>
		{
			using U = decltype(f(std::declval<const E&>()));
			if (is_err()) return Result<T, U>::err(f(unwrap_err()));
			else return Result<T, U>::ok(std::move(unwrap()));
		}

		template<typename F>
		auto map_err(F&& f) && ->
			std::enable_if_t<
			std::is_invocable_v<F, E&&>,
			Result<T, decltype(f(std::declval<E&&>()))>
			>
		{
			using U = decltype(f(std::declval<E&&>()));
			if (is_err()) return Result<T, U>::err(f(std::move(unwrap_err())));
			else return Result<T, U>::ok(std::move(unwrap()));
		}

	public:
		template<typename F>
		auto and_then(F&& f) const& ->
			std::enable_if_t<
			is_result<std::invoke_result_t<F, const T&>>::value
			&& std::is_same_v<E, typename std::invoke_result_t<F, const T&>::ErrType>,
			std::invoke_result_t<F, const T&>
			>
		{
			using Ret = std::invoke_result_t<F, const T&>;
			if (is_ok()) return std::invoke(std::forward<F>(f), unwrap());
			else return Ret::err(unwrap_err());
		}

		template<typename F>
		auto and_then(F&& f) && ->
			std::enable_if_t<
			is_result<std::invoke_result_t<F, const T&>>::value
			&& std::is_same_v<E, typename std::invoke_result_t<F, const T&>::ErrType>,
			std::invoke_result_t<F, const T&>
			>
		{
			using Ret = std::invoke_result_t<F, const T&>;
			if (is_ok()) return std::invoke(std::forward<F>(f), std::move(unwrap()));
			else return Ret::err(std::move(unwrap_err()));
		}

	public:
		template<
			typename OkFn, typename ErrFn,
			typename OkRet = std::invoke_result_t<OkFn, const T&>,
			typename ErrRet = std::invoke_result_t<ErrFn, const E&>,
			typename std::enable_if_t<std::is_same_v<OkRet, ErrRet>, int> = 0
		>
		decltype(auto) match(OkFn&& ok_func, ErrFn&& err_func) const&
		{
			if (is_ok()) return std::invoke(std::forward<OkFn>(ok_func), unwrap());
			else return std::invoke(std::forward<ErrFn>(err_func), unwrap_err());
		}

		template<
			typename OkFn, typename ErrFn,
			typename OkRet = std::invoke_result_t<OkFn, T&&>,
			typename ErrRet = std::invoke_result_t<ErrFn, E&&>,
			typename std::enable_if_t<std::is_same_v<OkRet, ErrRet>, int> = 0
		>
		decltype(auto) match(OkFn&& ok_func, ErrFn&& err_func)&&
		{
			if (is_ok()) return std::invoke(std::forward<OkFn>(ok_func), std::move(unwrap()));
			else return std::invoke(std::forward<ErrFn>(err_func), std::move(unwrap_err()));
		}
	};
}
	
#endif // INCLUDE_CMDKIT_RESULT