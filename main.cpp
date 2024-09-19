#include "dataBase.h"

int main() {

    dataBase db;

    std::string query;
	std::cout << "% MEGATRON3000" << std::endl << "    Welcome to MEGATRON 3000!" << std::endl;

    while (true) {
        std::cout << "& ";
        std::getline(std::cin, query);
        if (query.empty()) continue;
        db.parseAndExecuteQuery(query);
    }


	return 0;
}

// lectura de datos
    // esquemas e instancias

