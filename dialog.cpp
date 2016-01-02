#include "dialog.h"
#include "ui_dialog.h"
#include <QDebug>
#include <QtWidgets>
#include <qt_windows.h>

#pragma comment(lib, "User32.lib")

quint64 dt2ms(const QDateTime &dt)
{
    return dt.toMSecsSinceEpoch(); //dt.toString("yyyy/MM/dd hh:mm:ss.zzz");
}

QDateTime ms2dt(quint64 ms)
{
    QDateTime dt;
    dt.setMSecsSinceEpoch(ms);
    return dt;
}

QString msFormat(quint64 ms)
{
    return ms2dt(ms).toString("yyyy/MM/dd hh:mm:ss.zzz");
}

QMap<quint64, QString> s_data;
HHOOK s_hook = NULL;
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION) {
        PKBDLLHOOKSTRUCT pKeyboardHookStruct = (PKBDLLHOOKSTRUCT)lParam;
        if ((wParam == WM_KEYUP) || (wParam == WM_SYSKEYUP)) {
            BYTE KeyboardState[256];
            ZeroMemory(KeyboardState, sizeof(KeyboardState));
            GetKeyboardState(KeyboardState);

            KeyboardState[VK_SHIFT] = (BYTE)(GetKeyState(VK_LSHIFT) | GetKeyState(VK_RSHIFT));
            KeyboardState[VK_CAPITAL] = (BYTE)GetKeyState(VK_CAPITAL);

            wchar_t wstr[64] = {0};
            int iNumChar = ToUnicode(pKeyboardHookStruct->vkCode, pKeyboardHookStruct->scanCode, KeyboardState, wstr, _countof(wstr), 0);
            if (iNumChar >= 1) {
                quint64 key = QDateTime::currentDateTime().toMSecsSinceEpoch();
                QString value = QString::fromStdWString(wstr);
                s_data[key] = value;
                emit Global::instance()->sigNewInput(msFormat(key), value);
            } else {
                wchar_t keyText[64] = {0};
                LONG Flags = 0;
                Flags = pKeyboardHookStruct->scanCode << 16;
                Flags |= pKeyboardHookStruct->flags << 24;

                if (GetKeyNameText(Flags, keyText, 64) > 0) {
                    quint64 key = QDateTime::currentDateTime().toMSecsSinceEpoch();
                    QString value = QString("[%1]").arg(QString::fromWCharArray(keyText));
                    s_data[key] = value;
                    emit Global::instance()->sigNewInput(msFormat(key), value);
                }
            }
        }
    }

    return CallNextHookEx(s_hook, nCode, wParam, lParam);
}

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    m_willExit(false)
{
    this->setWindowFlags(Qt::WindowCloseButtonHint);
    ui->setupUi(this);
    connect(ui->pbStart, &QPushButton::clicked, this, &Dialog::slotStart);
    connect(ui->pbGet, &QPushButton::clicked, this, &Dialog::slotGet);
    connect(ui->pbResetStart, &QPushButton::clicked, this, &Dialog::slotResetStart);
    connect(ui->pbResetEnd, &QPushButton::clicked, this, &Dialog::slotResetEnd);
    connect(ui->pbClear, &QPushButton::clicked, this, &Dialog::slotClear);
    connect(Global::instance(), &Global::sigNewInput, this, &Dialog::slotNewInput);

    QMenu *menu = new QMenu;
    QAction *acShow = new QAction("Show", menu);
    connect(acShow, &QAction::triggered, this, [this]() {
        this->show();
    });

    QAction *acExit = new QAction("Exit", menu);
    connect(acExit, &QAction::triggered, this, [this]() {
        m_willExit = true;
        this->close();
    });

    QList<QAction*> acList;
    acList.append(acShow);
    acList.append(acExit);
    menu->addActions(acList);
    m_tray.setIcon(QPixmap(":/coolMale"));
    m_tray.setContextMenu(menu);
    m_tray.show();
    connect(&m_tray, &QSystemTrayIcon::activated, this, &Dialog::slotActivated);

    ui->dteStart->setDateTime(QDateTime::currentDateTime());
    ui->dteEnd->setDateTime(QDateTime::currentDateTime());
}

Dialog::~Dialog()
{
    delete ui;

    if (s_hook) {
        UnhookWindowsHookEx(s_hook);
    }
}

void Dialog::slotStart()
{
    if (NULL == s_hook) {
        s_hook =  SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    } else {
        UnhookWindowsHookEx(s_hook);
        s_hook = NULL;
    }
    ui->pbStart->setText(NULL == s_hook ? "start" : "stop");
}

void Dialog::slotGet()
{
    ui->pteShow->clear();

    quint64 start = dt2ms(ui->dteStart->dateTime());
    quint64 end = dt2ms(ui->dteEnd->dateTime());

    QMapIterator<quint64, QString> iter(s_data);
    while (iter.hasNext()) {
        iter.next();

        if (iter.key() >= start && iter.key() <= end) {
            ui->pteShow->insertPlainText(iter.value());
        }
    }
}

void Dialog::slotResetStart()
{
    ui->dteStart->setDateTime(QDateTime::currentDateTime());
}

void Dialog::slotResetEnd()
{
    ui->dteEnd->setDateTime(QDateTime::currentDateTime());
}

void Dialog::slotActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (QSystemTrayIcon::Trigger == reason) {
        if (this->isHidden()) {
            this->show();
        } else {
            this->hide();
        }
    }
}

void Dialog::slotClear()
{
    ui->lwContent->clear();
}

void Dialog::slotNewInput(const QString &dt, const QString &text)
{
    ui->lwContent->addItem(dt + " - " + text);
    ui->lwContent->scrollToBottom();
}

void Dialog::closeEvent(QCloseEvent *event)
{
    if (m_willExit) {
        event->accept();
    } else {
        event->ignore();
        this->hide();
    }
}
