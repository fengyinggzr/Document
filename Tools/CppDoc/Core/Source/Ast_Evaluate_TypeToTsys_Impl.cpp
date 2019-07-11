#include "Ast_Expr.h"
#include "Ast_Resolving.h"

namespace symbol_totsys_impl
{

	//////////////////////////////////////////////////////////////////////////////////////
	// ReferenceType
	//////////////////////////////////////////////////////////////////////////////////////

	ITsys* ProcessPrimitiveType(const ParsingArguments& pa, PrimitiveType* self)
	{
		switch (self->prefix)
		{
		case CppPrimitivePrefix::_none:
			switch (self->primitive)
			{
			case CppPrimitiveType::_void:			return pa.tsys->PrimitiveOf({ TsysPrimitiveType::Void,		TsysBytes::_1 });
			case CppPrimitiveType::_bool:			return pa.tsys->PrimitiveOf({ TsysPrimitiveType::Bool,		TsysBytes::_1 });
			case CppPrimitiveType::_char:			return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SChar,		TsysBytes::_1 });
			case CppPrimitiveType::_wchar_t:		return pa.tsys->PrimitiveOf({ TsysPrimitiveType::UWChar,	TsysBytes::_2 });
			case CppPrimitiveType::_char16_t:		return pa.tsys->PrimitiveOf({ TsysPrimitiveType::UChar,		TsysBytes::_2 });
			case CppPrimitiveType::_char32_t:		return pa.tsys->PrimitiveOf({ TsysPrimitiveType::UChar,		TsysBytes::_4 });
			case CppPrimitiveType::_short:			return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SInt,		TsysBytes::_2 });
			case CppPrimitiveType::_int:			return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SInt,		TsysBytes::_4 });
			case CppPrimitiveType::___int8:			return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SInt,		TsysBytes::_1 });
			case CppPrimitiveType::___int16:		return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SInt,		TsysBytes::_2 });
			case CppPrimitiveType::___int32:		return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SInt,		TsysBytes::_4 });
			case CppPrimitiveType::___int64:		return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SInt,		TsysBytes::_8 });
			case CppPrimitiveType::_long:			return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SInt,		TsysBytes::_4 });
			case CppPrimitiveType::_long_int:		return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SInt,		TsysBytes::_4 });
			case CppPrimitiveType::_long_long:		return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SInt,		TsysBytes::_8 });
			case CppPrimitiveType::_float:			return pa.tsys->PrimitiveOf({ TsysPrimitiveType::Float,		TsysBytes::_4 });
			case CppPrimitiveType::_double:			return pa.tsys->PrimitiveOf({ TsysPrimitiveType::Float,		TsysBytes::_8 });
			case CppPrimitiveType::_long_double:	return pa.tsys->PrimitiveOf({ TsysPrimitiveType::Float,		TsysBytes::_8 });
			}
			break;
		case CppPrimitivePrefix::_signed:
			switch (self->primitive)
			{
			case CppPrimitiveType::_char:			return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SInt,		TsysBytes::_1 });
			case CppPrimitiveType::_short:			return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SInt,		TsysBytes::_2 });
			case CppPrimitiveType::_int:			return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SInt,		TsysBytes::_4 });
			case CppPrimitiveType::___int8:			return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SInt,		TsysBytes::_1 });
			case CppPrimitiveType::___int16:		return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SInt,		TsysBytes::_2 });
			case CppPrimitiveType::___int32:		return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SInt,		TsysBytes::_4 });
			case CppPrimitiveType::___int64:		return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SInt,		TsysBytes::_8 });
			case CppPrimitiveType::_long:			return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SInt,		TsysBytes::_4 });
			case CppPrimitiveType::_long_int:		return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SInt,		TsysBytes::_4 });
			case CppPrimitiveType::_long_long:		return pa.tsys->PrimitiveOf({ TsysPrimitiveType::SInt,		TsysBytes::_8 });
			}
			break;
		case CppPrimitivePrefix::_unsigned:
			switch (self->primitive)
			{
			case CppPrimitiveType::_char:			return pa.tsys->PrimitiveOf({ TsysPrimitiveType::UInt,		TsysBytes::_1 });
			case CppPrimitiveType::_short:			return pa.tsys->PrimitiveOf({ TsysPrimitiveType::UInt,		TsysBytes::_2 });
			case CppPrimitiveType::_int:			return pa.tsys->PrimitiveOf({ TsysPrimitiveType::UInt,		TsysBytes::_4 });
			case CppPrimitiveType::___int8:			return pa.tsys->PrimitiveOf({ TsysPrimitiveType::UInt,		TsysBytes::_1 });
			case CppPrimitiveType::___int16:		return pa.tsys->PrimitiveOf({ TsysPrimitiveType::UInt,		TsysBytes::_2 });
			case CppPrimitiveType::___int32:		return pa.tsys->PrimitiveOf({ TsysPrimitiveType::UInt,		TsysBytes::_4 });
			case CppPrimitiveType::___int64:		return pa.tsys->PrimitiveOf({ TsysPrimitiveType::UInt,		TsysBytes::_8 });
			case CppPrimitiveType::_long:			return pa.tsys->PrimitiveOf({ TsysPrimitiveType::UInt,		TsysBytes::_4 });
			case CppPrimitiveType::_long_int:		return pa.tsys->PrimitiveOf({ TsysPrimitiveType::UInt,		TsysBytes::_4 });
			case CppPrimitiveType::_long_long:		return pa.tsys->PrimitiveOf({ TsysPrimitiveType::UInt,		TsysBytes::_8 });
			}
			break;
		}
		throw NotConvertableException();
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// ReferenceType
	//////////////////////////////////////////////////////////////////////////////////////

	ITsys* ProcessReferenceType(const ParsingArguments& pa, ReferenceType* self, ExprTsysItem arg)
	{
		switch (self->reference)
		{
		case CppReferenceType::LRef:
			return arg.tsys->LRefOf();
		case CppReferenceType::RRef:
			return arg.tsys->RRefOf();
		default:
			return arg.tsys->PtrOf();
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// ArrayType
	//////////////////////////////////////////////////////////////////////////////////////

	ITsys* ProcessArrayType(const ParsingArguments& pa, ArrayType* self, ExprTsysItem arg)
	{
		if (arg.tsys->GetType() == TsysType::Array)
		{
			return arg.tsys->GetElement()->ArrayOf(arg.tsys->GetParamCount() + 1);
		}
		else
		{
			return arg.tsys->ArrayOf(1);
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// DecorateType
	//////////////////////////////////////////////////////////////////////////////////////

	ITsys* ProcessDecorateType(const ParsingArguments& pa, DecorateType* self, ExprTsysItem arg)
	{
		return arg.tsys->CVOf({ (self->isConstExpr || self->isConst), self->isVolatile });
	}
}