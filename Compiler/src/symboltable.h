
#pragma once
#include <map>
#include <string>
#include "Errors.h"

using namespace std;

// OOP: Encapsulation. The map is private; the only way in is set()/get().
class SymbolTable {
private:
    map<string, int> variables;

public:
    void set(const string& name, int value) { variables[name] = value; }

    int get(const string& name, int line) const {
        auto it = variables.find(name);
        if (it == variables.end())
            throw RuntimeError("Undefined variable '" + name + "'", line);
        return it->second;
    }
};
