#include "ckernel.h"
#include"TcpClientMediator.h"
#include<QMessageBox>
#include<qdebug.h>
#include"md5.h"
#include <QMetaType>
#include<QCoreApplication>
#include<QList>
//初始化静态类对象指针
CKernel *CKernel::kernel = nullptr;
//获取类对象指针
CKernel *CKernel::GetInstance()
{
    if(kernel){
        return kernel;
    }
    kernel = new CKernel();
    return kernel;
}

#define NetPackMap(a) m_netPackMap[a - _DEF_PROTOCOL_BASE]
//创建协议映射表函数
void CKernel::setNetMap()
{
    memset(m_netPackMap,0,sizeof(PFUN)*_DEF_PROTOCOL_COUNT);
    NetPackMap( _DEF_PACK_REGISTER_RS ) = &CKernel::slot_dealRegisterRs;
    NetPackMap( _DEF_PACK_LOGIN_RS ) = &CKernel::slot_dealLoginRs;
    NetPackMap( _DEF_PACK_FILE_INFO ) = &CKernel::slot_dealFileInfo;
    NetPackMap( _DEF_PACK_FILE_HEAD_RQ ) = &CKernel::slot_dealFileHeadRq;
    NetPackMap( _DEF_PACK_FILE_CONTENT_RQ ) = &CKernel::slot_dealFileContentRq;
    NetPackMap( _DEF_PACK_UPLOAD_FILE_RS ) = &CKernel::slot_dealUploadFileRs;
    NetPackMap( _DEF_PACK_FILE_CONTENT_RS ) = &CKernel::slot_dealFileContentRs;
    NetPackMap( _DEF_PACK_ADD_FOLDER_RS ) = &CKernel::slot_dealAddFolderRs;
    NetPackMap( _DEF_PACK_QUICK_UPLOAD_RS ) = &CKernel::slot_dealQuickUpdateRs;
    NetPackMap( _DEF_PACK_DELETE_FILE_RS ) = &CKernel::slot_dealDeleteFileRs;
    NetPackMap( _DEF_PACK_SHARE_FILE_RS ) = &CKernel::slot_dealShareFileRs;
    NetPackMap( _DEF_PACK_MY_SHARE_RS ) = &CKernel::slot_dealMyshareFileRs;
    NetPackMap( _DEF_PACK_GET_SHARE_RS ) = &CKernel::slot_dealGetshareFileRs;
    NetPackMap( _DEF_PACK_FOLDER_HEAD_RQ ) = &CKernel::slot_dealFolderHeadRq;
    NetPackMap( _DEF_PACK_CONTINUE_UPLOAD_RS ) = &CKernel::slot_dealContinueUploadRs;
}

//md5

#define MD5_KEY "1234"

string getMD5(QString val){

    QString str = QString("%1_%2").arg(val).arg(MD5_KEY);
    MD5 md(str.toStdString());
    qDebug()<<"md5:"<<md.toString().c_str();

    return md.toString();
}

string getFileMd5(QString path){

    //转码，打开文件
    char buf[1000];
    CKernel::utf8toGB2312(buf,1000,path);

    FILE * file = fopen(buf,"rb");
    if(!file){
        return string();
    }

    int len = 0;
    MD5 md;
    do{
        len = fread(buf, 1 , 1000 , file);
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
        md.update(buf,len);

    }while(len > 0);
    qDebug()<<md.toString().c_str();
    return md.toString();

}

CKernel::CKernel(QObject *parent) : QObject(parent),m_id(0),m_curDir("/"),m_quit(0)
{
    //初始化协议映射表
    setNetMap();

    //初始化配置文件
    setConfig();
    //申请sqlite对象
    m_sql = new CSqlite;

    //中介类类初始化
    m_TcpClient = new TcpClientMediator;
    //连接接收数据处理
    connect( m_TcpClient , SIGNAL(SIG_ReadyData(uint,char*,int))
             , this , SLOT(slot_ReadyData(uint,char*,int)) );
    m_TcpClient->OpenNet( m_ip.toStdString().c_str() , m_port );

    //主窗口
    m_mainDialog = new MainDialog;
    //连接主窗口结束信号槽
    connect(m_mainDialog,SIGNAL(SIG_close()),
            this,SLOT( DestoryInstance() ));
    connect( m_mainDialog , SIGNAL(SIG_downloadFile(int))
             , this , SLOT( slot_downloadFile(int) ) );
    //下载进度条
    connect(this,SIGNAL(SIG_updateFileProgress(int ,int))
            , m_mainDialog, SLOT( slot_updateFileProgress(int ,int) ) );

    qRegisterMetaType<FileInfo>("FileInfo");//进行注册
    connect(this,SIGNAL( SIG_insertComplete(FileInfo ) )
            , m_mainDialog , SLOT( slot_insertComplete(FileInfo ) ) );
    //上传进度条
    connect(this,SIGNAL(SIG_updateUploadFileProgress(int ,int))
            , m_mainDialog, SLOT( slot_updateUploadFileProgress(int ,int) ) );
    connect(this,SIGNAL( SIG_uploadFileinsertComplete(FileInfo ) )
            , m_mainDialog , SLOT( slot_uploadFileinsertComplete(FileInfo ) ) );

    connect(m_mainDialog, SIGNAL(SIG_uploadFile(QString)),
             this, SLOT(slot_uploadFile(QString)) );
    connect(m_mainDialog, SIGNAL(SIG_uploadFolder(QString,QString)),
             this, SLOT(slot_uploadFolder(QString,QString)) );

    connect(m_mainDialog,SIGNAL(SIG_addFolder(QString)) ,
            this,SLOT(slot_addFolder(QString)) );

    connect(m_mainDialog,SIGNAL(SIG_changeDir(QString)) ,
            this,SLOT(slot_changeDir(QString)) );

    connect(m_mainDialog,SIGNAL(SIG_deleteFile(QString , QVector<int>) ) ,
            this,SLOT(slot_deleteFile(QString , QVector<int>)) );
    connect(m_mainDialog,SIGNAL(SIG_sharedFile(QString , QVector<int>) ) ,
            this,SLOT(slot_sharedFile(QString , QVector<int>)) );

    connect(m_mainDialog,SIGNAL(SIG_getShareFile(QString , QString)),
            this,SLOT(slot_getShareFile(QString , QString)) );

    connect(m_mainDialog,SIGNAL(SIG_setUploadPauseStatus(int ,int ) ),
            this,SLOT(slot_setUploadPauseStatus(int ,int ) ) );
    connect(m_mainDialog,SIGNAL(SIG_setDownloadPauseStatus(int ,int ) ),
            this,SLOT(slot_setDownloadPauseStatus(int ,int ) ) );


    //m_mainDialog->show();//显示主窗口


    //登录注册窗口
    m_loginDialog = new LoginDialog;
    //关闭
    connect(m_loginDialog,SIGNAL(SIG_close()),
            this,SLOT(DestoryInstance()));
    //登录提交
    connect(m_loginDialog,SIGNAL(SIG_loginCommit(QString,QString)),
            this,SLOT(slot_loginCommit(QString,QString)));
    //注册提交
    connect(m_loginDialog,SIGNAL(SIG_registerCommit(QString,QString,QString)),
            this,SLOT(slot_registerCommit(QString,QString,QString)));
    m_loginDialog->show();



}

CKernel::~CKernel()
{

}



//回收槽函数
void CKernel::DestoryInstance()
{
    qDebug()<<__func__;
    m_TcpClient->CloseNet();//关闭网络
    delete m_TcpClient;//回收网络客户端
    delete m_mainDialog;//回收主窗口
    delete m_loginDialog;//回收登录注册窗口

    //退出死循环
    m_quit = 1;
}


//处理接收数据槽函数
void CKernel::slot_ReadyData(unsigned int lSendIP, char *buf, int nlen)
{
    int type = *(int *)buf;
    if(type >=_DEF_PROTOCOL_BASE && type < _DEF_PROTOCOL_BASE + _DEF_PROTOCOL_COUNT){
        PFUN pf = NetPackMap(type);
        if(pf)
            (this->*pf)(lSendIP, buf, nlen);
    }

    //注意：处理完数据后，要记得释放
    delete []buf;
}

void CKernel::SendData(char *buf, int nlen)
{
    m_TcpClient->SendData(0,buf,nlen);
}

#include<QCoreApplication>
#include<QSettings>
#include<QFileInfo>
#include<QDir>

void CKernel::setConfig()
{
    ///  windows *.ini   --> config.ini
    /// [net] 组名
    /// key = value 键值对
    /// ip = "192.128.123"
    /// port = 8004
    /// 保存在哪里 .exe文件下
    /// exe文件目录获取
    /// D:/kelin/Qt_Thrid_stage/project/PlayHall/build  (注意最后面没有右斜杠)

    QString path = QCoreApplication::applicationDirPath() + "/config.ini";
    QFileInfo info(path);//根据文件路径获取到文件，然后判断文件是否存在

    //设置默认端口和ip
    m_ip = "192.168.43.21";
    m_port = 8004;
    if(info.exists()){//有就读取
        //创建该对象，对访问的文件不存在会自动创建
        QSettings settings(path,QSettings::IniFormat,nullptr);//IniFormat:ini文件
        settings.beginGroup("net");
        //可以通过key获取到写入文件中对应的value
        QVariant strip = settings.value("ip","");//参数：（key，默认值）
        if(!strip.toString().isEmpty()) m_ip = strip.toString();
        QVariant strport = settings.value("port",0);
        if(strport.toInt() != 0) m_port = strport.toInt();
        settings.endGroup();

    }else{//没有就写入默认值
        QSettings settings(path,QSettings::IniFormat,nullptr);//IniFormat:ini文件
        settings.beginGroup( "net" ); //变量是组名
        settings.setValue("ip",m_ip);
        settings.setValue("port",m_port);
        settings.endGroup();

    }
    qDebug()<<"ip:"<<m_ip<<"port:"<<m_port;

    //查看是否有默认路径，exe同级路径（没有就创建一个）
    QString syspath = QCoreApplication::applicationDirPath() + "/NetDisk/";
    QDir dir; //用于判断该文件目录是否存在的，没有可以用它去创建
    if(!dir.exists(syspath)){
        dir.mkdir(syspath);
    }
    m_sysPath = QCoreApplication::applicationDirPath() + "/NetDisk";
}

void CKernel::utf8toGB2312(char *gbbuf, int nlen, QString &uft8)
{
    //设置成gb2312类型的
    QTextCodec* gb2312=QTextCodec::codecForName("gb2312");
    QByteArray ba=gb2312->fromUnicode(uft8);
    strcpy(gbbuf,ba.data());
}

QString CKernel::GB2321toutf8(char *gbbuf)
{
    //设置成gb2312类型的
    //论2312的重要性，写错了就会报“程序异常的错误”，然后调试到return的时候提示一个信号错误，很离谱
    QTextCodec* gb2312=QTextCodec::codecForName("gb2312");
    return gb2312->toUnicode(gbbuf);
}

void CKernel::slot_updateFileList()
{
    //删除所有文件信息
    m_mainDialog->slot_deleteAllFileInfo();
    STRU_FILE_LIST_RQ rq;
    rq.userId = m_id;
    std::string dir = m_curDir.toStdString();
    strcpy(rq.dir,dir.c_str());
    SendData((char *)&rq,sizeof(rq));
}



//处理注册提交槽函数
void CKernel::slot_registerCommit(QString tel, QString password, QString name)
{
    STRU_REGISTER_RQ rq;
    std::string telStr = tel.toStdString();
    strcpy_s(rq.tel,telStr.c_str());

    std::string passwordStr = getMD5(password)/*password.toStdString()*/;
    strcpy_s(rq.password,passwordStr.c_str());

    std::string nameStr = name.toStdString();
    strcpy_s(rq.name,nameStr.c_str());

    SendData((char *)&rq,sizeof(rq));
}
//处理登录提交槽函数
void CKernel::slot_loginCommit(QString tel, QString password)
{
    STRU_LOGIN_RQ rq;
    std::string telStr = tel.toStdString();
    strcpy_s(rq.tel,telStr.c_str());

    std::string passwordStr = getMD5(password)/*password.toStdString()*/;
    strcpy_s(rq.password,passwordStr.c_str());


    SendData((char *)&rq,sizeof(rq));
}

void CKernel::slot_downloadFile(int fileid)
{
    //封包
    STRU_DOWNLOAD_RQ rq;
    rq.fileid = fileid;
    rq.userid = m_id;
    std::string tmp = m_curDir.toStdString();
    strcpy(rq.dir,tmp.c_str());
    //发送
    SendData( (char *)&rq , sizeof(rq) );

}
#include<QFileInfo>
#include<QDateTime>
void CKernel::slot_uploadFile(QString path)
{
    qDebug()<<__func__;
    //创建文件信息结构体
    QFileInfo fileinfo(path);
    FileInfo info;
    info.absolutePath = path;
    info.dir = m_curDir;
    //根据文件内容获取到该文件的md5
    info.md5 = QString::fromStdString(getFileMd5(path));
    info.name = fileinfo.fileName();
    info.size = fileinfo.size();
    info.time = QDateTime::currentDateTime()
            .toString("yyyy-MM-dd hh:mm:ss");
    info.type = "file";

    //打开文件获取文件描述符
    char pathbuf[1000] = "";
    utf8toGB2312(pathbuf,1000,path);
    info.pFile = fopen(pathbuf,"rb");
    if(!info.pFile){
        qDebug()<<"打开失败";
        return ;
    }

    //添加到map中
    m_mapFileMD5ToFileInfo[info.md5.toStdString()] = info;

    //封包
    STRU_UPLOAD_FILE_HEAD_RQ rq;
    strcpy(rq.dir,info.dir.toStdString().c_str());
    string name = info.name.toStdString();
    strcpy(rq.fileName,name.c_str());
    strcpy(rq.fileType,info.type.toStdString().c_str());
    strcpy(rq.md5,info.md5.toStdString().c_str());
    rq.size = info.size;
    strcpy(rq.time , info.time.toStdString().c_str());
    rq.userid = m_id;
    //发送文件
    SendData((char *)&rq,sizeof(rq));
}

void CKernel::slot_uploadFile(QString path, QString dir)
{
    qDebug()<<__func__;
    //创建文件信息结构体
    QFileInfo fileinfo(path);
    FileInfo info;
    info.absolutePath = path;
    info.dir = dir;
    //根据文件内容获取到该文件的md5
    info.md5 = QString::fromStdString(getFileMd5(path));
    info.name = fileinfo.fileName();
    info.size = fileinfo.size();
    info.time = QDateTime::currentDateTime()
            .toString("yyyy-MM-dd hh:mm:ss");
    info.type = "file";

    //打开文件获取文件描述符
    char pathbuf[1000] = "";
    utf8toGB2312(pathbuf,1000,path);
    info.pFile = fopen(pathbuf,"rb");
    if(!info.pFile){
        qDebug()<<"打开失败";
        return ;
    }

    //添加到map中
    m_mapFileMD5ToFileInfo[info.md5.toStdString()] = info;

    //封包
    STRU_UPLOAD_FILE_HEAD_RQ rq;
    strcpy(rq.dir,info.dir.toStdString().c_str());
    string name = info.name.toStdString();
    strcpy(rq.fileName,name.c_str());
    strcpy(rq.fileType,info.type.toStdString().c_str());
    strcpy(rq.md5,info.md5.toStdString().c_str());
    rq.size = info.size;
    strcpy(rq.time , info.time.toStdString().c_str());
    rq.userid = m_id;
    //发送文件
    SendData((char *)&rq,sizeof(rq));
}

void CKernel::slot_uploadFolder(QString path,QString dir)
{
    qDebug()<<__func__;
    QFileInfo info(path);
    QDir dr(path);
    //先创建该文件夹
    slot_addFolder(info.fileName(),dir);
    //获取该文件目录下的所有文件夹及文件信息
    QFileInfoList lst = dr.entryInfoList();
    dir += info.fileName() + "/";
    for(int i = 0;i<lst.size();i++){
        const QFileInfo & file = lst.at(i);
        if(file.fileName() == ".") continue;
        if(file.fileName() == "..") continue;
        if(file.isFile()){
            slot_uploadFile(file.absoluteFilePath(),dir);
        }
        if(file.isDir()){
            slot_uploadFolder(file.absoluteFilePath(),dir);
        }
    }

}

void CKernel::slot_addFolder(QString name)
{
    qDebug()<<__func__;
    //封包
    STRU_ADD_FOLDER_RQ rq;
    strcpy(rq.dir,m_curDir.toStdString().c_str());
    string sname = name.toStdString();
    strcpy(rq.fileName,sname.c_str());
    strcpy(rq.fileType,"dir");
    QString time = QDateTime::currentDateTime()
            .toString("yyyy-MM-dd hh:mm:ss");
    strcpy(rq.time , time.toStdString().c_str());
    rq.userid = m_id;

    //发送文件
    SendData((char *)&rq,sizeof(rq));
}

void CKernel::slot_addFolder(QString name, QString dir)
{
    qDebug()<<__func__;
    //封包
    STRU_ADD_FOLDER_RQ rq;
    strcpy(rq.dir,dir.toStdString().c_str());
    string sname = name.toStdString();
    strcpy(rq.fileName,sname.c_str());
    strcpy(rq.fileType,"dir");
    QString time = QDateTime::currentDateTime()
            .toString("yyyy-MM-dd hh:mm:ss");
    strcpy(rq.time , time.toStdString().c_str());
    rq.userid = m_id;

    //发送文件
    SendData((char *)&rq,sizeof(rq));
}

void CKernel::slot_changeDir(QString path)
{
    m_curDir = path;
    slot_updateFileList();
}

void CKernel::slot_deleteFile(QString path, QVector<int> fileIdArray)
{
    //根据fid数量确定数组的大小，进而确定结构体的大小
    int len = sizeof(STRU_DELETE_FILE_RQ) + sizeof(int)*fileIdArray.size();
    STRU_DELETE_FILE_RQ *rq = (STRU_DELETE_FILE_RQ *)malloc( len );
    //因为没有构造函数，所以先执行下初始化函数
    rq->init();
    std::string strPath = path.toStdString();
    strcpy(rq->dir,strPath.c_str());
    rq->fileCount = fileIdArray.size();
    rq->userid = m_id;
    for(int i = 0 ; i < rq->fileCount ; i++){
       rq->fileidArray[i] = fileIdArray[i];
    }
    //发送大小不能为sizeof（），因为存在柔性数组
    SendData((char *)rq,len);

    //别忘了释放
    free(rq);
}

void CKernel::slot_sharedFile(QString path, QVector<int> fileIdArray)
{
    qDebug()<<__func__;
    //根据fid数量确定数组的大小，进而确定结构体的大小

    int len = sizeof(STRU_SHARE_FILE_RQ) + fileIdArray.size()*sizeof(int);
    STRU_SHARE_FILE_RQ *rq = (STRU_SHARE_FILE_RQ *)malloc( len );
    qDebug()<<"len:"<<len;
    //因为没有构造函数，所以先执行下初始化函数
    rq->init();
    std::string strPath = path.toStdString();
    strcpy(rq->dir,strPath.c_str());
    rq->itemCount = fileIdArray.size();
    rq->userid = m_id;
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    strcpy(rq->shareTime,time.toStdString().c_str());
    qDebug()<<len;
    qDebug()<<rq->itemCount;
    for(int i = 0 ; i < rq->itemCount ; i++){
       rq->fileidArray[i] = fileIdArray[i];
       qDebug()<<fileIdArray[i];
    }
    //发送大小不能为sizeof（），因为存在柔性数组
    SendData((char *)rq,len);

    //别忘了释放
    free(rq);
}

void CKernel::slot_refreshMyShare()
{
    qDebug()<<__func__;
    STRU_MY_SHARE_RQ rq;
    rq.userid = m_id;

    SendData((char *)&rq,sizeof (rq));
}

void CKernel::slot_getShareFile(QString link, QString dir)
{
    //封包
    STRU_GET_SHARE_RQ rq;
    //设置包内容
    string strDir = dir.toStdString();
    strcpy(rq.dir,strDir.c_str());
    rq.shareLink = link.toInt();
    rq.userid = m_id;

    SendData((char *)&rq,sizeof(rq));
}

void CKernel::slot_setUploadPauseStatus(int fileId, int status)
{
    //map file正常有，可以进行暂停和恢复
    if(m_mapFileIdToFileInfo.count(fileId) > 0){

        m_mapFileIdToFileInfo[fileId].isPause = status;

    }else{
        //没有 ， 断点续传：把上传的信息写入到数据库，程序启动登录后加载到上传列表中，只可以继续
        if(status == 0){
            //断点续传..
            //获取文件信息
            FileInfo fileInfo =m_mainDialog->slot_getUploadFileinfobyFileid(fileId);

            char tmpName[_MAX_PATH_SIZE];
            memset(tmpName,0,_MAX_PATH_SIZE);

            utf8toGB2312(tmpName,fileInfo.absolutePath.size(),fileInfo.absolutePath);
            //打开文件
            fileInfo.pFile = fopen(tmpName,"rb");//二进制追加打开，wb就清空了
            if(!fileInfo.pFile){
               qDebug()<<"打开文件失败："<<fileInfo.absolutePath;
               return;
            }
            //避免继续之后还卡在那里,状态没有改变还是0，所以要改
            fileInfo.isPause = 0;
            //插入到map中
            m_mapFileIdToFileInfo[fileInfo.fileid] = fileInfo;
            //发送下载断点请求
            STRU_CONTINUE_UPLOAD_RQ rq;
            rq.fileid = fileInfo.fileid;
            rq.userid = m_id;
            string tmpdir = fileInfo.dir.toStdString();//防止有中文
            strcpy(rq.dir,tmpdir.c_str());
            SendData((char *)&rq,sizeof(rq));
        }

    }
}

void CKernel::slot_setDownloadPauseStatus(int fileId, int status)
{
    qDebug()<<__func__;
    //map file正常有，可以进行暂停和恢复
    if(m_mapFileIdToFileInfo.count(fileId) > 0){

        m_mapFileIdToFileInfo[fileId].isPause = status;

    }else{
        //没有 ， 断点续传：把下载的信息写入到数据库，程序启动登录后加载到下载列表中，只可以继续
        if(status == 0){
            //断点续传..
            //获取文件信息
            FileInfo fileInfo =m_mainDialog->slot_getDownloadFileinfobyFileid(fileId);

            char tmpName[_MAX_PATH_SIZE];
            memset(tmpName,0,_MAX_PATH_SIZE);

            utf8toGB2312(tmpName,fileInfo.absolutePath.size(),fileInfo.absolutePath);
            //打开文件
            fileInfo.pFile = fopen(tmpName,"ab");//二进制追加打开，wb就清空了
            if(!fileInfo.pFile){
               qDebug()<<"打开文件失败："<<fileInfo.absolutePath;
               return;
            }
            //避免继续之后还卡在那里,状态没有改变还是0，所以要改
            fileInfo.isPause = 0;
            //插入到map中
            m_mapFileIdToFileInfo[fileInfo.fileid] = fileInfo;
            //发送下载断点请求
            STRU_CONTINUE_DOWNLOAD_RQ rq;
            rq.fileid = fileInfo.fileid;
            rq.userid = m_id;
            rq.pos = fileInfo.pos;
            string tmpdir = fileInfo.dir.toStdString();//防止有中文
            strcpy(rq.dir,tmpdir.c_str());
            SendData((char *)&rq,sizeof(rq));
        }

    }
}
//注册请求的结果
//#define tel_is_exist		(0)
//#define name_is_exist		(1)
//#define register_success	(2)

//处理注册回复
void CKernel::slot_dealRegisterRs(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    //拆包
    STRU_REGISTER_RS * rs = (STRU_REGISTER_RS *)buf;
    //根据不同结果显示不同提示
    switch(rs->result){
    case tel_is_exist:
        QMessageBox::about(m_loginDialog,"注册提示","该手机号已注册");
        break;
    case name_is_exist:
        QMessageBox::about(m_loginDialog,"注册提示","该昵称已存在");
        break;
    case register_success:
        QMessageBox::about(m_loginDialog,"注册提示","注册成功");
        break;
    }

}

//登录请求的结果
//#define user_not_exist		(0)
//#define password_error		(1)
//#define login_success		(2)
//处理登录回复
void CKernel::slot_dealLoginRs(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    //拆包
    STRU_LOGIN_RS * rs = (STRU_LOGIN_RS *)buf;
    //根据不同结果显示不同提示
    switch(rs->result){
    case user_not_exist:
        QMessageBox::about(m_loginDialog,"注册提示","该用户不存在");
        break;
    case password_error:
        QMessageBox::about(m_loginDialog,"注册提示","密码错误");
        break;
    case login_success:

        //ui界面切换 前台
        m_loginDialog->hide();
        m_mainDialog->show();

        //后台
        m_name = rs->name;//记录用户名和用户id
        m_id = rs->userid;

        m_mainDialog->slot_setInfo(m_name);//设置主界面中的用户姓名

        //登录成功 追加请求
        //获取根目录 “/” 下所有文件
        m_curDir = "/";
        //更新目录
        slot_updateFileList();
        //更新分享列表
        slot_refreshMyShare();
        //添加断点续传内容
        initDatabase(m_id);
        break;
    }
}

void CKernel::slot_dealFileInfo(unsigned int lSendIP, char *buf, int nlen)
{
    //拆包
    STRU_FILE_INFO * info = (STRU_FILE_INFO *)buf;

    //判断该文件是否为当前路径下文件（有可能出现已经换文件了，但是上个文件目录下的文件信息才来，导致加载错文件目录）
    if(info->dir == m_curDir){
        FileInfo fileInfo;

        fileInfo.fileid = info->fileId;
        fileInfo.size = info->size;
        fileInfo.name = QString::fromStdString(info->fileName);
        fileInfo.time = QString::fromStdString(info->uploadTIme);
        fileInfo.md5  = QString::fromStdString(info->md5);
        fileInfo.type = QString::fromStdString(info->fileType);

        //更新到文件列表表格控件中
        m_mainDialog->slot_InsertFileInfo( fileInfo );
    }





}

void CKernel::slot_dealFileHeadRq(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    //拆包
    STRU_FILE_HEAD_RQ *rq = (STRU_FILE_HEAD_RQ *) buf;

    //获取文件信息
    FileInfo fileInfo;

    fileInfo.fileid = rq->fileid;
    fileInfo.size = rq->size;
    fileInfo.name = QString::fromStdString(rq->fileName);
    fileInfo.md5  = QString::fromStdString(rq->md5);
    fileInfo.type = QString::fromStdString(rq->fileType);
    fileInfo.dir = QString::fromStdString(rq->dir);
    char tmpName[_MAX_PATH_SIZE];
    memset(tmpName,0,_MAX_PATH_SIZE);

    fileInfo.absolutePath = m_sysPath + fileInfo.dir + fileInfo.name;

    QDir dir;
    //获取路径文件夹名集合
    QStringList subStr = fileInfo.dir.split("/");
    QString pathSum = m_sysPath + "/";//每个文件夹的路径
    for(int i =0 ; i < subStr.size() ; i++){
        // /0314/ split在分割的时候首尾有两个空格，所以要判断下是否为空
        if(subStr.at(i).isEmpty()) continue;
        //拼路径上文件夹路径
        pathSum +=subStr.at(i) + "/";
        //判断该文件夹是否存在，不存在就创建
        if(!dir.exists(pathSum)){
            dir.mkdir(pathSum);
        }
    }

    utf8toGB2312(tmpName,fileInfo.absolutePath.size(),fileInfo.absolutePath);
    //打开文件
    fileInfo.pFile = fopen(tmpName,"wb");//二进制写的方式打开，加个二进制
    if(!fileInfo.pFile){
       qDebug()<<"打开文件失败："<<fileInfo.absolutePath;
       return;
    }
    //插入断点信息
    slot_writeDownloadTask(fileInfo);
    //打开文件成功，创建下载信息（进度条）
    m_mainDialog->slot_InsertDownLoadFile(fileInfo);
    //插入到map中
    m_mapFileIdToFileInfo[rq->fileid] = fileInfo;
    STRU_FILE_HEAD_RS rs;
    rs.fileid = rq->fileid;
    rs.userid = m_id;
    rs.result = 1;
    //发送回复
    SendData((char *)&rs , sizeof(rs));
    qDebug()<<"--------------";
}
#include<QThread>
void CKernel::slot_dealFileContentRq(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    //拆包
    STRU_FILE_CONTENT_RQ *rq = (STRU_FILE_CONTENT_RQ *)buf;
    qDebug()<<"文件id："<<rq->fileid;

    if(m_mapFileIdToFileInfo.count(rq->fileid) == 0 ) return;

    //获取文件信息
    FileInfo & info = m_mapFileIdToFileInfo[rq->fileid];
    while(info.isPause){
        QThread::msleep(100);
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
        //程序退出时，退出循环
        if(m_quit) return;
    }
    //向文件中写入
    int len = fwrite(rq->content,1,rq->len,info.pFile);
    qDebug()<<len;

    STRU_FILE_CONTENT_RS rs;

    if(len != rq->len){
        //写入失败,跳回写入之前的位置
        fseek(info.pFile,-1*len,SEEK_CUR);
        rs.result = 0;
    }else{
        //写入成功
        info.pos +=len;
        rs.result = 1;
        //写入成功，发送给主窗口目前写入多少
        Q_EMIT SIG_updateFileProgress(info.fileid,info.pos);
    }
    //已经全部写入
    //qDebug()<<"pos:"<<info.pos<<"size:"<<info.size;
    if(info.pos == info.size){
        fclose(info.pFile);
        //删除记录的下载断点续传信息
        slot_deleteDownloadTask(info);
        m_mapFileIdToFileInfo.erase(info.fileid);
        qDebug()<<"fasong";
        //完成之后，将文件信息添加到已完成目录中
//        QVariant DataInfo;
//        FileInfo tmp = info;
//        DataInfo.setValue(tmp);

        //Q_EMIT SIG_insertComplete(info);
    }
    qDebug()<<info.pos<<"size:"<<info.size;
    rs.fileid = rq->fileid;
    rs.userid = rq->userid;
    rs.len = rq->len;

    SendData((char *)&rs,sizeof(rs));


}

void CKernel::slot_dealUploadFileRs(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    //拆包
    STRU_UPLOAD_FILE_HEAD_RS * rs = (STRU_UPLOAD_FILE_HEAD_RS *)buf;
    //添加到fid对应的映射
    m_mapFileIdToFileInfo[rs->fileid] = m_mapFileMD5ToFileInfo[rs->md5];
    //获取文件信息
    FileInfo & info = m_mapFileIdToFileInfo[rs->fileid];
    //之前的fid没有赋值，不赋值会导致上传文件进度条出错
    info.fileid = rs->fileid;
    //删除md5中对应的映射
    if(m_mapFileMD5ToFileInfo.count(rs->md5) != 0){
        m_mapFileMD5ToFileInfo.erase(rs->md5);
    }
    //添加上传的，断点续传信息
    slot_writeUploadTask(info);
    //添加进度条到上传界面
    m_mainDialog->slot_InsertUpLoadFile(info);

    STRU_FILE_CONTENT_RQ rq;
    //读文件内容
    rq.len = fread(rq.content,1,_DEF_BUFFER,info.pFile);

    rq.fileid = rs->fileid;
    rq.userid = rs->userid;
    SendData((char *)&rq,sizeof(rq));

}

void CKernel::slot_dealFileContentRs(unsigned int lSendIP, char *buf, int nlen)
{
    //qDebug()<<__func__;
    //拆包
    STRU_FILE_CONTENT_RS * rs = (STRU_FILE_CONTENT_RS *)buf;

    //获取文件信息
    if(m_mapFileIdToFileInfo.count(rs->fileid) == 0) return;
    FileInfo & info = m_mapFileIdToFileInfo[rs->fileid];
    //暂停循环
    while(info.isPause){
        QThread::msleep(100);
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
        //程序退出时，退出循环
        if(m_quit) return;
    }
    //写入失败
    if(rs->result == 0){
        fseek(info.pFile,-rs->len,SEEK_CUR);
    //成功
    }else{
        info.pos +=rs->len;
        Q_EMIT SIG_updateUploadFileProgress(info.fileid,info.pos);
        if(info.pos >= info.size){
            //判断该上传文件是否为当前路径下文件，是就更新目录
            if(info.dir == m_curDir)
                slot_updateFileList();
            //关文件描述符，删节点
            fclose(info.pFile);
            //删除上传的断点信息
            slot_deleteUploadTask(info);
            m_mapFileIdToFileInfo.erase(rs->fileid);
            //上传完成，在完成列表添加控件
            Q_EMIT SIG_uploadFileinsertComplete(info);
            return;
        }
    }

    STRU_FILE_CONTENT_RQ rq;
    //读文件内容
    rq.len = fread(rq.content,1,_DEF_BUFFER,info.pFile);

    rq.fileid = rs->fileid;
    rq.userid = rs->userid;
    SendData((char *)&rq,sizeof(rq));
}

void CKernel::slot_dealAddFolderRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_ADD_FOLDER_RS *rs = (STRU_ADD_FOLDER_RS *)buf;
    if(rs->result == 1){
        slot_updateFileList();
    }
}

void CKernel::slot_dealQuickUpdateRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_QUICK_UPLOAD_RS *rs = (STRU_QUICK_UPLOAD_RS *)buf;
    if(rs->result == 0) return;
    //根据md5，获取info
    if(m_mapFileMD5ToFileInfo.count(rs->md5) == 0) return;
    FileInfo &info = m_mapFileMD5ToFileInfo[rs->md5];
    //更新当前列表
    slot_updateFileList();
    //添加到上传目录下
    m_mainDialog->slot_uploadFileinsertComplete(info);
    m_mapFileMD5ToFileInfo.erase(rs->md5);
}

void CKernel::slot_dealDeleteFileRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_DELETE_FILE_RS * rs = (STRU_DELETE_FILE_RS *)buf;
    //删除失败
    if(rs->result == 0) return;
    //是当前目录，更新下
    if(QString::fromStdString(rs->dir) == m_curDir){
        slot_updateFileList();
    }
}

void CKernel::slot_dealShareFileRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_SHARE_FILE_RS *rs = (STRU_SHARE_FILE_RS *)buf;

    if(rs->result == 0) return;
    //刷新分享列表
    //删除所有信息
    m_mainDialog->slot_deleteSharedAllFileInfo();

    //获取分享列表
    slot_refreshMyShare();
}

void CKernel::slot_dealMyshareFileRs(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    STRU_MY_SHARE_RS * rs = (STRU_MY_SHARE_RS *)buf;
    int len = rs->itemCount;
    for(int i = 0;i<len ;i++){
        //插入每个文件信息
        qDebug()<<rs->items[i].name<<" "<<rs->items[i].size<<" "<<rs->items[i].time<<" "<<rs->items[i].shareLink<<endl;
        m_mainDialog->slot_insertSharedFile(rs->items[i].name,rs->items[i].size,rs->items[i].time,rs->items[i].shareLink);
    }
}

void CKernel::slot_dealGetshareFileRs(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_GET_SHARE_RS * rs = (STRU_GET_SHARE_RS *)buf;
    //获取失败
    if(rs->result == 0) return;
    //判断是否为当前路径，是就刷新
    if(rs->dir == m_curDir)
        slot_updateFileList();
}

void CKernel::slot_dealFolderHeadRq(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_FOLDER_HEAD_RQ * rq = (STRU_FOLDER_HEAD_RQ *)buf;
    //先创建该路径上没有的文件夹
    QDir dir;
    //获取路径文件夹名集合
    QStringList subStr = QString::fromStdString( rq->dir).split("/");
    QString pathSum = m_sysPath + "/";//每个文件夹的路径
    for(int i =0 ; i < subStr.size() ; i++){
        // /0314/ split在分割的时候首尾有两个空格，所以要判断下是否为空
        if(subStr.at(i).isEmpty()) continue;
        //拼路径上文件夹路径
        pathSum +=subStr.at(i) + "/";
        //判断该文件夹是否存在，不存在就创建
        if(!dir.exists(pathSum)){
            dir.mkdir(pathSum);
        }
    }
    //创建该文件夹
    pathSum += QString::fromStdString( rq->fileName) + "/";
    //判断该文件夹是否存在，不存在就创建
    if(!dir.exists(pathSum)){
        dir.mkdir(pathSum);
    }

}

void CKernel::slot_dealContinueUploadRs(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    STRU_CONTINUE_UPLOAD_RS * rs = (STRU_CONTINUE_UPLOAD_RS *)buf;
    if(m_mapFileIdToFileInfo.count(rs->fileid) == 0) return;
    FileInfo &info = m_mapFileIdToFileInfo[rs->fileid];
    //设置pos，设置进度条，设置文件流位置
    info.pos = rs->pos;
    fseek(info.pFile,rs->pos,SEEK_SET);
    m_mainDialog->slot_updateUploadFileProgress(rs->fileid,rs->pos);
    STRU_FILE_CONTENT_RQ rq;
    //读文件内容
    rq.len = fread(rq.content,1,_DEF_BUFFER,info.pFile);

    rq.fileid = rs->fileid;
    rq.userid = m_id;
    SendData((char *)&rq,sizeof(rq));

}

void CKernel::initDatabase( int id )
{
    QString path = QCoreApplication::applicationDirPath() +"/database/";
    //先看路径再看文件
    QDir dr;
    if( !dr.exists(path) ){
        dr.mkdir( path );
    }
    path = path + QString("%1.db").arg( id );
    QFileInfo info( path );
    //在exe同级下面  查看有没有这个文件
    if( info.exists() ){
        //存在  //有 加载数据库
        m_sql->ConnectSql( path );

        //加载终端内容
        QList<FileInfo> uploadList;
        QList<FileInfo> downList;
        //从数据库中获取终端数据
        slot_getUploadTask(uploadList);
        slot_getDownloadTask(downList);

        //获取上传信息，并添加到上传界面中
        for(FileInfo & info:uploadList){
            QFileInfo fi(info.absolutePath);
            if(!fi.exists()){
                continue;   //上传文件不存在，取消该续传
            }
            info.isPause = 1;
            m_mainDialog->slot_InsertUpLoadFile(info);

            //这里可以发送协议获取当前上传到的pos
            //请求协议内容，userid，fileid，dir确定一个文件，如果发现服务器中fileinfo的map映射以及删除，可以根据这三个条件再创建一个info添加到映射中
            //回复内容，pos,fileid，通过fileid找到对应空间，通过pos更新，可以通过update。。。函数去更新控件进度条信息
        }
        //获取下载信息，并添加到下载界面中
        for(FileInfo & info:downList){
            QFileInfo fi(info.absolutePath);
            if(!fi.exists()){
                continue;   //下载文件不存在，取消该续传
            }
            info.isPause = 1;
            info.pos = fi.size();
            m_mainDialog->slot_InsertDownLoadFile(info);
            //更新进度条
            m_mainDialog->slot_updateFileProgress(info.fileid,info.pos);

        }


    }else{
        //没有 创建
        QFile file(path);
        if( !file.open(QIODevice::WriteOnly) ) return;
        file.close();

        m_sql->ConnectSql( path );
        //写表
        QString sqlstr = "create table t_upload ( f_id int, f_name varchar(260),f_dir varchar(260),f_absolutePath varchar(260),f_size int,f_md5 varchar(40),f_time varchar(40),f_type varchar(10) );";

        m_sql->UpdateSql( sqlstr );

        sqlstr = "create table t_download ( f_id int, f_name varchar(260),f_dir varchar(260),f_absolutePath varchar(260),f_size int,f_md5 varchar(40),f_time varchar(40),f_type varchar(10) );";

        m_sql->UpdateSql( sqlstr );
    }

}

//写上传任务
void CKernel::slot_writeUploadTask(FileInfo &info)
{
    QString sqlstr = QString("insert into t_upload values(%1,'%2','%3','%4',%5,'%6','%7', '%8');").arg(info.fileid).arg(info.name).arg(info.dir).arg(info.absolutePath).arg(info.size).arg(info.md5).arg(info.time).arg(info.type);

    m_sql->UpdateSql( sqlstr );

}
//写下载任务
void CKernel::slot_writeDownloadTask(FileInfo &info)
{
    QString sqlstr = QString("insert into t_download values(%1,'%2','%3','%4',%5,'%6','%7', '%8');").arg(info.fileid).arg(info.name).arg(info.dir).arg(info.absolutePath).arg(info.size).arg(info.md5).arg(info.time).arg(info.type);

    m_sql->UpdateSql( sqlstr );
}

void CKernel::slot_deleteUploadTask(FileInfo &info)
{
    QString sqlstr = QString("delete from t_upload where f_id = %1 and f_dir= '%2';").arg( info.fileid) .arg(info.dir );

    m_sql->UpdateSql( sqlstr );

}

void CKernel::slot_deleteDownloadTask(FileInfo &info)
{
    QString sqlstr = QString("delete from t_download where f_id = %1 and f_dir= '%2' and f_absolutePath = '%3';").arg( info.fileid) .arg(info.dir ).arg(info.absolutePath);

    m_sql->UpdateSql( sqlstr );
}

//加载上传任务
void CKernel::slot_getUploadTask(QList<FileInfo> &infoList)
{
    QString sqltr = "select * from t_upload;";
    QStringList lst;
    m_sql->SelectSql( sqltr , 8, lst);

    while( lst.size() !=0)
    {
        FileInfo info;
        info.fileid = lst.front().toInt() ; lst.pop_front();
        info.name = lst.front();lst.pop_front();
        info.dir = lst.front();lst.pop_front();
        info.absolutePath = lst.front();lst.pop_front();
        info.size = lst.front().toInt() ; lst.pop_front();
        info.md5 = lst.front();lst.pop_front();
        info.time = lst.front();lst.pop_front();
        info.type = lst.front();lst.pop_front();

        infoList.push_back( info );
    }
}

//加载下载任务
void CKernel::slot_getDownloadTask(QList<FileInfo> &infoList)
{
    QString sqltr = "select * from t_download;";
    QStringList lst;
    m_sql->SelectSql( sqltr , 8, lst);

    while( lst.size() !=0)
    {

        FileInfo info;
        info.fileid = lst.front().toInt() ; lst.pop_front();
        info.name = lst.front();lst.pop_front();
        info.dir = lst.front();lst.pop_front();
        info.absolutePath = lst.front();lst.pop_front();
        info.size = lst.front().toInt() ; lst.pop_front();
        info.md5 = lst.front();lst.pop_front();
        info.time = lst.front();lst.pop_front();
        info.type = lst.front();lst.pop_front();

        infoList.push_back( info );
    }
}

