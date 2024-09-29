#include "../include/roskcci/tab2socketclient.h"
#include "ui_tab2socketclient.h"

Tab2SocketClient::Tab2SocketClient(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Tab2SocketClient)
{
    ui->setupUi(this);
    pSocketClient = new SocketClient(this);
    ui->pPBsendButton->setEnabled(false);
    connect(ui->pPBRecvDataClear,SIGNAL(clicked()),ui->pTErecvData,SLOT(clear()));
    connect(ui->pPBServerConnect,SIGNAL(clicked(bool)),this,SLOT(slotConnectToServer(bool)));
    connect(pSocketClient,SIGNAL(sigSocketRecv(QString)),this,SLOT(slotSocketRecvUpdate(QString)));
    connect(ui->pPBsendButton,SIGNAL(clicked()),this, SLOT(slotSocketSendData()));

}

void Tab2SocketClient::slotConnectToServer(bool check)
{
    bool bOk;
    if(check) {
        pSocketClient->slotConnectToServer(bOk);
        if(bOk) {
            ui->pPBServerConnect->setText("서버 해제");
            ui->pPBsendButton->setEnabled(true);
        } else {
            ui->pPBServerConnect->setChecked(false);
        }
    } else {
      pSocketClient->slotClosedByServer();
      ui->pPBServerConnect->setText("서버 연결");
      ui->pPBsendButton->setEnabled(false);
  }
}

void Tab2SocketClient::slotSocketRecvUpdate(QString strRecvData)
{
    QTime time = QTime::currentTime();
    QString strTime = time.toString();
    strRecvData.chop(1);  //'\n'제거
    strTime = strTime + " " + strRecvData;
    ui->pTErecvData->append(strTime);
    ui->pTErecvData->moveCursor(QTextCursor::End);

    if((strRecvData.indexOf("New connected") != -1) || \
            (strRecvData.indexOf("Already logged") != -1) || \
            (strRecvData.indexOf("Authentication Error") != -1))
        return;
    //수신포멧 : [KSH_QT]LED@1@ON
    //       @KSH_QT@LED@1@ON
    strRecvData.replace("[","@");
    strRecvData.replace("]","@");
    QStringList qlist = strRecvData.split("@"); //qlist[1] : 송신자ID

    int Len = qlist.size();
    double Data[10] = {0.0};

//    qDebug() << qlist[2];
    if(qlist[2] == "GOAL")
        emit goGoalSig(qlist[3].toDouble(),qlist[4].toDouble(),qlist[5].toDouble(),qlist[6].toDouble());

    if(qlist[2] == "CMDVEL")
         emit cmdVelPubSig(qlist[3].toInt(),qlist[4].toInt());

    if(qlist[2] == "CMDLIFT")
         emit cmdVelLiftPubSig(qlist[3].toInt());


}

void Tab2SocketClient::slotSocketSendData()
{
    QString strRecvId;
    QString strSendData;

    strRecvId = ui->pLErecvId->text();
    strSendData = ui->pLEsendData->text();
    if(strRecvId.isEmpty())
        strSendData = "[ALLMSG]" + strSendData;
    else
        strSendData = "["+strRecvId+"]" + strSendData;
    pSocketClient->slotSocketSendData(strSendData);
    ui->pLEsendData->clear();
}

void Tab2SocketClient::slotSocketSendData(QString strData)
{
    if(ui->pPBServerConnect->isChecked())
        pSocketClient->slotSocketSendData(strData);
}

Tab2SocketClient::~Tab2SocketClient()
{
    delete ui;
}
