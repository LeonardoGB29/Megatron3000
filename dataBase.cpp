#include "dataBase.h"

std::string removeChars(const std::string& str, char charToRemove) {
    std::string result;
    for (char c : str) {
        if (c != charToRemove) {
            result += c;
        }
    }
    return result;
}

std::string replaceWithSymbol(const std::string& input, char symbol) { // symbol =  ","
    std::string result;
    bool insideQuotes = false;

    for (char c : input) {
        if (c == '"') {
            insideQuotes = !insideQuotes;
            result += c;
        }
        else if (c == ',' && !insideQuotes) {  //a,b,"a,b"
            result += symbol;
        }
        else {
            result += c;
        }
    }

    return result;
}

void dataBase::parseAndExecuteQuery(const std::string& query) {
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

// CREATE TABLE employee(name VARCHAR, age INT);

void dataBase::createTable(const std::string& query) {

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

void dataBase::select(const std::string& query) {
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

void dataBase::displayRelations() {
    std::ifstream schemaFile("schema.txt");
    std::string line;

    if (!schemaFile.is_open()) {
        std::cerr << "No existe schema.txt. Por favor, crea un esquema en MEGATRON." << std::endl;
        return;
    }

    std::cout << "\nTienes estas relaciones:" << std::endl;

    while (std::getline(schemaFile, line)) {
        std::istringstream iss(line);
        std::string relationName;
        std::getline(iss, relationName, '#');
        std::cout << "    -> " << relationName << std::endl;
    }

    std::cout << std::endl;

    schemaFile.close();
}

std::string dataBase::selectRelation() {

    std::ifstream schemaFile("schema.txt");
    std::string line, relationName;
    bool found = false;
    std::cout << "Escriba una relación: ";
    std::cin >> relationName;

    while (std::getline(schemaFile, line)) {
        if (line.find(relationName) == 0) {
            found = true;
            break;
        }
    }

    schemaFile.close();
    return relationName;
}

bool dataBase::validateInstance(const std::string& schemaLine, const std::string& instance) { // en este punto la instancia ya esta generada, tengo que comprobar que 
    
    std::istringstream schemaStream(schemaLine);
    std::istringstream instanceStream(instance);
    std::string schemaPart, instanceValue;
        
    std::getline(schemaStream, schemaPart, '#');

    while (std::getline(schemaStream, schemaPart, '#') && std::getline(instanceStream, instanceValue, '#')) {

        std::getline(schemaStream, schemaPart, '#');

        if (schemaPart == "INT") {
            for (char c : instanceValue) {
                if (c < '0' || c > '9') return false;
            }
        }

        else if (schemaPart == "FLOAT") {

            bool puntoEncontrado = false;

            for (char c : instanceValue) {
                if (c == '.') {
                    if (puntoEncontrado) return false;
                    puntoEncontrado = true;
                }
                else if (c < '0' || c > '9') return false;
            }
        }

        else if (schemaPart == "CHAR") {
            if (instanceValue.length() != 1)
                return false;
        }

        else if (schemaPart == "VARCHAR") {
            if (instanceValue.empty()) 
                return false;
        }

        else if (schemaPart == "BOOL") {
            if (!(instanceValue == "0" || instanceValue == "1" ||
                instanceValue == "true" || instanceValue == "false" ||
                instanceValue == "TRUE" || instanceValue == "FALSE")) {
                return false;
            }
        }
    }

    return true;
}

void dataBase::uploadInstances(const std::string& data, const char& symbol) {
    std::ifstream schemaFile("schema.txt");
    std::string schemaLine, instanceFile, relationName;

    displayRelations();
    relationName = selectRelation();


    while (std::getline(schemaFile, schemaLine)) {
        if (schemaLine.find(relationName) == 0) {
            break;
        }
    }
    schemaFile.close();

    std::ifstream instancesFile(data);
    std::string instance;

    relationName += ".txt";

    std::ofstream outputFile(relationName, std::ios::app);

    while (std::getline(instancesFile, instance)) {

        instance = replaceWithSymbol(instance, '#');

        if (validateInstance(schemaLine, instance)) {
            outputFile << instance << std::endl;
            std::cout << "Instancia válida: " << instance << std::endl;
        }
        else {
            std::cout << "Instancia inválida: " << instance << std::endl;
        }
    }
    instancesFile.close();
    outputFile.close();
}



void dataBase::showMenu() {
    int option;
    do {
        std::cout << "\n########### MENU PRINCIPAL ###########" << std::endl;
        std::cout << "1. Entrar el MEGATRON3000" << std::endl;
        std::cout << "2. Generar instancias" << std::endl;
        std::cout << "3. Salir\n" << std::endl;
        std::cout << "Seleccione una opción: ";
        std::cin >> option;
        std::cout << std::endl;

        switch (option) {
        case 1: {

            std::string query;
            std::cout << "% MEGATRON3000" << std::endl << "    Welcome to MEGATRON 3000!" << std::endl;

            do {
                std::cout << "& ";
                std::getline(std::cin, query);
                if (query.empty()) continue;
                parseAndExecuteQuery(query);
            } while (query != "quit");

            break;
        }
        case 2: {

            std::string name;
            char symbol;
            std::cout << "Ingrese el nombre del archivo (con extensión): ";
            std::cin >> name;
            std::cout << std::endl <<"Con que caracter esta separado? ";
            std::cin >> symbol;
            std::cout << std::endl;
            //error cuando se mande otro char que no sea con el que se separa 
            uploadInstances(name, symbol);
            break;
        }
        case 3:
            std::cout << "Saliendo del programa..." << std::endl;
            break;
        default:
            std::cout << "Opción no válida, intente de nuevo." << std::endl;
        }

    } while (option != 3);
}