#ifndef CKERNEL_H
#define CKERNEL_H

#include <QObject>
#include"maindialog.h"
#include<qdebug.h>
#include"INetMediator.h"
#include"packdef.h"
#include"logindialog.h"
#include<QTextCodec>
#include"csqlite.h"
#include"common.h"

class CKernel;
//类函数指针类型
typedef void (CKernel::*PFUN)( unsigned int lSendIP , char* buf , int nlen );

//单例：不会无缘无故新建一个
class CKernel : public QObject
{
    Q_OBJECT
public:
    static CKernel * GetInstance();

    static void utf8toGB2312(char *gbbuf,int nlen,QString &uft8);

    static QString GB2321toutf8(char *gbbuf);

signals:

public slots:
    //控件处理
    //更新文件目录
    void slot_updateFileList();

    //回收核心类资源槽函数
    void DestoryInstance();
    //注册提交槽函数
    void slot_registerCommit( QString tel , QString password , QString name);
    //登录提交槽函数
    void slot_loginCommit( QString tel , QString password);

    void slot_downloadFile(int fileid);
    //上传文件槽函数
    void slot_uploadFile(QString path);
    void slot_uploadFile(QString path,QString dir);
    //上传文件夹槽函数
    void slot_uploadFolder(QString path,QString dir);
    //新建文件夹槽函数
    void slot_addFolder(QString name);
    void slot_addFolder(QString name,QString dir);
    //路径跳转
    void slot_changeDir(QString);
    //删除处理
    void slot_deleteFile(QString path, QVector<int> fileIdArray);

    void slot_sharedFile(QString path, QVector<int> fileIdArray);
    //获取分享列表
    void slot_refreshMyShare();
    //获取分享文件
    void slot_getShareFile(QString link, QString dir);
    //上传暂停槽函数
    void slot_setUploadPauseStatus(int fileId,int status);
    //下载暂停槽函数
    void slot_setDownloadPauseStatus(int fileId,int status);

    //网络处理
    //处理接收数据槽函数
    void slot_ReadyData( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealRegisterRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealLoginRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealFileInfo( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealFileHeadRq( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealFileContentRq( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealUploadFileRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealFileContentRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealAddFolderRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealQuickUpdateRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealDeleteFileRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealShareFileRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealMyshareFileRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealGetshareFileRs( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealFolderHeadRq( unsigned int lSendIP , char* buf , int nlen );
    void slot_dealContinueUploadRs( unsigned int lSendIP , char* buf , int nlen );

signals:
    void SIG_updateFileProgress(int ,int);
    void SIG_insertComplete(FileInfo info);

    void SIG_updateUploadFileProgress(int ,int);
    void SIG_uploadFileinsertComplete(FileInfo info);

private:
    void SendData(char* buf , int nlen);
    void setConfig();//设置配置文件
    //QString->GB2312

    //Sqlite对应使用的函数
    void initDatabase(int id);
    void slot_writeUploadTask( FileInfo& info );
    void slot_writeDownloadTask( FileInfo& info );

    void slot_deleteUploadTask( FileInfo& info );
    void slot_deleteDownloadTask( FileInfo& info );

    void slot_getUploadTask( QList<FileInfo> &infoList );
    void slot_getDownloadTask(QList<FileInfo> &infoList );


private:
    explicit CKernel(QObject *parent = 0);
    ~CKernel();
    CKernel(const CKernel & kernel){}
    CKernel & operator = (const CKernel & kernel){
        return *this;
    }

    //创建协议映射表函数
    void setNetMap();

    //成员属性 网络对象 ui对象
    static CKernel * kernel;
    MainDialog *m_mainDialog;//主窗口类
    INetMediator * m_TcpClient;
    LoginDialog * m_loginDialog;
    PFUN m_netPackMap[_DEF_PROTOCOL_COUNT];

    //当前所在路径
    QString m_curDir;

    QString m_name; // 昵称
    int m_id; //uid


    std::map<int,FileInfo> m_mapFileIdToFileInfo;

    std::map<std::string,FileInfo> m_mapFileMD5ToFileInfo;

    QString m_sysPath; //这个要在那个设置配置文件函数中写，（.ini文件中写）

    QString m_ip;
    int m_port;
    int m_quit;//程序退出标志位

    CSqlite * m_sql;//sqlite数据库分装对象

};

//

#endif // CKERNEL_H
