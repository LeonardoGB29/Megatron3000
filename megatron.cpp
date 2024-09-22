#include "megatron.h"
#include <iostream>

std::string removeChars(const std::string& str, char charToRemove) {
    std::string result;
    for (char c : str) {
        if (c != charToRemove) {
            result += c;
        }
    }
    return result;
}

void Megatron::parseAndExecuteQuery(const std::string& query) {
    std::istringstream iss(query);
    std::string command;
    iss >> command;

    if (command == "CREATE") {
        createTable(query);
    }
    else if (command == "SELECT") {
        select(query);
    }
    else {
        std::cout << "Comando no reconocido: " << command << std::endl;
    }
}

void Megatron::createTable(const std::string& query) {

    //std::cout << query << std::endl;
    std::istringstream iss(query);
    std::string command, tableKeyword, tableName;
    iss >> command >> tableKeyword >> tableName;

    if (tableKeyword == "TABLE") {
        std::ofstream schemaFile("schema.txt", std::ios::app);

        if (!schemaFile.is_open()) {
            std::cerr << "No se pudo abrir el archivo schema.txt" << std::endl;
            return;
        }

        schemaFile << tableName;

        std::string columnPart;
        while (std::getline(iss, columnPart, ',')) {
            columnPart = removeChars(columnPart, '(');
            columnPart = removeChars(columnPart, ')');
            columnPart = removeChars(columnPart, ';');

            std::istringstream columnStream(columnPart);
            std::string columnName, columnType;
            columnStream >> columnName >> columnType;

            if (columnType == "VARCHAR") {
                columnType = "STR";
            }

            schemaFile << " # " << columnName << " # " << columnType;
        }

        schemaFile << std::endl;
        schemaFile.close();
        std::cout << "Esquema creado correctamente en schema.txt" << std::endl;
    }
    else {
        std::cout << "Se esperaba 'CREATE TABLE'" << std::endl;
    }
}


void Megatron::select(const std::string& query) {
    std::istringstream iss(query);
    std::string command, star, from, tableName;
    iss >> command >> star >> from >> tableName;

    if (command == "SELECT" && star == "*" && from == "FROM") {
        std::cout << "Seleccionando todas las columnas de la tabla: " << tableName << std::endl;
        // Aquí leerías la tabla y mostrarías sus datos
    }
    else {
        std::cout << "Formato de SELECT inválido." << std::endl;
    }
}