// Usage: ./byte_packer [OUTPUT HPP FILE] [INPUT FILE #1] [IDENTIFIER #1] [INPUT
// FILE #2] [IDENTIFIER #2] ...

#include <cstring>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

inline void print_hpp_head(const char *hpp_name) {
	std::string hpp_identifier;
	int len = strlen(hpp_name);
	for (int i = 0; i < len; ++i)
		hpp_identifier.push_back(isalpha(hpp_name[i]) ? toupper(hpp_name[i]) : '_');

	printf("#ifndef %s\n#define %s\n", hpp_identifier.c_str(), hpp_identifier.c_str());
}

inline void print_hpp_tail() { printf("#endif\n"); }

inline void print_file(const char *filename, const char *identifier) {
	printf("constexpr unsigned char %s[] = {", identifier);
	std::ifstream fin{filename, std::ios::ate | std::ios::binary};
	if (fin.is_open()) {
		size_t file_size = (size_t) fin.tellg();
		std::vector<char> buffer(file_size);
		fin.seekg(0);
		fin.read(buffer.data(), file_size);
		fin.close();
		auto *ubuffer = (unsigned char *) buffer.data();
		std::ostringstream ss;
		for (size_t i = 0; i < file_size; ++i) {
			ss << (int) ubuffer[i];
			if (i != file_size - 1)
				ss << ',';
			if (ss.str().length() >= 80) {
				printf("\n\t%s", ss.str().c_str());
				ss.str("");
			}
		}
		printf("\n\t%s", ss.str().c_str());
	} else
		printf("\n\t//Failed to open file: %s", filename);
	printf("\n};\n");
}

int main(int argc, const char **argv) {
	--argc, ++argv;
	if (argc < 1 || argc % 2 == 0) {
		printf("Usage: ./byte_packer [OUTPUT HPP FILE] [INPUT FILE #1] [IDENTIFIER #1] [INPUT FILE #2] [IDENTIFIER #2] ...\n");
		return EXIT_FAILURE;
	}
	freopen(argv[0], "w", stdout);
	print_hpp_head(argv[0]);
	--argc, ++argv;
	for (int i = 0; i < argc; i += 2)
		print_file(argv[i], argv[i | 1]);
	print_hpp_tail();

	return EXIT_SUCCESS;
}
