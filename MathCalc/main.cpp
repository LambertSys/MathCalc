#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <fstream>

#include "Source.hpp"
#include "Lexer.hpp"

#include "Parser.hpp"

#include "SemaChecker.hpp"

#include "ASTDotPrinter.hpp"

#include "IR.hpp"

#include "SLM.h"
#include "SLAsm.h"
#include "SLMachine.h"

enum class MathType
{
	Value, 
	Op
};

struct MathTerm
{
	virtual MathType GetType() = 0;
};

struct Value : MathTerm
{
	int value;

	Value(int value) : value(value) {}

	MathType GetType() override
	{
		return MathType::Value;
	}
};

struct Operator : MathTerm
{
	OperatorTokenType type;

	bool isBinary;

	Operator(OperatorTokenType type, bool isBinary) : type(type), isBinary(isBinary) {}

	MathType GetType() override
	{
		return MathType::Op;
	}
};

class MathEvaluator
{
private:
	std::vector<MathTerm*> terms;

public:
	MathEvaluator(std::vector<MathTerm*> terms)
	{
		this->terms = terms;
	}

	int Calc()
	{
		std::stack<int> stack;

		for (int i = 0; i < terms.size(); i++)
		{
			if (terms[i]->GetType() == MathType::Value)
			{
				stack.push(static_cast<Value*>(terms[i])->value);
			}
			else
			{
				Operator* op = static_cast<Operator*>(terms[i]);

				if (op->isBinary)
				{
					int firstVal = stack.top();
					stack.pop();
					int secondVal = stack.top();
					stack.pop();

					switch (op->type)
					{
					case OperatorTokenType::Plus:
						stack.push(secondVal + firstVal);
						break;
					case OperatorTokenType::Minus:
						stack.push(secondVal - firstVal);
						break;
					case OperatorTokenType::Multiply:
						stack.push(secondVal * firstVal);
						break;
					case OperatorTokenType::Divide:
						stack.push(secondVal / firstVal);
						break;
					default:
						break;
					}
				}
				else
				{
					int firstVal = stack.top();
					stack.pop();

					switch (op->type)
					{
					case OperatorTokenType::UnaryMinus:
						stack.push(-firstVal);
						break;
					default:
						break;
					}
				}
			}
		}

		return stack.top();
	}
};

int main()
{
	InitKeywords();

	std::ifstream file;
	file.open("code.txt");

	std::string input;

	std::string buff;

	// Так не должно быть, просто я пока не знаю, как разом считать файл при помощи ебаной std
	while (std::getline(file, buff))
	{
		input += buff + "\n";
	}

	std::cout << input << std::endl;

	GetFileSource().SplitToLines(input);

	std::cout << "Source out:\n";

	for (auto& str : GetFileSource().GetLines())
	{
		std::cout << str << std::endl;
	}

	ast::Context mainContext;

	Parser parser;

	parser.SetExpression(input);
	parser.SetContext(mainContext);

	parser.Parse();

	if (parser.GetStatus() == false)
	{
		std::cout << "Parser Error\n";
	}

	SemaChecker checker(mainContext);

	checker.Analyze();

	std::cout << "\nGenerated code:\n";

	std::string code = parser.GenerateCode();

	std::cout << code << std::endl;

	ast::DotPrinter printer;
	printer.Print(mainContext, "ast_post_sema", true);


	ir::Program* output;

	slm::AsmBuilder builder;

	std::string funcName = "main";

	builder.WriteFunction(funcName);
	builder.SetEntryFunction(funcName);

	builder.WriteMov(0, 5, slm::mov_flags::USE_CONSTANT_SRC);
	builder.WriteMov(4, 1, slm::mov_flags::USE_CONSTANT_SRC);
	builder.WriteAdd(4, 0);
	builder.WriteMov(8, 57, slm::mov_flags::USE_CONSTANT_SRC);
	builder.WriteBreakPoint();
	builder.WriteAdd(8, 4);

	slm::ImageInfo program = builder.GetImage();

	slm::VMachine machine;

	machine.LoadImage(program.ptr, program.size);

	machine.Run();

	machine.WriteStackDump();

	/*std::vector<MathTerm*> terms = parser.GenerateRPNString();

	MathEvaluator proc(terms);

	for (auto item : terms)
	{
		switch (item->GetType())
		{
		case MathType::Value:
			std::cout << static_cast<Value*>(item)->value;
			break;
		case MathType::Op:
			switch (static_cast<Operator*>(item)->type)
			{
			case OperatorType::Plus:
				std::cout << "+";
				break;
			case OperatorType::Minus:
				std::cout << "-";
				break;
			case OperatorType::Multiply:
				std::cout << "*";
				break;
			case OperatorType::Divide:
				std::cout << "/";
				break;
			case OperatorType::UnaryMinus:
				std::cout << "-()";
				break;

			}
			break;
		}
	}

	std::cout << "\n";*/

	/*int number = proc.Calc();

	std::cout << "\nThe Ultimate Question of Life, the Universe, and Everything: " << number << std::endl;*/

	return 0;
}
