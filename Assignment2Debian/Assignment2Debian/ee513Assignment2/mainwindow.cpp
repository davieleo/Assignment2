#include "mainwindow.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QJsonDocument> //to use JSON format
#include<QDebug>

MainWindow *handle;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->count = 50;
    this->time = 0;
    this->setWindowTitle("EE513 Assignment 2");
    //For Pitch graph
    this->ui->customPlot->addGraph();
    this->ui->customPlot->yAxis->setLabel("Pitch (degrees)");
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    this->ui->customPlot->xAxis->setTicker(timeTicker);
    this->ui->customPlot->yAxis->setRange(-180,180);
    this->ui->customPlot->replot();

    //For Roll graph
    this->ui->customPlot_2->addGraph();
    this->ui->customPlot_2->yAxis->setLabel("Roll (degrees)");
    timeTicker->setTimeFormat("%h:%m:%s");
    this->ui->customPlot_2->xAxis->setTicker(timeTicker);
    this->ui->customPlot_2->yAxis->setRange(-180,180);
    this->ui->customPlot_2->replot();

    QObject::connect(this, SIGNAL(messageSignal(QString)),
                     this, SLOT(on_MQTTmessage(QString)));
    ::handle = this;

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update(){
    // For more help on real-time plots, see: http://www.qcustomplot.com/index.php/demos/realtimedatademo
    static QTime time(QTime::currentTime());
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds

    ui->customPlot->graph(0)->addData(key,Pitch); //Graph for Pitch
    ui->customPlot->graph(0)->rescaleKeyAxis(true);
    ui->customPlot->replot();
    ui->customPlot_2->graph(0)->addData(key,Roll); //graph for Roll
    ui->customPlot_2->graph(0)->rescaleKeyAxis(true);
    ui->customPlot_2->replot();
    //QString text = QString("Value added is %1").arg(this->count);
    //ui->outputEdit->setText(text);
}

void MainWindow::on_downButton_clicked()
{
    this->count-=10;
    this->update();
}

void MainWindow::on_upButton_clicked()
{
    this->count+=10;
    this->update();
}

void MainWindow::on_connectButton_clicked() //client 1, pitch and roll
{
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;

    if (MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)==0){
        ui->outputText->appendPlainText(QString("Callbacks set correctly"));
    }
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        ui->outputText->appendPlainText(QString("Failed to connect, return code %1").arg(rc));
    }
    ui->outputText->appendPlainText(QString("Subscribing to topic " TOPIC " for client " CLIENTID));
    int x = MQTTClient_subscribe(client, TOPIC, QOS);
    ui->outputText->appendPlainText(QString("Result of subscribe is %1 (0=success)").arg(x));
}
void MainWindow::on_TDTconnectButton_clicked() //client 2, the tap/double tap
{
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;
    MQTTClient_create(&client2, ADDRESS, CLIENTID2, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;

    if (MQTTClient_setCallbacks(client2, NULL, connlost, msgarrvd, delivered)==0){
        ui->outputText->appendPlainText(QString("Callbacks set correctly"));
    }
    if ((rc = MQTTClient_connect(client2, &opts)) != MQTTCLIENT_SUCCESS) {
        ui->outputText->appendPlainText(QString("Failed to connect, return code %1").arg(rc));
    }
    ui->outputText->appendPlainText(QString("Subscribing to topic " TOPIC2 " for client " CLIENTID2));
    int x = MQTTClient_subscribe(client2, TOPIC2, QOS);
    ui->outputText->appendPlainText(QString("Result of subscribe is %1 (0=success)").arg(x));
}

void delivered(void *context, MQTTClient_deliveryToken dt) {
    (void)context;
    // Please don't modify the Window UI from here
    qDebug() << "Message delivery confirmed";
    handle->deliveredtoken = dt;
}

/* This is a callback function and is essentially another thread. Do not modify the
 * main window UI from here as it will cause problems. Please see the Slot method that
 * is directly below this function. To ensure that this method is thread safe I had to
 * get it to emit a signal which is received by the slot method below */
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    (void)context; (void)topicLen;
    qDebug() << "Message arrived (topic is " << topicName << ")";
    qDebug() << "Message payload length is " << message->payloadlen;
    QString payload;
    payload.sprintf("%s", (char *) message->payload).truncate(message->payloadlen);
    emit handle->messageSignal(payload);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

/** This is the slot method. Do all of your message received work here. It is also safe
 * to call other methods on the object from this point in the code */
void MainWindow::on_MQTTmessage(QString payload){
    //ui->outputText->appendPlainText(payload);
    ui->outputText->ensureCursorVisible();
    str = payload;
    //ADD YOUR CODE HERE
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8());
    QJsonObject obj = doc.object();
    QJsonObject sample = obj["PR"].toObject();          //Reading the PR JSON package
    QJsonObject sample2 = obj["SetTDT"].toObject();     //Reading the SetTDT JSON package
    this->Pitch = (float) sample["Pitch"].toDouble();   //Extracting the Pitch values from the PR package
    this->Roll = (float) sample["Roll"].toDouble();     //Extracting the Roll values from the JSON package
    this->Tap = (int) sample2["T"].toInt();             //Using intergers to read the values for Tap and double tap. Tried boolean but proved problematic
    this->DoubleTap = (int) sample2["DT"].toInt();      //int is 48 when nothing is detected and 50 when it detects an interupt
    PRout = "Pitch : " + QString::number(Pitch) + "  Roll: " + QString::number(Roll) + " Tap " + QString::number(Tap) + " DoubleTap " + QString::number(DoubleTap); //format for output on Text box
    ui->outputText->appendPlainText(PRout);
    if (Tap==50){                                       //Handling the Tap double tap to display on the buttoooons
        ui->tapButton->setText("Tap");
    }
    else{
        ui->tapButton->setText("Waiting..");
    }
    if (DoubleTap==50){
        ui->doubleButton->setText("Doubletap");

    }
    else{
        ui->doubleButton->setText("Waiting..");
    } //end of tap double tap handling

    if (count>=0){ //This set of if statements handles the upper and lower limits of the Pitch alarm. Using the up/Downbuttons, the alarm limits can be set
        if (Pitch>=count){
            QString text = QString("Upper limit of %1 has been reached").arg(this->count);
            ui->outputEdit->setText(text);
        }
        if (Pitch<=count){
            QString text = QString("Upper limit of %1 not reached").arg(this->count);
            ui->outputEdit->setText(text);
        }
    }
    if (count<=0){
        if (Pitch<=count){
            QString text = QString("Lower limit of %1 has been reached").arg(this->count);
            ui->outputEdit->setText(text);
        }
        if (Pitch>=count){
            QString text = QString("Lower limit of %1 not reached").arg(this->count);
            ui->outputEdit->setText(text);
        }
    }
    if (count==0){
        QString text = QString("No limit set %1").arg(this->count);
        ui->outputEdit->setText(text);
    } //end of pitch limits warnings

    this->update();


}
void MainWindow::tapButton(){

}

void MainWindow::DoubleButton(){

}

void connlost(void *context, char *cause) {
    (void)context; (void)*cause;
    // Please don't modify the Window UI from here
    qDebug() << "Connection Lost" << endl;
}

void MainWindow::on_disconnectButton_clicked()
{
    qDebug() << "Disconnecting from the broker" << endl;
    MQTTClient_disconnect(client, 10000);
    //MQTTClient_destroy(&client);
}

void MainWindow::on_disconnectTDTButton_clicked()
{
    qDebug() << "Disconnecting from the broker" << endl;
    MQTTClient_disconnect(client2, 10000);
    //MQTTClient_destroy(&client);
}
