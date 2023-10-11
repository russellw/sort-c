#include "all.h"

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
					cout << "sort-c version 1\n";
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

		for (auto& file0: files) {
			file = file0;
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
					auto& s = V[j];
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
				for (auto& block: blocks)
					if (regex_search(V[block.first], fnRegex)) {
						blanks = 1;
						break;
					}

				// update
				vector<string> o;
				for (auto& block: blocks) {
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
			if (inplace) {
				if (old != V)
					writeLines();
				continue;
			}
			for (auto& s: V)
				cout << s << '\n';
		}
		return 0;
	} catch (exception& e) {
		cerr << e.what() << '\n';
		return 1;
	}
}
