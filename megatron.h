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
    bool leerSchema(const std::string& tableName, std::vector<Columna>& columnasDisponibles);
    //bool cumpleCondicion(const std::string& valor, const Columna& columna, const std::string& whereCondition);
    bool cumpleCondicion(const std::vector<std::string>& condiciones, const std::string& valor, const Columna& columna);
    void procesarConsulta(const std::string& query);
    bool procesarUpdate(const std::string& query);
};


bool leerSchema(const std::string& tableName, std::vector<Columna>& columnasDisponibles) {
    std::ifstream schemaFile("schema.txt");

    if (!schemaFile.is_open()) {
        std::cerr << "Error al abrir schema.txt. Verifica que el archivo existe y tiene permisos de lectura." << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(schemaFile, line)) {
        std::stringstream lineStream(line);
        std::string nombreTabla;
        std::getline(lineStream, nombreTabla, '#');

        // Limpiamos los espacios en blanco alrededor del nombre de la tabla
        nombreTabla.erase(nombreTabla.find_last_not_of(" \t") + 1);
        nombreTabla.erase(0, nombreTabla.find_first_not_of(" \t"));

        if (nombreTabla == tableName) {
            std::string columna, tipo;
            while (std::getline(lineStream, columna, '#') && std::getline(lineStream, tipo, '#')) {
                columna.erase(columna.find_last_not_of(" \t") + 1);
                columna.erase(0, columna.find_first_not_of(" \t"));
                tipo.erase(tipo.find_last_not_of(" \t") + 1);
                tipo.erase(0, tipo.find_first_not_of(" \t"));
                columnasDisponibles.push_back({ columna, tipo });
            }

            schemaFile.close();
            return true;
        }
    }

    std::cerr << "No se encontró la tabla " << tableName << " en schema.txt" << std::endl;
    return false;
}