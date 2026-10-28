#pragma once
#include <map>
#include <string>
#include "Errors.h"

// OOP: Encapsulation. The map is private; the only way in is set()/get().
class SymbolTable {
private:
    std::map<std::string, int> variables;
public:
    void set(const std::string& name, int value) { variables[name] = value; }

    int get(const std::string& name, int line) const {
        auto it = variables.find(name);
        if (it == variables.end())
            throw RuntimeError("Undefined variable '" + name + "'", line);
        return it->second;
    }
};