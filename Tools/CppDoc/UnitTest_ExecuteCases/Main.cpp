#include <Parser.h>

using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::console;

// skip these tokens to in space
struct TokenSkipping
{
	vint		rowSkip = -1;
	vint		columnSkip = -1;
	vint		rowUntil = -1;
	vint		columnUntil = -1;
};

bool NeedToSkip(RegexToken& token)
{
	switch ((CppTokens)token.token)
	{
	case CppTokens::SPACE:
	case CppTokens::COMMENT1:
	case CppTokens::COMMENT2:
	case CppTokens::SHARP:
		return true;
	}
	return false;
}

void CleanUpPreprocessFile(Ptr<RegexLexer> lexer, FilePath pathInput, FilePath pathPreprocessed, FilePath pathOutput, FilePath pathMapping)
{
	WString input = File(pathInput).ReadAllTextByBom();
	File(pathPreprocessed).WriteAllText(input, false, BomEncoder::Utf8);

	List<TokenSkipping> mapping;
	{
		CppTokenReader reader(lexer, input, false);
		auto cursor = reader.GetFirstToken();

		FileStream fileStream(pathOutput.GetFullPath(), FileStream::WriteOnly);
		Utf8Encoder encoder;
		EncoderStream encoderStream(fileStream, encoder);
		StreamWriter writer(encoderStream);

		while (cursor)
		{
			switch ((CppTokens)cursor->token.token)
			{
			case CppTokens::SPACE:
			case CppTokens::COMMENT1:
			case CppTokens::COMMENT2:
			case CppTokens::SHARP:
				{
					auto oldCursor = cursor;

					TokenSkipping ts;
					ts.rowSkip = cursor->token.rowStart;
					ts.columnSkip = cursor->token.columnStart;

					while (cursor)
					{
						ts.rowUntil = cursor->token.rowStart;
						ts.columnUntil = cursor->token.columnStart;

						if ((CppTokens)cursor->token.token == CppTokens::SHARP)
						{
							vint parenthesisCounter = 0;
							vint lastSharpRowEnd = ts.rowUntil;
							bool lastTokenIsSkipping = false;

							while (cursor)
							{
								ts.rowUntil = cursor->token.rowStart;
								ts.columnUntil = cursor->token.columnStart;
								if (lastTokenIsSkipping && lastSharpRowEnd != ts.rowUntil)
								{
									goto STOP_SHARP;
								}
								lastSharpRowEnd = ts.rowUntil;

								switch ((CppTokens)cursor->token.token)
								{
								case CppTokens::LPARENTHESIS:
									lastTokenIsSkipping = false;
									parenthesisCounter++;
									break;
								case CppTokens::RPARENTHESIS:
									lastTokenIsSkipping = false;
									parenthesisCounter--;
									if (parenthesisCounter == 0)
									{
										cursor = cursor->Next();
										goto STOP_SHARP;
									}
									break;
								case CppTokens::SPACE:
								case CppTokens::COMMENT1:
								case CppTokens::COMMENT2:
									lastTokenIsSkipping = true;
									break;
								default:
									lastTokenIsSkipping = false;
								}
								cursor = cursor->Next();
							}
						STOP_SHARP:;
						}
						else
						{
							switch ((CppTokens)cursor->token.token)
							{
							case CppTokens::SPACE:
							case CppTokens::COMMENT1:
							case CppTokens::COMMENT2:
								break;
							default:
								goto STOP_SKIPPING;
							}
							cursor = cursor->Next();
						}
					}
				STOP_SKIPPING:

					if (ts.rowSkip == ts.rowUntil)
					{
						cursor = oldCursor;
					}
					else
					{
						// prevent from stack overflowing in CppTokenCursor's destructor
						while (oldCursor != cursor)
						{
							oldCursor = oldCursor->Next();
						}
						mapping.Add(ts);
						writer.WriteLine(L"", 0);
					}
				}
			}

			if (cursor)
			{
				writer.WriteString(cursor->token.reading, cursor->token.length);
				cursor = cursor->Next();
			}
		}
	}
	{
		FileStream fileStream(pathMapping.GetFullPath(), FileStream::WriteOnly);
		{
			vint count = mapping.Count();
			fileStream.Write(&count, sizeof(count));
		}
		if (mapping.Count() > 0)
		{
			fileStream.Write(&mapping[0], sizeof(TokenSkipping) * mapping.Count());
		}
	}
}

class IndexRecorder : public Object, public virtual IIndexRecorder
{
public:
	struct Token
	{
		vint			rowStart;
		vint			columnStart;
		vint			rowEnd;
		vint			columnEnd;

		static vint Compare(const Token& a, const Token& b)
		{
			vint result;
			if ((result = a.rowStart - b.rowStart) != 0) return result;
			if ((result = a.columnStart - b.columnStart) != 0) return result;
			if ((result = a.rowEnd - b.rowEnd) != 0) return result;
			if ((result = a.columnEnd - b.columnEnd) != 0) return result;
			return 0;
		}

		bool operator <  (const Token& k)const { return Compare(*this, k) <  0; }
		bool operator <= (const Token& k)const { return Compare(*this, k) <= 0; }
		bool operator >  (const Token& k)const { return Compare(*this, k) >  0; }
		bool operator >= (const Token& k)const { return Compare(*this, k) >= 0; }
		bool operator == (const Token& k)const { return Compare(*this, k) == 0; }
		bool operator != (const Token& k)const { return Compare(*this, k) != 0; }
	};

	enum Reason
	{
		Resolved,
		OverloadedResolution,
		NeedValueButType,
	};

	using Key = Tuple<Token, Reason>;
	using IndexMap = Group<Key, Symbol*>;
	using ReverseIndexMap = Group<Symbol*, Key>;

	IndexMap&			index;
	ReverseIndexMap&	reverseIndex;

	IndexRecorder(IndexMap& _index, ReverseIndexMap& _reverseIndex)
		:index(_index)
		, reverseIndex(_reverseIndex)
	{
	}

	static Token GetToken(CppName& name)
	{
		return {
			name.nameTokens[0].rowStart,
			name.nameTokens[0].columnStart,
			name.nameTokens[name.tokenCount - 1].rowEnd,
			name.nameTokens[name.tokenCount - 1].columnEnd,
		};
	}

	void IndexInternal(CppName& name, Ptr<Resolving> resolving, Reason reason)
	{
		if (name.tokenCount > 0)
		{
			Key key = { GetToken(name),reason };
			for (vint i = 0; i < resolving->resolvedSymbols.Count(); i++)
			{
				auto symbol = resolving->resolvedSymbols[i];
				if (!index.Contains(key, symbol))
				{
					index.Add(key, symbol);
					reverseIndex.Add(symbol, key);
				}
			}
		}
	}

	void Index(CppName& name, Ptr<Resolving> resolving)override
	{
		IndexInternal(name, resolving, Resolved);
	}

	void IndexOverloadingResolution(CppName& name, Ptr<Resolving> resolving)override
	{
		IndexInternal(name, resolving, OverloadedResolution);
	}

	void ExpectValueButType(CppName& name, Ptr<Resolving> resolving)override
	{
		IndexInternal(name, resolving, NeedValueButType);
	}
};

void Compile(Ptr<RegexLexer> lexer, FilePath pathFolder, FilePath pathInput)
{
	Dictionary<WString, Symbol*> ids;
	IndexRecorder::IndexMap index;
	IndexRecorder::ReverseIndexMap reverseIndex;
	Dictionary<IndexRecorder::Token, Symbol*> decls;

	WString input = File(pathInput).ReadAllTextByBom();
	CppTokenReader reader(lexer, input);
	auto cursor = reader.GetFirstToken();

	ParsingArguments pa(new Symbol, ITsysAlloc::Create(), new IndexRecorder(index, reverseIndex));
	auto program = ParseProgram(pa, cursor);
	EvaluateProgram(pa, program);

	pa.root->GenerateUniqueId(ids, L"");
	for (vint i = 0; i < ids.Count(); i++)
	{
		auto symbol = ids.Values()[i];
		if (symbol->definition)
		{
			auto& name = symbol->definition->name;
			if (name.tokenCount > 0)
			{
				decls.Add(IndexRecorder::GetToken(name), symbol);
			}
		}
		for (vint i = 0; i < symbol->declarations.Count(); i++)
		{
			auto& name = symbol->declarations[i]->name;
			if (name.tokenCount > 0)
			{
				decls.Add(IndexRecorder::GetToken(name), symbol);
			}
		}
	}
}

int main()
{
	Folder folderCase(L"../UnitTest_Cases");
	List<File> files;
	folderCase.GetFiles(files);
	auto lexer = CreateCppLexer();

	FOREACH(File, file, files)
	{
		if (wupper(file.GetFilePath().GetFullPath().Right(2)) == L".I")
		{
			Folder folderOutput(file.GetFilePath().GetFullPath() + L".Output");
			if (!folderOutput.Exists())
			{
				folderOutput.Create(false);
			}

			Console::WriteLine(L"Preprocessing " + file.GetFilePath().GetName() + L" ...");
			CleanUpPreprocessFile(
				lexer,
				file.GetFilePath(),
				folderOutput.GetFilePath() / L"Preprocessed.cpp",
				folderOutput.GetFilePath() / L"Input.cpp",
				folderOutput.GetFilePath() / L"Mapping.bin"
			);

			Console::WriteLine(L"Compiling " + file.GetFilePath().GetName() + L" ...");
			Compile(
				lexer,
				file.GetFilePath(),
				folderOutput.GetFilePath() / L"Input.cpp"
			);
		}
	}

	return 0;
}