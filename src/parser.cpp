#include <parser.h>

#include <limits.h>
#include <stdlib.h>



Parser::Parser(const std::string &cmd) : cmd(cmd), pos(0), error("") {
}

std::string Parser::nextPart() {
    while(pos < cmd.length() && (cmd[pos] == ' ' || cmd[pos] == '\r')) pos++;
	const int begin = pos;
	while(pos < cmd.length() && cmd[pos] != ' ' && cmd[pos] != '\r') pos++;
	const int end = pos;

	return cmd.substr(begin, end-begin);
}

bool Parser::nextOnOff() {
	const std::string part = nextPart();
	if (part == "on") {
		return true;
	} else if (part == "off") {
		return false;
	} else {
		error = "expected on|off: " + part;
		return false;
	}
}

uint32_t Parser::nextUint32() {
	const std::string part = nextPart();
    const uint32_t value = strtoul(part.c_str(), 0, 10);
    if (value == ULONG_MAX) {
        error = "expected number: " + part;
    }
    return value;
}

void Parser::end() {
    int rest = pos;
	if (nextPart() != "") {
		error = "too many args:" + cmd.substr(rest);
	}
}

bool Parser::isOk() {
    return error == "";
}

const std::string Parser::getError()  {
    return error;
}