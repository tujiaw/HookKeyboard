#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QSystemTrayIcon>

namespace Ui {
class Dialog;
}

class Global : public QObject
{
    Q_OBJECT
public:
    static Global* instance()
    {
        static Global s_instance;
        return &s_instance;
    }

signals:
    void sigNewInput(const QString &dt, const QString &text);
};

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
    void slotClear();
    void slotNewInput(const QString &dt, const QString &text);

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::Dialog *ui;
    QSystemTrayIcon m_tray;
    bool m_willExit;
};

#endif // DIALOG_H
