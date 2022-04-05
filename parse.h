#ifndef PARSE_H
#define PARSE_h
#include <string>
#include <vector>

#define PARSE_TEST

struct parsed {
	std::string error;
	bool is_error;
	bool is_empty;
    bool is_bg;
    bool is_pipe;
	std::vector<std::vector<std::string> > cmd;
};

typedef struct parsed parsed_t; 
parsed_t parse(const std::string &line);
#endif

