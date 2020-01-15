#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <define_protocol.h>

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
    void closeEvent(QCloseEvent *e);
    void serial_connect();
    void serial_rescan();
    void widget_changed();
    void serial_received();
    void on_PUSH_clicked();
    void on_textEdit_destroyed(struct trainMacro data);
    void train_set(struct trainMacro traindata);


private slots:
    void on_pushButton_2_clicked();

    void on_PUSH_2_clicked();

    void on_PUSH_3_clicked();

//    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
