#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <regex>
#include <filesystem>

namespace fs = std::filesystem;

struct Columna {
    std::string nombre;
    std::string tipo;
};

class Megatron {
public:
    void parseAndExecuteQuery(const std::string& query);
    void createTable(const std::string& query);
    void select(const std::string& query); // Actualizar para la nueva implementación
    void updateTable(const std::string& query);
    bool insertTable(const std::string& query);

private:
    bool obtenerColumnasYTabla(const std::string& query, std::vector<std::string>& listaColumnas, std::string& tableName);
    bool validateInstance(const std::vector<Columna>& columnasDisponibles, const std::vector<std::string>& valoresAInsertar);
    //bool cumpleCondicion(const std::string& valor, const Columna& columna, const std::string& whereCondition);
    bool cumpleCondicion(const std::vector<std::string>& condiciones, const std::string& valor, const Columna& columna);
    void procesarConsulta(const std::string& query);
    //bool procesarUpdate(const std::string& query);
};