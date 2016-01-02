#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QSystemTrayIcon>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private:
    void slotStart();
    void slotGet();
    void slotResetStart();
    void slotResetEnd();
    void slotActivated(QSystemTrayIcon::ActivationReason reason);

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::Dialog *ui;
    QSystemTrayIcon m_tray;
    bool m_willExit;
};

#endif // DIALOG_H
