#include "EngineeringConsoleUI.h"
#include <iostream>

EngineeringConsoleUI::EngineeringConsoleUI(const QString& host, int port, QObject* parent)
    : QObject(parent)
    , m_host(host)
    , m_port(port)
{
}

EngineeringConsoleUI::~EngineeringConsoleUI() {}

void EngineeringConsoleUI::run() {
    std::cout << "Engineering Console UI started" << std::endl;
    std::cout << "Server: " << m_host.toStdString() << ":" << m_port << std::endl;
}
