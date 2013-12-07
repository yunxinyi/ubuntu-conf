#include "token.h"
#include <map>

using namespace std;
using namespace MICROCC;

Token::Token(TokenType type, TokenValue value)
	: m_type(type),
	m_value(value)
{
}

map<TokenType, const string> ttname = {
	{TokenType::BOOL, "bool"},
	{TokenType::INT, "int"},
	{TokenType::DOUBLE, "double"},
	{TokenType::CHAR, "char"},
	{TokenType::STR, "str"},
	{TokenType::TRUE, "true"},
	{TokenType::FALSE, "false"},

	{TokenType::IF, "if"},
	{TokenType::ELIF, "elif"},
	{TokenType::ELSE, "else"},
	{TokenType::FI, "fi"},

	{TokenType::CASE, "case"},
	{TokenType::IN, "in"},
	{TokenType::ESAC, "esac"},

	{TokenType::ASSIGN, "="},
	{TokenType::ADD, "+"},
	{TokenType::SUB, "-"},
	{TokenType::MUL, "*"},
	{TokenType::DIV, "/"},
	{TokenType::LT, "<"},
	{TokenType::LE, "<="},
	{TokenType::NE, "!="},
	{TokenType::EQ, "=="},
	{TokenType::GT, ">"},
	{TokenType::GE, ">="},
	{TokenType::NOT, "!"},

	{TokenType::COLON, ":"},
	{TokenType::AMP, "&"},
	{TokenType::CARET, "^"},
	{TokenType::UNDERSCORE, "_"},
	{TokenType::BACKSLASH, "\\"},
	{TokenType::DOT, "."},
	{TokenType::PIPE, "|"},
	{TokenType::POUND, "#"},

	{TokenType::LPAREN, "("},
	{TokenType::RPAREN, ")"},
	{TokenType::QUOTE, "\""},
	{TokenType::TILDE, "~"},
	{TokenType::LSUBSCRIPT, "["},
	{TokenType::RSUBSCRIPT, "]"},
	{TokenType::LBRACKET, "{"},
	{TokenType::RBRACKET, "}"},

	{TokenType::COMMA, ","},
	{TokenType::SEMICOLON, ";"},
	{TokenType::SPACE, " "},
	{TokenType::TAB, "\\t"},
	{TokenType::EOL, "\\n"},
	{TokenType::EOF_, "EOF"},

	{TokenType::EXPR, "expression"},
	{TokenType::TERM, "term"},
	{TokenType::FACTOR, "factor"},
};

ostream& MICROCC::operator<<(ostream& os, const Token& t) {
//	os << "token: [";
	switch (t.m_type) {
		case TokenType::POUND:
		case TokenType::IDENTIFIER:
		case TokenType::INTVAL:
		case TokenType::DOUBLEVAL:
		case TokenType::CHARVAL:
			os << t.m_value;
			break;
		case TokenType::STRVAL:
			os << "str(" << t.m_value << ")";
			break;
		default:
			os << ttname[t.m_type];
			break;
	}
//	os << "]";
	return os;
}

TokenTable::TokenTable() {}

TokenTable::~TokenTable() {
	this->clear();
}
