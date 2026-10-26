#include "ASTBuilder.hpp"

#include <iostream>

#include "ASTUtils.hpp"

namespace ast
{
	void Builder::OutputError(Token* token, std::string errorMessage)
	{
		//__debugbreak();

		ParseError error{};
		error.line = token->line;
		error.column = token->column;
		error.message = errorMessage;

		errors_.push_back(error);

		std::cout << "\t <-- " << errorMessage;
	}

	Node* Builder::GetDigit(int& tokenIndex)
	{
		std::cout << "\nGetDigit at tokenIndex = " + std::to_string(tokenIndex);

		if (tokenIndex == tokens_.size())
		{
			return nullptr;
		}

		if (tokens_[tokenIndex]->GetType() == TokenType::OpenBracket)
		{
			isParsingBrackets = true;

			blockContext_.bracketsLevel++;

			tokenIndex++;

			Node* eval = GetEval(tokenIndex);

			if (eval == nullptr)
			{
				return nullptr;
			}

			if (tokenIndex == tokens_.size() || tokens_[tokenIndex]->GetType() != TokenType::CloseBracket)
			{
				OutputError(tokens_[tokenIndex], "Close backet is missed");

				return nullptr;
			}

			isParsingBrackets = false;

			blockContext_.bracketsLevel--;

			tokenIndex++;

			return eval;
		}

		Node* ret = nullptr;

		switch (tokens_[tokenIndex]->GetType())
		{
		case TokenType::Digit:
			ret = new ValueNode(tokens_[tokenIndex]->AsToken<DigitToken>()->value);
			break;
		case TokenType::Id:
			ret = new VarNode(tokens_[tokenIndex]->AsToken<IdToken>()->idName);
			break;
		default:
			OutputError(tokens_[tokenIndex], "Excepted value or expression");

			return nullptr;
		}

		ret->SetSourceLocation(tokens_[tokenIndex]->location);

		tokenIndex++;

		return ret;
	}

	Node* Builder::GetCast(int& tokenIndex)
	{
		CastNode* cast = nullptr;

		// считываем '(' <type> ')'
		if (tokens_[tokenIndex]->GetType() == TokenType::OpenBracket)
		{			
			// TODO если в выражениях будут какие-то ключевые слова (см static_cast) кроме конверсий, то тут случится syntax error
			if (tokens_[tokenIndex+1]->GetType() != TokenType::Keyword)
			{
				return GetDigit(tokenIndex);
			}

			SourceLocation loc = tokens_[tokenIndex]->location;

			// перепрыгиваем (
			tokenIndex++;

			// TODO учитывать и ID токены
			KeywordToken* tok = tokens_[tokenIndex]->AsToken<KeywordToken>();

			std::optional<DataType> typeTo = GetTypeFromKeyword(tok);

			if (!typeTo.has_value())
			{
				OutputError(tokens_[tokenIndex], "Excepted Type name");

				return nullptr;
			}

			if (typeTo.value() == DataType::Void)
			{
				OutputError(tokens_[tokenIndex], "Conversion to void not allowed");
			}

			tokenIndex++;

			if (tokens_[tokenIndex]->GetType() != TokenType::CloseBracket)
			{
				OutputError(tokens_[tokenIndex], "Excepted )");

				return nullptr;
			}

			tokenIndex++;

			cast = new CastNode();

			cast->SetSourceLocation(loc);
			cast->SetCastType(typeTo.value());
		}

		Node* digit = GetDigit(tokenIndex);

		if (digit == nullptr)
		{
			return nullptr;
		}

		if (cast == nullptr)
		{
			return digit;
		}

		cast->SetExprNode(digit);

		return cast;
	}

	ArgSeqNode* Builder::GetArgs(int& tokenIndex)
	{
		std::cout << "\nGetArgs at tokenIndex = " + std::to_string(tokenIndex);

		isParsingBrackets = true;

		blockContext_.bracketsLevel++;

		Node* eval = GetEval(tokenIndex);

		ArgSeqNode* rootArg = new ArgSeqNode();

		ArgSeqNode* currentArg = rootArg;
		currentArg->SetExprNode(eval);

		if (eval == nullptr)
		{
			return nullptr;
		}

		if (tokenIndex == tokens_.size())
		{
			return nullptr;
		}

		bool isError = false;

		while (!isError)
		{
			switch (tokens_[tokenIndex]->GetType())
			{
			case TokenType::CloseBracket:
				tokenIndex++;
				isParsingBrackets = false;
				blockContext_.bracketsLevel--;
				return rootArg;
			case TokenType::Operator:
			{
				OperatorToken* op = tokens_[tokenIndex]->AsToken<OperatorToken>();

				if (op->opType != OperatorTokenType::Comma)
				{
					isError = true;
					break;
				}

				tokenIndex++;

				eval = GetEval(tokenIndex);

				if (eval == nullptr)
				{
					return nullptr;
				}

				ArgSeqNode* next = new ArgSeqNode();
				next->SetExprNode(eval);

				currentArg->SetNextArgNode(next);

				currentArg = next;
			}
			break;
			default:
				isError = true;
			}
		}

		OutputError(tokens_[tokenIndex], "Excepted comma or close bracket after arguments");

		return nullptr;
	}

	Node* Builder::GetValue(int& tokenIndex)
	{
		std::cout << "\nGetValue at tokenIndex = " + std::to_string(tokenIndex);

		switch (tokens_[tokenIndex]->GetType())
		{
		case TokenType::Id:
		{
			SourceLocation funcLoc = tokens_[tokenIndex]->location;

			std::string name = tokens_[tokenIndex]->AsToken<IdToken>()->idName;

			if (tokens_[tokenIndex + 1]->GetType() == TokenType::OpenBracket)
			{
				// чтоб перепрыгнуть имя токена и скобку
				tokenIndex++;
				tokenIndex++;
				blockContext_.isParsingArgs = true;

				ArgSeqNode* args = GetArgs(tokenIndex);

				blockContext_.isParsingArgs = false;

				if (args == nullptr)
				{
					return nullptr;
				}

				FunctionCallNode* node = new FunctionCallNode();
				node->SetName(name);
				node->SetRootArgNode(args);
				node->SetSourceLocation(funcLoc);

				while (args != nullptr)
				{
					node->AddArg(args->ExprNode());

					args = args->NextArg();
				}

				return node;
			}
			else
			{
				Node* digit = GetDigit(tokenIndex);

				return digit;
			}
		}
		case TokenType::OpenBracket:
			[[fallthrough]];
		case TokenType::Digit:
		{
			Node* digit = GetCast(tokenIndex);

			if (digit == nullptr)
			{
				return nullptr;
			}

			return digit;
		}
		case TokenType::Semicolon:
			return nullptr;
		}

		OutputError(tokens_[tokenIndex], "Excepted digit or expression");

		return nullptr;
	}

	Node* Builder::GetFactor(int& tokenIndex)
	{
		std::cout << "\nGetFactor at tokenIndex = " + std::to_string(tokenIndex);

		Node* value;

		if (tokens_[tokenIndex]->GetType() == TokenType::Operator &&
			tokens_[tokenIndex]->AsToken<OperatorToken>()->opType == OperatorTokenType::UnaryMinus)
		{
			SourceLocation location = tokens_[tokenIndex]->location;

			tokenIndex++;

			value = GetValue(tokenIndex);

			if (value == nullptr)
			{
				return nullptr;
			}

			UnaryOperatorNode* opNode = new UnaryOperatorNode(UnaryOperatorType::UnaryMinus);

			opNode->SetSourceLocation(location);
			opNode->SetOperand(value);

			return opNode;
		}

		value = GetValue(tokenIndex);

		if (value == nullptr)
		{
			return nullptr;
		}

		if (tokenIndex == tokens_.size())
		{
			return value;
		}

		return value;
	}

	Builder::Rest Builder::GetTerm1(int& tokenIndex, Rest inh)
	{
		std::cout << "\nGetTerm1 at tokenIndex = " + std::to_string(tokenIndex);

		Rest empty{};
		empty.expr = inh.inh;

		if (tokenIndex == tokens_.size())
		{
			return empty;
		}

		if (tokens_[tokenIndex]->GetType() == TokenType::Semicolon)
		{
			return empty;
		}

		BinaryOperatorNode* op;

		SourceLocation location;

		if (tokens_[tokenIndex]->GetType() == TokenType::Operator)
		{
			location = tokens_[tokenIndex]->location;

			switch (tokens_[tokenIndex]->AsToken<OperatorToken>()->opType)
			{
			case OperatorTokenType::Multiply:
				tokenIndex++;
				op = new BinaryOperatorNode(BinaryOperatorType::Multiply);
				break;
			case OperatorTokenType::Divide:
				tokenIndex++;
				op = new BinaryOperatorNode(BinaryOperatorType::Divide);
				break;
			case OperatorTokenType::Comma:
			{
				if (blockContext_.isParsingArgs)
				{
					return empty;
				}
				else
				{
					empty.expr = nullptr;

					OutputError(tokens_[tokenIndex], "Excepted * or /");

					return empty;
				}
			}
			break;
			default:
				empty.expr = nullptr;

				return empty;
			}
		}
		else if (tokens_[tokenIndex]->GetType() == TokenType::CloseBracket || tokens_[tokenIndex]->GetType() == TokenType::OpenBracket)
		{
			if (blockContext_.bracketsLevel == 0)
			{
				empty.expr = nullptr;

				OutputError(tokens_[tokenIndex], "Close Bracket without Open Bracket");
			}

			return empty;
		}
		else
		{
			empty.expr = nullptr;

			return empty;
		}

		Node* factor = GetFactor(tokenIndex);

		if (factor == nullptr)
		{
			//empty.expr = nullptr;

			return empty;
		}

		Rest rest{};
		rest.inh = op;

		op->SetSourceLocation(location);
		op->SetLeftOperand(inh.inh);
		op->SetRightOperand(factor);

		if (tokens_[tokenIndex]->GetType() == TokenType::Semicolon)
		{
			rest.expr = op;

			return rest;
		}

		Rest rest1 = GetTerm1(tokenIndex, rest);

		// возможно стоит заменить это, также как и в Expr
		if (rest1.expr != nullptr)
		{
			rest.expr = rest1.expr;
		}
		else
		{
			rest.expr = op;
		}

		return rest;
	}

	Node* Builder::GetTerm(int& tokenIndex)
	{
		std::cout << "\nGetTerm at tokenIndex = " + std::to_string(tokenIndex);

		Node* digit = GetFactor(tokenIndex);

		if (digit == nullptr)
		{
			return nullptr;
		}

		if (tokenIndex == tokens_.size())
		{
			return digit;
		}

		if (tokens_[tokenIndex]->GetType() == TokenType::Semicolon)
		{
			return digit;
		}

		Rest rest{};
		rest.inh = digit;

		Rest term1 = GetTerm1(tokenIndex, rest);

		if (term1.expr == nullptr)
		{
			return digit;
		}

		return term1.expr;
	}

	Builder::Rest Builder::GetExpr1(int& tokenIndex, Rest inh)
	{
		std::cout << "\nGetExpr1 at tokenIndex = " + std::to_string(tokenIndex);

		Rest empty{};
		empty.expr = inh.inh;

		Node* term = nullptr;

		if (tokenIndex == tokens_.size())
		{
			empty.expr = nullptr;

			return empty;
		}

		BinaryOperatorNode* expr;

		SourceLocation location;

		if (tokens_[tokenIndex]->GetType() == TokenType::Operator)
		{
			location = tokens_[tokenIndex]->location;

			switch (tokens_[tokenIndex]->AsToken<OperatorToken>()->opType)
			{
			case OperatorTokenType::Plus:
				tokenIndex++;
				expr = new BinaryOperatorNode(BinaryOperatorType::Plus);
				break;
			case OperatorTokenType::Minus:
				tokenIndex++;
				expr = new BinaryOperatorNode(BinaryOperatorType::Minus);
				break;
			case OperatorTokenType::Comma:
				if (blockContext_.isParsingArgs)
				{
					return empty;
				}
				else
				{
					empty.expr = nullptr;

					OutputError(tokens_[tokenIndex], "Excepted binary operator");

					return empty;
				}
				break;
			default:
				empty.expr = nullptr;

				return empty;
			}
		}
		else if (tokens_[tokenIndex]->GetType() == TokenType::CloseBracket || tokens_[tokenIndex]->GetType() == TokenType::OpenBracket)
		{
			if (blockContext_.bracketsLevel == 0)
			{
				empty.expr = nullptr;

				OutputError(tokens_[tokenIndex], "Close Bracket without Open Bracket");
			}

			return empty;
		}
		else
		{
			OutputError(tokens_[tokenIndex], "Excepted binary operator");

			empty.expr = nullptr;

			return empty;
		}

		term = GetTerm(tokenIndex);

		if (term == nullptr)
		{
			return Rest{};
		}

		Rest rest{};
		rest.inh = expr;

		expr->SetSourceLocation(location);
		expr->SetLeftOperand(inh.inh);
		expr->SetRightOperand(term);

		if (tokenIndex == tokens_.size())
		{
			rest.expr = expr;

			return rest;
		}

		if (tokens_[tokenIndex]->GetType() == TokenType::Semicolon)
		{
			rest.expr = expr;

			return rest;
		}

		Rest rest1 = GetExpr1(tokenIndex, rest);

		if (rest1.expr != nullptr)
		{
			rest.expr = rest1.expr;
		}
		else
		{
			rest.expr = expr;
		}


		//if (rest1.expr == nullptr && tokenIndex == tokens.size())
		//{
		//	rest.expr = expr;
		//}
		//else
		//{
		//	rest.expr = rest1.expr;
		//}

		return rest;
	}

	Node* Builder::GetExpr(int& tokenIndex)
	{
		std::cout << "\nGetExpr at tokenIndex = " + std::to_string(tokenIndex);

		Node* term = GetTerm(tokenIndex);

		if (term == nullptr)
		{
			return nullptr;
		}

		if (tokens_[tokenIndex]->GetType() == TokenType::Semicolon)
		{
			return term;
		}

		Rest rest{};
		rest.inh = term;

		Rest rest1 = GetExpr1(tokenIndex, rest);

		//if (rest1.expr == nullptr && tokenIndex == tokens.size())
		//{
		//	return term;
		//}

		//if (rest1.expr == nullptr)
		//{
		//	return term;
		//}

		//Rest term1 = GetTerm1(tokenIndex, rest);

		if (rest1.expr == nullptr)
		{
			return term;
		}

		return rest1.expr;
	}

	Builder::Rest Builder::GetCompare1(int& tokenIndex, Rest inh)
	{
		std::cout << "\nGetCompare1 at tokenIndex = " + std::to_string(tokenIndex);

		Rest empty{};
		empty.expr = inh.inh;

		if (tokenIndex == tokens_.size())
		{
			return empty;
		}

		if (tokens_[tokenIndex]->GetType() == TokenType::Semicolon)
		{
			return empty;
		}

		BinaryOperatorNode* op;

		SourceLocation location;

		if (tokens_[tokenIndex]->GetType() == TokenType::Operator)
		{
			location = tokens_[tokenIndex]->location;

			switch (tokens_[tokenIndex]->AsToken<OperatorToken>()->opType)
			{
			case OperatorTokenType::Less:
				tokenIndex++;
				op = new BinaryOperatorNode(BinaryOperatorType::Less);
				break;
			case OperatorTokenType::Greater:
				tokenIndex++;
				op = new BinaryOperatorNode(BinaryOperatorType::Greater);
				break;
			case OperatorTokenType::LessEqual:
				tokenIndex++;
				op = new BinaryOperatorNode(BinaryOperatorType::LessEqual);
				break;
			case OperatorTokenType::GreaterEqual:
				tokenIndex++;
				op = new BinaryOperatorNode(BinaryOperatorType::GreaterEqual);
				break;
			case OperatorTokenType::Equal:
				tokenIndex++;
				op = new BinaryOperatorNode(BinaryOperatorType::Equal);
				break;
			case OperatorTokenType::Comma:
			{
				if (blockContext_.isParsingArgs)
				{
					return empty;
				}
				else
				{
					empty.expr = nullptr;

					OutputError(tokens_[tokenIndex], "Excepted operator");

					return empty;
				}
			}
			break;
			default:
				empty.expr = nullptr;

				return empty;
			}
		}
		else if (tokens_[tokenIndex]->GetType() == TokenType::CloseBracket || tokens_[tokenIndex]->GetType() == TokenType::OpenBracket)
		{
			if (blockContext_.bracketsLevel == 0)
			{
				empty.expr = nullptr;

				OutputError(tokens_[tokenIndex], "Close Bracket without Open Bracket");
			}

			return empty;
		}
		else
		{
			empty.expr = nullptr;

			return empty;
		}

		Node* expr = GetExpr(tokenIndex);

		if (expr == nullptr)
		{
			//empty.expr = nullptr;

			return empty;
		}

		op->SetSourceLocation(location);
		op->SetLeftOperand(inh.inh);
		op->SetRightOperand(expr);

		Rest rest{};
		rest.inh = op;

		if (tokens_[tokenIndex]->GetType() == TokenType::Semicolon || tokens_[tokenIndex]->GetType() == TokenType::CloseBracket)
		{
			rest.expr = op;

			return rest;
		}
		else // дальнейшие операции сравнения недозволены, выражение должно окончится ) или ;
		{
			OutputError(tokens_[tokenIndex], "Excepted ; or )");

			return Rest{};
		}

		Rest rest1 = GetCompare1(tokenIndex, rest);

		// возможно стоит заменить это, также как и в Expr
		if (rest1.expr != nullptr)
		{
			rest.expr = rest1.expr;
		}
		else
		{
			rest.expr = op;
		}

		return rest;
	}

	Node* Builder::GetCompare(int& tokenIndex)
	{
		std::cout << "\nGetCompare at tokenIndex = " + std::to_string(tokenIndex);

		if (tokenIndex == tokens_.size())
		{
			return nullptr;
		}

		Node* expr = GetExpr(tokenIndex);

		if (tokenIndex == tokens_.size())
		{
			return expr;
		}

		if (tokens_[tokenIndex]->GetType() == TokenType::Semicolon)
		{
			return expr;
		}

		Rest rest{};
		rest.inh = expr;

		Rest rest1 = GetCompare1(tokenIndex, rest);

		if (rest1.expr == nullptr)
		{
			return expr;
		}

		return rest1.expr;
	}

	Node* Builder::GetEval(int& tokenIndex)
	{
		std::cout << "\nGetEval at tokenIndex = " + std::to_string(tokenIndex);

		Node* expr = GetCompare(tokenIndex);

		if (tokenIndex != tokens_.size() && tokens_[tokenIndex]->GetType() == TokenType::CloseBracket)
		{
			if (isParsingBrackets || blockContext_.bracketsLevel > 0)
			{
				if (!blockContext_.status)
				{
					return nullptr;
				}

				return expr;
			}
			else
			{
				OutputError(tokens_[tokenIndex], "Open bracket without close bracket");

				return nullptr;
			}
		}

		return expr;
	}

	StmtNode* Builder::GetStmt(int& tokenIndex)
	{
		std::cout << "\nGetStmt at tokenIndex = " + std::to_string(tokenIndex);

		StmtNode* stmt = new StmtNode();

		KeywordToken* tok = tokens_[tokenIndex]->AsToken<KeywordToken>();

		std::optional<DataType> type = GetTypeFromKeyword(tok);

		if (!type.has_value())
		{
			OutputError(tokens_[tokenIndex], "Excepted Data Type");
		}
		else
		{
			if (type.value() == DataType::Void)
			{
				OutputError(tokens_[tokenIndex], "void not allowed as variable data type");
			}
		}

		bool isRef = false;

		tokenIndex++;

		if (tokenIndex >= tokens_.size())
		{
			return nullptr;
		}

		// ссылка (<type>&)
		if (tokens_[tokenIndex]->GetType() == TokenType::Ampersand)
		{
			isRef = true;

			tokenIndex++;
		}

		// присваивание (<name> '=' Eval)
		if (tokens_[tokenIndex + 1]->GetType() == TokenType::Operator &&
			(tokens_[tokenIndex + 1]->AsToken<OperatorToken>()->opType == OperatorTokenType::Assign))
		{
			IdToken* tok = tokens_[tokenIndex]->AsToken<IdToken>();

			// перепрыгиваем ID
			tokenIndex++;

			// а тут перепрыгиваем assign
			tokenIndex++;

			Node* eval = GetEval(tokenIndex);

			if (tokens_[tokenIndex]->GetType() != TokenType::Semicolon)
			{
				return nullptr;
			}

			VarNode* id = new VarNode(tok->idName);

			id->SetDataType(type.value());
			id->SetSourceLocation(tok->location);

			if (isRef)
			{
				id->SetIsRef(true);
			}

			AssignNode* assign = new AssignNode(id, eval);

			stmt->SetNode(assign);
			stmt->SetSourceLocation(tok->location);

			return stmt;
		}
		else if (tokens_[tokenIndex + 1]->GetType() == TokenType::Semicolon) // просто объявление (VarDecl ';')
		{
			IdToken* tok = tokens_[tokenIndex]->AsToken<IdToken>();

			VarNode* id = new VarNode(tok->idName);

			id->SetDataType(type.value());
			id->SetSourceLocation(tok->location);

			// перепрыгиваем ID
			tokenIndex++;

			stmt->SetNode(id);
			stmt->SetSourceLocation(tok->location);

			return stmt;
		}
		else // просто eval (Eval ';')
		{
			Node* eval = GetEval(tokenIndex);

			if (tokens_[tokenIndex]->GetType() != TokenType::Semicolon)
			{
				return nullptr;
			}

			stmt->SetNode(eval);

			return stmt;
		}

		return nullptr;
	}

	IfStmtNode* Builder::GetIfStmt(int& tokenIndex)
	{
		std::cout << "\nGetIfStmt at tokenIndex = " + std::to_string(tokenIndex);

		SourceLocation loc = tokens_[tokenIndex]->location;

		// токен - keyword = if, это гарантирует GetSeq, нужно быть осторожным, если захочется использовать эту функцию ещё где-то
		tokenIndex++;

		if(tokens_[tokenIndex]->GetType() != TokenType::OpenBracket)
		{
			OutputError(tokens_[tokenIndex], "Excepted (");

			return nullptr;
		}

		tokenIndex++;

		blockContext_.bracketsLevel++;

		Node* cond = GetEval(tokenIndex);

		if (tokens_[tokenIndex]->GetType() != TokenType::CloseBracket)
		{
			OutputError(tokens_[tokenIndex], "Excepted )");

			return nullptr;
		}

		blockContext_.bracketsLevel--;

		tokenIndex++;

		IfStmtNode* ifStmt = new IfStmtNode();

		ifStmt->SetCond(cond);

		if (tokens_[tokenIndex]->GetType() != TokenType::OpenBrace)
		{
			OutputError(tokens_[tokenIndex], "Excepted {");

			return nullptr;
		}

		tokenIndex++;

		SeqNode* startSeq = GetSeq(tokenIndex);

		if (tokens_[tokenIndex]->GetType() != TokenType::CloseBrace)
		{
			OutputError(tokens_[tokenIndex], "Excepted }");

			return nullptr;
		}

		tokenIndex++;

		ifStmt->SetStartSeq(startSeq);
		ifStmt->SetSourceLocation(loc);

		return ifStmt;
	}

	ElseStmtNode* Builder::GetElseStmt(int& tokenIndex)
	{
		std::cout << "\nGetElseStmt at tokenIndex = " + std::to_string(tokenIndex);

		SourceLocation loc = tokens_[tokenIndex]->location;

		// токен - keyword = else, это гарантирует GetSeq, нужно быть осторожным, если захочется использовать эту функцию ещё где-то
		tokenIndex++;

		ElseStmtNode* ifStmt = new ElseStmtNode();

		if (tokens_[tokenIndex]->GetType() != TokenType::OpenBrace)
		{
			OutputError(tokens_[tokenIndex], "Excepted {");

			return nullptr;
		}

		tokenIndex++;

		SeqNode* startSeq = GetSeq(tokenIndex);

		if (tokens_[tokenIndex]->GetType() != TokenType::CloseBrace)
		{
			OutputError(tokens_[tokenIndex], "Excepted }");

			return nullptr;
		}

		tokenIndex++;

		ifStmt->SetStartSeq(startSeq);
		ifStmt->SetSourceLocation(loc);

		return ifStmt;
	}

	AssignNode* Builder::GetAssign(int& tokenIndex)
	{
		std::cout << "\nGetAssign at tokenIndex = " + std::to_string(tokenIndex);

		std::string varName = tokens_[tokenIndex]->AsToken<IdToken>()->idName;
		SourceLocation idLoc = tokens_[tokenIndex]->location;

		// перепрыгиваем ID
		tokenIndex++;

		if (tokens_[tokenIndex]->GetType() != TokenType::Operator)
		{
			OutputError(tokens_[tokenIndex], "Excepted operator =");

			return nullptr;
		}

		if (tokens_[tokenIndex]->AsToken<OperatorToken>()->opType != OperatorTokenType::Assign)
		{
			OutputError(tokens_[tokenIndex], "Excepted operator =");

			return nullptr;
		}

		SourceLocation loc = tokens_[tokenIndex]->location;

		//Перепрыгиваем =
		tokenIndex++;

		VarNode* node = new VarNode(varName);
		node->SetSourceLocation(idLoc);

		Node* eval = GetEval(tokenIndex);

		if (tokens_[tokenIndex]->GetType() != TokenType::Semicolon)
		{
			return nullptr;
		}

		AssignNode* assign = new AssignNode(node, eval);
		assign->SetSourceLocation(loc);

		return assign;
	}

	SeqNode* Builder::GetSeq(int& tokenIndex)
	{
		std::cout << "\n-----------------------\n";

		std::cout << "GetSeq at tokenIndex = " + std::to_string(tokenIndex);

		if (tokenIndex >= tokens_.size())
		{
			return nullptr;
		}

		if (tokens_[tokenIndex]->GetType() == TokenType::CloseBrace)
		{
			return nullptr;
		}

		SeqNode* seq = new SeqNode();

		// вариации return 
		if (tokens_[tokenIndex]->GetType() == TokenType::Keyword && tokens_[tokenIndex]->AsToken<KeywordToken>()->type == KeywordType::Return) 
		{
			// перепрыгиваем return
			tokenIndex++;

			// пустой return ('return' ';')
			if (tokens_[tokenIndex]->GetType() == TokenType::Semicolon)
			{
				ReturnStmtNode* ret = new ReturnStmtNode();

				tokenIndex++;

				seq->SetExprNode(ret);

				SeqNode* nextSeq = GetSeq(tokenIndex);

				if (nextSeq != nullptr)
				{
					seq->SetNextSeq(nextSeq);
				}

				return seq;
			}

			// return с выражением ('return' Eval ';')

			Node* eval = GetEval(tokenIndex);

			if (tokens_[tokenIndex]->GetType() != TokenType::Semicolon)
			{
				// начинается паника, ищем ;
				while (tokenIndex != tokens_.size() && tokens_[tokenIndex]->GetType() != TokenType::Semicolon)
				{
					tokenIndex++;
				}

				// переводим ;
				tokenIndex++;
			}
			else
			{
				tokenIndex++;
			}

			ReturnStmtNode* ret = new ReturnStmtNode();

			if (eval != nullptr)
			{
				ret->SetExprNode(eval);
			}

			seq->SetExprNode(ret);

			SeqNode* nextSeq = GetSeq(tokenIndex);

			if (nextSeq != nullptr)
			{
				seq->SetNextSeq(nextSeq);
			}

			return seq;
		}
		else if (tokens_[tokenIndex]->GetType() == TokenType::Keyword && tokens_[tokenIndex]->AsToken<KeywordToken>()->type == KeywordType::If) // if блок
		{
			IfStmtNode* stmt = GetIfStmt(tokenIndex);

			if (stmt == nullptr)
			{
				// начинается паника, ищем ;
				while (tokenIndex != tokens_.size() && tokens_[tokenIndex]->GetType() != TokenType::Semicolon)
				{
					tokenIndex++;
				}

				// переводим ;
				tokenIndex++;
			}

			if (stmt != nullptr)
			{
				seq->SetExprNode(stmt);
			}

			SeqNode* nextSeq = GetSeq(tokenIndex);

			if (nextSeq != nullptr)
			{
				seq->SetNextSeq(nextSeq);
			}

			return seq;
		}
		else if (tokens_[tokenIndex]->GetType() == TokenType::Keyword && tokens_[tokenIndex]->AsToken<KeywordToken>()->type == KeywordType::Else) // else блок
		{
			ElseStmtNode* stmt = GetElseStmt(tokenIndex);

			if (stmt == nullptr)
			{
				// начинается паника, ищем ;
				while (tokenIndex != tokens_.size() && tokens_[tokenIndex]->GetType() != TokenType::Semicolon)
				{
					tokenIndex++;
				}

				// переводим ;
				tokenIndex++;
			}

			if (stmt != nullptr)
			{
				seq->SetExprNode(stmt);
			}

			SeqNode* nextSeq = GetSeq(tokenIndex);

			if (nextSeq != nullptr)
			{
				seq->SetNextSeq(nextSeq);
			}

			return seq;
		}
		else if (tokens_[tokenIndex]->GetType() == TokenType::Keyword) // объявление переменной (Stmt ';')
		{
			StmtNode* node = GetStmt(tokenIndex);

			if (node == nullptr)
			{
				if (tokenIndex == tokens_.size())
				{
					OutputError(tokens_[tokenIndex - 1], "Excepted Semicolon");

					return nullptr;
				}

				// начинается паника, ищем ;
				while (tokenIndex != tokens_.size() && tokens_[tokenIndex]->GetType() != TokenType::Semicolon)
				{
					tokenIndex++;
				}

				// переводим ;
				tokenIndex++;
			}
			else
			{
				if (tokens_[tokenIndex]->GetType() != TokenType::Semicolon)
				{
					OutputError(tokens_[tokenIndex], "Excepted Semicolon");

					return nullptr;
				}

				// Паника не требуется, просто переводим символ
				tokenIndex++;
			}

			if (node != nullptr)
			{
				seq->SetExprNode(node);
			}

			SeqNode* nextSeq = GetSeq(tokenIndex);

			if (nextSeq != nullptr)
			{
				seq->SetNextSeq(nextSeq);
			}

			return seq;
		}
		else if (tokens_[tokenIndex]->GetType() == TokenType::Id && tokens_[tokenIndex + 1]->GetType() == TokenType::Operator &&
				 tokens_[tokenIndex + 1]->AsToken<OperatorToken>()->opType == OperatorTokenType::Assign) // assign без объявления переменной (Assign ';')
		{
			AssignNode* assign = GetAssign(tokenIndex);

			if (assign == nullptr)
			{
				// начинается паника, ищем ;
				while (tokenIndex != tokens_.size() && tokens_[tokenIndex]->GetType() != TokenType::Semicolon)
				{
					tokenIndex++;
				}

				// переводим ;
				tokenIndex++;
			}
			else
			{
				seq->SetExprNode(assign);
			}

			if (tokens_[tokenIndex]->GetType() != TokenType::Semicolon)
			{
				OutputError(tokens_[tokenIndex], "Excepted Semicolon");

				return nullptr;
			}

			tokenIndex++;

			SeqNode* nextSeq = GetSeq(tokenIndex);

			if (nextSeq != nullptr)
			{
				seq->SetNextSeq(nextSeq);
			}

			return seq;
		}
		else if (tokens_[tokenIndex]->GetType() == TokenType::Semicolon)
		{
			tokenIndex++;

			SeqNode* nextSeq = GetSeq(tokenIndex);

			if (nextSeq != nullptr)
			{
				seq->SetNextSeq(nextSeq);
			}

			return seq;
		}

		// просто пустой eval. Нет в грамматике, мб нужно удалить это? TODO

		Node* eval = GetEval(tokenIndex);

		if (eval == nullptr)
		{
			// паника, ищем ;
			while (tokenIndex != tokens_.size() && tokens_[tokenIndex]->GetType() != TokenType::Semicolon)
			{
				tokenIndex++;
			}
		}

		if (tokens_[tokenIndex]->GetType() != TokenType::Semicolon)
		{
			OutputError(tokens_[tokenIndex], "Excepted Semicolon");

			return nullptr;
		}

		tokenIndex++;


		if (eval != nullptr)
		{
			seq->SetExprNode(eval);
		}

		if (tokenIndex >= tokens_.size())
		{
			return seq;
		}

		SeqNode* nextSeq = GetSeq(tokenIndex);

		if (nextSeq != nullptr)
		{
			seq->SetNextSeq(nextSeq);
		}

		return seq;
	}

	FunctionArgNode* Builder::GetFunctionArgDecl(int& tokenIndex)
	{
		std::cout << "\nGetFunctionArgDecl at tokenIndex = " + std::to_string(tokenIndex);

		if (tokenIndex >= tokens_.size())
		{
			blockContext_.status = false;

			return nullptr;
		}

		if (tokens_[tokenIndex]->GetType() != TokenType::Keyword)
		{
			OutputError(tokens_[tokenIndex], "Excepted argument type");

			return nullptr;
		}

		std::optional<DataType> argType = GetTypeFromKeyword(tokens_[tokenIndex]->AsToken<KeywordToken>());

		if (!argType.has_value())
		{
			OutputError(tokens_[tokenIndex], "Excepted Data Type");

			return nullptr;
		}

		if (argType.value() == DataType::Void)
		{
			OutputError(tokens_[tokenIndex], "void as argument type not allowed");
		}

		tokenIndex++;

		if (tokens_[tokenIndex]->GetType() != TokenType::Id)
		{
			OutputError(tokens_[tokenIndex], "Excepted argument name");

			return nullptr;
		}

		IdToken* argToken = tokens_[tokenIndex]->AsToken<IdToken>();
		std::string argName = argToken->idName;

		tokenIndex++;

		if (tokens_[tokenIndex]->GetType() == TokenType::Operator ||
			tokens_[tokenIndex]->GetType() == TokenType::CloseBracket)
		{
			if (tokens_[tokenIndex]->GetType() == TokenType::Operator &&
				tokens_[tokenIndex]->AsToken<OperatorToken>()->opType != OperatorTokenType::Comma)
			{
				OutputError(tokens_[tokenIndex], "Excepted comma or close bracket");

				return nullptr;
			}

			FunctionArgNode* arg = new FunctionArgNode();

			arg->SetName(argName);
			arg->SetDataType(argType.value());
			arg->SetSourceLocation(argToken->location);

			VarNode* argVar = new VarNode(argName);
			argVar->SetDataType(argType.value());
			argVar->SetSourceLocation(argToken->location);

			arg->SetVariableNode(argVar);

			return arg;
		}

		OutputError(tokens_[tokenIndex], "Excepted comma or close bracket");

		return nullptr;
	}

	FunctionArgNode* Builder::GetFunctionArgs(int& tokenIndex)
	{
		std::cout << "\nGetFunctionArgs at tokenIndex = " + std::to_string(tokenIndex);

		if (tokens_[tokenIndex]->GetType() == TokenType::CloseBracket)
		{
			// Если за OpenBracket идёт CloseBracket, то значит, что функция без аргументов
			return nullptr;
		}

		FunctionArgNode* prevArg = nullptr;
		FunctionArgNode* firstArg = nullptr;

		while (true)
		{
			FunctionArgNode* arg = GetFunctionArgDecl(tokenIndex);

			if (arg == nullptr)
			{
				blockContext_.status = false;

				return nullptr;
			}

			if (tokens_[tokenIndex]->GetType() == TokenType::Operator &&
				tokens_[tokenIndex]->AsToken<OperatorToken>()->opType == OperatorTokenType::Comma)
			{
				tokenIndex++;

				if (prevArg == nullptr)
				{
					prevArg = arg;
					firstArg = arg;
				}
				else
				{
					prevArg->SetNextNode(arg);

					prevArg = arg;
				}

				continue;
			}

			if (tokens_[tokenIndex]->GetType() == TokenType::CloseBracket)
			{
				if (prevArg != nullptr && firstArg != nullptr)
				{
					prevArg->SetNextNode(arg);

					return firstArg;
				}
				else
				{
					return arg;
				}
			}
		}
	}

	int Builder::GetFunctionQualifier(int& tokenIndex)
	{
		return 0;
	}

	FunctionDeclNode* Builder::GetFunctionDecl(int& tokenIndex)
	{
		std::cout << "\n-----------------------\n";

		std::cout << "GetFunctionDecl at tokenIndex = " + std::to_string(tokenIndex);

		int flags = GetFunctionQualifier(tokenIndex);

		if (tokens_[tokenIndex]->GetType() != TokenType::Keyword)
		{
			OutputError(tokens_[tokenIndex], "Excepted Decl");

			return nullptr;
		}

		std::optional<DataType> retType = GetTypeFromKeyword(tokens_[tokenIndex]->AsToken<KeywordToken>());

		if (!retType.has_value())
		{
			OutputError(tokens_[tokenIndex], "Excepted Data Type");

			return nullptr;
		}

		tokenIndex++;

		if (tokens_[tokenIndex]->GetType() != TokenType::Id)
		{
			OutputError(tokens_[tokenIndex], "Excepted Name");

			return nullptr;
		}

		IdToken* funcToken = tokens_[tokenIndex]->AsToken<IdToken>();

		std::string funcName = funcToken->idName;

		tokenIndex++;

		if (tokens_[tokenIndex]->GetType() != TokenType::OpenBracket)
		{
			OutputError(tokens_[tokenIndex], "Excepted Open Bracket");

			return nullptr;
		}

		tokenIndex++;

		blockContext_.bracketsLevel++;

		FunctionArgNode* firstArg = GetFunctionArgs(tokenIndex);

		if (tokens_[tokenIndex]->GetType() != TokenType::CloseBracket)
		{
			return nullptr;
		}

		tokenIndex++;

		blockContext_.bracketsLevel--;

		FunctionDeclNode* node = new FunctionDeclNode();

		node->SetReturnValueType(retType.value());
		node->SetName(funcName);
		node->SetSourceLocation(funcToken->location);

		FunctionArgNode* temp = firstArg;

		while (temp != nullptr)
		{
			node->AddArg(temp);

			temp->Parents().clear();

			temp->AddParent(node);

			node->GetSymbolTable().AddName(temp->GetName(), temp);

			temp = temp->GetNextNode();
		}

		if (tokens_[tokenIndex]->GetType() != TokenType::OpenBrace)
		{
			OutputError(tokens_[tokenIndex], "Excepted Open Brace");

			return nullptr;
		}

		tokenIndex++;

		currentTable_ = &node->GetSymbolTable();

		SeqNode* firstSeq = GetSeq(tokenIndex);

		if (tokens_[tokenIndex]->GetType() != TokenType::CloseBrace)
		{
			OutputError(tokens_[tokenIndex], "Excepted Close Brace");

			return nullptr;
		}

		currentTable_ = nullptr;

		tokenIndex++;

		if (firstSeq != nullptr)
		{
			node->SetStartSeqNode(firstSeq);
		}

		return node;
	}

	void Builder::GetMain()
	{
		std::cout << "\n-----------------------\n";

		std::cout << "GetMain";

		int tokenIndex = 0;

		while (tokenIndex != tokens_.size() && tokenIndex < tokens_.size())
		{
			FunctionDeclNode* node = GetFunctionDecl(tokenIndex);

			if (node != nullptr)
			{
				currentContext_->AddFunctionDecl(node);

				node->GetSymbolTable().SetPrevTable(&currentContext_->GetGlobalTable());

				currentContext_->GetGlobalTable().AddName(node->GetName(), node);
			}
			else
			{
				// паника, ищем } 
				while (tokenIndex != tokens_.size() && tokens_[tokenIndex]->GetType() != TokenType::CloseBrace)
				{
					tokenIndex++;
				}

				tokenIndex++;

				blockContext_.Reset();
			}
		}
	}

	void Builder::Build(std::vector<Token*>& tokens)
	{
		this->tokens_ = tokens;

		GetMain();

		std::cout << "\n";
	}

	std::vector<ParseError>& Builder::GetErrorList()
	{
		return errors_;
	}
}