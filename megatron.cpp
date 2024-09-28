#include "megatron.h"
#include <iostream>
#include <map>

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
    else if (command == "INSERT") {
        insertTable(query);
    }
    else if (command == "SELECT") {
        select(query);
    }
    else if (command == "UPDATE") {
        updateTable(query);
    }
    else {
        std::cout << "Comando no reconocido: " << command << std::endl;
    }
}

bool Megatron::insertTable(const std::string& query) {

    std::regex sqlRegex(R"(INSERT INTO\s+(\w+)\s*(\([^\)]+\))?\s+VALUES\s*\((.+)\))", std::regex::icase);
    std::smatch match;

    if (!std::regex_match(query, match, sqlRegex)) {
        std::cerr << "Formato de INSERT inválido." << std::endl;
        return false;
    }

    std::string tableName = match[1];
    std::string columnsPart = match[2];
    std::string valuesPart = match[3];

    // Leer el esquema para obtener columnas y tipos de datos
    std::vector<Columna> columnasDisponibles;
    if (!leerSchema(tableName, columnasDisponibles)) {
        return false;
    }

    // Obtener columnas a insertar
    std::vector<std::string> columnasAInsertar;
    if (!columnsPart.empty()) {
        columnsPart.erase(0, 1); // Eliminar paréntesis
        columnsPart.erase(columnsPart.size() - 1, 1); // Eliminar paréntesis final
        std::stringstream ss(columnsPart);
        std::string columna;
        while (std::getline(ss, columna, ',')) {
            columna.erase(columna.find_last_not_of(" \t") + 1);
            columna.erase(0, columna.find_first_not_of(" \t"));
            columnasAInsertar.push_back(columna);
        }
    }
    else {
        for (const auto& col : columnasDisponibles) {
            columnasAInsertar.push_back(col.nombre);
        }
    }

    // Parsear los valores a insertar
    std::vector<std::string> valoresAInsertar;
    std::stringstream ssValues(valuesPart);
    std::string valor;
    while (std::getline(ssValues, valor, ',')) {
        valor.erase(valor.find_last_not_of(" \t") + 1);
        valor.erase(0, valor.find_first_not_of(" \t"));
        valoresAInsertar.push_back(valor);
    }

    // Verificar que el número de columnas y valores coinciden
    if (columnasAInsertar.size() != valoresAInsertar.size()) {
        std::cerr << "El número de columnas no coincide con el número de valores." << std::endl;
        return false;
    }

    // Verificar tipos de datos
    for (size_t i = 0; i < columnasAInsertar.size(); ++i) {
        const std::string& valor = valoresAInsertar[i];
        const Columna& columna = columnasDisponibles[i];

        if (columna.tipo == "INT") {
            try {
                std::stoi(valor);  // Intentar convertir a entero
            }
            catch (...) {
                std::cerr << "Error: el valor '" << valor << "' no es un INT válido para la columna '" << columna.nombre << "'." << std::endl;
                return false;
            }
        }
        else if (columna.tipo == "STR") {
            if (valor.front() != '\'' || valor.back() != '\'') {
                std::cerr << "Error: el valor '" << valor << "' debe estar entre comillas simples para la columna '" << columna.nombre << "'." << std::endl;
                return false;
            }
        }
    }

    // Verificar restricciones adicionales (clave primaria única)
    // Aquí podrías agregar la validación de unicidad, si es necesario.

    // Si todo es correcto, insertar los valores en el archivo
    std::ofstream file("db/" + tableName + ".txt", std::ios::app);
    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo: db/" << tableName << ".txt" << std::endl;
        return false;
    }

    // Construir la fila a insertar
    std::string nuevaFila;
    for (size_t i = 0; i < valoresAInsertar.size(); ++i) {
        nuevaFila += valoresAInsertar[i];
        if (i < valoresAInsertar.size() - 1) {
            nuevaFila += "#";  // Separador entre valores
        }
    }

    // Insertar la fila
    file << nuevaFila << std::endl;
    file.close();

    std::cout << "Registro insertado correctamente." << std::endl;
    return true;
}

void Megatron::updateTable(const std::string& query) {
    procesarUpdate(query);
}

bool Megatron::procesarUpdate(const std::string& query) {

    std::regex sqlRegex(R"(UPDATE\s+(\w+)\s+SET\s+([^WHERE]+)\s*(?:WHERE\s+(.+))?)", std::regex::icase);
    std::smatch match;

    if (!std::regex_match(query, match, sqlRegex)) {
        std::cerr << "Formato de UPDATE inválido." << std::endl;
        return false;
    }

    //std::string query = match[0];
    std::string tableName = match[1]; // tabla
    std::string setClause = match[2]; // columnas y valores a actualizar
    std::string whereClause = match.size() > 3 ? std::string(match[3]) : "";  // clausula del where

    // Leer esquema para obtener las columnas disponibles
    std::vector<Columna> columnasDisponibles;
    if (!leerSchema(tableName, columnasDisponibles)) {
        return false;
    }

    // Parsear la cláusula SET
    std::map<std::string, std::string> columnasAActualizar;
    if (!parseSetClause(setClause, columnasAActualizar)) {
        return false;
    }

    // Leer las filas actuales de la tabla
    std::ifstream file("db/" + tableName + ".txt");
    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo: db/" << tableName << ".txt" << std::endl;
        return false;
    }

    std::vector<std::string> nuevasFilas;
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream lineStream(line);
        std::vector<std::string> filaValores;
        std::string valor;

        // Separar los valores de cada fila
        while (std::getline(lineStream, valor, '#')) {
            filaValores.push_back(valor);
        }

        // Verificar si la fila cumple con la condición WHERE
        bool cumpleCondicion = true;
        if (!whereClause.empty()) {
            cumpleCondicion = verificarCondicion(filaValores, whereClause, columnasDisponibles);
        }

        // Si cumple con la condición, actualizar las columnas especificadas
        if (cumpleCondicion) {
            for (size_t i = 0; i < columnasDisponibles.size(); ++i) {
                const auto& columna = columnasDisponibles[i];
                if (columnasAActualizar.find(columna.nombre) != columnasAActualizar.end()) {
                    filaValores[i] = columnasAActualizar[columna.nombre];
                }
            }
        }

        // Construir la fila actualizada y agregarla al vector de nuevas filas
        std::string nuevaFila;
        for (size_t i = 0; i < filaValores.size(); ++i) {
            nuevaFila += filaValores[i];
            if (i < filaValores.size() - 1) {
                nuevaFila += "#";
            }
        }
        nuevasFilas.push_back(nuevaFila);
    }
    file.close();

    // Sobreescribir el archivo de la tabla con las filas actualizadas
    std::ofstream outFile("db/" + tableName + ".txt");
    if (!outFile.is_open()) {
        std::cerr << "No se pudo abrir el archivo para escribir: db/" << tableName << ".txt" << std::endl;
        return false;
    }

    for (const auto& nuevaFila : nuevasFilas) {
        outFile << nuevaFila << std::endl;
    }

    outFile.close();
    std::cout << "Actualización realizada correctamente." << std::endl;
    return true;
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

    std::ifstream file("db/" + tableName + ".txt");
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