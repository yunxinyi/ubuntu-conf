#include "ident.h"

using namespace std;

MICROCC::Ident::Ident(const Token& tok, size_t scope, const string& name,
		size_t addr)
: Token(tok),
m_scope(scope),
m_name(name),
m_addr(addr),
m_init(false)
{
	if (tok.m_type == TokenType::IDENTIFIER) {
		m_name = m_value;
		//m_value = "nonset";
	} else {
		//cout << "-----> " << tok << endl;
		//cout << "Ident ===> " << *this << endl;
	}
}

MICROCC::Ident::Ident(size_t scope, IdentType idt, const std::string& name,
				const std::string& value, size_t addr)
: Token(static_cast<TokenType>(idt), value),
	m_scope(scope),
	m_name(name),
	m_addr(addr)
{}


MICROCC::Ident::~Ident() {}

std::ostream&
MICROCC::operator<<(std::ostream& os, const Ident& ident) {
	os << static_cast<Token>(ident) << "\t" << ident.m_scope << "\t"
		<< ident.m_name << "\t" << static_cast<ssize_t>(
				ident.m_addr == static_cast<size_t>(-1) ? -1 : ident.m_addr);
	return os;
}

std::ostream&
MICROCC::operator<<(std::ostream& os, const IdentTable& idtbl) {
	os << "====> Identifier Table (size: " << idtbl.size() <<
		"):\n----------------------------------------------------------\n"
		<< "position\ttype\tvalue\tscope\tname\taddr\n";
	for (const auto id : idtbl)
		os << id << endl;
	os << "==========================================================\n";

	return os;
}

MICROCC::IdentTable::IdentTable()
: m_tmpVar(0),
m_varAddr(0)
{}

MICROCC::IdentTable::~IdentTable() {
	this->clear();
}

size_t
MICROCC::IdentTable::genAddr(IdentType idt) {
	size_t sz(0);
	size_t ret(m_varAddr);
	switch (idt) {
		case IdentType::FUNC:
		case IdentType::INT:
			sz = 4;
			break;
		case IdentType::DOUBLE:
			sz = 8;
			break;
		case IdentType::BOOL:
			sz = 2;
			break;
	}
	m_varAddr += sz;
	return ret;
}

MICROCC::Ident&
MICROCC::IdentTable::genTmp(MICROCC::IdentType idt) {
	push_back({0, idt, string("_tmp").append(to_string(m_tmpVar++)),
			"val", genAddr(idt)});
	return this->back();
}

std::list<MICROCC::Ident>::iterator
MICROCC::IdentTable::lookup(const std::string& name) {
	for (auto it = this->begin(); it != this->end(); ++it) {
		if (it->m_name == name)
			return it;
	}
	return this->end();
}

bool
MICROCC::IdentTable::remove(const std::string& name) {
	auto it = lookup(name);
	if (it != this->end()) {
		erase(it);
		return true;
	}
	return false;
}

