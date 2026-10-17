#pragma once

#include "AST.hpp"

namespace ast
{
	// Dev
	/*
	* Seq := Stmt ';' | Assign ';' | ReturnStmt ';' | Seq | IfStmt | ElseStmt | <0>
	* IfStmt := 'if' '(' Eval ')' '{' Seq '}'
	* ElseStmt := 'else' '{' Seq '}'
	* ReturnStmt := 'return' Eval ';' | 'return' ';'
	* Assign := <name> '=' Eval ';'
	* VarDecl = <type> <name> ';' | <type> '&' <name> ';'
	* Stmt := VarDecl ';' | Eval ';' | <0>
	* Eval := Expr Compare | Expr
	* Compare := Expr Compare1 | Expr
	* Compare1 := '>' Expr | '<' Expr | '>=' Expr | '<=' Expr | '==' Expr | <0>
	* Expr := Term Expr1 | Term
	* Expr1 := '+' Term Expr1 | '-' Term Expr1 | <0>
	* Term := Factor Term1 | Factor
	* Term1 := '*' Factor Term1 | '/' Factor Term1
	* Factor := '-' Value | Value
	* Value := <name> '(' Args ')' | Cast | <name> . <name> '(' Args ')'
	* Args := Eval ',' Args | Eval
	* Cast := '(' <type> ')' Digit | Digit
	* Digit := <number> | '(' Eval ')' | MemberAccess
	* MemberAccess := <name> '.' <name>
	*
	* FuncArgsDecl := <type> <name> ',' | <type> <name> ')'
	* FuncArgs := FuncArgsDecl ',' FuncArgs | FuncArgsDecl | <0>
	* FuncQualifier := 'extern' FuncQualifier | <0>
	* FuncDecl := FuncQualifier <type> <name> '(' FuncArgs ')' '{' Seq '}' | FuncQualifier <type> <name> '(' FuncArgs ')' ';'
	*
	* FieldDecl := <type> <name> ';' | <type> <name> '=' Eval ';'
	*
	* ClassDecl := 'class' <name> '{' ClassBlock '}' ';'
	* ClassBlock := <access_modifier> ':' ClassStmts ClassBlock | ClassStmts ClassBlock |  <0>
	* ClassStmts := FuncDecl ClassStmts | FieldDecl ClassStmts | <0>
	*
	* StructStmt := Stmt ';'
	* StructStmts := StructStmt StructStmts | ';' | '}'
	* StructDecl := 'struct' '{' StructStmts '}' ';'
	*
	* Main := FuncDecl | ClassDecl | StructDecl | <0>
	*
	* <type> := int | uint | long | ulong | byte | ubyte | char | ... когда-нибудь допишу
	*/

	/**
	* @brief Класс, строящий AST из списка токенов. 
	*/
	class Builder
	{
	private:
		struct BlockContext
		{
			bool status = true;

			bool isParsingArgs = false;

			// ()
			int bracketsLevel = 0;

			// {}
			int braceLevel = 0;

			void Reset()
			{
				status = true;

				isParsingArgs = false;

				bracketsLevel = 0;

				braceLevel = 0;
			}
		};

	private:
		std::vector<ParseError> errors_;
		std::vector<Token*> tokens;

		bool isParsingBrackets = false;

		BlockContext blockContext_;

		Context* currentContext_ = nullptr;

		SymbolTable* currentTable_ = nullptr;

		struct Rest
		{
			Node* expr;
			Node* inh;
		};

	private:
		void OutputError(Token* token, std::string errorMessage);

		Node* GetDigit(int& tokenIndex);

		Node* GetCast(int& tokenIndex);

		ArgSeqNode* GetArgs(int& tokenIndex);

		Node* GetValue(int& tokenIndex);

		Node* GetFactor(int& tokenIndex);

		Rest GetTerm1(int& tokenIndex, Rest inh);

		Node* GetTerm(int& tokenIndex);

		Rest GetExpr1(int& tokenIndex, Rest inh);

		Node* GetExpr(int& tokenIndex);

		Rest GetCompare1(int& tokenIndex, Rest inh);

		Node* GetCompare(int& tokenIndex);

		/**
		* @brief Парсит математические выражения, включая скобки
		* 
		* @details Обработку скобок (')', '}', etc) берет на себя процедура, которая инициировала повышение уровня скобок, 
			процедуры ниже по коллстеку не должны переводить токены вперед при встрече скобок
		* 
		* @warning Не включает в себя обработку присваивания, см GetAssign
		*/
		Node* GetEval(int& tokenIndex);

		/**
		* @brief Рзабирает объявление переменной
		* 
		* @details Разбирает как обычные объявления, так и ссылки, однако окончательные ссылки проставляются на семантической фазе. Также разбирает и присваивание при инициализации
		*/
		StmtNode* GetStmt(int& tokenIndex);

		/**
		* @details связь if-else выставляется на семантической фазе
		*/
		IfStmtNode* GetIfStmt(int& tokenIndex);

		ElseStmtNode* GetElseStmt(int& tokenIndex);

		/**
		* @brief Парсит конструкцию присваивания переменной 
		*/
		AssignNode* GetAssign(int& tokenIndex);

		SeqNode* GetSeq(int& tokenIndex);

		FunctionArgNode* GetFunctionArgDecl(int& tokenIndex);

		FunctionArgNode* GetFunctionArgs(int& tokenIndex);

		int GetFunctionQualifier(int& tokenIndex);

		FunctionDeclNode* GetFunctionDecl(int& tokenIndex);

		void GetMain();

	public:
		void Build(std::vector<Token*>& tokens);

		void SetContext(Context* context) { currentContext_ = context; }

		std::vector<ParseError>& GetErrorList();
	};
}