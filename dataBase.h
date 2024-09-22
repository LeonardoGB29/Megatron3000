#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

class dataBase {
public:
    void showMenu();
    void displayRelations();
    std::string selectRelation();
    bool validateInstance(const std::string& schemaLine, const std::string& instance);
    void uploadInstances(const std::string& data, const char& symbol);
};