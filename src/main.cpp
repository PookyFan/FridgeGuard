#include "MainWindow.hpp"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    FG::Ui::MainWindow window;
    window.show();

    return app.exec();
}