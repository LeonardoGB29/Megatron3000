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

bool leerSchema(const std::string& tableName, std::vector<Columna>& columnasDisponibles) { // devuelve las columnas de una tabla x
    std::ifstream schemaFile("schema.txt");

    if (!schemaFile.is_open()) {
        std::cerr << "Error al abrir schema" << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(schemaFile, line)) {
        std::stringstream lineStream(line);
        std::string nombreTabla;
        std::getline(lineStream, nombreTabla, '#');

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

bool Megatron::validateInstance(const std::vector<Columna>& columnasDisponibles, const std::vector<std::string>& valoresAInsertar) {

    if (columnasDisponibles.size() != valoresAInsertar.size()) {
        std::cerr << "El número de valores no coincide con el número de columnas." << std::endl;
        return false;
    }

    for (int i = 0; i < columnasDisponibles.size(); ++i) {
        const std::string& tipo = columnasDisponibles[i].tipo;
        const std::string& valor = valoresAInsertar[i];

        if (valor.empty()) {
            std::cerr << "Error: el valor para la columna '" << columnasDisponibles[i].nombre << "' no puede estar vacio." << std::endl;
            return false;
        }

        if (tipo == "INT") {
            for (char c : valor) {
                if (!isdigit(c)) {
                    std::cerr << "Error: el valor '" << valor << "' no es un INT valido para la columna '" << columnasDisponibles[i].nombre << "'." << std::endl;
                    return false;
                }
            }
        }

        else if (tipo == "FLOAT") {
            bool puntoEncontrado = false;
            for (char c : valor) {
                if (c == '.') {
                    if (puntoEncontrado) {
                        std::cerr << "Error: el valor '" << valor << "' no es un FLOAT valido para la columna '" << columnasDisponibles[i].nombre << "'." << std::endl;
                        return false;
                    }
                    puntoEncontrado = true;
                }
                else if (!isdigit(c)) {
                    std::cerr << "Error: el valor '" << valor << "' no es un FLOAT valido para la columna '" << columnasDisponibles[i].nombre << "'." << std::endl;
                    return false;
                }
            
            }
        }

        else if (tipo == "CHAR") {
            if (valor.length() != 1) {
                std::cerr << "Error: el valor '" << valor << "' no es un CHAR valido para la columna '" << columnasDisponibles[i].nombre << "'." << std::endl;
                return false;
            }
        }

        else if (tipo == "STR") {
            if (valor.front() != '\'' || valor.back() != '\'') {
                std::cerr << "Error: el valor '" << valor << "' debe estar entre comillas simples para la columna '" << columnasDisponibles[i].nombre << "'." << std::endl;
                return false;
            }
        }

        else if (tipo == "BOOL") {
            if (valor != "0" && valor != "1" && valor != "true" && valor != "false" && valor != "TRUE" && valor != "FALSE") {
                std::cerr << "Error: el valor '" << valor << "' no es un BOOL valido para la columna '" << columnasDisponibles[i].nombre << "'." << std::endl;
                return false;
            }
        }
    }

    return true;

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

    std::regex sqlRegex(R"(INSERT INTO\s+(\w+)\s*(\([^\)]+\))?\s+VALUES\s*\(([^;]+)\)(?:\s*,\s*\(([^;]+)\))*\s*;?\s*$)", std::regex::icase);
    std::smatch match;

    if (!std::regex_match(query, match, sqlRegex)) {
        std::cerr << "Formato de INSERT inválido." << std::endl;
        return false;
    }

    std::string tableName = match[1];
    std::string columnsPart = match[2];
    std::string valuesPart = match[3];

    std::vector<Columna> columnasDisponibles;
    if (!leerSchema(tableName, columnasDisponibles))
        return false;

    std::vector<std::string> columnasAInsertar;

    if (!columnsPart.empty()) {
        columnsPart.erase(0, 1);
        columnsPart.erase(columnsPart.size() - 1, 1);

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

    std::regex valuesRegex(R"(\((.*?)\))");
    auto valuesBegin = std::sregex_iterator(query.begin(), query.end(), valuesRegex);
    auto valuesEnd = std::sregex_iterator();

    std::ofstream file("db/" + tableName + ".txt", std::ios::app);
    if (!file.is_open()) {
        std::cerr << "No se pudo abrir : db/" << tableName << ".txt" << std::endl;
        return false;
    }

    for (std::sregex_iterator i = valuesBegin; i != valuesEnd; ++i) {
        std::smatch matchValues = *i;
        std::string currentValuesPart = matchValues.str(1);

        std::vector<std::string> valoresAInsertar;
        std::stringstream ssValues(currentValuesPart);
        std::string valor;

        while (std::getline(ssValues, valor, ',')) {
            valor.erase(valor.find_last_not_of(" \t") + 1);
            valor.erase(0, valor.find_first_not_of(" \t"));
            valoresAInsertar.push_back(valor);
        }

        if (columnasAInsertar.size() != valoresAInsertar.size()) {
            std::cerr << "El numero de columnas no coincide con el numero de valores" << std::endl;
            return false;
        }

        if (!validateInstance(columnasDisponibles, valoresAInsertar))
            return false;


        std::string nuevaFila;

        for (int i = 0; i < valoresAInsertar.size(); ++i) {
            std::string valor = valoresAInsertar[i];

            if (columnasDisponibles[i].tipo == "STR") { // Para poder borrar las comillas ''

                if (valor.length() >= 2 && valor.front() == '\'' && valor.back() == '\'')
                    valor = valor.substr(1, valor.length() - 2);

            }

            nuevaFila += valor;

            if (i < valoresAInsertar.size() - 1)
                nuevaFila += "#";

        }

        file << std::endl << nuevaFila;
    }

    file.close();
    std::cout << "Registros insertados correctamente" << std::endl;
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


void Megatron::updateTable(const std::string& query) {

    std::regex sqlRegex(R"(UPDATE\s+(\w+)\s+SET\s+((?:\w+\s*=\s*[^\s,]+)(?:\s*,\s*\w+\s*=\s*[^\s,]+)*)\s*(?:WHERE\s+(.+))?)", std::regex::icase);
    std::smatch match;

    if (!std::regex_match(query, match, sqlRegex)) {
        std::cerr << "Formato de UPDATE inválido." << std::endl;
        return;
    }

    std::string tableName = match[1].str();
    std::string setClause = match[2].str();
    std::string whereClause = match.size() > 3 ? match[3].str() : "";

    std::vector<Columna> columnasDisponibles;
    if (!leerSchema(tableName, columnasDisponibles))
        return;

    std::stringstream ssSet(setClause);
    std::map<std::string, std::string> columnasAActualizar;
    std::string setItem;

    while (std::getline(ssSet, setItem, ',')) {

        std::string columna, valor;
        int igualPos = setItem.find('=');

        if (igualPos != std::string::npos) {

            columna = setItem.substr(0, igualPos);
            valor = setItem.substr(igualPos + 1);

            columna.erase(columna.find_last_not_of(" \t") + 1);
            columna.erase(0, columna.find_first_not_of(" \t"));

            valor.erase(valor.find_last_not_of(" \t") + 1);
            valor.erase(0, valor.find_first_not_of(" \t"));

            columnasAActualizar[columna] = valor;
        }
    }
    
    std::ifstream file("db/" + tableName + ".txt");

    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo: db/" << tableName << ".txt" << std::endl;
        return;
    }

    std::vector<std::string> nuevasFilas;
    std::string line;

    while (std::getline(file, line)) {

        std::stringstream lineStream(line);
        std::vector<std::string> filaValores;
        std::string valor;

        while (std::getline(lineStream, valor, '#')) {

            valor.erase(valor.find_last_not_of(" \t") + 1);
            valor.erase(0, valor.find_first_not_of(" \t"));

            filaValores.push_back(valor);
        }

        bool cumpleTodas = true;

        if (!whereClause.empty()) {

            std::vector<std::string> condiciones;

            std::regex orRegex(R"(\s+OR\s+)");
            std::regex andRegex(R"(\s+AND\s+)");

            std::sregex_token_iterator iter(whereClause.begin(), whereClause.end(), orRegex, -1);
            std::sregex_token_iterator end;

            for (; iter != end; ++iter) {
                std::string orCondition = *iter;
                std::sregex_token_iterator iterAnd(orCondition.begin(), orCondition.end(), andRegex, -1);
                while (iterAnd != end) {
                    condiciones.push_back(*iterAnd++);
                }
            }

            for (size_t j = 0; j < columnasDisponibles.size(); ++j) {
                std::string valorFila = filaValores[j];
                if (!cumpleCondicion(condiciones, valorFila, columnasDisponibles[j])) {
                    cumpleTodas = false;
                    break;
                }
            }
        }

        if (cumpleTodas) {

            for (size_t i = 0; i < columnasDisponibles.size(); ++i) {

                const auto& columna = columnasDisponibles[i];

                if (columnasAActualizar.find(columna.nombre) != columnasAActualizar.end()) {



                    std::string nuevoValor = columnasAActualizar[columna.nombre];

                    if (columna.tipo == "STR") {
                        if (nuevoValor.length() >= 2 && nuevoValor.front() == '\'' && nuevoValor.back() == '\'') {
                            nuevoValor = nuevoValor.substr(1, nuevoValor.length() - 2);
                        }
                    }

                    filaValores[i] = nuevoValor;
                }
            }
        }

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

    std::ofstream outFile("db/" + tableName + ".txt");

    if (!outFile.is_open()) {
        std::cerr << "No se pudo abrir el archivo para escribir: db/" << tableName << ".txt" << std::endl;
        return;
    }

    for (const auto& nuevaFila : nuevasFilas) {
        outFile << nuevaFila << std::endl;
    }

    outFile.close();
    std::cout << "Actualización realizada correctamente." << std::endl;
}
