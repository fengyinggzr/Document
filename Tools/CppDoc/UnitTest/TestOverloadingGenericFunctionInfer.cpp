#include "Util.h"

namespace Input__TestOverloadingGenericFunction_TypeInferSimple
{
	TEST_DECL(
		const int ci = 0;
		volatile int vi = 0;
		const volatile int cvi = 0;

		template<typename T>
		auto Simple(T t) { return t; }

		template<typename T>
		auto Simple2(T a, T b = {}) { return a; }
		void Simple2(...) {}
	);
}

namespace Input__TestOverloadingGenericFunction_TypeInferVariant
{
	TEST_DECL(
		const int ci = 0;
		volatile int vi = 0;
		const volatile int cvi = 0;

		template<typename... Ts>
		struct Types {};

		template<typename... Ts>
		auto Variant(Ts... ts) { return Types<Ts...>(); }
	);
}

namespace Input__TestOverloadingGenericFunction_TypeInferKinds
{
	TEST_DECL(
		template<typename... Ts>
		struct Types {};

		template<typename T>
		T Value();

		template<typename R, typename... TArgs>
		using FunctionOf = R(*)(TArgs...);

		template<typename T, typename U>
		using MemberOf = T U::*;

		template<typename T>							auto LRef(T&)								-> Types<T>					;
		template<typename T>							auto RRef(T&&)								-> Types<T>					;
		template<typename T>							auto Pointer(T*)							-> Types<T>					;
		template<typename T>							auto C(const T)								-> Types<T>					;
		template<typename T>							auto V(volatile T)							-> Types<T>					;
		template<typename T>							auto CV(const volatile T)					-> Types<T>					;
		template<typename R, typename... TArgs>			auto Function(R(*)(TArgs...))				-> Types<R, TArgs...>		;
		template<typename T, typename U>				auto Member(T U::*)							-> Types<T, U>				;
		template<typename... TArgs>						auto VtaPtr(TArgs*...)						-> Types<TArgs...>			;
		template<typename... TArgs>						auto VtaTypes(Types<TArgs...>)				-> Types<TArgs...>			;
		template<typename R, typename... TArgs>			auto VtaFunc(Types<R(*)(TArgs*)...>)		-> Types<R, TArgs...>		;
		template<typename... TRs, typename... TArgs>	auto VtaFunc2(Types<TRs&(*)(TArgs*)...>)	-> Types<TRs..., TArgs...>	;
	);
}

namespace Input__TestOverloadingGenericFunction_TypeInferChildTypes
{
	TEST_DECL(
		template<typename... Ts>
		struct Types {};

		template<typename TA, typename TB>
		struct A
		{
			struct _
			{
				template<typename TC, typename... Ts>
				struct B
				{
				};
			};
		};

		template<typename TA>
		struct C : A<TA*, TA&>
		{
			struct D
				: A<TA*, TA&>::_::template B<const TA, volatile TA, const volatile TA>
				, A<TA&, TA*>::_::template B<TA>
				, A<float, double>::_::B<char, bool, void>
			{
			};
		};

		template<template<typename, typename...> class X, typename TC, typename... Ts>
		auto UseX(X<TC, Ts...>)->Types<X<TC, Ts...>, TC, Ts...>;

		template<typename TA, typename TB>
		auto UseA(A<TA, TB>)->Types<TA, TB>;

		template<typename TA, typename TB, typename TC, typename... Ts>
		auto UseB(typename A<TA, TB>::_::template B<TC, Ts...>)->Types<TA, TB, TC, Ts...>;

		template<typename TC, typename... Ts>
		auto UseB(A<float, double>::_::B<TC, Ts...>)->Types<TC, Ts...>;
	);
}

TEST_FILE
{
	TEST_CATEGORY(L"Partially apply template arguments (simple)")
	{
		using namespace Input__TestOverloadingGenericFunction_TypeInferSimple;
		COMPILE_PROGRAM(program, pa, input);

		AssertExpr(pa, L"Simple",				L"Simple",					L"<::Simple::[T]> ::Simple::[T] __cdecl(::Simple::[T]) * $PR");
		AssertExpr(pa, L"Simple<>",				L"Simple<>",				L"<::Simple::[T]> ::Simple::[T] __cdecl(::Simple::[T]) * $PR");
		AssertExpr(pa, L"Simple<bool>",			L"Simple<bool>",			L"bool __cdecl(bool) * $PR");
		AssertExpr(pa, L"Simple2",				L"Simple2",					L"<::Simple2::[T]> ::Simple2::[T] __cdecl(::Simple2::[T], ::Simple2::[T]) * $PR", L"void __cdecl(...) * $PR");
		AssertExpr(pa, L"Simple2<>",			L"Simple2<>",				L"<::Simple2::[T]> ::Simple2::[T] __cdecl(::Simple2::[T], ::Simple2::[T]) * $PR");
		AssertExpr(pa, L"Simple2<bool>",		L"Simple2<bool>",			L"bool __cdecl(bool, bool) * $PR");
		// test value argument
	});

	TEST_CATEGORY(L"Partially apply template arguments (variant)")
	{
		using namespace Input__TestOverloadingGenericFunction_TypeInferVariant;
		COMPILE_PROGRAM(program, pa, input);
		
		AssertExpr(pa, L"Variant",				L"Variant",					L"<...::Variant::[Ts]> any_t $PR");
		AssertExpr(pa, L"Variant<>",			L"Variant<>",				L"::Types<{}> __cdecl() * $PR");
		AssertExpr(pa, L"Variant<bool>",		L"Variant<bool>",			L"::Types<{bool $PR}> __cdecl(bool) * $PR");
		AssertExpr(pa, L"Variant<bool, char>",	L"Variant<bool, char>",		L"::Types<{bool $PR, char $PR}> __cdecl(bool, char) * $PR");
	});

	TEST_CATEGORY(L"Partially apply template arguments (kinds)")
	{
		using namespace Input__TestOverloadingGenericFunction_TypeInferKinds;
		COMPILE_PROGRAM(program, pa, input);
		
		AssertExpr(pa, L"LRef",								L"LRef",							L"<::LRef::[T]> ::Types<{::LRef::[T] $PR}> __cdecl(::LRef::[T] &) * $PR");
		AssertExpr(pa, L"LRef<>",							L"LRef<>",							L"<::LRef::[T]> ::Types<{::LRef::[T] $PR}> __cdecl(::LRef::[T] &) * $PR");
		AssertExpr(pa, L"LRef<bool>",						L"LRef<bool>",						L"::Types<{bool $PR}> __cdecl(bool &) * $PR");

		AssertExpr(pa, L"RRef",								L"RRef",							L"<::RRef::[T]> ::Types<{::RRef::[T] $PR}> __cdecl(::RRef::[T] &&) * $PR");
		AssertExpr(pa, L"RRef<>",							L"RRef<>",							L"<::RRef::[T]> ::Types<{::RRef::[T] $PR}> __cdecl(::RRef::[T] &&) * $PR");
		AssertExpr(pa, L"RRef<bool>",						L"RRef<bool>",						L"::Types<{bool $PR}> __cdecl(bool &&) * $PR");

		AssertExpr(pa, L"Pointer",							L"Pointer",							L"<::Pointer::[T]> ::Types<{::Pointer::[T] $PR}> __cdecl(::Pointer::[T] *) * $PR");
		AssertExpr(pa, L"Pointer<>",						L"Pointer<>",						L"<::Pointer::[T]> ::Types<{::Pointer::[T] $PR}> __cdecl(::Pointer::[T] *) * $PR");
		AssertExpr(pa, L"Pointer<bool>",					L"Pointer<bool>",					L"::Types<{bool $PR}> __cdecl(bool *) * $PR");

		AssertExpr(pa, L"Function",							L"Function",						L"<::Function::[R], ...::Function::[TArgs]> ::Types<any_t> __cdecl(any_t) * $PR");
		AssertExpr(pa, L"Function<>",						L"Function<>",						L"<::Function::[R], ...::Function::[TArgs]> ::Types<any_t> __cdecl(any_t) * $PR");
		AssertExpr(pa, L"Function<bool>",					L"Function<bool>",					L"::Types<{bool $PR}> __cdecl(bool __cdecl() *) * $PR");
		AssertExpr(pa, L"Function<bool, char>",				L"Function<bool, char>",			L"::Types<{bool $PR, char $PR}> __cdecl(bool __cdecl(char) *) * $PR");
		AssertExpr(pa, L"Function<bool, char, float>",		L"Function<bool, char, float>",		L"::Types<{bool $PR, char $PR, float $PR}> __cdecl(bool __cdecl(char, float) *) * $PR");

		AssertExpr(pa, L"Member",							L"Member",							L"<::Member::[T], ::Member::[U]> ::Types<{::Member::[T] $PR, ::Member::[U] $PR}> __cdecl(::Member::[T] (::Member::[U] ::) *) * $PR");
		AssertExpr(pa, L"Member<>",							L"Member<>",						L"<::Member::[T], ::Member::[U]> ::Types<{::Member::[T] $PR, ::Member::[U] $PR}> __cdecl(::Member::[T] (::Member::[U] ::) *) * $PR");
		AssertExpr(pa, L"Member<bool>",						L"Member<bool>",					L"<=bool, ::Member::[U]> ::Types<{bool $PR, ::Member::[U] $PR}> __cdecl(bool (::Member::[U] ::) *) * $PR");
		AssertExpr(pa, L"Member<bool, Types<>>",			L"Member<bool, Types<>>",			L"::Types<{bool $PR, ::Types<{}> $PR}> __cdecl(bool (::Types<{}> ::) *) * $PR");

		AssertExpr(pa, L"C",								L"C",								L"<::C::[T]> ::Types<{::C::[T] $PR}> __cdecl(::C::[T] const) * $PR");
		AssertExpr(pa, L"C<>",								L"C<>",								L"<::C::[T]> ::Types<{::C::[T] $PR}> __cdecl(::C::[T] const) * $PR");
		AssertExpr(pa, L"C<bool>",							L"C<bool>",							L"::Types<{bool $PR}> __cdecl(bool const) * $PR");

		AssertExpr(pa, L"V",								L"V",								L"<::V::[T]> ::Types<{::V::[T] $PR}> __cdecl(::V::[T] volatile) * $PR");
		AssertExpr(pa, L"V<>",								L"V<>",								L"<::V::[T]> ::Types<{::V::[T] $PR}> __cdecl(::V::[T] volatile) * $PR");
		AssertExpr(pa, L"V<bool>",							L"V<bool>",							L"::Types<{bool $PR}> __cdecl(bool volatile) * $PR");

		AssertExpr(pa, L"CV",								L"CV",								L"<::CV::[T]> ::Types<{::CV::[T] $PR}> __cdecl(::CV::[T] const volatile) * $PR");
		AssertExpr(pa, L"CV<>",								L"CV<>",							L"<::CV::[T]> ::Types<{::CV::[T] $PR}> __cdecl(::CV::[T] const volatile) * $PR");
		AssertExpr(pa, L"CV<bool>",							L"CV<bool>",						L"::Types<{bool $PR}> __cdecl(bool const volatile) * $PR");

		AssertExpr(pa, L"VtaPtr",							L"VtaPtr",							L"<...::VtaPtr::[TArgs]> any_t $PR");
		AssertExpr(pa, L"VtaPtr<>",							L"VtaPtr<>",						L"::Types<{}> __cdecl() * $PR");
		AssertExpr(pa, L"VtaPtr<bool>",						L"VtaPtr<bool>",					L"::Types<{bool $PR}> __cdecl(bool *) * $PR");
		AssertExpr(pa, L"VtaPtr<bool, char>",				L"VtaPtr<bool, char>",				L"::Types<{bool $PR, char $PR}> __cdecl(bool *, char *) * $PR");

		AssertExpr(pa, L"VtaTypes",							L"VtaTypes",						L"<...::VtaTypes::[TArgs]> ::Types<any_t> __cdecl(::Types<any_t>) * $PR");
		AssertExpr(pa, L"VtaTypes<>",						L"VtaTypes<>",						L"::Types<{}> __cdecl(::Types<{}>) * $PR");
		AssertExpr(pa, L"VtaTypes<bool>",					L"VtaTypes<bool>",					L"::Types<{bool $PR}> __cdecl(::Types<{bool $PR}>) * $PR");
		AssertExpr(pa, L"VtaTypes<bool, char>",				L"VtaTypes<bool, char>",			L"::Types<{bool $PR, char $PR}> __cdecl(::Types<{bool $PR, char $PR}>) * $PR");

		AssertExpr(pa, L"VtaFunc",							L"VtaFunc",							L"<::VtaFunc::[R], ...::VtaFunc::[TArgs]> ::Types<any_t> __cdecl(::Types<any_t>) * $PR");
		AssertExpr(pa, L"VtaFunc<>",						L"VtaFunc<>",						L"<::VtaFunc::[R], ...::VtaFunc::[TArgs]> ::Types<any_t> __cdecl(::Types<any_t>) * $PR");
		AssertExpr(pa, L"VtaFunc<bool>",					L"VtaFunc<bool>",					L"::Types<{bool $PR}> __cdecl(::Types<{}>) * $PR");
		AssertExpr(pa, L"VtaFunc<bool, char>",				L"VtaFunc<bool, char>",				L"::Types<{bool $PR, char $PR}> __cdecl(::Types<{bool __cdecl(char *) * $PR}>) * $PR");
		AssertExpr(pa, L"VtaFunc<bool, char, float>",		L"VtaFunc<bool, char, float>",		L"::Types<{bool $PR, char $PR, float $PR}> __cdecl(::Types<{bool __cdecl(char *) * $PR, bool __cdecl(float *) * $PR}>) * $PR");

		AssertExpr(pa, L"VtaFunc2",							L"VtaFunc2",						L"<...::VtaFunc2::[TRs], ...::VtaFunc2::[TArgs]> ::Types<any_t> __cdecl(::Types<any_t>) * $PR");
		AssertExpr(pa, L"VtaFunc2<>",						L"VtaFunc2<>",						L"<...={}, ...::VtaFunc2::[TArgs]> ::Types<any_t> __cdecl(::Types<any_t>) * $PR");
		AssertExpr(pa, L"VtaFunc2<bool>",					L"VtaFunc2<bool>",					L"<...={bool $PR}, ...::VtaFunc2::[TArgs]> ::Types<any_t> __cdecl(::Types<any_t>) * $PR");
		AssertExpr(pa, L"VtaFunc2<bool, char>",				L"VtaFunc2<bool, char>",			L"<...={bool $PR, char $PR}, ...::VtaFunc2::[TArgs]> ::Types<any_t> __cdecl(::Types<any_t>) * $PR");
	});

	TEST_CATEGORY(L"Template argument deduction (simple)")
	{
		using namespace Input__TestOverloadingGenericFunction_TypeInferSimple;
		COMPILE_PROGRAM(program, pa, input);

		ASSERT_OVERLOADING_SIMPLE(Simple(1),					__int32);
		ASSERT_OVERLOADING_SIMPLE(Simple(1.0),					double);
		ASSERT_OVERLOADING_SIMPLE(Simple(1.f),					float);
		ASSERT_OVERLOADING_SIMPLE(Simple(ci),					__int32);
		ASSERT_OVERLOADING_SIMPLE(Simple(vi),					__int32);
		ASSERT_OVERLOADING_SIMPLE(Simple(cvi),					__int32);

		ASSERT_OVERLOADING_SIMPLE(Simple2(1),					__int32);
		ASSERT_OVERLOADING_SIMPLE(Simple2(1.0),					double);
		ASSERT_OVERLOADING_SIMPLE(Simple2(1.f),					float);
		ASSERT_OVERLOADING_SIMPLE(Simple2(ci),					__int32);
		ASSERT_OVERLOADING_SIMPLE(Simple2(vi),					__int32);
		ASSERT_OVERLOADING_SIMPLE(Simple2(cvi),					__int32);

		ASSERT_OVERLOADING_SIMPLE(Simple2(1, 2),				__int32);
		ASSERT_OVERLOADING_SIMPLE(Simple2(1.0, 2.0),			double);
		ASSERT_OVERLOADING_SIMPLE(Simple2(1.f, 2.f),			float);
		ASSERT_OVERLOADING_SIMPLE(Simple2(ci, vi),				__int32);
		ASSERT_OVERLOADING_SIMPLE(Simple2(vi, cvi),				__int32);
		ASSERT_OVERLOADING_SIMPLE(Simple2(cvi, ci),				__int32);

		ASSERT_OVERLOADING_SIMPLE(Simple2(1, 2.0),				void);
		ASSERT_OVERLOADING_SIMPLE(Simple2(1.0, 2.f),			void);
		ASSERT_OVERLOADING_SIMPLE(Simple2(1.f, 2),				void);
		// test 0, 1 arguments
	});

	TEST_CATEGORY(L"Template argument deduction (variant)")
	{
		using namespace Input__TestOverloadingGenericFunction_TypeInferVariant;
		COMPILE_PROGRAM(program, pa, input);

		ASSERT_OVERLOADING_FORMATTED_VERBOSE(
			Variant(1, 1.0, 1.f, ci, vi, cvi),
			L"::Types<{__int32 $PR, double $PR, float $PR, __int32 $PR, __int32 $PR, __int32 $PR}> $PR",
			Types<int, double, float, int, int, int>
		);
		// test 0, 1, 2 arguments
		// when the first vta is {}, it could be treated as "to be inferred", if function arguments suggest so
	});

	TEST_CATEGORY(L"Template argument deduction (kinds)")
	{
		using namespace Input__TestOverloadingGenericFunction_TypeInferKinds;
		COMPILE_PROGRAM(program, pa, input);

		// test 0, 1, 2, 3 arguments

		ASSERT_OVERLOADING_FORMATTED_VERBOSE(LRef(Value<bool &>()),									L"::Types<{bool $PR}> $PR",								Types<bool>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(LRef(Value<bool const &>()),							L"::Types<{bool const $PR}> $PR",						Types<bool const>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(LRef(Value<bool volatile &>()),						L"::Types<{bool volatile $PR}> $PR",					Types<bool volatile>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(LRef(Value<bool const volatile &>()),					L"::Types<{bool const volatile $PR}> $PR",				Types<bool const volatile>);

		ASSERT_OVERLOADING_FORMATTED_VERBOSE(RRef(Value<bool &>()),									L"::Types<{bool & $PR}> $PR",							Types<bool &>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(RRef(Value<bool const &>()),							L"::Types<{bool const & $PR}> $PR",						Types<bool const &>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(RRef(Value<bool volatile &>()),						L"::Types<{bool volatile & $PR}> $PR",					Types<bool volatile &>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(RRef(Value<bool const volatile &>()),					L"::Types<{bool const volatile & $PR}> $PR",			Types<bool const volatile &>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(RRef(Value<bool &&>()),								L"::Types<{bool $PR}> $PR",								Types<bool>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(RRef(Value<bool const &&>()),							L"::Types<{bool const $PR}> $PR",						Types<bool const>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(RRef(Value<bool volatile &&>()),						L"::Types<{bool volatile $PR}> $PR",					Types<bool volatile>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(RRef(Value<bool const volatile &&>()),					L"::Types<{bool const volatile $PR}> $PR",				Types<bool const volatile>);

		ASSERT_OVERLOADING_FORMATTED_VERBOSE(Pointer(Value<bool *>()),								L"::Types<{bool $PR}> $PR",								Types<bool>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(Pointer(Value<bool const *>()),						L"::Types<{bool const $PR}> $PR",						Types<bool const>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(Pointer(Value<bool volatile *>()),						L"::Types<{bool volatile $PR}> $PR",					Types<bool volatile>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(Pointer(Value<bool const volatile *>()),				L"::Types<{bool const volatile $PR}> $PR",				Types<bool const volatile>);

		ASSERT_OVERLOADING_FORMATTED_VERBOSE(C(Value<bool>()),										L"::Types<{bool $PR}> $PR",								Types<bool>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(C(Value<bool const>()),								L"::Types<{bool $PR}> $PR",								Types<bool>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(C(Value<bool volatile>()),								L"::Types<{bool $PR}> $PR",								Types<bool>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(C(Value<bool const volatile>()),						L"::Types<{bool $PR}> $PR",								Types<bool>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(V(Value<bool>()),										L"::Types<{bool $PR}> $PR",								Types<bool>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(V(Value<bool const>()),								L"::Types<{bool $PR}> $PR",								Types<bool>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(V(Value<bool volatile>()),								L"::Types<{bool $PR}> $PR",								Types<bool>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(V(Value<bool const volatile>()),						L"::Types<{bool $PR}> $PR",								Types<bool>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(CV(Value<bool>()),										L"::Types<{bool $PR}> $PR",								Types<bool>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(CV(Value<bool const>()),								L"::Types<{bool $PR}> $PR",								Types<bool>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(CV(Value<bool volatile>()),							L"::Types<{bool $PR}> $PR",								Types<bool>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(CV(Value<bool const volatile>()),						L"::Types<{bool $PR}> $PR",								Types<bool>);
		
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(Function(Value<FunctionOf<bool>>()),					L"::Types<{bool $PR}> $PR",								Types<bool>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(Function(Value<FunctionOf<bool, float, double>>()),	L"::Types<{bool $PR, float $PR, double $PR}> $PR",		Types<bool, float, double>);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(Member(Value<MemberOf<bool, Types<>>>()),				L"::Types<{bool $PR, ::Types<{}> $PR}> $PR",			Types<bool, Types<>>);

		ASSERT_OVERLOADING_FORMATTED_VERBOSE(
			VtaPtr(Value<bool *>(), Value<bool const *>(), Value<bool volatile *>()),
			L"::Types<{bool $PR, bool const $PR, bool volatile $PR}> $PR",
			Types<bool, bool const, bool volatile>
		);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(
			VtaTypes(Value<Types<bool *, bool const *, bool volatile *>>()),
			L"::Types<{bool * $PR, bool const * $PR, bool volatile * $PR}> $PR",
			Types<bool *, bool const *, bool volatile *>
		);

		AssertExpr(
			pa,
			L"Value<Types<FunctionOf<bool, float *>, FunctionOf<bool, double *>>>()",
			nullptr,
			L"::Types<{bool __cdecl(float *) * $PR, bool __cdecl(double *) * $PR}> $PR"
		);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(
			VtaFunc(Value<Types<FunctionOf<bool, float *>, FunctionOf<bool, double *>>>()),
			L"::Types<{bool $PR, float $PR, double $PR}> $PR",
			Types<bool, float, double>
		);

		AssertExpr(
			pa,
			L"Value<Types<FunctionOf<bool &, float *>, FunctionOf<char &, double *>>>()",
			nullptr,
			L"::Types<{bool & __cdecl(float *) * $PR, char & __cdecl(double *) * $PR}> $PR"
		);
		ASSERT_OVERLOADING_FORMATTED_VERBOSE(
			VtaFunc2(Value<Types<FunctionOf<bool &, float *>, FunctionOf<char &, double *>>>()),
			L"::Types<{bool $PR, char $PR, float $PR, double $PR}> $PR",
			Types<bool, char, float, double>
		);
	});

	TEST_CATEGORY(L"Template argument deduction (child types)")
	{
		using namespace Input__TestOverloadingGenericFunction_TypeInferChildTypes;
		COMPILE_PROGRAM(program, pa, input);

		ASSERT_OVERLOADING_FORMATTED_VERBOSE(
			UseX(A<float, double> :: _ :: B<bool, char, void>()),
			L"::Types<{::A<float, double>::_::B<bool, {char $PR, void $PR}> $PR, bool $PR, char $PR, void $PR}> $PR",
			Types<A<float, double>::_::B<bool, char, void>, bool, char, void>
		);

		ASSERT_OVERLOADING_FORMATTED_VERBOSE(
			UseA(A<float, double>()),
			L"::Types<{float $PR, double $PR}> $PR",
			Types<float, double>
		);

		ASSERT_OVERLOADING_FORMATTED_VERBOSE(
			UseB(A<float, double> :: _ :: B<bool, char, void>()),
			L"::Types<{bool $PR, char $PR, void $PR}> $PR",
			Types<bool, char, void>
		);

		ASSERT_OVERLOADING_FORMATTED_VERBOSE(
			(UseB<float, double>(A<float, double> :: _ :: B<bool, char, void>())),
			L"::Types<{float $PR, double $PR, bool $PR, char $PR, void $PR}> $PR",
			Types<float, double, bool, char, void>
		);

		// test variadic function argument
	});

	TEST_CATEGORY(L"Template argument deduction (base types)")
	{
		using namespace Input__TestOverloadingGenericFunction_TypeInferChildTypes;
		COMPILE_PROGRAM(program, pa, input);

		ASSERT_OVERLOADING_FORMATTED_VERBOSE(
			UseA(C<float>()),
			L"::Types<{float * $PR, float & $PR}> $PR",
			Types<float*, float&>
		);

		ASSERT_OVERLOADING_FORMATTED_VERBOSE(
			(UseB<float *, float &>(C<float> :: D())),
			L"::Types<{float * $PR, float & $PR, float const $PR, float volatile $PR, float const volatile $PR}> $PR",
			Types<float*, float&, const float, volatile float, const volatile float>
		);

		ASSERT_OVERLOADING_FORMATTED_VERBOSE(
			(UseB<float &, float *>(C<float> :: D())),
			L"::Types<{float & $PR, float * $PR, float $PR}> $PR",
			Types<float&, float*, float>
		);

		ASSERT_OVERLOADING_FORMATTED_VERBOSE(
			(UseB(C<float> :: D())),
			L"::Types<{char $PR, bool $PR, void $PR}> $PR",
			Types<char, bool, void>
		);

		// test variadic function argument
	});

	// test matching Types<A..., B...>
	// test template value argument (assign non-variadic to nullptr since this is the only choice, and calculate amounts of variadic ones)
	// test known/unknown variadic arguments/parameters
	// test generic methods		(TestOverloadingGenericMethodInfer.cpp)
	// test generic operators	(TestOverloadingGenericMethodInfer.cpp)
	// test overloading			(TestOverloadingGenericFunction.cpp)
}