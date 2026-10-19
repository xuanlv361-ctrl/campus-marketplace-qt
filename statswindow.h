#ifndef STATSWINDOW_H
#define STATSWINDOW_H

#include <QWidget>

class QPushButton;
class MainWindow;
class QCloseEvent;

class StatsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit StatsWindow(MainWindow *mainWindow, QWidget *parent = nullptr);
    ~StatsWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QPushButton *btnBack;
    MainWindow *m_mainWindow;
};

#endif // STATSWINDOW_H
