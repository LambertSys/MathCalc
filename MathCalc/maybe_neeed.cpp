	// Обход АСТ
	//void Traseval(AST::Node* node)
	//{
	//	if (node == nullptr)
	//	{
	//		return;
	//	}

	//	Traseval(node->left);
	//	switch (node->GetType())
	//	{
	//	case AST::NodeType::Value:
	//		std::cout << std::to_string(static_cast<AST::ValueNode*>(node)->value);
	//		break;
	//	case AST::NodeType::Operator:
	//		switch (static_cast<AST::OperatorNode*>(node)->opType)
	//		{
	//		case OperatorType::Plus:
	//			std::cout << "+";
	//			break;
	//		case OperatorType::Minus:
	//			std::cout << "-";
	//			break;
	//		case OperatorType::Multiply:
	//			std::cout << "*";
	//			break;
	//		case OperatorType::Divide:
	//			std::cout << "/";
	//			break;
	//		default:
	//			break;
	//		}
	//		break;
	//	}

	//	Traseval(node->right);
	//}