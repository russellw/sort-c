#include "all.h"

// SORT
regex caseRegex(R"(\s*(case|default)\W.*)");
regex rbraceRegex(R"(\s*\})");
regex switchRegex(R"(\s*switch .*)");
//

struct Block {
	int first, last;

	Block(int dent, int i): first(i) {
		// preconditions
		if (i == V.size())
			throw runtime_error(file + ": unexpected end of file");
		if (!regex_match(V[i], caseRegex))
			throw runtime_error(file + ':' + to_string(i + 1) + ": expected case");

		// case labels
		do {
			++i;
			if (i == V.size())
				throw runtime_error(file + ": unexpected end of file");
		} while (regex_match(V[i], caseRegex));

		// opening brace
		auto brace = V[i - 1].back() == '{';

		// end of block
		do
			++i;
		while (dent < indent(i));

		// closing brace
		if (brace) {
			if (!regex_match(V[i], rbraceRegex))
				throw runtime_error(file + ':' + to_string(i + 1) + ": expected '}'");
			++i;
		}

		last = i;
	}

	string& key() const {
		return V[first];
	}

	bool operator<(const Block& b) {
		return key() < b.key();
	}

	void to(vector<string>& o) {
		auto i = last;
		while (first < i && V[i - 1].empty())
			--i;
		o.insert(o.end(), V.begin() + first, V.begin() + i);
	}
};

int main(int argc, char** argv) {
	try {
		bool inplace = 0;
		vector<string> files;
		for (int i = 1; i < argc; ++i) {
			auto s = argv[i];
			if (*s == '-') {
				while (*s == '-')
					++s;
				switch (*s) {
				case 'V':
				case 'v':
					cout << "sort-cases version 1\n";
					return 0;
				case 'h':
					cout << "-h  Show help\n";
					cout << "-V  Show version\n";
					cout << "-i  Edit files in place\n";
					return 0;
				case 'i':
					inplace = 1;
					continue;
				}
				throw runtime_error(string(argv[i]) + ": unknown option");
			}
			files.push_back(s);
		}
		for (int i = 1; i < argc; ++i) {
			file = argv[i];
			readLines();
			auto old = V;

			// case labels
			for (int i = 0; i < V.size();) {
				if (!regex_match(V[i], caseRegex)) {
					++i;
					continue;
				}

				// group of case labels
				auto j = i + 1;
				while (regex_match(V[j], caseRegex))
					++j;

				// does the last one have a brace?
				auto& s = V[j - 1];
				bool brace = 0;
				if (s.back() == '{') {
					s.pop_back();
					if (s.back() == ' ')
						s.pop_back();
					brace = 1;
				}

				// just sorting lines, so can sort in place
				sort(V.begin() + i, V.begin() + j);

				// brace needs to be restored on the new last label
				if (brace)
					V[j - 1] += " {";

				i = j;
			}

			// case blocks
			for (int i = 0; i < V.size();) {
				if (!regex_match(V[i], switchRegex)) {
					++i;
					continue;
				}

				auto dent = indent(i);
				++i;

				// if an entire case block is surrounded by preprocessor directives
				// the syntax is too complicated to handle confidently
				// so bail
				if (V[i][0] == '#')
					continue;

				// get group of blocks
				int j = i;
				vector<Block> blocks;
				for (;;) {
					// end of group?
					if (indent(j) == dent && regex_match(V[j], rbraceRegex))
						break;

					// get the next block
					Block block(dent, j);
					j = block.last;
					blocks.push_back(block);
				}

				// sort
				sort(blocks.begin(), blocks.end());

				// update
				vector<string> o;
				for (auto block: blocks)
					block.to(o);
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
