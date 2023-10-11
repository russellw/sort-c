#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <ostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

#ifdef NDEBUG
#define debug(a)
#else
#define debug(a) cout << __FILE__ << ':' << __LINE__ << ": " << __func__ << ": " << #a << ": " << (a) << '\n'
#endif

// input
string file;
string text;
vector<string> V;

void readText() {
	ifstream is(file, std::ios::in);
	text = {istreambuf_iterator<char>(is), istreambuf_iterator<char>()};

	// make sure input ends with a newline, to simplify parser code
	if (text.empty() || text.back() != '\n')
		text += '\n';
}

void readLines() {
	readText();
	auto s = text.data();
	V.clear();
	while (*s) {
		auto u = strchr(s, '\n');
		auto t = u;
		while (s < t && (t[-1] == ' ' || t[-1] == '\t' || t[-1] == '\r'))
			--t;
		V.push_back(string(s, t));
		s = u + 1;
	}
}

// SORT
string at(int i) {
	if (i < V.size())
		return V[i];
	return "}";
}

int indent(int i) {
	// end of file is end of scope, so semantically a dedent
	if (i == V.size())
		return -1;

	auto s = V[i];

	// blank line does not meaningfully have an indent level
	if (s.empty())
		return INT_MAX;

	// in C++, nor does a preprocessor directive
	if (s[0] == '#')
		return INT_MAX;

	// assuming each file uses either tabs or spaces consistently
	int j = 0;
	while (s[j] == '\t' || s[j] == ' ')
		++j;
	return j;
}

bool startsWith(const string& s, const char* t) {
	auto n = strlen(t);
	if (s.size() < n)
		return 0;
	return memcmp(s.data(), t, n) == 0;
}

// output
void writeLines() {
	ofstream os(file, std::ios::binary);
	for (auto& s: V)
		os << s << '\n';
}
