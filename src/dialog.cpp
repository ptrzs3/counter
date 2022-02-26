#include "dialog.h"
#include "ui_dialog.h"
#include "windows.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->label_background->setVisible(false);
    ui->label_logo->move(0,0);
    ui->label_logo->setVisible(false);
    ui->label_5->setVisible(false);
    ui->label_6->setVisible(false);
    ui->label_7->setVisible(false);
    ui->label_8->setVisible(false);
    ui->label_english->setVisible(false);
    ui->label_chinese->setVisible(false);
    ui->label_logo->setStyleSheet("image: url(logo.png);");
    INIT();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::clockCalibration()
{
    ntpIP = "ntp1.aliyun.com";
    udpSocket = new QUdpSocket(this);
    QObject::connect(udpSocket, &QUdpSocket::connected, this, &Dialog::sendData);
    QObject::connect(udpSocket, &QUdpSocket::readyRead, this, &Dialog::readDate);
    udpSocket->connectToHost(ntpIP, 123);
    udpSocket->waitForConnected();
}

void Dialog::sendData()
{
    qint8 LI = 0;
    qint8 VN = 3;
    qint8 MODE = 3;
    qint8 STRATUM = 0;
    qint8 POLL = 4;
    qint8 PREC = -6;
    QDateTime epoch(QDate(1900, 1, 1), QTime(0, 0, 0));
    qint32 second = quint32(epoch.secsTo(QDateTime::currentDateTime()));

    qint32 temp = 0;
    QByteArray timeRequest(48, 0);
    timeRequest[0] = (LI << 6) | (VN << 3) | (MODE);
    timeRequest[1] = STRATUM;
    timeRequest[2] = POLL;
    timeRequest[3] = PREC & 0xff;
    timeRequest[5] = 1;
    timeRequest[9] = 1;
    timeRequest[40] = (temp = (second & 0xff000000) >> 24);
    temp = 0;
    timeRequest[41] = (temp = (second & 0x00ff0000) >> 16);
    temp = 0;
    timeRequest[42] = (temp = (second & 0x0000ff00) >> 8);
    temp = 0;
    timeRequest[43] = ((second & 0x000000ff));

    udpSocket->write(timeRequest);
}

void Dialog::readDate()
{
    QByteArray newTime;
    QDateTime epoch(QDate(1900, 1, 1), QTime(0, 0, 0));
    QDateTime unixStart(QDate(1970, 1, 1), QTime(0, 0, 0));

    while (udpSocket->hasPendingDatagrams())
    {
        newTime.resize(udpSocket->pendingDatagramSize());
        udpSocket->read(newTime.data(), newTime.size());
    };

    QByteArray transmitTimeStamp ;
    transmitTimeStamp = newTime.right(8);
    quint32 seconds = transmitTimeStamp.at(0);
    quint8 temp = 0;

    for (int i = 1; i <= 3; i++)
    {
        seconds = (seconds << 8);
        temp = transmitTimeStamp.at(i);
        seconds = seconds + temp;
    }
    time_error = seconds - epoch.secsTo(unixStart) - QDateTime::currentDateTime().toSecsSinceEpoch();
    udpSocket->disconnectFromHost();
}


void Dialog::INIT()
{
    QApplication::setQuitOnLastWindowClosed(false);
    createDatebaseConnection();
    timer =  new QTimer();
    timer2 = new QTimer();
    timer3 = new QTimer();
    timer4 = new QTimer();
    //本地时间与网络时间的误差
    time_error = 0;

    dirPath = QApplication::applicationDirPath() + "/config.ini";
    filePath = QApplication::applicationFilePath();
    appName = QApplication::applicationName();

    //设置托盘
    trayIcon = new QSystemTrayIcon(this);
    trayIconMenu = new QMenu;
    trayStartAtSystemBoot = new QAction(tr("开机启动"),this);
    trayClockCalibration = new QAction(tr("校准时间"), this);
    trayClockDisplayAtLock = new QAction(tr("锁屏时钟"), this);
    trayProcessQuit = new QAction(tr("退出"),this);

    trayIconMenu->addSeparator();
    trayIconMenu->addAction(trayStartAtSystemBoot);
    trayIconMenu->addAction(trayClockCalibration);
    trayIconMenu->addAction(trayClockDisplayAtLock);
    trayIconMenu->addAction(trayProcessQuit);

    trayIcon->setToolTip(tr("托盘菜单"));
    trayIcon->setIcon(QIcon("://Resource/tray.png"));
    trayIcon->setContextMenu(trayIconMenu);     //设置托盘上下文菜单
    trayIcon->show();
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayDoubleClickLeft(QSystemTrayIcon::ActivationReason)));
    connect(trayProcessQuit,SIGNAL(triggered()),this,SLOT(processExit()));
    connect(trayClockCalibration,SIGNAL(triggered()),this,SLOT(clockCalibration()));
    connect(trayStartAtSystemBoot,SIGNAL(triggered()),this,SLOT(traySetBoot()));
    connect(trayClockDisplayAtLock,SIGNAL(triggered()),this,SLOT(registryClockDisplay()));


    //查询开机启动
    trayTextCheck();

    //读取配置文件
    writeConfig();
    registryConfig();
    readConfig();

    //设置窗口透明镂空和总在最前
    setWindowFlags(Qt::Tool|Qt::FramelessWindowHint);
    if(top_hint)
        setWindowFlag(Qt::WindowStaysOnTopHint);
    setWindowFlags(this->windowFlags());
    setWindowOpacity(1);
    setAttribute(Qt::WA_TranslucentBackground);

    //设置显示的文本
    font.setFamily(font_family);
    font.setPointSize(font_size);
    font.setBold(font_bold);
    ui->label->setText(word);
    ui->label->setFont(font);
    ui->label->setStyleSheet("color:" + font_color);

    //连接信号与槽
    QObject::connect(timer,SIGNAL(timeout()),this,SLOT(updateTime()));
    QObject::connect(timer2, SIGNAL(timeout()), this, SLOT(shine()));
    QObject::connect(timer3, SIGNAL(timeout()), this, SLOT(moveLogo()));
    QObject::connect(timer4, SIGNAL(timeout()), this, SLOT(updateLockScreenTime()));

    //启动timer
    timer4->start(100);
    timer->start(100);
    timer2->start(500);
    timer3->setInterval(3000);
    screen = QGuiApplication::primaryScreen ();
    this->setGeometry(0,0,screen->geometry().width(),screen->geometry().height());
    this->move(0, 0);
    ui->label_background->setGeometry(0,0,screen->geometry().width(),screen->geometry().height());

}
void Dialog::setTimeLabelText(int Days,int Hours,int Minutes,int Seconds)
{
    //获取日期的每一位数
    int Days1,Days2,Days3;
    Days3=Days%10;
    Days/=10;
    Days2=Days%10;
    Days/=10;
    Days1=Days%10;
    //获取小时的每一位数
    int Hours1,Hours2;
    Hours2=Hours%10;
    Hours/=10;
    Hours1=Hours%10;
    //获取分钟的每一位数
    int Minutes1,Minutes2;
    Minutes2=Minutes%10;
    Minutes/=10;
    Minutes1=Minutes%10;
    //获取秒的每一位数
    int Seconds1,Seconds2;
    Seconds2=Seconds%10;
    Seconds/=10;
    Seconds1=Seconds%10;
    //设置时间标签
    this->ui->label_Days1->setText(QString::number(Days1));
    this->ui->label_Days2->setText(QString::number(Days2));
    this->ui->label_Days3->setText(QString::number(Days3));

    this->ui->label_Hours1->setText(QString::number(Hours1));
    this->ui->label_Hours2->setText(QString::number(Hours2));

    this->ui->label_Minutes1->setText(QString::number(Minutes1));
    this->ui->label_Minutes2->setText(QString::number(Minutes2));

    this->ui->label_Seconds1->setText(QString::number(Seconds1));
    this->ui->label_Seconds2->setText(QString::number(Seconds2));

}

void Dialog::updateTime()
{
    //计算时间差
    qint64 machine_time = QDateTime::currentDateTime().toSecsSinceEpoch();
    QDateTime begin_time = QDateTime::fromSecsSinceEpoch(machine_time + time_error);
    QDateTime end_time = QDateTime::fromString(deadline,"yyyy.MM.dd HH:mm:ss");
    QTime time;
    time.setHMS(0, 0, 0, 0);
    int LeftSeconds=begin_time.secsTo(end_time);
    int Days,Hours,Minutes,Seconds;
    int _LeftSeconds = LeftSeconds;

    //获取秒
    Seconds=LeftSeconds%60;

    LeftSeconds-=Seconds;
    //转化成分钟
    LeftSeconds/=60;
    //获取分钟
    Minutes=LeftSeconds%60;

    LeftSeconds-=Minutes;
    //转化成小时
    LeftSeconds/=60;
    //获取小时
    Hours=LeftSeconds%24;

    //获取天
    Days=(LeftSeconds-Hours)/24;
    this->setTimeLabelText(Days,Hours,Minutes,Seconds);
    if(_LeftSeconds <= 0)
    {
        this->setTimeLabelText(0,0,0,0);
        shining = false;
        timer->disconnect();
        playMusic();
        return;
    }
}

void Dialog::processExit()
{
    db.close();
    exit(0);
}

void Dialog::counterdownHideOrDisplay()
{
    boolean flag = ! ui->label->isVisible();
    shine_ = flag;
    ui->label->setVisible(flag);
    ui->label_2->setVisible(flag);
    ui->label_3->setVisible(flag);
    ui->label_4->setVisible(flag);
    ui->label_num_background->setVisible(flag);
    ui->label_Days1->setVisible(flag);
    ui->label_Days2->setVisible(flag);
    ui->label_Days3->setVisible(flag);
    ui->label_Hours1->setVisible(flag);
    ui->label_Hours2->setVisible(flag);
    ui->label_Minutes1->setVisible(flag);
    ui->label_Minutes2->setVisible(flag);
    ui->label_Seconds1->setVisible(flag);
    ui->label_Seconds2->setVisible(flag);
}

void Dialog::shine()
{
    if(!shine_ || !shining)
        return;
    boolean flag = !ui->label_2->isVisible();
    ui->label_2->setVisible(flag);
    ui->label_3->setVisible(flag);
    ui->label_4->setVisible(flag);
}

void Dialog::writeConfig()
{
    QFileInfo ini_file(dirPath);
    if(!ini_file.exists())
    {
        QSettings settings(dirPath, QSettings::IniFormat);
        settings.setIniCodec("UTF-8");
        settings.setValue("/settings/word", "距离考研时间还有");
        settings.setValue("/settings/deadline", "2022.12.25 00:00:00");
        settings.setValue("/settings/font_size", "20");
        settings.setValue("/settings/font_color", "#000000");
        settings.setValue("/settings/font_family", "微软雅黑");
        settings.setValue("/settings/font_bold", "true");
        settings.setValue("/settings/shining", "true");
        settings.setValue("settings/music_volume", "30");
        settings.setValue("settings/top_hint", "true");
        settings.setValue("settings/word_start", "1");
        settings.setValue("settings/word_end", "5493");
    }
}

void Dialog::readConfig()
{
    QSettings *reg=new QSettings(INI_POS, QSettings::Registry64Format);
    QVariant value = reg->value( appName);
    iniFilePath = value.toString();

    QSettings settings(iniFilePath,QSettings::IniFormat);
    settings.setIniCodec("UTF-8");
    word = settings.value("settings/word").toString().toUtf8();
    deadline = settings.value("settings/deadline").toString();
    font_size = settings.value("settings/font_size").toInt();
    font_color = settings.value("settings/font_color").toString();
    font_family = settings.value("settings/font_family").toString().toUtf8();
    font_bold = settings.value("settings/font_bold").toBool();
    shining = settings.value("settings/shining").toBool();
    music_volume = settings.value("settings/music_volume").toInt();
    top_hint = settings.value("settings/top_hint").toBool();
    word_start = settings.value("settings/word_start").toInt();
    word_end = settings.value("settings/word_end").toInt();
    delete  reg;
    checkVolumeIsValid();
}

void Dialog::checkVolumeIsValid()
{
    if(music_volume<0 && music_volume>100)
        music_volume=30;
    if(word_start<1 && word_start>5493)
        word_start=1;
    if(word_end<1 && word_end>5493)
        word_end=5493;
    if(word_start>word_end)
    {
        word_start=1;
        word_end=5493;
    }
}

void Dialog::trayTextCheck()
{
    QSettings *reg=new QSettings(AUTO_RUN, QSettings::Registry64Format);
    QVariant value = reg->value( appName);
    QString tmp = value.toString();
    if(tmp.compare(filePath.replace("/", "\\"))==0)
    {
        trayStartAtSystemBoot->setText("√ 开机启动");
    }
    else
    {
        trayStartAtSystemBoot->setText("开机启动");
    }
    delete reg;

    QSettings *reg2=new QSettings(INI_POS, QSettings::Registry64Format);
    QVariant value2 = reg2->value("clock_display");
    boolean status = value2.toInt() > 0;
    if(status){
        trayClockDisplayAtLock->setText("√ 锁屏时钟");
    }
    else {
        trayClockDisplayAtLock->setText("锁屏时钟");
    }

    delete reg2;
}

void Dialog::traySetBoot()
{
    QSettings *reg=new QSettings(AUTO_RUN, QSettings::Registry64Format);
    QVariant value = reg->value( appName);
    QString tmp = value.toString();
    //开机自动运行
    if (tmp.compare(filePath.replace("/", "\\")) != 0)
    {
        reg->setValue( appName,filePath.replace("/", "\\"));
    }
    else
    {
        reg->setValue( appName,"");
    }
    delete reg;
    trayTextCheck();
}

void Dialog::registryConfig()
{
    QSettings *reg=new QSettings(INI_POS, QSettings::Registry64Format);
    reg->setValue( appName, dirPath);
    delete reg;
}

void Dialog::registryClockDisplay()
{
    QSettings *reg=new QSettings(INI_POS, QSettings::Registry64Format);
    QVariant value = reg->value("clock_display");
    boolean status = value.toInt() > 0;
    if(status){
        reg->setValue("clock_display", 0);
    }
    else{
        reg->setValue("clock_display", 1);
    }
    status = !status;
    if(status){
        trayClockDisplayAtLock->setText("√ 锁屏时钟");
    }
    else {
        trayClockDisplayAtLock->setText("锁屏时钟");
    }
    delete reg;
}
void Dialog::playMusic()
{
    player = new QMediaPlayer;
    QString music_file_path = QApplication::applicationDirPath() + "/music.mp3";
    QFileInfo music_file(music_file_path);
    if(music_file.exists())
    {
        player->setMedia(QUrl::fromLocalFile(music_file_path));
    }
    else
    {
        player->setMedia(QUrl("qrc:/Resource/music.mp3"));
    }
    player->setVolume(music_volume);
    player->play();
}

void Dialog::setFullScreen()
{
    timer3->start();
    ui->label_background->setVisible(true);
    ui->label_logo->setVisible(true);
    QSettings *reg=new QSettings(INI_POS, QSettings::Registry64Format);
    QVariant value = reg->value("clock_display");
    boolean status = value.toInt() > 0;
    ui->label_5->setVisible(status);
    ui->label_6->setVisible(status);
    ui->label_7->setVisible(status);
    ui->label_8->setVisible(status);
    ui->label->setStyleSheet("color:#ffffff");
    this->setCursor(Qt::BlankCursor);
    if(!ui->label->isVisible())
        counterdownHideOrDisplay();
    if(eng_ch)
    {
        ui->label_english->setVisible(true);
        ui->label_chinese->setVisible(true);
    }
    moveLogo();
}

void Dialog::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Escape)
    {
        quitFullScreen();
    }
}

void Dialog::moveLogo()
{
    //显示中英字幕
    if(eng_ch)
    {
        ui->label_logo->move(0,0);
        ui->label_english->setVisible(true);
        ui->label_chinese->setVisible(true);
        int x = QRandomGenerator::global()->bounded(0, QApplication::desktop()->width() - ui->label_english->geometry().width());
        int y = QRandomGenerator::global()->bounded(0, QApplication::desktop()->height() - ui->label_english->geometry().height() - ui->label_chinese->geometry().height());
        while((x>ui->label_Days1->geometry().x() - ui->label_english->geometry().width() && y < ui->label_Days1->geometry().y() + ui->label_Days1->geometry().height())
              || (x<ui->label_8->geometry().x() + ui->label_8->geometry().width() && y>ui->label_5->geometry().y() - (2 * ui->label_english->geometry().height())))
        {
            x = QRandomGenerator::global()->bounded(0, QApplication::desktop()->width() - ui->label_english->geometry().width());
            y = QRandomGenerator::global()->bounded(0, QApplication::desktop()->height() - ui->label_english->geometry().height());
        }
        readDatebase();
        // ui->label_english->setGeometry(x,y,300,200);
        ui->label_english->move(x, y);
        ui->label_chinese->move(x, y+79);
    }
    else
    {
        ui->label_english->setVisible(false);
        ui->label_chinese->setVisible(false);
        int x = QRandomGenerator::global()->bounded(0, QApplication::desktop()->width() - ui->label_logo->geometry().width());
        int y = QRandomGenerator::global()->bounded(0, QApplication::desktop()->height() - ui->label_logo->geometry().height());
        while((x>ui->label_Days1->geometry().x() - ui->label_logo->geometry().width() && y < ui->label_Days1->geometry().y() + ui->label_Days1->geometry().height())
              || (x<ui->label_8->geometry().x() + ui->label_8->geometry().width() && y>ui->label_5->geometry().y() - (2 * ui->label_logo->geometry().height())))
        {
            x = QRandomGenerator::global()->bounded(0, QApplication::desktop()->width() - ui->label_logo->geometry().width());
            y = QRandomGenerator::global()->bounded(0, QApplication::desktop()->height() - ui->label_logo->geometry().height());
        }
        ui->label_logo->move(x, y);
    }
}

void Dialog::mouseDoubleClickEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton)
    {
        if(ui->label_background->isHidden())
            setFullScreen();
        else
        {
            eng_ch = !eng_ch;
            moveLogo();
        }
    }

    else if(ev->button() == Qt::RightButton)
    {
        if(ui->label_background->isVisible())
            quitFullScreen();
        else
            counterdownHideOrDisplay();
    }
}

void Dialog::quitFullScreen()
{
    if(ui->label_background->isVisible())
    {
        timer3->stop();
        ui->label_background->setVisible(false);
        ui->label_logo->setVisible(false);
        ui->label_5->setVisible(false);
        ui->label_6->setVisible(false);
        ui->label_7->setVisible(false);
        ui->label_8->setVisible(false);
        ui->label_english->setVisible(false);
        ui->label_chinese->setVisible(false);
        ui->label->setStyleSheet("color:" + font_color);
        this->setCursor(Qt::ArrowCursor);
    }
}

void Dialog::trayDoubleClickLeft(QSystemTrayIcon::ActivationReason ev)
{
    if(ev == QSystemTrayIcon::DoubleClick)
    {
        counterdownHideOrDisplay();
    }
}

void Dialog::updateLockScreenTime()
{
    qint64 lock_screen_time_secs = QDateTime::currentDateTime().toSecsSinceEpoch() + time_error;
    QDateTime lock_screen_time = QDateTime::fromSecsSinceEpoch(lock_screen_time_secs);
    int month = lock_screen_time.date().month();
    int day = lock_screen_time.date().day();
    QString week = lock_screen_time.toString("ddd");
    QString hour = lock_screen_time.toString("HH");
    QString minute = lock_screen_time.toString("mm");
    ui->label_5->setText(hour);
    ui->label_7->setText(minute);
    QString a = QString::asprintf("%d月%d日，", month, day);
    week = tranferWeek(week);
    ui->label_8->setText(a+week); 
}

QString Dialog::tranferWeek(QString week)
{
    if(week == "周一")
        return "星期一";
    else if(week == "周二")
        return "星期二";
    else if(week == "周三")
        return "星期三";
    else if(week == "周四")
        return "星期四";
    else if(week == "周五")
        return "星期五";
    else if(week == "周六")
        return "星期六";
    else
        return "星期日";
}

bool Dialog::createDatebaseConnection()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    //QString db_file = QApplication::applicationDirPath() + "/words.db";
    db.setDatabaseName("words.db");
    if(!db.open())
    {
        qDebug()<<"OPENFAILED";
        return false;
    }
    return true;
}

void Dialog::readDatebase()
{
    QSqlQuery query;
    int x = QRandomGenerator::global()->bounded(word_start, word_end+1);
    QString cmd = QString::asprintf("select * from dancibiao where id = %d;", x);
    query.exec(cmd);
    qDebug()<<query.value(1).toString();
    if(query.first())
    {
        ui->label_english->setText(query.value(1).toString());
        ui->label_chinese->setText(query.value(2).toString());
    }
}
