#include "Ast_Evaluate_ExpandPotentialVta.h"

using namespace symbol_type_resolving;

namespace symbol_totsys_impl
{
	//////////////////////////////////////////////////////////////////////////////////////
	// ProcessLiteralExpr
	//////////////////////////////////////////////////////////////////////////////////////
	
	void ProcessLiteralExpr(const ParsingArguments& pa, ExprTsysList& result, LiteralExpr* self)
	{
		switch ((CppTokens)self->tokens[0].token)
		{
		case CppTokens::INT:
		case CppTokens::HEX:
		case CppTokens::BIN:
			{
				auto& token = self->tokens[0];
				{
					auto reading = token.reading;
					auto end = token.reading + token.length;
					if (reading[0] == L'0')
					{
						switch (reading[1])
						{
						case L'x':
						case L'X':
						case L'b':
						case L'B':
							reading += 2;
						}
					}

					while (reading < end)
					{
						if (L'1' <= *reading && *reading <= L'9')
						{
							goto NOT_ZERO;
						}
						reading++;
					}

					AddTempValue(result, pa.tsys->Zero());
					return;
				}
			NOT_ZERO:
#define COUNT_CHAR(NUM, UC, LC) ((_##NUM == UC || _##NUM == LC) ? 1 : 0)
#define COUNT_U(NUM) COUNT_CHAR(NUM, L'u', L'U')
#define COUNT_L(NUM) COUNT_CHAR(NUM, L'l', L'L')
				wchar_t _1 = token.length > 2 ? token.reading[token.length - 3] : 0;
				wchar_t _2 = token.length > 1 ? token.reading[token.length - 2] : 0;
				wchar_t _3 = token.reading[token.length - 1];
				vint us = COUNT_U(1) + COUNT_U(2) + COUNT_U(3);
				vint ls = COUNT_L(1) + COUNT_L(2) + COUNT_L(3);
				AddTempValue(result, pa.tsys->PrimitiveOf({ (us > 0 ? TsysPrimitiveType::UInt : TsysPrimitiveType::SInt),{ls > 1 ? TsysBytes::_8 : TsysBytes::_4} }));
#undef COUNT_CHAR
#undef COUNT_U
#undef COUNT_L
			}
			return;
		case CppTokens::FLOAT:
			{
				auto& token = self->tokens[0];
				wchar_t _1 = token.reading[token.length - 1];
				if (_1 == L'f' || _1 == L'F')
				{
					AddTempValue(result, pa.tsys->PrimitiveOf({ TsysPrimitiveType::Float, TsysBytes::_4 }));
				}
				else
				{
					AddTempValue(result, pa.tsys->PrimitiveOf({ TsysPrimitiveType::Float, TsysBytes::_8 }));
				}
			}
			return;
		case CppTokens::STRING:
		case CppTokens::CHAR:
			{
				ITsys* tsysChar = nullptr;
				auto reading = self->tokens[0].reading;
				if (reading[0] == L'\"' || reading[0] == L'\'')
				{
					tsysChar = pa.tsys->PrimitiveOf({ TsysPrimitiveType::SChar,TsysBytes::_1 });
				}
				else if (reading[0] == L'L')
				{
					tsysChar = pa.tsys->PrimitiveOf({ TsysPrimitiveType::UWChar,TsysBytes::_2 });
				}
				else if (reading[0] == L'U')
				{
					tsysChar = pa.tsys->PrimitiveOf({ TsysPrimitiveType::UChar,TsysBytes::_4 });
				}
				else if (reading[0] == L'u')
				{
					if (reading[1] == L'8')
					{
						tsysChar = pa.tsys->PrimitiveOf({ TsysPrimitiveType::SChar,TsysBytes::_1 });
					}
					else
					{
						tsysChar = pa.tsys->PrimitiveOf({ TsysPrimitiveType::UChar,TsysBytes::_2 });
					}
				}

				if (!tsysChar)
				{
					throw IllegalExprException();
				}

				if ((CppTokens)self->tokens[0].token == CppTokens::CHAR)
				{
					AddTempValue(result, tsysChar);
				}
				else
				{
					AddTempValue(result, tsysChar->CVOf({ true,false })->ArrayOf(1)->LRefOf());
				}
			}
			return;
		case CppTokens::EXPR_TRUE:
		case CppTokens::EXPR_FALSE:
			AddTempValue(result, pa.tsys->PrimitiveOf({ TsysPrimitiveType::Bool,TsysBytes::_1 }));
			return;
		}
		throw IllegalExprException();
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// ProcessThisExpr
	//////////////////////////////////////////////////////////////////////////////////////

	void ProcessThisExpr(const ParsingArguments& pa, ExprTsysList& result, ThisExpr* self)
	{
		if (pa.functionBodySymbol)
		{
			if (auto cache = pa.functionBodySymbol->GetClassMemberCache_NFb())
			{
				if (cache->thisType)
				{
					if (pa.taContext)
					{
						AddTempValue(result, cache->thisType->ReplaceGenericArgs(pa));
					}
					else
					{
						AddTempValue(result, cache->thisType);
					}
					return;
				}
			}
		}
		throw IllegalExprException();
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// ProcessNullptrExpr
	//////////////////////////////////////////////////////////////////////////////////////
	
	void ProcessNullptrExpr(const ParsingArguments& pa, ExprTsysList& result, NullptrExpr* self)
	{
		AddTempValue(result, pa.tsys->Nullptr());
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// ProcessThrowExpr
	//////////////////////////////////////////////////////////////////////////////////////
	
	void ProcessThrowExpr(const ParsingArguments& pa, ExprTsysList& result, ThrowExpr* self)
	{
		if (self->expr)
		{
			ExprTsysList types;
			ExprToTsysNoVta(pa, self->expr, types);
		}

		AddTempValue(result, pa.tsys->Void());
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// ProcessParenthesisExpr
	//////////////////////////////////////////////////////////////////////////////////////

	void ProcessParenthesisExpr(const ParsingArguments& pa, ExprTsysList& result, ParenthesisExpr* self, ExprTsysItem arg)
	{
		if (arg.type == ExprTsysType::LValue)
		{
			AddTempValue(result, arg.tsys->LRefOf());
		}
		else
		{
			AddTempValue(result, arg.tsys);
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// ProcessCastExpr
	//////////////////////////////////////////////////////////////////////////////////////

	void ProcessCastExpr(const ParsingArguments& pa, ExprTsysList& result, CastExpr* self, ExprTsysItem argType, ExprTsysItem argExpr)
	{
		AddTempValue(result, argType.tsys);
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// ProcessTypeidExpr
	//////////////////////////////////////////////////////////////////////////////////////

	void ProcessTypeidExpr(const ParsingArguments& pa, ExprTsysList& result, TypeidExpr* self)
	{
		auto global = pa.root.Obj();

		auto pStds = global->TryGetChildren_NFb(L"std");
		if (!pStds) return;
		if (pStds->Count() != 1) return;

		auto pTis = pStds->Get(0)->TryGetChildren_NFb(L"type_info");
		if (!pTis) return;

		for (vint i = 0; i < pTis->Count(); i++)
		{
			auto ti = pTis->Get(i);
			switch (ti->kind)
			{
			case symbol_component::SymbolKind::Class:
			case symbol_component::SymbolKind::Struct:
				AddInternal(result, { nullptr,ExprTsysType::LValue,pa.tsys->DeclOf(ti.Obj()) });
				return;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// ProcessArrayAccessExpr
	//////////////////////////////////////////////////////////////////////////////////////

	void ProcessArrayAccessExpr(const ParsingArguments& pa, ExprTsysList& result, ArrayAccessExpr* self, ExprTsysItem argArray, ExprTsysItem argIndex, bool& indexed)
	{
		TsysCV cv;
		TsysRefType refType;
		auto entityType = argArray.tsys->GetEntity(cv, refType);

		if (entityType->IsUnknownType())
		{
			AddTempValue(result, pa.tsys->Any());
		}
		else if (entityType->GetType() == TsysType::Decl || entityType->GetType() == TsysType::DeclInstant)
		{
			ExprTsysList funcTypes;
			VisitFunctors(pa, argArray, L"operator []", funcTypes);

			Array<ExprTsysItem> argTypes(1);
			argTypes[0] = argIndex;

			bool needIndex = pa.recorder && pa.IsGeneralEvaluation();
			SortedList<vint> boundedAnys;
			ExprTsysList selectedFunctions;
			VisitOverloadedFunction(pa, funcTypes, argTypes, boundedAnys, result, (needIndex ? &selectedFunctions : nullptr));

			if (needIndex && selectedFunctions.Count() > 0)
			{
				AddSymbolsToOperatorResolving(pa, self->opName, self->opResolving, selectedFunctions, indexed);
			}
		}
		else if (entityType->GetType() == TsysType::Array)
		{
			auto tsys = entityType->GetElement();
			if (refType == TsysRefType::LRef)
			{
				AddInternal(result, { nullptr,ExprTsysType::LValue,tsys->CVOf(cv)->LRefOf() });
			}
			else if (refType == TsysRefType::RRef)
			{
				if (argArray.type == ExprTsysType::LValue)
				{
					AddInternal(result, { nullptr,ExprTsysType::LValue,tsys->CVOf(cv)->LRefOf() });
				}
				else
				{
					AddInternal(result, { nullptr,ExprTsysType::XValue,tsys->CVOf(cv)->RRefOf() });
				}
			}
			else
			{
				AddInternal(result, { nullptr,argArray.type,tsys->CVOf(cv)->LRefOf() });
			}
		}
		else if (entityType->GetType() == TsysType::Ptr)
		{
			AddTempValue(result, entityType->GetElement()->LRefOf());
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// Process(Operator)Expr
	//////////////////////////////////////////////////////////////////////////////////////

	bool VisitOperator(const ParsingArguments& pa, ExprTsysList& result, ExprTsysItem* leftType, ExprTsysItem* rightType, CppName& resolvableName, Ptr<Resolving>& resolving, bool& indexed)
	{
		TsysCV leftCV, rightCV;
		TsysRefType leftRef, rightRef;
		auto leftEntity = leftType->tsys->GetEntity(leftCV, leftRef);
		auto rightEntity = rightType ? rightType->tsys->GetEntity(rightCV, rightRef) : nullptr;

		if (leftEntity->IsUnknownType())
		{
			AddTempValue(result, pa.tsys->Any());
			return true;
		}
		if (rightEntity && rightEntity->IsUnknownType())
		{
			AddTempValue(result, pa.tsys->Any());
			return true;
		}

		CppName opName;
		opName.type = CppNameType::Operator;
		opName.name = L"operator " + resolvableName.name;

		if (leftEntity->GetType() == TsysType::Decl || leftEntity->GetType() == TsysType::DeclInstant)
		{
			auto newPa = pa.WithScope(leftEntity->GetDecl());
			auto opMethods = ResolveSymbol(newPa, opName, SearchPolicy::ChildSymbolFromOutside);

			if (opMethods.values)
			{
				ExprTsysList opTypes;
				for (vint j = 0; j < opMethods.values->resolvedSymbols.Count(); j++)
				{
					auto symbol = opMethods.values->resolvedSymbols[j];
					auto adjusted = AdjustThisItemForSymbol(newPa, *leftType, symbol).Value();
					VisitSymbolForField(pa, &adjusted, symbol, opTypes);
				}
				FilterFieldsAndBestQualifiedFunctions(leftCV, leftRef, opTypes);

				Array<ExprTsysItem> argTypes;
				if (rightType)
				{
					argTypes.Resize(1);
					argTypes[0] = *rightType;
				}
				FindQualifiedFunctors(pa, {}, TsysRefType::None, opTypes, false);

				SortedList<vint> boundedAnys;
				ExprTsysList selectedFunctions;
				VisitOverloadedFunction(pa, opTypes, argTypes, boundedAnys, result, (pa.recorder ? &selectedFunctions : nullptr));
				if (pa.recorder && pa.IsGeneralEvaluation())
				{
					AddSymbolsToOperatorResolving(pa, resolvableName, resolving, selectedFunctions, indexed);
				}
				return true;
			}
		}
		{
			ExprTsysList opTypes;

			auto opFuncs = ResolveSymbol(pa, opName, SearchPolicy::SymbolAccessableInScope);
			if (opFuncs.values)
			{
				for (vint j = 0; j < opFuncs.values->resolvedSymbols.Count(); j++)
				{
					// prevent from getting non-static operators inside a class
					auto funcSymbol = opFuncs.values->resolvedSymbols[j];
					VisitSymbol(pa, funcSymbol, opTypes);
				}
			}
			if (!opFuncs.values || IsAdlEnabled(pa, opFuncs.values))
			{
				SortedList<Symbol*> nss, classes;
				SearchAdlClassesAndNamespaces(pa, leftEntity, nss, classes);
				if (rightEntity)
				{
					SearchAdlClassesAndNamespaces(pa, rightEntity, nss, classes);
				}
				SearchAdlFunction(pa, nss, opName.name, opTypes);
			}

			if (opTypes.Count() > 0)
			{
				Array<ExprTsysItem> argTypes(rightType ? 2 : 1);
				{
					argTypes[0] = *leftType;
				}
				if (rightType)
				{
					argTypes[1] = *rightType;
				}
				FindQualifiedFunctors(pa, {}, TsysRefType::None, opTypes, false);

				SortedList<vint> boundedAnys;
				ExprTsysList selectedFunctions;
				VisitOverloadedFunction(pa, opTypes, argTypes, boundedAnys, result, (pa.recorder ? &selectedFunctions : nullptr));
				if (pa.recorder && pa.IsGeneralEvaluation())
				{
					AddSymbolsToOperatorResolving(pa, resolvableName, resolving, selectedFunctions, indexed);
				}
				if (result.Count() > 0)
				{
					return true;
				}
			}
		}
		return false;
	}

	void ProcessPostfixUnaryExpr(const ParsingArguments& pa, ExprTsysList& result, PostfixUnaryExpr* self, ExprTsysItem arg, bool& indexed)
	{
		TsysCV cv;
		TsysRefType refType;
		auto entity = arg.tsys->GetEntity(cv, refType);

		if (entity->IsUnknownType())
		{
			AddTempValue(result, pa.tsys->Any());
		}
		else if (entity->GetType() == TsysType::Decl || entity->GetType() == TsysType::DeclInstant)
		{
			ExprTsysItem extraParam(nullptr, ExprTsysType::PRValue, pa.tsys->Int());
			VisitOperator(pa, result, &arg, &extraParam, self->opName, self->opResolving, indexed);
		}
		else if (entity->GetType() == TsysType::Primitive)
		{
			auto primitive = entity->GetPrimitive();
			switch (primitive.type)
			{
			case TsysPrimitiveType::Bool:
				AddTempValue(result, arg.tsys->LRefOf());
				break;
			default:
				AddTempValue(result, arg.tsys);
			}
		}
		else if (entity->GetType() == TsysType::Ptr)
		{
			AddTempValue(result, entity);
		}
	}

	void ProcessPrefixUnaryExpr(const ParsingArguments& pa, ExprTsysList& result, PrefixUnaryExpr* self, ExprTsysItem arg, bool& indexed)
	{
		TsysCV cv;
		TsysRefType refType;
		auto entity = arg.tsys->GetEntity(cv, refType);

		if (entity->IsUnknownType())
		{
			AddTempValue(result, pa.tsys->Any());
			return;
		}
		else if (entity->GetType() == TsysType::Decl || entity->GetType() == TsysType::DeclInstant)
		{
			if (VisitOperator(pa, result, &arg, nullptr, self->opName, self->opResolving, indexed))
			{
				return;
			}
		}

		switch (self->op)
		{
		case CppPrefixUnaryOp::Increase:
		case CppPrefixUnaryOp::Decrease:
			AddTempValue(result, arg.tsys->LRefOf());
			break;
		case CppPrefixUnaryOp::Revert:
		case CppPrefixUnaryOp::Positive:
		case CppPrefixUnaryOp::Negative:
			if (entity->GetType() == TsysType::Primitive)
			{
				auto primitive = entity->GetPrimitive();
				Promote(primitive);

				auto promotedEntity = pa.tsys->PrimitiveOf(primitive);
				if (promotedEntity == entity && primitive.type != TsysPrimitiveType::Float)
				{
					AddTempValue(result, pa.tsys->PrimitiveOf(primitive)->CVOf(cv));
				}
				else
				{
					AddTempValue(result, pa.tsys->PrimitiveOf(primitive));
				}
			}
			break;
		case CppPrefixUnaryOp::Not:
			AddTempValue(result, pa.tsys->PrimitiveOf({ TsysPrimitiveType::Bool, TsysBytes::_1 }));
			break;
		case CppPrefixUnaryOp::AddressOf:
			if (entity->GetType() == TsysType::Ptr && arg.type == ExprTsysType::PRValue)
			{
				if (auto catIcgExpr = self->operand.Cast<Category_Id_Child_Generic_Expr>())
				{
					if (entity->GetElement()->GetType() == TsysType::Member)
					{
						MatchCategoryExpr(
							catIcgExpr,
							[](const Ptr<IdExpr>& idExpr)
							{
							},
							[&result, arg](const Ptr<ChildExpr>& childExpr)
							{
								AddTempValue(result, arg.tsys);
							},
							[&result, arg](const Ptr<GenericExpr>& genericExpr)
							{
								AddTempValue(result, arg.tsys);
							}
						);
					}
					else if (entity->GetElement()->GetType() == TsysType::Function)
					{
						AddTempValue(result, arg.tsys);
					}
				}
			}
			else if (arg.tsys->GetType() == TsysType::LRef || arg.tsys->GetType() == TsysType::RRef)
			{
				AddTempValue(result, arg.tsys->GetElement()->PtrOf());
			}
			else
			{
				AddTempValue(result, arg.tsys->PtrOf());
			}
			break;
		case CppPrefixUnaryOp::Dereference:
			if (entity->GetType() == TsysType::Ptr || entity->GetType() == TsysType::Array)
			{
				AddTempValue(result, entity->GetElement()->LRefOf());
			}
			break;
		}
	}

	void ProcessBinaryExpr(const ParsingArguments& pa, ExprTsysList& result, BinaryExpr* self, ExprTsysItem argLeft, ExprTsysItem argRight, bool& indexed)
	{
		TsysCV leftCV, rightCV;
		TsysRefType leftRefType, rightRefType;
		auto leftEntity = argLeft.tsys->GetEntity(leftCV, leftRefType);
		auto rightEntity = argRight.tsys->GetEntity(rightCV, rightRefType);

		if (self->op == CppBinaryOp::ValueFieldDeref || self->op == CppBinaryOp::PtrFieldDeref)
		{
			if (rightEntity->GetType() == TsysType::Ptr && rightEntity->GetElement()->GetType() == TsysType::Member)
			{
				auto fieldEntity = rightEntity->GetElement()->GetElement();
				if (fieldEntity->GetType() == TsysType::Function)
				{
					AddInternal(result, { nullptr,ExprTsysType::PRValue,fieldEntity->PtrOf() });
				}
				else
				{
					ExprTsysItem parentItem = argLeft;
					if (self->op == CppBinaryOp::PtrFieldDeref && leftEntity->GetType() == TsysType::Ptr)
					{
						parentItem = { nullptr,ExprTsysType::LValue,leftEntity->GetElement()->LRefOf() };
					}
					CalculateValueFieldType(&parentItem, nullptr, fieldEntity, true, result);
				}
			}
			else if (rightEntity->IsUnknownType())
			{
				AddTempValue(result, pa.tsys->Any());
			}
			return;
		}

		if (VisitOperator(pa, result, &argLeft, &argRight, self->opName, self->opResolving, indexed))
		{
			return;
		}

		if (self->op == CppBinaryOp::Comma)
		{
			AddInternal(result, argRight);
			return;
		}

		auto leftPrim = leftEntity->GetType() == TsysType::Primitive;
		auto rightPrim = rightEntity->GetType() == TsysType::Primitive;
		auto leftPtrArr = leftEntity->GetType() == TsysType::Ptr || leftEntity->GetType() == TsysType::Array;
		auto rightPtrArr = rightEntity->GetType() == TsysType::Ptr || rightEntity->GetType() == TsysType::Array;

		if (leftPrim && rightPrim)
		{
			switch (self->op)
			{
			case CppBinaryOp::LT:
			case CppBinaryOp::GT:
			case CppBinaryOp::LE:
			case CppBinaryOp::GE:
			case CppBinaryOp::EQ:
			case CppBinaryOp::NE:
			case CppBinaryOp::And:
			case CppBinaryOp::Or:
				AddTempValue(result, pa.tsys->PrimitiveOf({ TsysPrimitiveType::Bool,TsysBytes::_1 }));
				break;
			case CppBinaryOp::Assign:
			case CppBinaryOp::MulAssign:
			case CppBinaryOp::DivAssign:
			case CppBinaryOp::ModAssign:
			case CppBinaryOp::AddAssign:
			case CppBinaryOp::SubAddisn:
			case CppBinaryOp::ShlAssign:
			case CppBinaryOp::ShrAssign:
			case CppBinaryOp::AndAssign:
			case CppBinaryOp::OrAssign:
			case CppBinaryOp::XorAssign:
				AddTempValue(result, argLeft.tsys->LRefOf());
				break;
			case CppBinaryOp::Shl:
			case CppBinaryOp::Shr:
				{
					auto primitive = leftEntity->GetPrimitive();
					Promote(primitive);
					if (primitive.type == TsysPrimitiveType::UChar)
					{
						primitive.type = TsysPrimitiveType::UInt;
					}
					AddTempValue(result, pa.tsys->PrimitiveOf(primitive));
				}
				break;
			default:
				{
					auto leftP = leftEntity->GetPrimitive();
					auto rightP = rightEntity->GetPrimitive();
					auto primitive = ArithmeticConversion(leftP, rightP);
					AddTempValue(result, pa.tsys->PrimitiveOf(primitive));
				}
			}
		}
		else if (leftPrim && rightPtrArr)
		{
			AddTempValue(result, rightEntity->GetElement()->PtrOf());
		}
		else if (leftPtrArr && rightPrim)
		{
			AddTempValue(result, leftEntity->GetElement()->PtrOf());
		}
		else if (leftPtrArr && rightPtrArr)
		{
			AddTempValue(result, pa.tsys->IntPtr());
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// ProcessIfExpr
	//////////////////////////////////////////////////////////////////////////////////////

	void ProcessIfExpr(const ParsingArguments& pa, ExprTsysList& result, IfExpr* self, ExprTsysItem argCond, ExprTsysItem argLeft, ExprTsysItem argRight)
	{
		auto leftType = argLeft.type == ExprTsysType::LValue ? argLeft.tsys->LRefOf() : argLeft.tsys;
		auto rightType = argRight.type == ExprTsysType::LValue ? argRight.tsys->LRefOf() : argRight.tsys;

		TsysCV leftCV, rightCV;
		TsysRefType leftRefType, rightRefType;

		auto leftEntity = leftType->GetEntity(leftCV, leftRefType);
		auto rightEntity = rightType->GetEntity(rightCV, rightRefType);
		if (leftType == rightType)
		{
			AddTempValue(result, leftType);
		}
		else if (leftEntity == rightEntity)
		{
			auto cv = leftCV;
			cv.isGeneralConst |= rightCV.isGeneralConst;
			cv.isVolatile |= rightCV.isVolatile;

			auto refType = leftRefType == rightRefType ? leftRefType : TsysRefType::None;
			AddTempValue(result, CvRefOf(leftEntity, cv, refType));
		}
		else
		{
			auto l2r = TestTypeConversion(pa, rightType, argLeft);
			auto r2l = TestTypeConversion(pa, leftType, argRight);
			if (l2r.anyInvolved || r2l.anyInvolved)
			{
				AddInternal(result, argLeft);
				AddInternal(result, argRight);
			}

			switch (TypeConv::CompareIgnoreAny(l2r, r2l))
			{
			case -1:
				AddTempValue(result, rightType);
				break;
			case 1:
				AddTempValue(result, leftType);
				break;
			case 0:
				{
					auto leftPrim = leftEntity->GetType() == TsysType::Primitive;
					auto rightPrim = rightEntity->GetType() == TsysType::Primitive;
					auto leftPtrArr = leftEntity->GetType() == TsysType::Ptr || leftEntity->GetType() == TsysType::Array;
					auto rightPtrArr = rightEntity->GetType() == TsysType::Ptr || rightEntity->GetType() == TsysType::Array;
					auto leftNull = leftEntity->GetType() == TsysType::Zero || leftEntity->GetType() == TsysType::Nullptr;
					auto rightNull = rightEntity->GetType() == TsysType::Zero || rightEntity->GetType() == TsysType::Nullptr;

					if (l2r.cat == TypeConvCat::Standard && leftPrim && rightPrim)
					{
						auto leftP = leftEntity->GetPrimitive();
						auto rightP = rightEntity->GetPrimitive();
						auto primitive = ArithmeticConversion(leftP, rightP);
						AddTempValue(result, pa.tsys->PrimitiveOf(primitive));
						return;
					}

					if (leftPtrArr && rightNull)
					{
						AddTempValue(result, leftEntity->GetElement()->PtrOf());
						return;
					}

					if (leftNull && rightPtrArr)
					{
						AddTempValue(result, rightEntity->GetElement()->PtrOf());
						return;
					}

					AddInternal(result, argLeft);
					AddInternal(result, argRight);
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// Indexing
	//////////////////////////////////////////////////////////////////////////////////////

	bool IsOperator(Symbol* symbol)
	{
		if (auto funcDecl = symbol->GetAnyForwardDecl<ForwardFunctionDeclaration>())
		{
			return funcDecl->name.type == CppNameType::Operator;
		}
		return false;
	}

	void AddSymbolToResolving(Ptr<Resolving>* resolving, Symbol* symbol, bool& first)
	{
		if (first)
		{
			*resolving = MakePtr<Resolving>();
		}
		first = false;

		auto& resolvedSymbols = (*resolving)->resolvedSymbols;
		if (!resolvedSymbols.Contains(symbol))
		{
			resolvedSymbols.Add(symbol);
		}
	}

	bool AddSymbolToNameResolving(const CppName& name, Ptr<Resolving>* resolving, Symbol* symbol, bool& first)
	{
		bool isOperator = IsOperator(symbol);
		if (!isOperator || name.type == CppNameType::Operator)
		{
			AddSymbolToResolving(resolving, symbol, first);
			return true;
		}
		return false;
	}

	bool AddSymbolToOperatorResolving(const CppName& name, Ptr<Resolving>* resolving, Symbol* symbol, bool& first)
	{
		bool isOperator = IsOperator(symbol);
		if (isOperator)
		{
			AddSymbolToResolving(resolving, symbol, first);
			return true;
		}
		return false;
	}

	void AddSymbolsToResolvings(const ParsingArguments& pa, const CppName* name, Ptr<Resolving>* nameResolving, const CppName* op, Ptr<Resolving>* opResolving, ExprTsysList& symbols, bool& addedName, bool& addedOp)
	{
		if (!pa.IsGeneralEvaluation()) return;
		bool firstName = true;
		bool firstOp = true;

		for (vint i = 0; i < symbols.Count(); i++)
		{
			if (auto symbol = symbols[i].symbol)
			{
				if (name && AddSymbolToNameResolving(*name, nameResolving, symbol, firstName))
				{
					addedName = true;
					continue;
				}

				if (op && AddSymbolToOperatorResolving(*op, opResolving, symbol, firstOp))
				{
					addedOp = true;
				}
			}
		}
	}

	void AddSymbolsToOperatorResolving(const ParsingArguments& pa, const CppName& op, Ptr<Resolving>& opResolving, ExprTsysList& symbols, bool& addedOp)
	{
		bool addedName = false;
		AddSymbolsToResolvings(pa, nullptr, nullptr, &op, &opResolving, symbols, addedName, addedOp);
	}
}