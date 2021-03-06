#include "preproc.h"
#include "lexical.h"
#include "parser.h"
#include "codegen.h"
#include "semantic.h"
#include "optimize.h"
#include "target.h"
#include "errproc.h"
#include "microcc.h"

#include <fstream>

using namespace std;

MICROCC::Microcc::Microcc()
	: m_pparser(new Parser())
{
}

MICROCC::Microcc::~Microcc() {
}

int
MICROCC::Microcc::compile(const char* fpath) {
	fstream sf(fpath);
	if (!sf.is_open()) {
		cerr << "cannot open file: " << fpath << endl;
		return -1;
	}
	sf.seekg(0, ios::end);
	m_sfsiz = sf.tellg();
	m_psf = new char[++m_sfsiz];
	sf.seekg(0, ios::beg);
	sf.read(m_psf, m_sfsiz);
	sf.close();
	m_psf[m_sfsiz-1] = -1;

	TokenTable toktbl;
	if (lex(m_psf, m_sfsiz, toktbl) == -1) {
		cerr << "lex parse error" << endl;
		return -1;
	}
	cout << toktbl << flush;
	CodeGen codegen;
	m_pparser->parse(toktbl, codegen);
	cout << codegen.identTable() << flush;
	codegen.dumpMCode(cout);

	return 0;
}
