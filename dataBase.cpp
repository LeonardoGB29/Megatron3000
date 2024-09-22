#include "dataBase.h"
#include "megatron.h"
#include <filesystem>

namespace fs = std::filesystem;

std::string replaceWithSymbol(const std::string& input, char symbol) { // symbol =  "#"
    std::string result;
    bool insideQuotes = false;

    for (char c : input) {
        if (c == '"') {
            insideQuotes = !insideQuotes;
            result += c;
        }
        else if (c == ',' && !insideQuotes) {  //a,b,"a,b"
            result += " ";
            result += symbol;
            result += " ";
        }
        else {
            result += c;
        }
    }

    return result;
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
    std::cout << "Seleccion una relación: ";
    std::cin >> relationName;
    std::cout << std::endl;

    while (std::getline(schemaFile, line)) {
        if (line.find(relationName) == 0) {
            found = true;
            break;
        }
    }

    schemaFile.close();
    return relationName;
}

bool dataBase::validateInstance(const std::string& schemaLine, const std::string& instance) { 
    // en este punto la instancia ya esta generada, tengo que comprobar con el schema si cumple los requisitos

    int schemaCount = 0, instanceCount = 0;
    std::istringstream schemaStreamCount(schemaLine);
    std::istringstream instanceStreamCount(instance);
    std::string temp;

    std::getline(schemaStreamCount, temp, '#');

    while (std::getline(schemaStreamCount, temp, '#')) {
        std::getline(schemaStreamCount, temp, '#');
        schemaCount++;
    }

    while (std::getline(instanceStreamCount, temp, '#')) {
        instanceCount++;
    }

    if (schemaCount != instanceCount) {
        std::cerr << "Numero de atributos del registro no coincide con el esquema" << std::endl << std::endl;
        return false;
    }

    std::istringstream schemaStream(schemaLine);
    std::istringstream instanceStream(instance);
    std::string schemaPart, instanceValue;
    
    std::getline(schemaStream, schemaPart, '#'); // pasar el nombre de la relacion

    while (std::getline(schemaStream, schemaPart, '#') && std::getline(instanceStream, instanceValue, '#')) {

        std::getline(schemaStream, schemaPart, '#'); // pasar el nombre del atributo

        std::cout << instanceValue << std::endl;

        if (instanceValue.empty()) {
            continue;
        }

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

        else if (schemaPart == "STR") {}

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

void dataBase::uploadInstances(const std::string& data, const char& symbol) { // symbol = caracter que separa los registros

    std::string dbFolder = "db";

    if (!fs::exists(dbFolder)) { // crear directorio db si no existe
        fs::create_directory(dbFolder);
        std::cout << "Directorio db creado" << std::endl;
    }

    std::ifstream schemaFile("schema.txt");
    std::string schemaLine, instanceFile, relationName;

    displayRelations(); // mostrar todas la relations
    relationName = selectRelation(); // escribes una relation

    while (std::getline(schemaFile, schemaLine)) {
        if (schemaLine.find(relationName) == 0) {
            break;
        }
    }
    schemaFile.close();

    std::ifstream instancesFile(data);
    std::string instance;
    std::string filePath = dbFolder + "/" + relationName + ".txt";


    std::ofstream outputFile(filePath, std::ios::app);

    std::getline(instancesFile, instance);  // Saltar la primera línea (nombre de las columnas)

    while (std::getline(instancesFile, instance)) {
        instance = replaceWithSymbol(instance, '#'); 

        // schemaLine = línea del esquema correspondiente a la relación
        // instance = instancia generada, separada por '#'

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

    Megatron megatron; // aqui?

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
            std::cout << "\n% MEGATRON3000" << std::endl << "    Welcome to MEGATRON 3000!" << std::endl;

            do {
                std::cout << "& ";
                std::getline(std::cin, query);
                if (query.empty()) continue;
                megatron.parseAndExecuteQuery(query);
            } while (query != "quit");

            break;
        }
        case 2: {

            std::string name;
            char symbol;
            std::cout << "Ingrese el nombre del archivo (con extension): ";
            std::cin >> name;
            std::cout << std::endl <<"Con que caracter esta separado? ";
            std::cin >> symbol;
            std::cout << std::endl;
            uploadInstances(name, symbol);
            break;
        }
        case 3:
            std::cout << "Saliendo del programa..." << std::endl;
            break;
        default:
            std::cout << "Opcion no valida, intente de nuevo." << std::endl;
        }

    } while (option != 3);
}