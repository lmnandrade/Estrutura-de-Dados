#include "cadastro.h"
#include <iostream>
#include <algorithm> // Para std::find

void manage_email_cadastro(std::vector<std::string>& emails) {
    std::string email_input;
    std::cout << "\n--- Cadastro de Email para Alertas ---" << std::endl;
    std::cout << "Emails atualmente cadastrados: ";
    if (emails.empty()) {
        std::cout << "Nenhum." << std::endl;
    } else {
        for (size_t i = 0; i < emails.size(); ++i) {
            std::cout << emails[i] << (i == emails.size() - 1 ? "" : ", ");
        }
        std::cout << std::endl;
    }
    while (true) {
        std::cout << "Digite um email para adicionar (ou 'fim' para concluir): ";
        std::getline(std::cin, email_input);

        if (email_input.empty() || email_input == "fim") {
            break;
        }

        if (email_input.find('@') != std::string::npos && email_input.find('.') != std::string::npos) {
            if (std::find(emails.begin(), emails.end(), email_input) == emails.end()) {
                emails.push_back(email_input);
                std::cout << "Email '" << email_input << "' adicionado." << std::endl;
            } else {
                std::cout << "Email '" << email_input << "' já cadastrado." << std::endl;
            }
        } else {
            std::cout << "Formato de email inválido. Tente novamente." << std::endl;
        }
    }
    std::cout << "Cadastro de emails concluído." << std::endl;
}