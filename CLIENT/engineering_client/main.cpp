#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTimer>
#include <QSslSocket>
#include <iostream>
#include <memory>

#include "userserviceclient/ApiClient.h"
#include "userserviceclient/AuthService.h"

using namespace UsersService;

class EngineeringConsoleUI : public QObject
{
    Q_OBJECT

public:
    EngineeringConsoleUI(const QString& host, int port, QObject* parent = nullptr)
        : QObject(parent)
        , m_host(host)
        , m_port(port)
    {
        m_apiClient = std::make_shared<ApiClient>();
        m_apiClient->setServerUrl(m_host, m_port);
        m_authService = std::make_shared<AuthService>(m_apiClient);
        
        connect(m_authService.get(), &AuthService::loginCompleted,
                this, &EngineeringConsoleUI::onLoginCompleted);
        connect(m_authService.get(), &AuthService::passwordExpiryInfo,
                this, &EngineeringConsoleUI::onPasswordExpiryInfo);
    }

public slots:
    void run() {
        printBanner();
        showMainMenu();
    }

private slots:
    void onLoginCompleted(const AuthResult& result) {
        if (result.success) {
            std::cout << "\n✅ Login successful!\n";
            std::cout << "  User: " << result.session.profile.userName.toStdString() << "\n";
            std::cout << "  Email: " << result.session.profile.email.toStdString() << "\n";
           // std::cout << "  Position: " << result.session.profile.position.toStdString() << "\n";
            m_currentSession = result.session;
        } else {
            std::cout << "\n❌ Login failed!\n";
            std::cout << "  Error: " << result.errorMessage.toStdString() << "\n";
        }
        showMainMenu();
    }
    
    void onPasswordExpiryInfo(int daysRemaining, bool isExpired, const QString& expiresAt) {
        std::cout << "\n📅 Password expiry info:\n";
        std::cout << "  Expires at: " << expiresAt.toStdString() << "\n";
        std::cout << "  Days remaining: " << daysRemaining << "\n";
        std::cout << "  Status: " << (isExpired ? "EXPIRED" : "Active") << "\n";
        showMainMenu();
    }

private:
    void printBanner() {
        std::cout << "\n╔════════════════════════════════════════════════╗\n";
        std::cout << "║     Engineering Client v1.0                     ║\n";
        std::cout << "║     User Microservice Tester                    ║\n";
        std::cout << "╠════════════════════════════════════════════════╣\n";
        std::cout << "║  Server: " << m_host.toStdString() << ":" << m_port << "\n";
        std::cout << "╚════════════════════════════════════════════════╝\n";
    }
    
    void showMainMenu() {
        while (true) {
            std::cout << "\n════════════════════════════════════════════\n";
            if (m_currentSession.isValid) {
                std::cout << "  ✅ Logged in as: " << m_currentSession.profile.email.toStdString() << "\n";
                std::cout << "────────────────────────────────────────\n";
                std::cout << "  1. Check password expiry\n";
                std::cout << "  2. Change password\n";
                std::cout << "  3. Refresh token\n";
                std::cout << "  4. Logout\n";
                std::cout << "────────────────────────────────────────\n";
                std::cout << "  5. Health check\n";
                std::cout << "  0. Exit\n";
            } else {
                std::cout << "  🔓 Not logged in\n";
                std::cout << "────────────────────────────────────────\n";
                std::cout << "  1. Login\n";
                std::cout << "  2. Health check\n";
                std::cout << "  0. Exit\n";
            }
            std::cout << "────────────────────────────────────────\n";
            std::cout << "  Choice: ";
            
            int choice;
            std::cin >> choice;
            
            if (choice == 0) {
                std::cout << "\n👋 Goodbye!\n";
                QCoreApplication::quit();
                return;
            }
            
            if (m_currentSession.isValid) {
                switch (choice) {
                    case 1: checkPasswordExpiry(); break;
                    case 2: changePassword(); break;
                    case 3: refreshToken(); break;
                    case 4: logout(); break;
                    case 5: healthCheck(); break;
                    default: std::cout << "  Invalid choice!\n";
                }
            } else {
                switch (choice) {
                    case 1: login(); break;
                    case 2: healthCheck(); break;
                    default: std::cout << "  Invalid choice!\n";
                }
            }
            return;
        }
    }
    
    void login() {
        std::string email, password;
        std::cout << "  Email: ";
        std::cin >> email;
        std::cout << "  Password: ";
        std::cin >> password;
        m_authService->login(QString::fromStdString(email), 
                             QString::fromStdString(password));
    }
    
    void checkPasswordExpiry() {
        std::cout << "\n[Checking password expiry...]\n";
        m_authService->checkPasswordExpiry();
    }
    
    void changePassword() {
        std::string currentPwd, newPwd;
        std::cout << "  Current password: ";
        std::cin >> currentPwd;
        std::cout << "  New password: ";
        std::cin >> newPwd;
        m_authService->changePassword(QString::fromStdString(currentPwd),
                                      QString::fromStdString(newPwd),
                                      QString::fromStdString(newPwd));
    }
    
    void refreshToken() {
        if (!m_currentSession.refreshToken.isEmpty()) {
            m_authService->refreshToken(m_currentSession.refreshToken);
        }
    }
    
    void logout() {
        m_authService->logout();
        m_currentSession = UserSession();
        std::cout << "  ✅ Logged out\n";
        showMainMenu();
    }
    
    void healthCheck() {
        std::cout << "\n[Health check...]\n";
        m_apiClient->get("/health", [this](const ApiResponse& response) {
            if (response.success) {
                std::cout << "  ✅ Server is healthy\n";
            } else {
                std::cout << "  ❌ Health check failed: " 
                          << response.errorString.toStdString() << "\n";
            }
            showMainMenu();
        });
    }

private:
    QString m_host;
    int m_port;
    std::shared_ptr<ApiClient> m_apiClient;
    std::shared_ptr<AuthService> m_authService;
    UserSession m_currentSession;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("EngineeringClient");
    
    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption hostOption(QStringList() << "s" << "server",
        "Server hostname", "server", "localhost");
    parser.addOption(hostOption);
    QCommandLineOption portOption(QStringList() << "p" << "port",
        "Server port", "port", "8000");
    parser.addOption(portOption);
    parser.process(app);
    
    QString host = parser.value(hostOption);
    int port = parser.value(portOption).toInt();
    
    EngineeringConsoleUI console(host, port);
    QTimer::singleShot(100, &console, &EngineeringConsoleUI::run);
    
    return app.exec();
}

#include "main.moc"
