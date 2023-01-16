#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "vector.h"

double mean(const mdp::vector<int>& v) {
	puts("double mean(mdp::vector v)");

	double s = 0.0;
	for (size_t i = 0; i < v.size(); ++i) {
		s += v[i];
	}
	return s / v.size();
}

void double_all_elements(mdp::vector<int>& v) {
	for (size_t i = 0; i < v.size(); ++i) {
		v[i] *= 2;
	}
}

struct person {
	char name[100];
	int age;

	bool operator<(const person& rhs) const {
		return age < rhs.age;
		//return strcmp(name, rhs.name) < 0;
	}
};

int main(int argc, char *argv[])
{
	{
		if (argc != 3) {
			return EXIT_FAILURE;
		}

		mdp::vector<int> v;
		
		v = mdp::read(argv[1]);

		mdp::vector<int> please_copy_v = v;
		double d = mean(v);
		double_all_elements(v);

		if (v.empty()) {
			return EXIT_FAILURE;
		}
		
		// Sort v
		sort(v);

		// Save the output to file
		write(argv[2], v);
	}
    return EXIT_SUCCESS;
}
