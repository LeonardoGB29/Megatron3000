#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

struct Column {
    std::string name;
    std::string type;
};

struct Table {
    std::string name;
    std::vector<Column> columns;
};

void dataBase::saveSchema(const Table& table) {

    std::ofstream schemaFile("/scheme", std::ios::app);

    if (schemaFile.is_open()) {
        schemaFile << table.name;
        for (const auto& column : table.columns) {
            schemaFile << "#" << column.name << "#" << column.type;
        }
        schemaFile << std::endl;
        schemaFile.close();
        std::cout << "Esquema guardado correctamente en /usr/db/scheme" << std::endl;
    }

    else {
        std::cerr << "No se pudo abrir el archivo del esquema" << std::endl;
    }
}

class dataBase {
public:
    std::vector<Relation> relations;

    void addRelation(const Relation& relation);
    void parseAndExecuteQuery(const std::string& query);
    void saveSchema(const Table& table);
};