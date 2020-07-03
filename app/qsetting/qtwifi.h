#ifndef QTWIFI_H
#define QTWIFI_H

#include <QDebug>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QProcess>
#include <QThread>
#include <QTimer>

class wifiStatusThread : public QThread
{
    Q_OBJECT
    void run();
signals:
    void updateText(QString t);
};

class wifiScanThread : public QThread
{
    Q_OBJECT
    void run();
signals:
    void resultReady(const QStringList &s);
};

class qtWifi : public QListWidget
{
    Q_OBJECT

public:
    qtWifi(QWidget *parent = nullptr, QLabel *label = nullptr, QPushButton *btn = nullptr, bool on = false);
    ~qtWifi();
    int cnt;
    bool isOn();
    void turnOn();
    void turnOff();
public slots:
    void updateText(QString t);
    void on_btnClicked();
    void on_itemClicked(QListWidgetItem *item);
    void handleResults(const QStringList &list);
private:
    QLabel *text;
    QPushButton *switchBtn;
    wifiScanThread *scanThread;
    wifiStatusThread *statusThread;
};

#endif // QTWIFI_H
