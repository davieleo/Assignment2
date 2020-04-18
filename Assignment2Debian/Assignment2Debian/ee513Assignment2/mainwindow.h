#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <stdio.h>
#include "MQTTClient.h"

#define ADDRESS     "tcp://192.168.1.16:1883"
#define CLIENTID    "rpi2" //ttopic for pitch and roll
#define CLIENTID2   "rpi3" //topic for Tap/double tap
#define AUTHMETHOD  "david"
#define AUTHTOKEN   "passwd"
#define TOPIC       "ee513/PR"
#define TOPIC2      "ee513/TDT"
#define PAYLOAD     "Hello World!"
#define QOS         2
#define TIMEOUT     10000L

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void on_downButton_clicked();
    void on_upButton_clicked();
    void tapButton();
    void DoubleButton();
    void on_connectButton_clicked();
    void on_TDTconnectButton_clicked(); //Subscribe button for Tap
    void on_disconnectButton_clicked();
    void on_disconnectTDTButton_clicked(); //Unsubscribe to tap
    void on_MQTTmessage(QString message);

signals:
    void messageSignal(QString message);

private:
    Ui::MainWindow *ui;
    void update();
    QString str, charac, PRout;
    int count, time;
    int Tap, DoubleTap;
    float Pitch, Roll, Pitch1;
    MQTTClient client, client2;
    volatile MQTTClient_deliveryToken deliveredtoken;

    friend void delivered(void *context, MQTTClient_deliveryToken dt);
    friend int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
    friend void connlost(void *context, char *cause);
};

void delivered(void *context, MQTTClient_deliveryToken dt);
int  msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
void connlost(void *context, char *cause);

#endif // MAINWINDOW_H

