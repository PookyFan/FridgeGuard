#include "MainWindow.hpp"

namespace FG::Ui
{
MainWindow::MainWindow() : QMainWindow(), mainWindow(std::make_unique<::Ui::MainWindow>())
{
    mainWindow->setupUi(this);
}

void MainWindow::on_someTestButton_clicked()
{
}
}
