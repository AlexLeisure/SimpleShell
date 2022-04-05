#ifndef PARSE_H
#define PARSE_h
#include <string>
#include <vector>

//#define PARSE_TEST

/**
 * @brief parsed struct for using after running the parse command
 */
struct parsed {
	std::string error;
	bool is_error;
	bool is_empty;
    bool is_bg;
    bool is_pipe;
	std::vector<std::vector<std::string>> cmd;
};

typedef struct parsed parsed_t; 

/**
 * @brief This function parses a command line for execution.
 * The parse function takes a command line and seperates out the commands into lists. Each command list has it's program and arguments seperated out into another list. 
 * 
 * ex: cmd "ls -l /folder/" gets parsed into [["ls", "-l", "/folder/"]]
 * @param line 
 * @return parsed_t 
 */
parsed_t parse(const std::string &line);
#endif

