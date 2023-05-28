#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include<QCloseEvent>
#include"common.h"
#include"mytablewightitem.h"
#include<QMenu>
namespace Ui {
class MainDialog;
}

class MainDialog : public QDialog
{
    Q_OBJECT
signals:
    void SIG_close();

    void SIG_downloadFile(int fileid);

    void SIG_uploadFile(QString);
    void SIG_uploadFolder(QString,QString);

    void SIG_addFolder(QString);
    void SIG_changeDir(QString);
    void SIG_deleteFile(QString , QVector<int>);
    void SIG_sharedFile(QString , QVector<int>);
    void SIG_getShareFile(QString , QString);
    void SIG_setUploadPauseStatus(int ,int );
    void SIG_setDownloadPauseStatus(int ,int );

public:
    explicit MainDialog(QWidget *parent = 0);
    ~MainDialog();

    void closeEvent(QCloseEvent *);//点击右上角的叉，触发窗口的结束事件。

public slots:
    //设置用户信息（用户名）
    void slot_setInfo(QString name);
    //删除所有文件目录中的控件
    void slot_deleteAllFileInfo();

    //删除分享列表中所有文件目录中的控件
    void slot_deleteSharedAllFileInfo();

    void slot_InsertFileInfo(FileInfo & info);
    //设置下载的进度条
    void slot_InsertDownLoadFile(FileInfo & info);
    //设置上传的进度条
    void slot_InsertUpLoadFile(FileInfo & info);
    //接收右键信号槽函数
    void slot_menuShow(QPoint point);
    //接收点击右键菜单发送的槽函数
    void slot_dealTriggered(QAction* action);
    //接收上传页面点击右键菜单发送的槽函数
    void slot_dealUploadTriggered(QAction* action);
    //接收下载页面点击右键菜单发送的槽函数
    void slot_dealDownloadTriggered(QAction* action);
    //接收上传页面右键信号槽函数
    void slot_menuUploadShow(QPoint point);
    //接收下载页面右键信号槽函数
    void slot_menuDownloadShow(QPoint point);
    //更新下载进度条槽函数
    void slot_updateFileProgress(int fileId,int pos);
    //
    void slot_updateUploadFileProgress(int fileId,int pos);
    //下载文件完成添加到已完成目录
    void slot_insertComplete(FileInfo info);
    //
    void slot_uploadFileinsertComplete(FileInfo info);
    //点击按键获取打开下载完的文件所在目录并选中该文件
    void slot_folderButtonClick();

    void slot_dealMuneAddFile(QAction* action);

    //插入文件信息到分享列表中
    void slot_insertSharedFile(QString name ,int size ,QString time ,int shareLink );

    //根据fileid从控件中获取信息
    //从下载列表中
    FileInfo & slot_getDownloadFileinfobyFileid(int fileid);
    //从上传列表中
    FileInfo & slot_getUploadFileinfobyFileid(int fileid);

private slots:
    //左侧窗口单击按钮，右侧分页标签显示对应页
    void on_pb_filepage_clicked();

    void on_pb_transmitpage_clicked();

    void on_pb_sharepage_clicked();

    void on_table_file_cellClicked(int row, int column);


    //显示上传文件菜单
    void on_pb_addFile_clicked();



    void on_table_file_cellDoubleClicked(int row, int column);

    void on_pb_prevDir_clicked();


    void on_table_upload_cellClicked(int row, int column);

    void on_table_download_cellClicked(int row, int column);

private:
    Ui::MainDialog *ui;

    QMenu m_menu;//菜单对象

    QMenu m_menuAddFile;//菜单对象
    QMenu m_menuDownLoadFile;//菜单对象
    QMenu m_menuUpLoadFile;//菜单对象
};

#endif // MAINDIALOG_H
