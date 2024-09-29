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

void validarData() {}




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

        if (tipo == "int") {
            for (char c : valor) {
                if (!isdigit(c)) {
                    std::cerr << "Error: el valor '" << valor << "' no es un INT valido para la columna '" << columnasDisponibles[i].nombre << "'." << std::endl;
                    return false;
                }
            }
        }

        else if (tipo == "float") {
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

        else if (tipo == "char") {
            if (valor.length() != 1) {
                std::cerr << "Error: el valor '" << valor << "' no es un CHAR valido para la columna '" << columnasDisponibles[i].nombre << "'." << std::endl;
                return false;
            }
        }

        else if (tipo == "str") {
            if (valor.front() != '\'' || valor.back() != '\'') {
                std::cerr << "Error: el valor '" << valor << "' debe estar entre comillas simples para la columna '" << columnasDisponibles[i].nombre << "'." << std::endl;
                return false;
            }
        }

        else if (tipo == "bool") {
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

            if (columnasDisponibles[i].tipo == "str") { // Para poder borrar las comillas ''

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
                columnType = "str";
            }

            schemaFile << "#" << columnName << "#" << columnType;
        }

        schemaFile << std::endl;
        schemaFile.close();
        std::cout << "Esquema creado correctamente en schema.txt" << std::endl;
    }
    else {
        std::cout << "Se esperaba 'CREATE TABLE'" << std::endl;
    }
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

    bool resultado = true;
    for (const auto& whereCondition : condiciones) {
        if (std::regex_match(whereCondition, match, conditionRegex)) {
            std::string columnaWhere = match[1];
            std::string operador = match[2];
            std::string valorCondicion = match[3];

            valorCondicion = std::regex_replace(valorCondicion, std::regex(R"(^\s+|\s+$)"), "");
            valorCondicion = std::regex_replace(valorCondicion, std::regex(R"(^'|'$)"), "");

            if (columna.nombre == columnaWhere) {
                bool cumple = false;

                if (columna.tipo == "int") {
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
                else if (columna.tipo == "float") {
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
                else if (columna.tipo == "str") {
                    std::string valorLimpio = std::regex_replace(valor, std::regex(R"(^\s+|\s+$)"), "");
                    std::string valorCondicionLimpio = std::regex_replace(valorCondicion, std::regex(R"(^\s+|\s+$)"), "");
                    cumple = (operador == "=" && valorLimpio == valorCondicionLimpio) ||
                        (operador == "!=" && valorLimpio != valorCondicionLimpio);
                }

                resultado = (resultado && cumple);
            }
        }
    }

    return resultado;
}

bool Megatron::cumpleCondicion(const std::string& whereCondition, const std::vector<std::string>& fila1, const std::vector<Columna>& columnasTabla1,
    const std::vector<std::string>& fila2, const std::vector<Columna>& columnasTabla2, const std::string& nombreTabla1, const std::string& nombreTabla2) {

    std::regex conditionRegex(R"((\w+)(?:\.(\w+))?\s*([<>=!]+)\s*(\S+))", std::regex::icase);
    std::smatch match;

    std::vector<std::string> condiciones;
    size_t pos = 0;
    std::string delimitador = "AND";
    std::string where = whereCondition;
    while ((pos = where.find(delimitador)) != std::string::npos) {
        condiciones.push_back(where.substr(0, pos));
        where.erase(0, pos + delimitador.length());
    }
    condiciones.push_back(where);

    bool resultado = true;

    for (const auto& condicion : condiciones) {
        if (std::regex_match(condicion, match, conditionRegex)) {
            std::string nombreTablaWhere = match[1];
            std::string columnaWhere = match[2].matched ? match[2] : match[1];
            std::string operador = match[3];            
            std::string valorCondicion = match[4];      

            valorCondicion = std::regex_replace(valorCondicion, std::regex(R"(^\s+|\s+$)"), "");
            valorCondicion = std::regex_replace(valorCondicion, std::regex(R"(^'|'$)"), "");

            bool cumple = false;

            //buscar en columnas de tabla1 si el . coincide o si no 
            if (nombreTablaWhere.empty() || nombreTablaWhere == nombreTabla1) {
                for (size_t i = 0; i < columnasTabla1.size(); ++i) {
                    if (columnasTabla1[i].nombre == columnaWhere) {
                        cumple = evaluarCondicion(operador, fila1[i], valorCondicion, columnasTabla1[i].tipo);
                        break;
                    }
                }
            }

            //buscar en columnas de tabla2 si el . coincide o si no 
            if (!cumple && (nombreTablaWhere.empty() || nombreTablaWhere == nombreTabla2)) {
                for (size_t j = 0; j < columnasTabla2.size(); ++j) {
                    if (columnasTabla2[j].nombre == columnaWhere) {
                        cumple = evaluarCondicion(operador, fila2[j], valorCondicion, columnasTabla2[j].tipo);
                        break;
                    }
                }
            }

            //std::cout << "Condición: " << condicion << " -> Cumple: " << (cumple ? "Sí" : "No") << std::endl;

            resultado = (resultado && cumple); 
        }
        else {
            std::cerr << "Formato de condición no reconocido: " << condicion << std::endl;
        }
    }

    return resultado; // Retorna el valor final (resultado de todas las condiciones)
}

bool Megatron::evaluarCondicion(const std::string& operador, const std::string& valor, const std::string& valorCondicion, const std::string& tipo) {
    if (tipo == "int") {
        try {
            int valorInt = std::stoi(valor);
            int valorCondicionInt = std::stoi(valorCondicion);
            return (operador == "=" && valorInt == valorCondicionInt) ||
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
    else if (tipo == "float") {
        try {
            float valorFloat = std::stof(valor);
            float valorCondicionFloat = std::stof(valorCondicion);
            return (operador == "=" && valorFloat == valorCondicionFloat) ||
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
    }//STR
    else if (tipo == "str") {
        std::string valorLimpio = std::regex_replace(valor, std::regex(R"(^\s+|\s+$)"), "");
        std::string valorCondicionLimpio = std::regex_replace(valorCondicion, std::regex(R"(^\s+|\s+$)"), "");
        return (operador == "=" && valorLimpio == valorCondicionLimpio) ||
            (operador == "!=" && valorLimpio != valorCondicionLimpio);
    }
    return false; // Si no se puede comparar, retorna falso
}

void Megatron::select(const std::string& query) {
    std::regex sqlRegex(R"(^\s*SELECT\s+([^FROM]+)\s+FROM\s+(\w+)(?:\s+JOIN\s+(\w+)\s+ON\s+(\w+\.\w+)\s*=\s*(\w+\.\w+))?(?:\s+WHERE\s+([^|]*))?(?:\s*\|\s*(\w+))?\s*$)");
    std::smatch match;

    if (!std::regex_match(query, match, sqlRegex)) {
        std::cout << "Formato de SELECT inválido." << std::endl;
        return;
    }

    std::string columnas = match[1];
    std::string tableName = match[2];
    std::string joinTable, joinLeftColumn, joinRightColumn;
    std::string whereCondition;
    std::string name_new_table;

    if (match.size() > 4 && match[3].matched) {
        /*joinTable = match[3]; // Nombre de la tabla a unir
        joinLeftColumn = match[4]; // Nombre de la columna en la tabla principal
        joinRightColumn = match[5]; // Nombre de la columna en la tabla JOIN*/
        procesarConsultaJoin(query);
        return;
    }

    if (match.size() > 6) {
        whereCondition = match[6];
    }
    else {
        whereCondition = "";
    }

    if (match.size() > 7) {
        name_new_table = match[7];
    }
    else {
        name_new_table = "";
    }

    std::vector<Columna> columnasDisponibles;
    if (!leerSchema(tableName, columnasDisponibles)) {
        std::cerr << "No se encontró la tabla " << tableName << " o error al leer datos." << std::endl;
        return;
    }


    if (name_new_table != "") {

        if (leerSchema(name_new_table, columnasDisponibles)) {
            std::cout << "tabla ya existente";
            return;
        }

        else {
            std::filesystem::path Schema_Dir = std::filesystem::current_path().parent_path() / "db" / "schema.txt";
            std::ofstream schemaFile(Schema_Dir, std::ios::app);
            schemaFile << '\n' << name_new_table;
            int colum_size = columnasDisponibles.size();
            for (size_t i = 0; i < colum_size; ++i) {
                schemaFile << " # " << columnasDisponibles[i].nombre << " # " << columnasDisponibles[i].tipo;
            }
            schemaFile.close();
        }
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

    std::filesystem::path table_Dir = std::filesystem::current_path().parent_path() / "db" / (tableName + ".txt");
    std::ifstream file(table_Dir);

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

            std::vector<std::string> condiciones;
            std::regex orRegex(R"(\s+OR\s+)");
            std::regex andRegex(R"(\s+AND\s+)");
            std::string token;

            std::sregex_token_iterator iter(whereCondition.begin(), whereCondition.end(), orRegex, -1);
            std::sregex_token_iterator end;

            while (iter != end) {
                std::string orCondition = *iter++;
                std::sregex_token_iterator iterAnd(orCondition.begin(), orCondition.end(), andRegex, -1);
                while (iterAnd != end) {
                    condiciones.push_back(*iterAnd++);
                }
            }

            bool cumpleTodas = true;
            for (size_t j = 0; j < columnasDisponibles.size(); ++j) {
                std::string valor = filaValores[j];
                if (!cumpleCondicion(condiciones, valor, columnasDisponibles[j])) {
                    cumpleTodas = false;
                    break;
                }
            }

            if (cumpleTodas) {
                std::string resultado;
                std::string resultado_archivo;

                for (const auto& col : listaColumnas) {
                    if (col == "*") {
                        for (const auto& val : filaValores) {
                            resultado += val + " ";
                            resultado_archivo += val + " # ";
                        }
                        break;
                    }
                    else {
                        for (size_t j = 0; j < columnasDisponibles.size(); ++j) {
                            if (col == columnasDisponibles[j].nombre) {
                                resultado += filaValores[j] + " ";
                                resultado_archivo += filaValores[j] + " # ";
                                break;
                            }
                        }
                    }
                }
                std::cout << resultado << std::endl;

                if (!name_new_table.empty()) {

                    std::filesystem::path table_Dir = std::filesystem::current_path().parent_path() / "db" / (name_new_table + ".txt");
                    std::ofstream outFile(table_Dir, std::ios::app);
                    if (outFile.is_open()) {
                        outFile << resultado << std::endl;
                        outFile.close();
                    }
                    else {
                        std::cerr << "Error al abrir el archivo: " << name_new_table << ".txt" << std::endl;
                    }
                }
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

                    if (columna.tipo == "str") {

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


void Megatron::procesarConsultaJoin(const std::string& query) {
    //modificación del patrón regex para soportar el JOIN en la consulta
    std::regex sqlJoinRegex(R"(^\s*SELECT\s+([^FROM]+)\s+FROM\s+(\w+)\s+JOIN\s+(\w+)\s+ON\s+(\w+)\.(\w+)\s*=\s*(\w+)\.(\w+)(?:\s+WHERE\s+([^|]*))?\s*$)");
    std::smatch match;

    if (!std::regex_match(query, match, sqlJoinRegex)) {
        std::cout << "Formato de SELECT JOIN inválido." << std::endl;
        return;
    }

    std::string columnas = match[1];
    std::string tabla1 = match[2];
    std::string tabla2 = match[3];
    std::string aliasTabla1 = match[4];
    std::string columnaTabla1 = match[5];
    std::string aliasTabla2 = match[6];
    std::string columnaTabla2 = match[7];
    std::string whereCondition = match.size() > 8 ? std::string(match[8]) : "";

    //leer esquemas para ambas tablas para obtener las columnas disponibles
    std::vector<Columna> columnasTabla1, columnasTabla2;
    if (!leerSchema(tabla1, columnasTabla1)) {
        std::cerr << "No se encontró la tabla " << tabla1 << " en schema.txt" << std::endl;
        return;
    }
    if (!leerSchema(tabla2, columnasTabla2)) {
        std::cerr << "No se encontró la tabla " << tabla2 << " en schema.txt" << std::endl;
        return;
    }

    //encontrar los índices de las columnas para realizar el JOIN
    int indexColumnaTabla1 = -1, indexColumnaTabla2 = -1;
    for (size_t i = 0; i < columnasTabla1.size(); ++i) {
        if (columnasTabla1[i].nombre == columnaTabla1) {
            indexColumnaTabla1 = static_cast<int>(i);
            break;
        }
    }

    for (size_t j = 0; j < columnasTabla2.size(); ++j) {
        if (columnasTabla2[j].nombre == columnaTabla2) {
            indexColumnaTabla2 = static_cast<int>(j);
            break;
        }
    }

    if (indexColumnaTabla1 == -1 || indexColumnaTabla2 == -1) {
        std::cerr << "No se encontraron las columnas de unión en las tablas correspondientes." << std::endl;
        return;
    }

    //abrir los archivos de las tablas
    std::filesystem::path pathTabla1 = std::filesystem::current_path().parent_path() / "db" / (tabla1 + ".txt");
    std::filesystem::path pathTabla2 = std::filesystem::current_path().parent_path() / "db" / (tabla2 + ".txt");

    std::ifstream archivoTabla1(pathTabla1);
    std::ifstream archivoTabla2(pathTabla2);

    if (!archivoTabla1.is_open()) {
        std::cerr << "No se pudo abrir el archivo de la tabla: " << tabla1 << std::endl;
        return;
    }
    if (!archivoTabla2.is_open()) {
        std::cerr << "No se pudo abrir el archivo de la tabla: " << tabla2 << std::endl;
        return;
    }

    // Procesar el JOIN entre las tablas línea por línea
    std::string fila1, fila2;
    while (std::getline(archivoTabla1, fila1)) {
        std::stringstream ssFila1(fila1);
        std::vector<std::string> valoresFila1;
        std::string valor1;

        while (std::getline(ssFila1, valor1, '#')) {
            valor1.erase(valor1.find_last_not_of(" \t") + 1);
            valor1.erase(0, valor1.find_first_not_of(" \t"));
            valoresFila1.push_back(valor1);
        }

        std::string valorUnionTabla1 = valoresFila1[indexColumnaTabla1];

        archivoTabla2.clear();
        archivoTabla2.seekg(0, std::ios::beg);

        while (std::getline(archivoTabla2, fila2)) {
            std::stringstream ssFila2(fila2);
            std::vector<std::string> valoresFila2;
            std::string valor2;

            while (std::getline(ssFila2, valor2, '#')) {
                valor2.erase(valor2.find_last_not_of(" \t") + 1);
                valor2.erase(0, valor2.find_first_not_of(" \t"));
                valoresFila2.push_back(valor2);
            }

            std::string valorUnionTabla2 = valoresFila2[indexColumnaTabla2];

            if (valorUnionTabla1 == valorUnionTabla2) {
                std::string filaUnida;
                for (const auto& val : valoresFila1) {
                    filaUnida += val + " ";
                }
                for (const auto& val : valoresFila2) {
                    filaUnida += val + " ";
                }

                //evaluar WHERE 
                if (whereCondition.empty() || cumpleCondicion(whereCondition, valoresFila1, columnasTabla1, valoresFila2, columnasTabla2, tabla1, tabla2)) {
                    //std::cout << "where: " << whereCondition << std::endl;
                    std::cout << filaUnida << std::endl;
                }
            }
        }
    }

    archivoTabla1.close();
    archivoTabla2.close();
}