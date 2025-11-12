#include <QApplication>
#include "f1_ui/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QApplication::setApplicationName("Vision por Computador");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("Universidad");
    QApplication::setOrganizationDomain("universidad.edu");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
