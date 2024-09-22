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
    procesarConsulta(query);
}

bool Megatron::obtenerColumnasYTabla(const std::string& query, std::vector<std::string>& listaColumnas, std::string& tableName) {
    std::regex sqlRegex(R"(SELECT\s+([^FROM]+)\s+FROM\s+(\w+)(?:\s+WHERE\s+(.+))?)", std::regex::icase);
    std::smatch match;

    if (std::regex_match(query, match, sqlRegex)) {
        std::string columnas = match[1];
        tableName = match[2];
        std::stringstream ss(columnas);
        std::string columna;
        while (std::getline(ss, columna, ',')) {
            size_t start = columna.find_first_not_of(" \t");
            size_t end = columna.find_last_not_of(" \t");
            if (start != std::string::npos && end != std::string::npos) {
                listaColumnas.push_back(columna.substr(start, end - start + 1));
            }
        }
        return true;
    }
    return false;
}

bool Megatron::leerSchema(const std::string& tableName, std::vector<Columna>& columnasDisponibles) {
    std::ifstream schemaFile("C:/Users/USUARIO/Source/Repos/Megatronn3000/db/schema.txt");
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
bool Megatron::cumpleCondicion(const std::vector<std::string>& condiciones, const std::string& valor, const Columna& columna) {
    std::regex conditionRegex(R"((\w+)\s*([<>=!]+)\s*(\S+))", std::regex::icase);
    std::smatch match;

    bool resultado = true; // Valor inicial para AND
    for (const auto& whereCondition : condiciones) {
        if (std::regex_match(whereCondition, match, conditionRegex)) {
            std::string columnaWhere = match[1];
            std::string operador = match[2];
            std::string valorCondicion = match[3];

            // Limpiar el valor de la condición
            valorCondicion = std::regex_replace(valorCondicion, std::regex(R"(^\s+|\s+$)"), "");
            valorCondicion = std::regex_replace(valorCondicion, std::regex(R"(^'|'$)"), "");

            if (columna.nombre == columnaWhere) {
                bool cumple = false;

                // Comparar según el tipo de columna
                if (columna.tipo == "INT") {
                    try {
                        int valorInt = std::stoi(valor);
                        int valorCondicionInt = std::stoi(valorCondicion);
                        cumple = (operador == "=" && valorInt == valorCondicionInt) ||
                            (operador == "!=" && valorInt != valorCondicionInt) ||
                            (operador == "<" && valorInt < valorCondicionInt) ||
                            (operador == ">" && valorInt > valorCondicionInt) ||
                            (operador == "<=" && valorInt <= valorCondicionInt) ||
                            (operador == ">=" && valorInt >= valorCondicionInt);
                    }
                    catch (...) {
                        std::cerr << "Error al convertir valores a INT." << std::endl;
                        return false;
                    }
                }
                else if (columna.tipo == "FLOAT") {
                    try {
                        float valorFloat = std::stof(valor);
                        float valorCondicionFloat = std::stof(valorCondicion);
                        cumple = (operador == "=" && valorFloat == valorCondicionFloat) ||
                            (operador == "!=" && valorFloat != valorCondicionFloat) ||
                            (operador == "<" && valorFloat < valorCondicionFloat) ||
                            (operador == ">" && valorFloat > valorCondicionFloat) ||
                            (operador == "<=" && valorFloat <= valorCondicionFloat) ||
                            (operador == ">=" && valorFloat >= valorCondicionFloat);
                    }
                    catch (...) {
                        std::cerr << "Error al convertir valores a FLOAT." << std::endl;
                        return false;
                    }
                }
                else if (columna.tipo == "STR") {
                    std::string valorLimpio = std::regex_replace(valor, std::regex(R"(^\s+|\s+$)"), "");
                    std::string valorCondicionLimpio = std::regex_replace(valorCondicion, std::regex(R"(^\s+|\s+$)"), "");
                    cumple = (operador == "=" && valorLimpio == valorCondicionLimpio) ||
                        (operador == "!=" && valorLimpio != valorCondicionLimpio);
                }

                // Aplicar resultado lógico
                resultado = (resultado && cumple); // Para AND
            }
        }
    }

    return resultado; // Si se necesita un AND global, solo retorna el resultado
}


void Megatron::procesarConsulta(const std::string& query) {
    std::regex sqlRegex(R"(^\s*SELECT\s+([^FROM]+)\s+FROM\s+(\w+)(?:\s+WHERE\s+(.+))?\s*$)");
    std::smatch match;

    if (!std::regex_match(query, match, sqlRegex)) {
        std::cout << "Formato de SELECT inválido." << std::endl;
        return;
    }

    std::string columnas = match[1];
    std::string tableName = match[2];
    std::string whereCondition = match.size() > 3 ? std::string(match[3]) : "";

    std::vector<Columna> columnasDisponibles;
    if (!leerSchema(tableName, columnasDisponibles)) {
        return;
    }

    std::vector<std::string> listaColumnas;
    std::stringstream ss(columnas);
    std::string columna;

    while (std::getline(ss, columna, ',')) {
        size_t start = columna.find_first_not_of(" \t");
        size_t end = columna.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos) {
            listaColumnas.push_back(columna.substr(start, end - start + 1));
        }
    }

    std::ifstream file("C:/Users/USUARIO/Source/Repos/Megatronn3000/db/" + tableName + ".txt");
    if (!file.is_open()) {
        std::cout << "No se pudo abrir el archivo: db/" << tableName << ".txt" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream lineStream(line);
        std::string field;
        std::vector<std::string> filaValores;

        while (std::getline(lineStream, field, '#')) {
            field.erase(field.find_last_not_of(" \t") + 1);
            field.erase(0, field.find_first_not_of(" \t"));
            filaValores.push_back(field);
        }

        bool imprimeFila = false;

        for (const auto& col : listaColumnas) {
            if (col == "*") {
                imprimeFila = true;
                break;
            }
            else {
                for (const auto& c : columnasDisponibles) {
                    if (col == c.nombre) {
                        imprimeFila = true;
                        break;
                    }
                }
            }
        }

        if (imprimeFila) {
            // Ahora aplicamos las condiciones WHERE
            std::vector<std::string> condiciones;
            std::regex orRegex(R"(\s+OR\s+)");
            std::regex andRegex(R"(\s+AND\s+)");
            std::string token;

            // Separar condiciones por OR
            std::sregex_token_iterator iter(whereCondition.begin(), whereCondition.end(), orRegex, -1);
            std::sregex_token_iterator end;
            while (iter != end) {
                std::string orCondition = *iter++;
                // Separar condiciones por AND dentro de cada condición OR
                std::sregex_token_iterator iterAnd(orCondition.begin(), orCondition.end(), andRegex, -1);
                while (iterAnd != end) {
                    condiciones.push_back(*iterAnd++);
                }
            }

            // Comprobar si cumple con las condiciones
            bool cumpleTodas = true; // Para condiciones AND global
            for (size_t j = 0; j < columnasDisponibles.size(); ++j) {
                std::string valor = filaValores[j]; // Asegúrate de que valor se extrae correctamente de filaValores
                if (!cumpleCondicion(condiciones, valor, columnasDisponibles[j])) {
                    cumpleTodas = false;
                    break; // Salir si no cumple
                }
            }

            if (cumpleTodas) {
                std::string resultado;
                for (const auto& col : listaColumnas) {
                    if (col == "*") {
                        for (const auto& val : filaValores) {
                            resultado += val + " ";
                        }
                        break;
                    }
                    else {
                        for (size_t j = 0; j < columnasDisponibles.size(); ++j) {
                            if (col == columnasDisponibles[j].nombre) {
                                resultado += filaValores[j] + " ";
                                break;
                            }
                        }
                    }
                }
                std::cout << resultado << std::endl;
            }
        }
    }

    file.close();
}