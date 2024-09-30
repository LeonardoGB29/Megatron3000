#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <regex>
#include <map>
#include <filesystem>

namespace fs = std::filesystem;

const fs::path Schema_Dir = fs::current_path().parent_path() / "db" / "scheme.txt";
const fs::path DB_Dir = fs::current_path().parent_path() / "db";

struct Columna {
    std::string nombre;
    std::string tipo;
};

class Megatron {
public:
    void parseAndExecuteQuery(const std::string& query);
    void createTable(const std::string& query);
    void select(const std::string& query);
    void updateTable(const std::string& query);
    bool insertTable(const std::string& query);

private:

    bool obtenerColumnasYTabla(const std::string& query, std::vector<std::string>& listaColumnas, std::string& tableName);
    bool validateInstance(const std::vector<Columna>& columnasDisponibles, const std::vector<std::string>& valoresAInsertar);
    bool cumpleCondicion(const std::vector<std::string>& condiciones, const std::string& valor, const Columna& columna);

    bool cumpleCondicion(const std::string& whereCondition, const std::vector<std::string>& fila1, const std::vector<Columna>& columnasTabla1,
        const std::vector<std::string>& fila2, const std::vector<Columna>& columnasTabla2, const std::string& nombreTabla1, const std::string& nombreTabla2);
    
    bool evaluarCondicion(const std::string& operador, const std::string& valor, const std::string& valorCondicion, const std::string& tipo);
    void MultipleSelect(const std::string& query);
    bool cumpleCondicion(const std::string& whereCondition,
        const std::vector<std::string>& filaCombinada,
        const std::vector<std::vector<std::string>>& nombresColumnas,
        const std::vector<std::string>& listaTablas);
    bool leerDatos(const std::string& nombreTabla, std::vector<std::vector<std::string>>& filasTabla);
};