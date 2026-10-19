#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QWidget>

class QPushButton;
class MainWindow;
class QCloseEvent;

class AdminWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AdminWindow(MainWindow *mainWindow, QWidget *parent = nullptr);
    ~AdminWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QPushButton *btnBack;
    MainWindow *m_mainWindow;
};

#endif // ADMINWINDOW_H
