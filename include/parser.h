#ifndef PARSER_H_
#define PARSER_H_

#include <string>

class Parser {
private:
    std::string cmd;
    int pos;
    std::string error;

public:
    Parser(const std::string &cmd);
    std::string nextPart();
    bool nextOnOff();
    uint32_t nextUint32();
    void end();

    bool isOk();
    const std::string getError();
};

#endif /* PARSER_H_ */
