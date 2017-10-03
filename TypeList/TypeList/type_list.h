#pragma once
#include <type_traits>

namespace gc {
	template<class ... Args>
	class type_list;
	namespace priv {
		template<class T>
		struct is_type_list {
			static constexpr bool value = false;
		};
		template<class ... T>
		struct is_type_list<type_list<T ...>> {
			static constexpr bool value = true;
		};
	}
	template<class T>
	constexpr bool is_type_list_v = priv::is_type_list<T>::value;

	template<class ... Args>
	class type_list {
		//¯ \ _ (ツ) _ / ¯
		using empty_list = type_list<>;
	private:
		//+----------------------------------------------+
		//|-----------------at_t<size_t>-----------------|
		//+----------------------------------------------+
		template<size_t I, class T, class ... _Args>struct at_impl {
			using type = typename at_impl<I - 1, _Args ...>::type;};
		template<class T, class ... _Args> struct at_impl<0, T, _Args ...> {
			using type = T;};
		template<size_t I, class ... _Args> struct at_facade {
			static_assert(I < sizeof...(_Args), "in gc::type_list<T ...>::at_t<I> => I greater than (list::size_v - 1)");
			using type = typename at_impl<I, _Args ...>::type;};
	public:
		template<size_t I>
		using at_t = typename at_facade<I, Args ...>::type;

		static constexpr size_t size_v = sizeof...(Args);
		static constexpr bool is_empty_v = sizeof...(Args) == 0;
	private:
		//+-----------------------------------------------------+
		//|-----------------is_contain_v<T ...>-----------------|
		//+-----------------------------------------------------+
		template<class T, class Y, class ... _Args>
		struct is_contain_impl {
			static constexpr bool value = std::is_same<T, Y>::value ? true : is_contain_impl<T, _Args ...>::value;
		};
		template<class T, class Y>
		struct is_contain_impl<T, Y> {
			static constexpr bool value = std::is_same<T, Y>::value;
		};
		template<class T, class ... _Args>
		struct is_contain_facade {
			static constexpr bool value = sizeof...(_Args) == 0 ? false : is_contain_impl<T, _Args ...>::value;
		};
	public:
		template<class T>
		static constexpr bool is_contain_v = is_contain_facade<T, Args ...>::value;
	private:
		//+------------------------------------------------------------+
		//|-----------------is_contain_any_of_v<T ...>-----------------|
		//+------------------------------------------------------------+
		template<class L, class T, class ... _Args>
		struct is_contain_any_impl {};
		template<class ... A, class T, class ... _Args>
		struct is_contain_any_impl<type_list<A ...>, T, _Args ...> {
			static constexpr bool value = is_contain_facade<T, A...>::value || is_contain_any_impl<type_list<A ...>, _Args ...>::value;};
		template<class ... A, class T>
		struct is_contain_any_impl<type_list<A ...>, T> {
			static constexpr bool value = is_contain_facade<T, A...>::value;};
		template<class L, class ... _Args>
		struct is_contain_any_facade {};
		template<class ... A, class ... B>
		struct is_contain_any_facade<type_list<A ...>, B ...> {
			template<bool E>struct inner {static constexpr bool value = is_contain_any_impl<type_list<A ...>, B ...>::value;};
			template<> struct inner<true> {static constexpr bool value = false;};
			static constexpr bool value = inner<((sizeof...(A) == 0) || (sizeof...(B) == 0))>::value;};
	public:
		template<class ... Find>
		static constexpr bool is_contain_any_of_v = is_contain_any_facade<type_list<Find ...>, Args ...>::value;
	private:
		//+--------------------------------------------------------------------------------+
		//|-----------------is_intersect_v<type_list<T ...>>, add_t<T ...>-----------------|
		//+--------------------------------------------------------------------------------+
		template<class L, class ... _Args>
		//L - type_list
		struct is_intersect_facade {
			static_assert(is_type_list_v<L>, "in gc::type_list<T ...>::is_intersect_v<L> => L is not type_list");
			static constexpr bool value = L::is_contain_any_of_v<_Args ...>;
		};
	public:
		template<class L>
		static constexpr bool is_intersect_v = is_intersect_facade<L, Args ...>::value;
	private:
		//+--------------------------------------------------------------------------+
		//|-----------------append_t<type_list<T ...>>, add_t<T ...>-----------------|
		//+--------------------------------------------------------------------------+
		template<class T, class ... _Args>
		struct append_impl {
			static_assert(is_type_list_v<T>, "in gc::type_list<Args...>::append_t<T> T is not gc::type_list<...>");
		};
		template<class ... A, class ... _Args>
		struct append_impl<type_list<A ...>, _Args ...> {
			using type = type_list<_Args ..., A ...>;
		};
	public:
		template<class T>
		using append_t = typename append_impl<T, Args ...>::type;
		template<class ... T>
		using add_t = type_list<Args ..., T ...>;
	private:
		//+--------------------------------------------------------------------------+
		//|-----------------foreach_t<F<T1, T2>, Aggregator<V1, V2>>-----------------|
		//+--------------------------------------------------------------------------+
		template<template<class C> class F, template<class A, class B> class Aggregator, class T, class ... _Args>
		struct foreach_impl {
			static_assert(sizeof...(_Args) > 0, "in gc::type_list<Args...>::foreach<F, A> Args is empty");
			//Aggregate(F(a1), Aggregate(F(a2), Aggregate(F(a3), ...))
			//(f(a1) && (f(a2) && (f(a3) && (...)))
			using type = typename Aggregator<F<T>, typename foreach_impl<F, Aggregator, _Args ...>::type>;
		};
		template<template<class C> class F, template<class A, class B> class Aggregator, class T, class Y>
		struct foreach_impl<F, Aggregator, T, Y> {
			using type = typename Aggregator<F<T>, F<Y>>;
		};
		template<template<class C> class F, template<class A, class B> class Aggregator, class ... _Args>
		struct foreach_facade {
			static_assert(sizeof...(_Args) != 0, "some 1");
			using type = typename foreach_impl<F, Aggregator, _Args ...>::type;
		};
	public:
		template<template<class C> class F, template<class A, class B>class T>
		using foreach_t = typename foreach_facade<F, T, Args ...>::type;
	private:
		//+--------------------------------------------------+
		//|-----------------map_t<F<T1, T2>>------------------
		//+--------------------------------------------------+
		template<template<class T> class F, class T, class ... _Args>
		struct map_impl {
			using type = typename append_impl<typename map_impl<F, _Args...>::type, F<T>>::type;
		};
		template<template<class T> class F, class T>
		struct map_impl<F, T> {
			using type = typename type_list<F<T>>;
		};
		template<template<class T> class F, class ... _Args>
		struct map_facade {
			static_assert(sizeof...(_Args) != 0, "some 2");
			using type = typename map_impl<F, _Args ...>::type;
		};
	public:
		template<template<class T> class F>
		using map_t = typename map_facade<F, Args ...>::type;
	private:
		//+-----------------------------------------+
		//------------------print()------------------
		//+-----------------------------------------+
		template<class T, class ... _Args>
		struct print_impl {
			static void print() {
				std::cout << typeid(T).name() << ", ";
				print_impl<_Args ...>::print();}};
		template<class T>
		struct print_impl<T> {static void print() {std::cout << typeid(T).name();}};
		template<bool F, class ... _Args>
		struct print_facade {static void print() {print_impl<_Args ...>::print();}};
		template<class ... _Args>
		struct print_facade<true, _Args ...> {static void print() {}};
	public:
		static void print() {
			std::cout << '{';
			print_facade<sizeof...(Args) == 0, Args ...>::print();
			std::cout << '}';
		}
	private:
		//+-------------------------------------------------+
		//------------------remove_t<T ...>------------------
		//+-------------------------------------------------+
		template<class L, class E, class T, class ... _Args>
		//L - list of types which must be erased
		//E - empty_list (BICOZ)
		//T, _Args for iterating
		struct remove_impl {
			using next_list = typename remove_impl<L, E, _Args ...>::type;
			using on_true = typename E::template append_t<typename next_list>;
			using on_false = typename E::template add_t<T>::template append_t<typename next_list>;
			static constexpr bool v = L::is_contain_v<T>;
			using type = std::conditional_t<v, on_true, on_false>;
		};
		template<class L, class E, class T>
		//L - list of types which must be erased
		//E - empty_list (BICOZ)
		//T - last element
		struct remove_impl<L, E, T> {
			static constexpr bool v = L::is_contain_v<T>;
			using type = std::conditional_t<v, E, typename E::template add_t<T>>;
		};
		template<class L, class ... _Args>
		struct remove_facade {
			//to dispatch if _Args is empty
			template<size_t A> struct inner {using type = typename remove_impl<L, empty_list, _Args ...>::type;};
			template<> struct inner<0> {using type = empty_list;};
			using type = typename inner<sizeof...(_Args)>::type;
		};
	public:
		template<class ... T>
		using remove_t = typename remove_facade<type_list<T ...>, Args ...>::type;
		//+-----------------------------------------------------+
		//------------------transform<F<T ...>>------------------
		//+-----------------------------------------------------+
		template<template<class ... Y>class T>
		using transform_t = T<Args ...>;
	private:
		//+-----------------------------------------------------+
		//|-----------------remove_if_t<F<T ...>>---------------|
		//+-----------------------------------------------------+
		template<template<class T> class F, class E, class T, class ... _Args>
		//L - list of types which must be erased
		//E - empty_list (BICOZ)
		//T, _Args for iterating
		struct remove_if_impl {
			using next_list = typename remove_if_impl<F, E, _Args ...>::type;
			using on_true = typename E::template append_t<typename next_list>;
			using on_false = typename E::template add_t<T>::template append_t<typename next_list>;
			static constexpr bool v = F<T>::value;
			using type = std::conditional_t<v, on_true, on_false>;
		};
		template<template<class T> class F, class E, class T>
		//L - list of types which must be erased
		//E - empty_list (BICOZ)
		//T - last element
		struct remove_if_impl<F, E, T> {
			static constexpr bool v = F<T>::value;
			using type = std::conditional_t<v, E, typename E::template add_t<T>>;
		};
		template<template<class T> class F, class ... _Args>
		struct remove_if_facade {
			//to dispatch if _Args is empty
			template<size_t A> struct inner { using type = typename remove_if_impl<F, empty_list, _Args ...>::type; };
			template<> struct inner<0> { using type = empty_list; };
			using type = typename inner<sizeof...(_Args)>::type;
		};
		//+---------------------------------------------------------+
		//|-----------------remove_if_not_t<F<T ...>>---------------|
		//+---------------------------------------------------------+
		template<template<class T> class F, class E, class T, class ... _Args>
		//L - list of types which must be erased
		//E - empty_list (BICOZ)
		//T, _Args for iterating
		struct remove_if_not_impl {
			using next_list = typename remove_if_not_impl<F, E, _Args ...>::type;
			using on_true = typename E::template append_t<typename next_list>;
			using on_false = typename E::template add_t<T>::template append_t<typename next_list>;
			static constexpr bool v = !F<T>::value;
			using type = std::conditional_t<v, on_true, on_false>;
		};
		template<template<class T> class F, class E, class T>
		//L - list of types which must be erased
		//E - empty_list (BICOZ)
		//T - last element
		struct remove_if_not_impl<F, E, T> {
			static constexpr bool v = !F<T>::value;
			using type = std::conditional_t<v, E, typename E::template add_t<T>>;
		};
		template<template<class T> class F, class ... _Args>
		struct remove_if_not_facade {
			//to dispatch if _Args is empty
			template<size_t A> struct inner { using type = typename remove_if_not_impl<F, empty_list, _Args ...>::type; };
			template<> struct inner<0> { using type = empty_list; };
			using type = typename inner<sizeof...(_Args)>::type;
		};
	public:
		template<template<class T> class F>
		using remove_if_t = typename remove_if_facade<F, Args ...>::type;

		template<template<class T> class F>
		using remove_if_not_t = typename remove_if_not_facade<F, Args ...>::type;
	};
}