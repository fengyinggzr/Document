#include <Ast_Resolving.h>

TEST_CASE(TestTypeSystem_Primitive)
{
	auto tsys = ITsysAlloc::Create();
#define TEST_PRIMITIVE_BYTES(TYPE, BYTES)\
	TEST_ASSERT(\
		tsys->PrimitiveOf({TsysPrimitiveType::TYPE,TsysBytes::BYTES})==\
		tsys->PrimitiveOf({TsysPrimitiveType::TYPE,TsysBytes::BYTES}))\

#define TEST_PRIMITIVE(TYPE)\
	TEST_PRIMITIVE_BYTES(TYPE, _1);\
	TEST_PRIMITIVE_BYTES(TYPE, _2);\
	TEST_PRIMITIVE_BYTES(TYPE, _4);\
	TEST_PRIMITIVE_BYTES(TYPE, _8)\

	TEST_PRIMITIVE(SInt);
	TEST_PRIMITIVE(UInt);
	TEST_PRIMITIVE(Float);
	TEST_PRIMITIVE(SChar);
	TEST_PRIMITIVE(UChar);
	TEST_PRIMITIVE(Bool);
	TEST_PRIMITIVE(Void);

#undef TEST_PRIMITIVE
#undef TEST_PRIMITIVE_BYTES

#define TEST_PRIMITIVE_BYTES(TYPE1, BYTES1, TYPE2, BYTES2)\
	TEST_ASSERT(\
		tsys->PrimitiveOf({ TsysPrimitiveType::TYPE1,TsysBytes::BYTES1 }) != \
		tsys->PrimitiveOf({ TsysPrimitiveType::TYPE2,TsysBytes::BYTES2 }))\

#define TEST_PRIMITIVE(TYPE1, TYPE2)\
	TEST_PRIMITIVE_BYTES(TYPE1, _1, TYPE2, _2);\
	TEST_PRIMITIVE_BYTES(TYPE1, _2, TYPE2, _4);\
	TEST_PRIMITIVE_BYTES(TYPE1, _4, TYPE2, _8);\
	TEST_PRIMITIVE_BYTES(TYPE1, _8, TYPE2, _1)\

	TEST_PRIMITIVE(SInt, UInt);
	TEST_PRIMITIVE(UInt, Float);
	TEST_PRIMITIVE(Float, SChar);
	TEST_PRIMITIVE(SChar, UChar);
	TEST_PRIMITIVE(UChar, Bool);
	TEST_PRIMITIVE(Bool, Void);
	TEST_PRIMITIVE(Void, SInt);

#undef TEST_PRIMITIVE
#undef TEST_PRIMITIVE_BYTES
}

TEST_CASE(TestTypeSystem_LRef)
{
	auto tsys = ITsysAlloc::Create();
	auto tvoid = tsys->Void();
	TEST_ASSERT(tvoid->LRefOf() == tvoid->LRefOf());
}

TEST_CASE(TestTypeSystem_RRef)
{
	auto tsys = ITsysAlloc::Create();
	auto tvoid = tsys->Void();
	TEST_ASSERT(tvoid->RRefOf() == tvoid->RRefOf());
}

TEST_CASE(TestTypeSystem_Ptr)
{
	auto tsys = ITsysAlloc::Create();
	auto tvoid = tsys->Void();
	TEST_ASSERT(tvoid->PtrOf() == tvoid->PtrOf());
}

TEST_CASE(TestTypeSystem_Array)
{
	auto tsys = ITsysAlloc::Create();
	auto tvoid = tsys->Void();
	TEST_ASSERT(tvoid->ArrayOf(1) == tvoid->ArrayOf(1));
	TEST_ASSERT(tvoid->ArrayOf(2) == tvoid->ArrayOf(2));
	TEST_ASSERT(tvoid->ArrayOf(1) != tvoid->ArrayOf(2));
}

TEST_CASE(TestTypeSystem_Function)
{
	auto n1 = MakePtr<Symbol>();
	auto n2 = MakePtr<Symbol>();
	auto tsys = ITsysAlloc::Create();
	auto tvoid = tsys->Void();
	auto tdecl1 = tsys->DeclOf(n1.Obj());
	auto tdecl2 = tsys->DeclOf(n2.Obj());

	List<ITsys*> types1;
	types1.Add(tdecl1);
	types1.Add(tdecl2);

	List<ITsys*> types2;
	types2.Add(tdecl2);
	types2.Add(tdecl1);

	TsysFunc data1;
	TsysFunc data2;

	data1.ellipsis = false;
	data2.ellipsis = true;

	TEST_ASSERT(tvoid->FunctionOf(types1, data1) == tvoid->FunctionOf(types1, data1));
	TEST_ASSERT(tvoid->FunctionOf(types1, data1) != tvoid->FunctionOf(types1, data2));
	TEST_ASSERT(tvoid->FunctionOf(types1, data1) != tvoid->FunctionOf(types2, data1));
}

TEST_CASE(TestTypeSystem_Member)
{
	auto tsys = ITsysAlloc::Create();
	auto tvoid = tsys->Void();
	auto tbool = tsys->PrimitiveOf({ TsysPrimitiveType::Bool,TsysBytes::_1 });
	auto tchar = tsys->PrimitiveOf({ TsysPrimitiveType::SChar,TsysBytes::_1 });
	TEST_ASSERT(tvoid->MemberOf(tbool) == tvoid->MemberOf(tbool));
	TEST_ASSERT(tvoid->MemberOf(tbool) != tvoid->MemberOf(tchar));
}

TEST_CASE(TestTypeSystem_CV)
{
	auto tsys = ITsysAlloc::Create();
	auto tvoid = tsys->Void();

#define CV(INDEX) {((INDEX)>>1)%2==1, ((INDEX)>>0)%2==1}
	for (vint i = 0; i < 4; i++)
	{
		TEST_ASSERT(tvoid->CVOf(CV(i)) == tvoid->CVOf(CV(i)));
		for (vint j = 0; j < 4; j++)
		{
			if (i != j)
			{
				TEST_ASSERT(tvoid->CVOf(CV(i)) != tvoid->CVOf(CV(j)));
			}
		}
	}
#undef CV
}

TEST_CASE(TestTypeSystem_Decl)
{
	auto n1 = MakePtr<Symbol>();
	auto n2 = MakePtr<Symbol>();
	auto tsys = ITsysAlloc::Create();
	TEST_ASSERT(tsys->DeclOf(n1.Obj()) == tsys->DeclOf(n1.Obj()));
	TEST_ASSERT(tsys->DeclOf(n1.Obj()) != tsys->DeclOf(n2.Obj()));
}

TEST_CASE(TestTypeSystem_Init)
{
	auto tsys = ITsysAlloc::Create();
	auto tvoid = tsys->Void();
	auto tnull = tsys->Nullptr();

	Array<ExprTsysItem> types[4] = { 2,2,2,2 };
	types[0][0] = { nullptr,	ExprTsysType::LValue,	tvoid };
	types[0][1] = { nullptr,	ExprTsysType::PRValue,	tnull };
	types[1][0] = { nullptr,	ExprTsysType::PRValue,	tvoid };
	types[1][1] = { nullptr,	ExprTsysType::LValue,	tnull };
	types[2][0] = { nullptr,	ExprTsysType::LValue,	tnull };
	types[2][1] = { nullptr,	ExprTsysType::PRValue,	tvoid };
	types[3][0] = { nullptr,	ExprTsysType::PRValue,	tnull };
	types[3][1] = { nullptr,	ExprTsysType::LValue,	tvoid };

	for (vint i = 0; i < 4; i++)
	{
		for (vint j = 0; j < 4; j++)
		{
			if (i == j)
			{
				TEST_ASSERT(tsys->InitOf(types[i]) == tsys->InitOf(types[j]));
			}
			else
			{
				TEST_ASSERT(tsys->InitOf(types[i]) != tsys->InitOf(types[j]));
			}
		}
	}
}

TEST_CASE(TestTypeSystem_GenericFunction)
{
	auto n = MakePtr<Symbol>();
	auto narg = MakePtr<Symbol>();
	auto tsys = ITsysAlloc::Create();
	auto tdecl = tsys->DeclOf(n.Obj());

	TsysGenericArg arg = { 0,nullptr };
	auto targ = tsys->DeclOf(n.Obj())->GenericArgOf(arg);

	TsysGenericFunction gf;

	List<ITsys*> types1;
	types1.Add(targ->LRefOf());
	types1.Add(targ->PtrOf());

	List<ITsys*> types2;
	types2.Add(targ->PtrOf());
	types2.Add(targ->LRefOf());

	TEST_ASSERT(tdecl->GenericFunctionOf(types1, gf) == tdecl->GenericFunctionOf(types1, gf));
	TEST_ASSERT(tdecl->GenericFunctionOf(types1, gf) != tdecl->GenericFunctionOf(types2, gf));
}

TEST_CASE(TestTypeSystem_GenericArg)
{
	auto n = MakePtr<Symbol>();
	auto narg1 = MakePtr<Symbol>();
	auto narg2 = MakePtr<Symbol>();
	auto tsys = ITsysAlloc::Create();
	TsysGenericArg arg1 = { 0,narg1.Obj() };
	TsysGenericArg arg2 = { 0,narg2.Obj() };
	auto targ1 = tsys->DeclOf(n.Obj())->GenericArgOf(arg1);
	auto targ2 = tsys->DeclOf(n.Obj())->GenericArgOf(arg2);

	TEST_ASSERT(targ1->GetElement()->GetDecl() == n.Obj());
	TEST_ASSERT(targ2->GetElement()->GetDecl() == n.Obj());
	TEST_ASSERT(targ1->GetElement() == targ2->GetElement());
	TEST_ASSERT(tsys->DeclOf(n.Obj())->GenericArgOf(arg1) == targ1);
	TEST_ASSERT(tsys->DeclOf(n.Obj())->GenericArgOf(arg1) != targ2);
}

TEST_CASE(TestTypeSystem_DeclInstance)
{
	auto tsys = ITsysAlloc::Create();

	auto sa1 = MakePtr<Symbol>();
	sa1->kind = symbol_component::SymbolKind::GenericTypeArgument;
	auto sa2 = MakePtr<Symbol>();
	sa2->kind = symbol_component::SymbolKind::GenericTypeArgument;

	auto spec = MakePtr<TemplateSpec>();
	{
		TemplateSpec::Argument arg;
		arg.argumentSymbol = sa1.Obj();
		arg.argumentType = CppTemplateArgumentType::Type;
		spec->arguments.Add(arg);

		TsysGenericArg tgArg;
		tgArg.argIndex = 0;
		tgArg.argSymbol = sa1.Obj();

		sa1->GetEvaluationForUpdating_NFb().Allocate();
		sa1->GetEvaluationForUpdating_NFb().Get().Add(tsys->DeclOf(sa1.Obj())->GenericArgOf(tgArg));
	}
	{
		TemplateSpec::Argument arg;
		arg.argumentSymbol = sa2.Obj();
		arg.argumentType = CppTemplateArgumentType::Type;
		spec->arguments.Add(arg);

		TsysGenericArg tgArg;
		tgArg.argIndex = 0;
		tgArg.argSymbol = sa2.Obj();

		sa2->GetEvaluationForUpdating_NFb().Allocate();
		sa2->GetEvaluationForUpdating_NFb().Get().Add(tsys->DeclOf(sa2.Obj())->GenericArgOf(tgArg));
	}

	auto f1 = MakePtr<ClassDeclaration>();
	f1->name.name = L"F1";
	f1->templateSpec = spec;

	auto f2 = MakePtr<ClassDeclaration>();
	f2->name.name = L"F2";
	f2->templateSpec = spec;

	auto root = MakePtr<Symbol>();
	auto n1 = root->AddImplDeclToSymbol_NFb(f1, symbol_component::SymbolKind::Class);
	auto n2 = root->AddImplDeclToSymbol_NFb(f2, symbol_component::SymbolKind::Class);
	auto pn1 = MakePtr<Symbol>();
	auto pn2 = MakePtr<Symbol>();

	auto b1 = tsys->DeclOf(pn1.Obj());
	auto b2 = tsys->DeclOf(pn2.Obj());

	List<ITsys*> p1, p2;
	p1.Add(tsys->Void());
	p1.Add(tsys->Nullptr());
	p2.Add(tsys->Nullptr());
	p2.Add(tsys->Void());

	{
		IEnumerable<ITsys*>* ps[] = { nullptr,&p1,&p2 };
		for (vint i = 0; i < 3; i++)
		{
			for (vint j = 0; j < 3; j++)
			{
				if (i == j)
				{
					TEST_ASSERT(tsys->DeclInstantOf(n1, ps[i], b1) == tsys->DeclInstantOf(n1, ps[j], b1));
				}
				else
				{
					TEST_ASSERT(tsys->DeclInstantOf(n1, ps[i], b1) != tsys->DeclInstantOf(n1, ps[j], b1));
				}
			}
		}
	}

	auto t1 = tsys->DeclInstantOf(n1, nullptr, nullptr);
	TEST_ASSERT(t1->GetType() == TsysType::DeclInstant);
	const auto& data1 = t1->GetDeclInstant();
	TEST_ASSERT(data1.declSymbol == n1);
	TEST_ASSERT(data1.parentDeclType == nullptr);
	TEST_ASSERT(!data1.taContext);
	TEST_ASSERT(t1->GetParamCount() == 0);

	const_cast<TsysDeclInstant&>(data1).taContext = MakePtr<TemplateArgumentContext>();

	auto t2 = tsys->DeclInstantOf(n2, &p2, t1);
	TEST_ASSERT(t2->GetType() == TsysType::DeclInstant);
	const auto& data2 = t2->GetDeclInstant();
	TEST_ASSERT(data2.declSymbol == n2);
	TEST_ASSERT(data2.parentDeclType == t1);
	TEST_ASSERT(t2->GetParamCount() == 2);
	TEST_ASSERT(t2->GetParam(0) == tsys->Nullptr());
	TEST_ASSERT(t2->GetParam(1) == tsys->Void());
	TEST_ASSERT(data2.taContext->arguments.Count() == 2);
	TEST_ASSERT(data2.taContext->parent == data1.taContext.Obj());
	TEST_ASSERT(data2.taContext->symbolToApply == n2);

	auto k1 = symbol_type_resolving::GetTemplateArgumentKey(spec->arguments[0], tsys.Obj());
	auto k2 = symbol_type_resolving::GetTemplateArgumentKey(spec->arguments[1], tsys.Obj());
	TEST_ASSERT(data2.taContext->arguments[k1][0] == tsys->Nullptr());
	TEST_ASSERT(data2.taContext->arguments[k2][0] == tsys->Void());
}

TEST_CASE(TestTypeSystem_Type)
{
	auto n1 = MakePtr<Symbol>();
	auto n2 = MakePtr<Symbol>();
	auto tsys = ITsysAlloc::Create();
	auto tvoid = tsys->Void();
	auto tdecl1 = tsys->DeclOf(n1.Obj());
	auto tdecl2 = tsys->DeclOf(n2.Obj());

	TEST_ASSERT(tvoid->GetType() == TsysType::Primitive);
	TEST_ASSERT(tdecl1->GetType() == TsysType::Decl);
	TEST_ASSERT(tdecl2->GetType() == TsysType::Decl);

	TEST_ASSERT(tvoid->LRefOf()->GetType() == TsysType::LRef);
	TEST_ASSERT(tvoid->RRefOf()->GetType() == TsysType::RRef);
	TEST_ASSERT(tvoid->PtrOf()->GetType() == TsysType::Ptr);
	TEST_ASSERT(tvoid->ArrayOf(1)->GetType() == TsysType::Array);
	TEST_ASSERT(tvoid->CVOf({ false,false }) == tvoid);
	TEST_ASSERT(tvoid->CVOf({ true,true })->GetType() == TsysType::CV);
	TEST_ASSERT(tvoid->MemberOf(tdecl1)->GetType() == TsysType::Member);

	TsysGenericFunction gf;
	List<ITsys*> types;
	types.Add(tdecl1);
	types.Add(tdecl2);
	TEST_ASSERT(tvoid->FunctionOf(types, {})->GetType() == TsysType::Function);
	TEST_ASSERT(tvoid->GenericFunctionOf(types, gf)->GetType() == TsysType::GenericFunction);
}