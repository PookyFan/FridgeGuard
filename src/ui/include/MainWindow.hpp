#pragma once

#include <memory>
#include "../generated/UiMainWindow.hpp"

namespace FG::Ui
{
class MainWindow : public QMainWindow
{
Q_OBJECT

public:
    MainWindow();

private slots:
    void on_someTestButton_clicked();

private:
    std::unique_ptr<::Ui::MainWindow> mainWindow;
};
}