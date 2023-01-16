#include "vector.h"

#include <cstdlib>
#include <cstdio>

namespace mdp {

// Save the output to file
bool write(const char *filename, const vector<int>& v)
{
	FILE *fout = fopen(filename, "w");
	if (fout == NULL) {
		return false;
	}
	for (size_t i = 0; i < v.size(); ++i) {
		fprintf(fout, "%d\n", v.at(i));
	}
	fclose(fout);
	return true;
}

// Read a vector from file
vector<int> read(const char *filename)
{
	vector<int> v;
	FILE *f = fopen(filename, "r"); // <-- 0x0d 0x0a becomes 0x0a
	if (f != NULL) {
		int val;
		while (fscanf(f, "%d", &val) == 1) {
			v.push_back(val);
		}
		v.shrink_to_fit();
		fclose(f);
	}
	return v;
}

}