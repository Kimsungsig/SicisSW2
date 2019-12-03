#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void serial_connect();
    void serial_rescan();
    void widget_changed();
    void send_test();
    void serial_received();
    void on_PUSH_clicked();
    void on_textEdit_destroyed();
    void readDateSet(QString data, int j);


private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
