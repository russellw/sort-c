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
#include <filesystem>
#include <fstream>
#include <iomanip>
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
#define debug(a) cout << __FILE__ << ':' << __LINE__ << ": " << __func__ << ": " << #a << ": " << a << '\n'
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
bool endsWith(string s, const char* t) {
	auto n = strlen(t);
	if (s.size() < n)
		return 0;
	return memcmp(s.data() + s.size() - n, t, n) == 0;
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

bool startsWith(string s, const char* t) {
	auto n = strlen(t);
	if (s.size() < n)
		return 0;
	return memcmp(s.data(), t, n) == 0;
}

// output
void writeLines() {
	ofstream os(file, std::ios::binary);
	for (auto s: V)
		os << s << '\n';
}

// SORT
regex assignRegex(R"((\w+) = )");
regex callRegex(R"((\w+)\()");
regex commentRegex(R"(\s*//.*)");
regex fnRegex(R"((\w+)\(.*\{$)");
regex lbraceRegex(R"(.*\{$)");
regex rbraceRegex(R"(\s*\};?)");
regex sortCommentRegex(R"(\s*// SORT)");
regex varRegex(R"((\w+)[;,])");
//

string at(int i) {
	if (i < V.size())
		return V[i];
	return "}";
}

struct Block {
	int first, last;
	string key;

	Block(int dent, int i): first(i) {
		while (regex_match(at(i), commentRegex))
			++i;
		auto s = at(i);
		key = s;
		smatch m;
		if (regex_search(s, m, fnRegex)) {
			key = m[1].str() + ':' + s;
			do {
				++i;
				if (i == V.size())
					throw runtime_error(file + ':' + to_string(first + 1) + ": unclosed function");
			} while (!(indent(i) == dent && regex_match(at(i), rbraceRegex)));
		} else if (regex_search(s, m, lbraceRegex))
			do {
				++i;
				if (i == V.size())
					throw runtime_error(file + ':' + to_string(first + 1) + ": unclosed brace");
			} while (!(indent(i) == dent && regex_match(at(i), rbraceRegex)));
		else if (regex_search(s, m, assignRegex) || regex_search(s, m, callRegex) || regex_search(s, m, varRegex))
			key = m[1].str() + ':' + s;
		last = i + 1;
	}

	bool operator<(const Block& b) {
		return key < b.key;
	}

	void to(vector<string>& o) {
		o.insert(o.end(), V.begin() + first, V.begin() + last);
	}
};

int main(int argc, char** argv) {
	try {
		for (int i = 1; i < argc; ++i) {
			file = argv[i];
			readLines();
			auto old = V;
			for (int i = 0; i < V.size();) {
				if (!regex_match(V[i], sortCommentRegex)) {
					++i;
					continue;
				}

				// sortable blocks should be indented at the same level as the marker comment
				auto dent = indent(i);
				++i;

				// get group of blocks
				int j = i;
				vector<Block> blocks;
				for (;;) {
					// skip intervening blank lines
					while (at(j).empty())
						++j;

					// end of group?
					if (indent(j) < dent)
						break;
					auto s = V[j];
					if (regex_match(s, commentRegex))
						break;
					if (startsWith(s, "} // namespace"))
						break;

					// get the next block
					Block block(dent, j);
					j = block.last;
					blocks.push_back(block);
				}

				// sort
				sort(blocks.begin(), blocks.end());

				// if blocks are functions, separate with blank lines
				bool blanks = 0;
				for (auto block: blocks)
					if (regex_search(V[block.first], fnRegex)) {
						blanks = 1;
						break;
					}

				// update
				vector<string> o;
				for (auto block: blocks) {
					if (blanks && o.size())
						o.push_back("");
					block.to(o);
				}
				if (blanks && regex_match(at(j), commentRegex))
					o.push_back("");
				V.erase(V.begin() + i, V.begin() + j);
				V.insert(V.begin() + i, o.begin(), o.end());

				i += o.size();
			}
			if (old != V)
				writeLines();
		}
		return 0;
	} catch (exception& e) {
		cout << e.what() << '\n';
		return 1;
	}
}
