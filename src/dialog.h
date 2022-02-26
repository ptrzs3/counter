#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QDateTime>
#include <QTimer>
#include <QRect>
#include <QDesktopWidget>
#include <QSystemTrayIcon>
#include <QDebug>
#include <QAction>
#include <QMenu>
#include <QSettings>
#include <QFont>
#include <QFileInfo>
#include <QDir>
#include <QScreen>
#include <QUdpSocket>
#include <QHostInfo>
#include <QMediaPlayer>
#include <QMouseEvent>
#include <QtMath>
#include <QRandomGenerator>
#include <QSqlDatabase>
#include <QSqlQuery>

#define AUTO_RUN "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define INI_POS "HKEY_CURRENT_USER\\Software\\CountDownTimer"
namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();
    void INIT();




private:
    Ui::Dialog *ui;
    qint64 time_error;
    QTimer *timer;
    QTimer *timer2;
    QTimer *timer3;
    QTimer *timer4;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QAction *trayProcessQuit;
    QAction *trayStartAtSystemBoot;
    QAction *trayClockCalibration;
    QAction *trayClockDisplayAtLock;

    QString word;
    QString deadline;
    int font_size;
    QString font_family;
    QString font_color;
    bool font_bold;
    int music_volume;
    bool top_hint;
    bool eng_ch = false;
    int word_start;
    int word_end;

    QScreen *screen;
    QFont font;

    bool shine_ = true;
    bool shining;

    QString dirPath;
    QString filePath;
    QString appName;
    QString iniFilePath;

    QString ntpIP;
    QUdpSocket *udpSocket;

    QMediaPlayer *player;

    QSqlDatabase db;

    void setTimeLabelText(int,int,int,int);
    void checkVolumeIsValid();
    bool createDatebaseConnection();
    void readDatebase();
    bool _setsystemtime();
    void writeConfig();
    void readConfig();
    void registryConfig();
    QString tranferWeek(QString);



private slots:
    void updateTime();
    void processExit();
    void counterdownHideOrDisplay();
    void shine();
    void traySetBoot();
    void trayTextCheck();
    void sendData();
    void readDate();
    void clockCalibration();
    void registryClockDisplay();
    void playMusic();
    void setFullScreen();
    void quitFullScreen();
    void moveLogo();
    void trayDoubleClickLeft(QSystemTrayIcon::ActivationReason reason);
    void updateLockScreenTime();

protected:
    virtual void keyPressEvent(QKeyEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
//    virtual void wheelEvent(QWheelEvent *);
};

#endif // DIALOG_H
