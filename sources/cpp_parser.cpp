#include "cpp_parser.hpp"

#include "cpp_language_definitions.hpp"

#include <string>
#include <vector>

// Typedef complexity
// https://en.cppreference.com/w/cpp/language/typedef

enum class State
{
	GlobalScope,
	CommentLine,
	CommentBlock,
	MacroExpression,

	Eof
};

std::string stateNames[] = {
	"GlobalScope",
	"CommentLine",
	"CommentBlock",
	"MacroExpression",

	"Eof"
};

void parse_cpp(const std::vector<Token>& tokens, Cpp_Parse_Result& result)
{
	Token       nameToken;
	string_ref	stringLiteral;
	size_t      previousLine = 0;
	bool        inStringLiteral = false;
	bool        inLineCount = false;
	size_t      lineMarker = 0;
	size_t      originalLine = 0;

	// Those stacks should have the same size
	// As stack to handle members and methods

	size_t  printStart = 0;
	size_t  printEnd = 0;

	states.push(State::GlobalScope);
	scopes.push(Scope());
	visibilities.push(Visibility::Public);

	for (const Token& token : tokens)
	{
		State& state = states.top();
		Scope& scope = scopes.top();

		if (types.size())
			type = &types.top();
		else
			type = nullptr;

		// Handle here states that have to be poped on new line detection
		if (token.line > previousLine
			&& (state == State::MacroExpression
				|| state == State::CommentLine))
		{
			states.pop();
			state = states.top();
		}

		if (keywordToIgnore(token.keyword))
			continue;

		// State::TypeSequence can also let the token be processed by the previous state
		if (state == State::TypeSequence)
		{
			if (isPartOfTypeSequence(result, token))
			{
				type->tokens.push_back(token);
			}
			else
			{
				states.pop();
				state = states.top();
			}
		}
		// --

		if (token.line >= printStart && token.line < printEnd)
			std::cout << std::string(states.size() - 1, ' ') << stateNames[(size_t)state]
			<< " " << token.line << " " << token.column << " " << token.text.to_string() << std::endl;

		if (state == State::CommentBlock)
		{
			if (token.punctuation == Punctuation::CloseBlockComment)
				states.pop();
		}
		else if (state == State::ExternCScope)
		{
			if (token.punctuation == Punctuation::CloseBrace)
			{
				states.pop();
			}
			else
				parseScope(result, token, states, types, type);
		}
		else if (state == State::NamespaceScope)
		{
			if (token.punctuation == Punctuation::OpenBrace)
			{
				states.pop();
				states.push(State::BraceScope);
			}
			else    // Should be the name of the namespace
			{
			}
		}
		else if (state == State::ExternInstruction)
		{
			if (token.punctuation == Punctuation::OpenBrace)
			{
				if (languages.top() == "C")
				{
					states.pop();
					states.push(State::ExternCScope);
				}
				else
				{
					states.pop();
					states.push(State::BraceScope);
				}
			}
			else if (token.punctuation == Punctuation::CloseBrace)
			{
				states.pop();
			}
			else if (token.punctuation == Punctuation::Semicolon)
			{
				states.pop();
			}
			else if (token.punctuation == Punctuation::DoubleQuote)
			{
				if (inStringLiteral)
				{
					inStringLiteral = false;
					languages.push(stringLiteral);
				}
				else
				{
					inStringLiteral = true;
					stringLiteral = string_ref();
				}
			}
			else if (inStringLiteral)
			{
				if (stringLiteral.length() == 0)
					stringLiteral = token.text;
				else
					stringLiteral = string_ref(
						token.text.string(),
						stringLiteral.position(),
						(token.text.position() + token.text.length()) - stringLiteral.position());    // Can't simply add length of currentFileName and token.text because white space tokens are skipped
			}
		}
		else if (state == State::MacroExpression)
		{
			if (token.punctuation == Punctuation::DoubleQuote)
			{
				if (inStringLiteral)
				{
					inStringLiteral = false;
				}
				else
				{
					inStringLiteral = true;
					currentFilePath = string_ref();
				}
			}
			else if (token.keyword == Keyword::Unknown
				&& token.punctuation == Punctuation::Unknown)
			{
				if (inStringLiteral)
				{
					if (currentFilePath.length() == 0)
						currentFilePath = token.text;
					else
						currentFilePath = string_ref(
							token.text.string(),
							currentFilePath.position(),
							(token.text.position() + token.text.length()) - currentFilePath.position());    // Can't simply add length of currentFileName and token.text because white space tokens are skipped
				}
				else if (inLineCount)
				{
					lineMarker = token.line;
					originalLine = (size_t)std::stoull(token.text.to_string());
					inLineCount = false;
				}
				else if (token.text == "line")
				{
					inLineCount = true;
				}
			}
		}
		else if (state == State::StaticAssertStatement)
		{
			if (token.punctuation == Punctuation::Semicolon)
				states.pop();
		}
		else if (state == State::ParentsList)
		{
			Visibility& visibility = visibilities.top();    // take a look to State::ObjectDeclaration, there is only one level of visibility here, so we change the current one directly
			if (token.keyword == Keyword::Public)
			{
				visibility = Visibility::Public;
			}
			else if (token.keyword == Keyword::Protected)
			{
				visibility = Visibility::Protected;
			}
			else if (token.keyword == Keyword::Private)
			{
				visibility = Visibility::Private;
			}
			else if (token.punctuation == Punctuation::Comma)
			{
			}
			else if (token.punctuation == Punctuation::OpenBrace)
			{
				unsetFlag(type->flags, Type::Flag::Forwarded);
				scopes.push(Scope());

				states.pop();
				visibilities.pop();
				states.push(State::ObjectScope);
			}
			else
			{
				// This assert breaks tests, but should be here
//                assert(result.userTypes.find(token.text.to_string()) != result.userTypes.end());
				Parent  parent;

				const auto& it = result.userTypes.find(token.text.to_string());
				parent.token = token;
				parent.visibility = visibilities.top();
				if (it != result.userTypes.end())
					parent.type = &it->second;
				type->parents.push_back(parent);
			}
		}
		else if (state == State::TypedefInstruction)
		{
			if (token.keyword == Keyword::ConstModifier)    // Special case to avoid TypeSequence State with "typedef const struct * ..."
			{
				type->tokens.push_back(token);
			}
			else if (isPartOfTypeSequence(result, token))
			{
				if (result.userTypes.find(token.text.to_string()) != result.userTypes.end())    // It is a typedef on a UserType (maybe typedefed)
					type->primitive = token;

				type->tokens.push_back(token);
				// TODO check consequences of those 2 lines (it seems to break function ptr typedef)
				states.pop();
				states.push(State::TypedefList);
				states.push(State::TypeSequence);
			}
			else if (token.punctuation == Punctuation::Semicolon)
			{
				// Clearing values
				types.pop();
				states.pop();
			}
			else if (token.keyword == Keyword::Class)
			{
				states.pop();
				states.push(State::TypedefClass);
			}
			else if (token.keyword == Keyword::Struct)
			{
				states.pop();
				states.push(State::TypedefStruct);
			}
			else if (token.keyword == Keyword::Union)
			{
				states.pop();
				states.push(State::TypedefUnion);
			}
			else if (token.keyword == Keyword::Enum)
			{
				states.pop();
				states.push(State::TypedefEnum);
			}
			else if (token.keyword == Keyword::Typename)
			{
				states.pop();
				states.push(State::TypedefTypename);
			}
			else if (token.keyword == Keyword::MS_DECLSPEC)
			{
				states.push(State::DeclspecDeclaration);
			}
			else if (token.punctuation == Punctuation::OpenParenthesis)
			{
				states.pop();
				states.push(State::FunctionPointerName);
			}
			else
			{
				type->filePath = currentFilePath;
				insertUserType(*type, token.text.to_string(), result);
			}
		}
		else if (state == State::FunctionPointerName)
		{
			if (token.punctuation == Punctuation::CloseParenthesis)
			{
				states.pop();
				states.push(State::FunctionPointerArgumentsList);
			}
			else if (token.punctuation == Punctuation::OpenParenthesis)
			{
				states.push(State::ParenthesisScope);   // We may have some parenthethis in the some cases
			}
			else if (token.keyword == Keyword::GNU_ATTRIBUTE)
			{
				states.push(State::CompilerModifier);
			}
			else if (token.keyword == Keyword::MS_DECLSPEC)
			{
				states.push(State::CompilerModifier);
			}
			else if (token.keyword == Keyword::MS_STDCALL)
			{
				// TODO Ignored on function ptr that we don't really need
			}
			else if (token.punctuation == Punctuation::Star)
			{
			}
			else
			{
				setFlag(type->flags, Type::Flag::FunctionPointer);
				type->userName = token;
				insertUserType(*type, token.text.to_string(), result);
			}
		}
		else if (state == State::FunctionPointerArgumentsList)
		{
			// TODO should be factored with State::FunctionArgumentsList
			// but in this case State::FunctionPointerName is complete in
			// the Type building
			if (token.punctuation == Punctuation::OpenParenthesis)  // Should start by this
			{
				states.push(State::ParenthesisScope);
			}
			else if (token.punctuation == Punctuation::Semicolon)
			{
				types.pop();
				states.pop();
			}
		}
		else if (state == State::TypedefStruct
			|| state == State::TypedefClass
			|| state == State::TypedefUnion)
		{
			const auto& it = result.userTypes.find(token.text.to_string());

			if (token.keyword == Keyword::MS_DECLSPEC)
			{
				states.push(State::CompilerModifier);
			}
			else if (token.punctuation == Punctuation::OpenBrace)
			{
				unsetFlag(type->flags, Type::Flag::Forwarded);
				scopes.push(Scope());

				states.pop();
				states.push(State::TypedefList);
				states.push(State::ObjectScope);
			}
			else if (it != result.userTypes.end())    // struct is already defined
			{
				type->primitive = token;
				type->tokens.push_back(token);
				states.pop();
				if (isFlagSet(it->second.flags, Type::Flag::Forwarded))   // Type was only forward declared
				{
					setObjectFlag(*type, state);    // Object is newly created (this type isn't the one already registered ;-))
					states.push(State::TypedefObjectList);          // To retrieve new definition (with methods and members)
					states.push(State::TypedefObjectDeclaration);
				}
				else
					states.push(State::TypedefList);
			}
			else
			{
				setFlag(type->flags, Type::Flag::Forwarded);
				setObjectFlag(*type, state);
				type->filePath = currentFilePath;
				type->userName = token;
				//                insertUserType(*type, token.text.to_string(), result);
				type->tokens.push_back(token);  // This token necessary for the typedef type not the for the source type
				type->primitive = token;

				states.pop();
				states.push(State::TypedefObjectList);        // Will fall back on this state after the fill of the struct declaration
				states.push(State::TypedefObjectDeclaration);
			}
		}
		else if (state == State::TypedefEnum)
		{
			if (result.userTypes.find(token.text.to_string()) != result.userTypes.end())    // enum is already defined
			{
				type->tokens.push_back(token);
				states.pop();
				states.push(State::TypedefList);
			}
			else if (token.punctuation == Punctuation::OpenBrace)   // The enum can be anonymous
			{
				// We may want a special state to get the enums values
				states.pop();
				states.push(State::TypedefList);
				states.push(State::EnumList);
			}
			else
			{
				type->filePath = currentFilePath;
				type->userName = token;
				insertUserType(*type, token.text.to_string(), result);
				type->tokens.push_back(token);  // This token necessary for the typedef type not the for the source type
				type->primitive = token;

				states.pop();
				states.push(State::TypedefObjectList);        // Will fall back on this state after the fill of the struct declaration
				states.push(State::EnumList);
			}
		}
		else if (state == State::TypedefTypename)
		{
			if (token.punctuation == Punctuation::OpenBrace)   // The enum can be anonymous
			{
				states.pop();
				states.push(State::BraceScope);
			}
			else if (token.punctuation == Punctuation::Semicolon)
			{
				states.pop();
			}
		}
		else if (state == State::TypedefList
			|| state == State::TypedefObjectList)
		{
			if (token.punctuation == Punctuation::Star)
			{
				setFlag(type->flags, Type::Flag::Pointer);
				type->nbPointers++;
				type->tokens.push_back(token);
			}
			else if (token.punctuation == Punctuation::Comma)
			{
			}
			else if (token.punctuation == Punctuation::Semicolon)
			{
				if (state == State::TypedefObjectList)
				{
					type->userName = type->primitive;
					type->primitive = Token();
					insertUserType(*type, type->userName.text.to_string(), result);
				}

				// Clearing values
				types.pop();
				states.pop();
			}
			else if (token.punctuation == Punctuation::OpenParenthesis) // We got a typedef on a function ptr, it also work for typedef on function declaration
			{
				states.pop();
				if (type->userName.text.length() == 0)
					states.push(State::FunctionPointerName);
				else
				{
					setFlag(type->flags, Type::Flag::Function);
					states.push(State::FunctionPointerArgumentsList);
				}
			}
			else if (token.punctuation == Punctuation::OpenBracket)
			{
				states.push(State::ArraySize);
			}
			else if (token.punctuation == Punctuation::Hash)    // Macro
				states.push(State::MacroExpression);
			else if (token.keyword == Keyword::MS_STDCALL)
			{
				setFlag(function.flags, Function::Flag::StdCall);
			}
			else    // typedef on function declaration goes here before in the ParenthesisScope
			{
				std::string typeName = token.text.to_string();
				if (result.userTypes.find(typeName) == result.userTypes.end())
				{
					Type    typedefedType;

					if (state == State::TypedefList)    // Typedef on basic type or functions (the typedefed target ins't defined at the same time)
					{
						setFlag(type->flags, Type::Flag::Typedef);
						type->userName = token;              // This enforce the name
						type->filePath = currentFilePath;
						insertUserType(*type, typeName, result);
					}
					else    // Create a new type, because the typedefed type don't hold the methods and members
					{
						typedefedType.flags = type->flags;
						setFlag(typedefedType.flags, Type::Flag::Typedef);
						typedefedType.userName = token;
						typedefedType.primitive = type->primitive;
						typedefedType.filePath = currentFilePath;
						typedefedType.tokens = type->tokens;
						insertUserType(typedefedType, typeName, result);
					}
				}
				if (type->tokens.size()
					&& type->tokens.back().punctuation == Punctuation::Star)
				{
					unsetFlag(type->flags, Type::Flag::Pointer);
					type->nbPointers++;
					type->tokens.resize(type->tokens.size() - 1); // remove it
				}
			}
		}
		else if (state == State::ArraySize)
		{
			if (token.punctuation == Punctuation::OpenBracket)
			{
				states.push(State::ArraySize);
			}
			else if (token.punctuation == Punctuation::CloseBracket)
			{
				states.pop();
			}
			else
			{
				if (variable.name.text.length())  // Hacky to avoid potential issues
				{
					variable.flags = Variable::Flag::Array;
					variable.count = (size_t)std::stoull(token.text.to_string());
				}
			}
		}
		else if (state == State::OperatorDeclaration)
		{
			if (token.punctuation == Punctuation::OpenParenthesis) // It seems to be a function declaration
			{
				types.pop();    // The return type of the operator
				states.push(State::ParenthesisScope);   // skip function ptr definitions
			}
			else if (token.punctuation == Punctuation::OpenBrace)
			{
				states.pop();
				states.push(State::BraceScope);
			}
			else if (token.punctuation == Punctuation::Semicolon)   // It is a declaration
			{
				states.pop();
			}
		}
		else if (state == State::VariableOrFunctionDeclaration
			|| state == State::VirtualFunctionDeclaration)
		{
			if (token.punctuation == Punctuation::OpenParenthesis) // It seems to be a function declaration
			{
				if (nameToken.text.length())
				{
					function.returnType = *type;
					function.name = nameToken;
					function.filePath = currentFilePath;
					if (state == State::VirtualFunctionDeclaration)
						setFlag(function.flags, Function::Flag::Virtual);

					// Clearing values
					types.pop();
					nameToken = Token();

					types.push(Type()); // Arguments have a type
					states.pop();
					states.push(State::FunctionArgumentsList);
				}
				else    // It should be a function ptr if there is no name
				{
					states.push(State::ParenthesisScope);   // skip function ptr definitions
				}
			}
			else if (token.punctuation == Punctuation::OpenBracket)
			{
				states.push(State::ArraySize);
			}
			else if (token.punctuation == Punctuation::Semicolon   // It is a variable declaration
				|| token.punctuation == Punctuation::Comma)
			{
				variable.name = nameToken;
				variable.type = *type;

				insertVariable(variable, scope);

				// Clearing values
				variable = Variable();
				nameToken = Token();

				types.pop();
				states.pop();
			}
			else if (token.keyword == Keyword::Operator)
			{
				states.pop();
				states.push(State::OperatorDeclaration);
			}
			else if (token.keyword == Keyword::GNU_ATTRIBUTE)
			{
				states.push(State::CompilerModifier);
			}
			else if (token.keyword == Keyword::MS_DECLSPEC)
			{
				states.push(State::CompilerModifier);
			}
			else if (token.keyword == Keyword::MS_STDCALL)
			{
				setFlag(function.flags, Function::Flag::StdCall);
			}
			else if (token.keyword == Keyword::Unknown)
			{
				nameToken = token;
			}
		}
		else if (state == State::VariablesDeclaration)
		{
			if (token.punctuation == Punctuation::Semicolon)
			{
				states.pop();
			}
			else if (token.punctuation == Punctuation::Comma)
			{
			}
			else
			{
				variable.type = *type;
				variable.name = token;

				// Clearing values
				variable = Variable();

				insertVariable(variable, scope);
			}
		}
		else if (state == State::FunctionArgumentsList)
		{
			if (isPartOfTypeSequence(result, token))
			{
				type->tokens.push_back(token);
				states.push(State::TypeSequence);

				if (token.line >= printStart && token.line < printEnd)
					std::cout << "function: " << function.name.text.to_string()
					<< std::endl;
			}
			else if (token.keyword == Keyword::Throw)
			{
				setFlag(function.flags, Function::Flag::Throw);
			}
			else if (token.keyword == Keyword::Noexcept)
			{
				setFlag(function.flags, Function::Flag::Noexcept);
			}
			else if (token.punctuation == Punctuation::OpenParenthesis)
			{
				//                if (isFlagSet(function.flags, Function::Flag::Throw)
				//                    || isFlagSet(function.flags, Function::Flag::Noexcept))
				states.push(State::ParenthesisScope);
			}
			else if (token.punctuation == Punctuation::Comma
				|| token.punctuation == Punctuation::CloseParenthesis)
			{
				polishType(result, *type, currentFilePath, lineMarker, originalLine);
				variable.type = *type;

				if (!(variable.type.tokens.size() == 1
					&& variable.type.tokens[0].keyword == Keyword::Void))
					function.arguments.push_back(variable);

				// Clearing values
				unsetFlag(flags, ParsingFlag::FunctionArgumentDefaultValue);
				unsetFlag(flags, ParsingFlag::ConstructorInitilizationList);
				types.pop();
				variable = Variable();

				if (token.punctuation == Punctuation::Comma)
					types.push(Type());
			}
			else if (token.punctuation == Punctuation::Semicolon)   // We can push the Function declaration
			{
				insertFunction(function, scope);

				// Clearing values
				unsetFlag(flags, ParsingFlag::FunctionArgumentDefaultValue);
				unsetFlag(flags, ParsingFlag::ConstructorInitilizationList);
				function = Function();

				states.pop();
			}
			else if (token.punctuation == Punctuation::OpenBrace)
			{
				insertFunction(function, scope);

				// Clearing values
				unsetFlag(flags, ParsingFlag::FunctionArgumentDefaultValue);
				unsetFlag(flags, ParsingFlag::ConstructorInitilizationList);
				function = Function();

				states.pop();
				states.push(State::FunctionDefinition);
			}
			else if (token.punctuation == Punctuation::OpenBracket)
			{
				states.push(State::ArraySize);
			}
			else if (token.punctuation == Punctuation::Equals)
			{
				setFlag(flags, ParsingFlag::FunctionArgumentDefaultValue);
			}
			else if (token.punctuation == Punctuation::Colon)   // Constructor initialization list (before the function definition scope)
			{
				setFlag(flags, ParsingFlag::ConstructorInitilizationList);
			}
			else if (isFlagSet(flags, ParsingFlag::FunctionArgumentDefaultValue) == false)
			{
				variable.name = token;
			}
		}
		else if (state == State::ObjectDeclaration
			|| state == State::TypedefObjectDeclaration)
		{
			const auto& it = result.userTypes.find(token.text.to_string());

			if (token.punctuation == Punctuation::OpenBrace)
			{
				unsetFlag(type->flags, Type::Flag::Forwarded);
				scopes.push(Scope());

				states.pop();
				if (state != State::TypedefObjectDeclaration)   // If there is no typedef the list after the '}' will be a list of variable names
					states.push(State::ObjectVariablesDeclaration);
				states.push(State::ObjectScope);
			}
			else if (token.punctuation == Punctuation::Semicolon)
			{
				states.pop();
			}
			else if (token.punctuation == Punctuation::Colon)
			{
				states.pop();
				states.push(State::ParentsList);
				// There is only one level of visibility in the parents list
				// Will be poped by the State::ParentsList
				visibilities.push(Visibility::Public);  // TODO check if it is a struct or a class because the default visibility level is different
			}
			else if (token.keyword == Keyword::GNU_ATTRIBUTE)
			{
				states.push(State::CompilerModifier);
			}
			else if (token.keyword == Keyword::MS_DECLSPEC)   // Alignement attributs can come here
			{
				states.push(State::DeclspecDeclaration);
			}
			else if (token.punctuation == Punctuation::Star)
			{
				setFlag(type->flags, Type::Flag::Pointer);
				type->nbPointers++;
				type->tokens.push_back(token);
				states.pop();
			}
			//            else if (isPartOfTypeSequence(result, token))
			//            {
			//                type->userName = token;  // Can be modified later in case of a typedef
			//                states.push(State::TypeSequence);
			//            }
			else if (it == result.userTypes.end()
				|| isFlagSet(it->second.flags, Type::Flag::Forwarded))   // it is a new structure or class definition
			{
				type->userName = token;  // Can be modified later in case of a typedef
				type->filePath = currentFilePath;
				insertUserType(*type, token.text.to_string(), result);

				states.pop();
				if (state != State::TypedefObjectDeclaration)   // If there is no typedef the list after the '}' will be a list of variable names
					states.push(State::ObjectVariablesDeclaration);
			}
			else // it should be a return type of a function or a Variable declaration specified with the struct keyword specifier (look at test_parse_tricky_things)
			{
				type->userName = token;
				type->filePath = currentFilePath;

				states.pop();
				if (state != State::TypedefObjectDeclaration)
					states.push(State::VariableOrFunctionDeclaration);
			}
		}
		else if (state == State::ObjectVariablesDeclaration)
		{
			if (token.punctuation == Punctuation::OpenBrace)
			{
				unsetFlag(type->flags, Type::Flag::Forwarded);
				scopes.push(Scope());
				states.push(State::ObjectScope);
			}
			else if (token.punctuation == Punctuation::Semicolon)
			{
				if (type->userName.text.length())   // Avoid anonymous struct "struct {...} variableA;"
					insertUserType(*type, type->userName.text.to_string(), result);  // We have to update the type

				types.pop();
				states.pop();
			}
			else if (isPartOfTypeSequence(result, token))   // It seems to be a forward declared type finally (take a look to test_parse_tricky_things_5)
			{
				type->tokens.push_back(token);
				states.pop();
				states.push(State::VariableOrFunctionDeclaration);
				states.push(State::TypeSequence);
			}
			else if (token.punctuation == Punctuation::Colon)
			{
				states.push(State::ParentsList);
				// There is only one level of visibility in the parents list
				// Will be poped by the State::ParentsList
				visibilities.push(Visibility::Public);  // TODO check if it is a struct or a class because the default visibility level is different
			}
		}
		else if (state == State::ObjectScope)
		{
			// TODO factorise the code with the State::GlobalScope and State::NamespaceScope
			if (token.punctuation == Punctuation::CloseBrace)
			{
				type->members = scope.variables;
				type->methods = scope.functions;

				scopes.pop();
				states.pop();
			}
			else
				parseScope(result, token, states, types, type);
		}
		else if (state == State::ParenthesisScope)
		{
			// TODO We may want to store the list of the tokens between parenthesis
			// In this way it could be easier to process it for the arguments list
			// of function per example
			if (token.punctuation == Punctuation::CloseParenthesis)
				states.pop();
			else if (token.punctuation == Punctuation::OpenParenthesis)
				states.push(State::ParenthesisScope);
		}
		else if (state == State::BraceScope
			|| state == State::FunctionDefinition)
		{
			if (token.punctuation == Punctuation::CloseBrace)
				states.pop();
			else if (token.punctuation == Punctuation::OpenBrace)
				states.push(State::BraceScope);
		}
		else if (state == State::EnumList)
		{
			if (token.punctuation == Punctuation::CloseBrace)
				states.pop();
		}
		else if (state == State::CompilerModifier)
		{
			if (token.punctuation == Punctuation::OpenParenthesis)
			{
				states.pop();
				states.push(State::ParenthesisScope);
			}
		}
		else if (state == State::DeclspecDeclaration)
		{
			if (token.punctuation == Punctuation::OpenParenthesis)
			{
				states.push(State::DeclspecDefinition);
			}
			else if (token.punctuation == Punctuation::CloseParenthesis)
			{
				states.pop();
			}
		}
		else if (state == State::DeclspecDefinition)
		{
			if (token.keyword == Keyword::MS_UUID)
			{
				states.pop();
				states.push(State::UUID);
			}
			else if (token.punctuation == Punctuation::OpenParenthesis)
			{
				states.push(State::ParenthesisScope);
			}
			else    // Fall back on Declspec declaration for this undefined definition
			{
				states.pop();
			}
		}
		else if (state == State::UUID)
		{
			if (token.punctuation == Punctuation::OpenParenthesis)
			{
			}
			else if (token.punctuation == Punctuation::CloseParenthesis)
			{
				states.pop();
			}
			else if (token.punctuation == Punctuation::DoubleQuote)
			{
				if (inStringLiteral)
				{
					inStringLiteral = false;
					type->uuid = stringLiteral;
				}
				else
				{
					inStringLiteral = true;
					stringLiteral = string_ref();
				}
			}
			else if (inStringLiteral)
			{
				if (stringLiteral.length() == 0)
					stringLiteral = token.text;
				else
					stringLiteral = string_ref(
						token.text.string(),
						stringLiteral.position(),
						(token.text.position() + token.text.length()) - stringLiteral.position());    // Can't simply add length of currentFileName and token.text because white space tokens are skipped
			}
		}
		else if (state == State::TempateArgumentsList)
		{
			if (token.punctuation == Punctuation::Less)
			{

			}
			else if (token.punctuation == Punctuation::Greater)
			{
				states.pop();
			}
		}
		else if (state == State::GlobalScope)
		{
			parseScope(result, token, states, types, type);
		}
		previousLine = token.line;
	}

	// scopes stack should have a size of 1
	result.functions = scopes.top().functions;
	result.globals = scopes.top().variables;

	//    while (states.size())
	//    {
	//        const State& state = states.top();
	//        std::cout << std::string(states.size() - 1, ' ') << stateNames[(size_t)state] << std::endl;
	//        states.pop();
	//    }

	assert(types.size() == 0);
	assert(scopes.size() == 1);
	assert(states.size() >= 1 && states.size() <= 2);
	assert(states.top() == State::GlobalScope
		|| states.top() == State::MacroExpression);

	return result;
}
