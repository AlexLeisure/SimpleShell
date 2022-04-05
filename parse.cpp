#include <string>
#include <vector>
#include <regex.h>
#include "parse.h"

parsed_t parse(const std::string &line) {
	static regex_t compiled;
	static bool is_compiled = false;
    std::vector<std::string> parsed;
    parsed_t ret;

    ret.is_error = false;
    ret.is_pipe = false;
    ret.is_bg = false;
    ret.is_empty = false;

	if (! is_compiled) {
		if (regcomp(&compiled, "^[[:alpha:]/\\._-]+|^[\\|&]", REG_EXTENDED)) {
            ret.is_error = true;
            ret.error = "Internal parse error: regexp didn not compile";
			return ret;
		}
	}

	regmatch_t matches[2];

	char *ln = const_cast<char *>(line.c_str());
    
	bool is_last_pipe = false;
    bool is_last_bg = false;
    while ((*ln == ' ') | (*ln == '\t') || (*ln == '\n'))
            ln++;
	while (regexec(&compiled, ln, 2, matches,  0) == 0) {
		std::string tok = std::string(ln).substr(matches[0].rm_so, matches[0].rm_eo - matches[0].rm_so);

        if (tok != "|" && tok != "&")
		    parsed.push_back(tok);

        if (is_last_bg) {
            ret.is_error = true;
            ret.error = "backgound token must be end the command";
            return ret;
        }

		if (tok == "|") {
            ret.is_pipe = true;
			if (is_last_pipe) {
                ret.is_error = true;
                ret.error = "back-to-back pipes not allowed";
				return ret;
			}
		}

        if (tok == "&") {
            ret.is_bg = true;
            if (is_last_pipe) {
                ret.is_error = true;
                ret.error = "background after pipe not allowed";
                return ret;
            }
        }

        is_last_pipe = false;
        is_last_bg = false;
        if (tok == "&") {
            is_last_bg = true;
            ret.cmd.push_back(parsed);
            parsed.clear();

        } else if (tok == "|") {
            is_last_pipe = true;
            // beginning new command, so add this one   
            ret.cmd.push_back(parsed);
            parsed.clear();
        }

		ln += matches[0].rm_eo;
        while ((*ln == ' ') | (*ln == '\t') || (*ln == '\n'))
            ln++;
		
	}

    if (*ln != 0) {
        ret.is_error = true;
        ret.error = std::string("parse error at '") + std::string(ln) + "'";
        return ret;
    }

	if (is_last_pipe) {
        ret.is_error = true;
        ret.error = "command cannod end with a pipe";
		return ret;
	}	
    if (is_last_bg) {
        ret.is_bg = true;
    } else if (parsed.size() != 0) {
        // command did not end with a bg, so add last command
        ret.cmd.push_back(parsed);
    }

	return ret;
}


#ifdef PARSE_TEST
#include <iostream>
int main(int argc, char *argv[]) {
    parsed_t parsed;
    parsed = parse(std::string(argv[1]));
	if (parsed.is_error) {
		std::cerr << "Error:" << parsed.error << std::endl;
		exit(1);
	}
    if (parsed.is_pipe)
        std::cout << "command is a pipe" << std::endl;
    if (parsed.is_bg)
        std::cout << "command is backgrounded" << std::endl;
    if (parsed.is_empty)
    std::cout << "command is empty" << std::endl;

    std::cout << "size: " << parsed.cmd.size() << "\n";
    for(int i=0; i<parsed.cmd.size(); i++){
        std::cout << "cmdSize: " << parsed.cmd[i].size() << "\n";
    }

	for (std::vector<std::vector<std::string> >::iterator it = parsed.cmd.begin();
			it != parsed.cmd.end(); it++) {
                for(std::vector<std::string>::iterator onecmd = it->begin(); onecmd != it->end(); onecmd++)
		            std::cout << *onecmd << "\n";
                std::cout << std::endl;
	}
	return 0;
}
#endif
