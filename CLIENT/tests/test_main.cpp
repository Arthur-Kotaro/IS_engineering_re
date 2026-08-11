#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTimer>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    
    // Запускаем тесты
    int result = RUN_ALL_TESTS();
    
    // Завершаем приложение
    QTimer::singleShot(0, &app, [&]() {
        app.quit();
    });
    
    app.exec();
    return result;
}
