#include "maindialog.h"
#include "ui_maindialog.h"
#include<QMessageBox>
#include<qdebug.h>
#include"common.h"
MainDialog::MainDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainDialog)
{
    ui->setupUi(this);

    this->setWindowTitle("百度网盘");
    this->setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint
                         |Qt::WindowCloseButtonHint);



    //右键弹出菜单
    //With Qt::CustomContextMenu,
    //the signal customContextMenuRequested() is emitted.
    //设置上面的参数时，有一个信号会被发送（下面的那个函数）
    ui->table_file->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->table_file,SIGNAL(customContextMenuRequested(QPoint)),
            this,SLOT(slot_menuShow(QPoint)));
    m_menu.addAction(QIcon(":/images/folder.png"),"新建文件夹");
    m_menu.addAction("下载");
    m_menu.addAction("删除");
    m_menu.addAction("收藏");
    m_menu.addAction("分享");
    m_menu.addAction("获取分享");
    //triggered 触发，点击菜单会发送信号
    connect(&m_menu,SIGNAL(triggered(QAction*)),
             this,SLOT(slot_dealTriggered(QAction*)));

    m_menuAddFile.addAction("上传文件");
    m_menuAddFile.addAction("上传文件夹");

    connect(&m_menuAddFile,SIGNAL(triggered(QAction*)),
             this,SLOT(slot_dealMuneAddFile(QAction*)));

    //上传菜单
    //设置右键触发
    ui->table_upload->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->table_upload,SIGNAL(customContextMenuRequested(QPoint)),
            this,SLOT(slot_menuUploadShow(QPoint)));
    //添加菜单行为
    m_menuUpLoadFile.addAction("暂停");
    m_menuUpLoadFile.addAction("开始");
    connect(&m_menuUpLoadFile,SIGNAL(triggered(QAction*)),
             this,SLOT(slot_dealUploadTriggered(QAction*)));
    //下载菜单
    ui->table_download->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->table_download,SIGNAL(customContextMenuRequested(QPoint)),
            this,SLOT(slot_menuDownloadShow(QPoint)));
    m_menuDownLoadFile.addAction("暂停");
    m_menuDownLoadFile.addAction("开始");

    connect(&m_menuDownLoadFile,SIGNAL(triggered(QAction*)),
             this,SLOT(slot_dealDownloadTriggered(QAction*)));

    //设置初始页面显示项
    ui->sw_right->setCurrentIndex(0);
    ui->tw_transmit->setCurrentIndex(1);

}

MainDialog::~MainDialog()
{
    delete ui;
}

void MainDialog::closeEvent(QCloseEvent *e)
{
    if((QMessageBox::question(this , "退出" ,"是否退出？"))
            == QMessageBox::Yes){
        //发送释放核心类信号
        Q_EMIT SIG_close();

        //同意关闭事件
        e->accept();
    }else{
        //忽略关闭事件
        e->ignore();
    }

}

void MainDialog::on_pb_filepage_clicked()
{
    ui->sw_right->setCurrentIndex( 0 );
}

void MainDialog::on_pb_transmitpage_clicked()
{
    ui->sw_right->setCurrentIndex( 1 );
}

void MainDialog::on_pb_sharepage_clicked()
{
    ui->sw_right->setCurrentIndex( 2 );
}
//设置用户信息（用户名）
void MainDialog::slot_setInfo(QString name)
{
    ui->pb_user->setText(name);
}

void MainDialog::slot_deleteAllFileInfo()
{
    //ui->table_file->clear(); //这个是不行的，这个删除完行数还在

    int row = ui->table_file->rowCount();
    for(int i = row - 1 ; i >= 0 ;i--){
        ui->table_file->removeRow(i);
    }

}

void MainDialog::slot_deleteSharedAllFileInfo()
{
    int row = ui->table_share->rowCount();
    for(int i = row - 1 ; i >= 0 ;i--){
        ui->table_share->removeRow(i);
    }
}

void MainDialog::slot_InsertFileInfo(FileInfo &info)
{
    mytablewightitem *item0 = new mytablewightitem;

    //保存文件信息到该控件中，并对该控件进行设置
    item0->setInfo(info);

    QTableWidgetItem *item1 = new QTableWidgetItem;
    item1->setText("2022.02.20 12:30:12");

    QTableWidgetItem *item2 = new QTableWidgetItem;
    //文件夹不显示大小
    if(info.type != "dir"){
        item2->setText(FileInfo::getSize(info.size));
    }


    //获取当前行数
    int row = ui->table_file->rowCount();
    //设置行数
    ui->table_file->setRowCount(row + 1);

    //控件是从0行0列开始添加的。
    ui->table_file->setItem(row,0,item0);
    ui->table_file->setItem(row,1,item1);
    ui->table_file->setItem(row,2,item2);
}

#include<QProgressBar>
//设置下载的进度条
void MainDialog::slot_InsertDownLoadFile(FileInfo &info)
{
    //获取当前行数
    int row = ui->table_download->rowCount();
    //设置行数
    ui->table_download->setRowCount(row + 1);

    mytablewightitem *item0 = new mytablewightitem;

    //保存文件信息到该控件中，并对该控件进行设置
    item0->setInfo(info);

    QTableWidgetItem *item1 = new QTableWidgetItem;
    item1->setText(FileInfo::getSize(info.size));

    QProgressBar *progress = new QProgressBar;
    progress->setMaximum(info.size);
    progress->setValue(0);

    QPushButton * button = new QPushButton;
    if(info.isPause == 0){
        button->setText("暂停");
    }else{
        button->setText("开始");
    }


    //控件是从0行0列开始添加的。
    ui->table_download->setItem(row,0,item0);
    ui->table_download->setItem(row,1,item1);
    ui->table_download->setCellWidget(row,2,progress);
    ui->table_download->setCellWidget(row,3,button);
}

void MainDialog::slot_InsertUpLoadFile(FileInfo &info)
{
    //获取当前行数
    int row = ui->table_upload->rowCount();
    //设置行数
    ui->table_upload->setRowCount(row + 1);

    mytablewightitem *item0 = new mytablewightitem;

    //保存文件信息到该控件中，并对该控件进行设置
    item0->setInfo(info);

    QTableWidgetItem *item1 = new QTableWidgetItem;
    item1->setText(FileInfo::getSize(info.size));

    QProgressBar *progress = new QProgressBar;
    progress->setMaximum(info.size);
    progress->setValue(0);
    QPushButton * button = new QPushButton;
    if(info.isPause == 0){
        button->setText("暂停");
    }else{
        button->setText("开始");
    }


    //控件是从0行0列开始添加的。
    ui->table_upload->setItem(row,0,item0);
    ui->table_upload->setItem(row,1,item1);
    ui->table_upload->setCellWidget(row,2,progress);
    ui->table_upload->setCellWidget(row,3,button);
}
//设置点击一行中的任何一个item，都可以设置勾选状态
void MainDialog::on_table_file_cellClicked(int row, int column)
{
    QTableWidgetItem * item = ui->table_file->item(row,0);

    if(item->checkState() == Qt::Unchecked){
        item->setCheckState(Qt::Checked);
    }else{
        item->setCheckState(Qt::Unchecked);
    }
}

void MainDialog::slot_menuShow(QPoint point)
{
    //鼠标点击的位置显示菜单（该函数显示是以左上角为显示的原点）（绝对坐标）
    //传进来的是以应用窗口为原点的鼠标的坐标，用这个不能正常显示（相对坐标）
    //QCursor::pos(),这个以整个windows窗口的左上角为原点，可以使用（绝对坐标）
    m_menu.exec(QCursor::pos());
}

#include<QInputDialog>
void MainDialog::slot_dealTriggered(QAction *action)
{
    if(action->text()== "新建文件夹"){
        qDebug()<<"新建文件夹";

        QString name =QInputDialog::getText(this,"新建文件夹", "输入文件姓名");

        //去重
        int row = ui->table_file->rowCount();
        for(int i = 0;i<row ;i++){
            QString text = ui->table_file->item(i,0)->text();
            if(text == name)
                return;
        }
        //过滤  " / \\ ? * : | < >
        //如果输入的都是空格也不行
        QString tmp = name;
        if(tmp.remove(" ").isEmpty()){
            QMessageBox::about(this,"文件名输入有误","输入文件名不能都为空格");
            return;
        }
        if(name.contains("/") || name.contains("\\") || name.contains("?")
           || name.contains("*") || name.contains(":") || name.contains("\"")
           || name.contains("|") || name.contains("<") || name.contains(">")){
             QMessageBox::about(this,"文件名输入有误","输入文件名为 \" / \\ ? * : | < > ");
             return;
        }

        //发送文件名给核心类
        SIG_addFolder(name);

    }else if(action->text()== "下载"){
        qDebug()<<"下载";
        //获取被勾选的item，通过item获取对应的文件信息
        int len = ui->table_file->rowCount();
        for(int i =0;i<len;i++){
            //查看文件id
            mytablewightitem * item = (mytablewightitem *)ui->table_file->item(i,0);
            //没打勾直接跳过
            if(item->checkState() == Qt::Unchecked){
                continue;
            }
            int fileid = item->m_info.fileid;
            //判断该文件是否在下载列表中 todo
            //不是，发送一个信号
            Q_EMIT SIG_downloadFile(fileid);
        }


    }else if(action->text()== "删除"){
        qDebug()<<"删除";
        //获取被勾选的item，通过item获取对应的文件信息
        int len = ui->table_file->rowCount();
        QString path = ui->pb_path->text();
        QVector<int> fileIdArray;
        for(int i =0;i<len;i++){
            //查看文件id
            mytablewightitem * item = (mytablewightitem *)ui->table_file->item(i,0);
            //没打勾直接跳过
            if(item->checkState() == Qt::Unchecked){
                continue;
            }
            int fileid = item->m_info.fileid;
            fileIdArray.push_back(fileid);

        }
         Q_EMIT SIG_deleteFile(path , fileIdArray);

    }else if(action->text()== "收藏"){
        qDebug()<<"收藏";
    }else if(action->text()== "分享"){
        qDebug()<<"分享";
        //获取被勾选的item，通过item获取对应的文件信息
        int len = ui->table_file->rowCount();
        QString path = ui->pb_path->text();
        QVector<int> fileIdArray;
        for(int i =0;i<len;i++){
            //查看文件id
            mytablewightitem * item = (mytablewightitem *)ui->table_file->item(i,0);
            //没打勾直接跳过
            if(item->checkState() == Qt::Unchecked){
                continue;
            }
            int fileid = item->m_info.fileid;
            fileIdArray.push_back(fileid);

        }
         Q_EMIT SIG_sharedFile(path , fileIdArray);
    }else if(action->text()== "获取分享"){
        qDebug()<<"获取分享";
        QString link;
        QString dir;
        while(1){
            bool ok;
            //获取分享码

            link = QInputDialog::getText(this,"获取分享","输入分享码",QLineEdit::Normal,"",&ok);
            //点击取消键ok=false，点击右上角叉，link会返回空
            if(ok == false || link.isEmpty()){
                qDebug()<<ok<<" "<<link;
                return;
            }
            //获取当前路径
            dir = ui->pb_path->text();

            //过滤
            int code = link.toInt();
            if(code < 10000000 || code >=100000000 || link.length() != 8){
                QMessageBox::about(this,"错误","请重新输入");
                continue;
            }
            break;
        }
        qDebug()<<link;
        Q_EMIT SIG_getShareFile(link,dir);

    }
}

void MainDialog::slot_dealUploadTriggered(QAction *action)
{
    qDebug()<<__func__;
    if(action->text() == "暂停"){
        qDebug()<<"暂停";
        //获取行数
        int row = ui->table_upload->rowCount();
        for(int i = 0;i<row;i++){
            //先获取第一列控件，判断勾选状态
            mytablewightitem * item =
                    (mytablewightitem *)ui->table_upload->item(i,0);
            if(item->checkState() == Qt::Checked){
                QPushButton * button =
                        (QPushButton *)ui->table_upload->cellWidget(i,3);
                //如果是暂停，就改成开始，否则就忽略掉
                if(button->text() == "暂停"){
                    //换字
                    button->setText("开始");
                    //发送信号
                    Q_EMIT SIG_setUploadPauseStatus(item->m_info.fileid,1);

                }
            }
        }

    }else if(action->text() == "开始"){
        qDebug()<<"开始";
        //获取行数
        int row = ui->table_upload->rowCount();
        for(int i = 0;i<row;i++){
            //先获取第一列控件，判断勾选状态
            mytablewightitem * item =
                    (mytablewightitem *)ui->table_upload->item(i,0);
            if(item->checkState() == Qt::Checked){
                QPushButton * button =
                        (QPushButton *)ui->table_upload->cellWidget(i,3);
                //如果是开始，就改成暂停，否则就忽略掉
                if(button->text() == "开始"){
                    //换字
                    button->setText("暂停");
                    //发送信号
                    Q_EMIT SIG_setUploadPauseStatus(item->m_info.fileid,0);

                }
            }
        }
    }
}

void MainDialog::slot_dealDownloadTriggered(QAction *action)
{
    qDebug()<<__func__;
    if(action->text() == "暂停"){
        qDebug()<<"暂停";
        //获取行数
        int row = ui->table_download->rowCount();
        for(int i = 0;i<row;i++){
            //先获取第一列控件，判断勾选状态
            mytablewightitem * item =
                    (mytablewightitem *)ui->table_download->item(i,0);
            if(item->checkState() == Qt::Checked){
                QPushButton * button =
                        (QPushButton *)ui->table_download->cellWidget(i,3);
                //如果是暂停，就改成开始，否则就忽略掉
                if(button->text() == "暂停"){
                    //换字
                    button->setText("开始");
                    //发送信号
                    Q_EMIT SIG_setDownloadPauseStatus(item->m_info.fileid,1);

                }
            }
        }

    }else if(action->text() == "开始"){
        qDebug()<<"开始";
        //获取行数
        int row = ui->table_download->rowCount();
        for(int i = 0;i<row;i++){
            //先获取第一列控件，判断勾选状态
            mytablewightitem * item =
                    (mytablewightitem *)ui->table_download->item(i,0);
            if(item->checkState() == Qt::Checked){
                QPushButton * button =
                        (QPushButton *)ui->table_download->cellWidget(i,3);
                //如果是开始，就改成暂停，否则就忽略掉
                if(button->text() == "开始"){
                    //换字
                    button->setText("暂停");
                    //发送信号
                    Q_EMIT SIG_setDownloadPauseStatus(item->m_info.fileid,0);

                }
            }
        }
    }
}

void MainDialog::slot_menuUploadShow(QPoint point)
{
    m_menuUpLoadFile.exec(QCursor::pos());
}

void MainDialog::slot_menuDownloadShow(QPoint point)
{
    m_menuDownLoadFile.exec(QCursor::pos());
}

void MainDialog::slot_updateFileProgress(int fileId, int pos)
{
    qDebug()<<__func__;
    int row = ui->table_download->rowCount();

    for(int i = 0 ;i<row ;i++){
        mytablewightitem * item = (mytablewightitem *)ui->table_download->item(i,0);

        //获取对应的item
        if(item->m_info.fileid == fileId){

            QProgressBar * progress = (QProgressBar *)ui->table_download->cellWidget(i,2);
            progress->setValue(pos);
            //下载完成删除该行 todo
            if(progress->value() >= progress->maximum()){
                this->slot_insertComplete(item->m_info);
            }

            return ;
        }
    }
}

void MainDialog::slot_updateUploadFileProgress(int fileId, int pos)
{
    //qDebug()<<__func__;
    int row = ui->table_upload->rowCount();

    for(int i = 0 ;i<row ;i++){
        mytablewightitem * item = (mytablewightitem *)ui->table_upload->item(i,0);

        //获取对应的item
        if(item->m_info.fileid == fileId){

            QProgressBar * progress = (QProgressBar *)ui->table_upload->cellWidget(i,2);
            progress->setValue(pos);
            //上传完成删除该行 todo
            return ;
        }
    }
}
#include<QDateTime>
void MainDialog::slot_insertComplete(FileInfo info)
{

    qDebug()<<__func__;
    //添加一行
    int rows = ui->table_complete->rowCount();
    ui->table_complete->setRowCount(rows + 1);
    //创建控件
    mytablewightitem * item0 = new mytablewightitem;
    item0->setInfo(info);

    QTableWidgetItem * item1 = new QTableWidgetItem;
    item1->setText(FileInfo::getSize(info.size));

    QTableWidgetItem * item2 = new QTableWidgetItem;
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    item2->setText(time);

    QPushButton * item3 = new QPushButton;
    item3->setIcon(QIcon(":/images/folder.png"));

    //设置个显示，用来存路径
    item3->setToolTip(info.absolutePath);

    connect(item3,SIGNAL(clicked()), this
            , SLOT( slot_folderButtonClick() ) );
    //添加控件到新增加的行中
    ui->table_complete->setItem(rows,0,item0);
    ui->table_complete->setItem(rows,1,item1);
    ui->table_complete->setItem(rows,2,item2);
    ui->table_complete->setCellWidget(rows,3,item3);
}

void MainDialog::slot_uploadFileinsertComplete(FileInfo info)
{
    qDebug()<<__func__;
    //添加一行
    int rows = ui->table_complete->rowCount();
    ui->table_complete->setRowCount(rows + 1);
    //创建控件
    mytablewightitem * item0 = new mytablewightitem;
    item0->setInfo(info);

    QTableWidgetItem * item1 = new QTableWidgetItem;
    item1->setText(FileInfo::getSize(info.size));

    QTableWidgetItem * item2 = new QTableWidgetItem;
    QString time = QDateTime::currentDateTime().toString("yyyy-mm-dd hh-mm-ss");
    item2->setText(time);

    QPushButton * item3 = new QPushButton;
    item3->setText("下载完成");

    //设置个显示，用来存路径
//    item3->setToolTip(info.absolutePath);

//    connect(item3,SIGNAL(clicked()), this
//            , SLOT( slot_folderButtonClick() ) );
    //添加控件到新增加的行中
    ui->table_complete->setItem(rows,0,item0);
    ui->table_complete->setItem(rows,1,item1);
    ui->table_complete->setItem(rows,2,item2);
    ui->table_complete->setCellWidget(rows,3,item3);
}
#include<QProcess>
void MainDialog::slot_folderButtonClick()
{
    qDebug()<<__func__;
    QPushButton * button = (QPushButton *)QObject::sender();
    QString path = button->toolTip().replace('/','\\');

    QProcess process;
    process.startDetached("explorer",QStringList()<<QString("/select,")<<path);
}

#include<QFileDialog>
void MainDialog::slot_dealMuneAddFile(QAction *action)
{
    //这里是可以进行合并的
    //先判断该文件是，文件还是文件夹，文件进行文件的操作，文件夹进行递归操作，这个可以直接在上传文件夹中直接修改下就能用
    if(action->text()== "上传文件"){
       qDebug()<<"上传文件";
       //开弹窗，点击返回文件路径，不点击返回空
       QString path = QFileDialog::getOpenFileName(this,"上传文件");//参数：父窗口指针，左上角标题名,默认打开路径，过滤器（只打开MP3之类的）
       QFileInfo info(path);

       if(path.isEmpty()){
           return;
       }
       //判断是否是下载中的文件，是就不下载否则下载 todo

       //发送
       Q_EMIT SIG_uploadFile(path);

    }else if(action->text()== "上传文件夹"){
        qDebug()<<"上传文件夹";
        //开弹窗，点击返回文件路径，不点击返回空
        QString path = QFileDialog::getExistingDirectory(this,"上传文件夹","./");//参数：父窗口指针，左上角标题名,默认打开路径，过滤器（只打开MP3之类的）
        QFileInfo info(path);

        if(path.isEmpty()){
            return;
        }
        //判断是否是下载中的文件，是就不下载否则下载 todo
        QString dir = ui->pb_path->text();
        //发送
        Q_EMIT SIG_uploadFolder(path,dir);
    }
}

void MainDialog::slot_insertSharedFile(QString name, int size, QString time, int shareLink)
{
    qDebug()<<__func__;
    //添加一行
    int rows = ui->table_share->rowCount();
    ui->table_share->setRowCount(rows + 1);
    //创建控件
    QTableWidgetItem * item0 = new QTableWidgetItem;
    item0->setText(name);

    QTableWidgetItem * item1 = new QTableWidgetItem;
    item1->setText(FileInfo::getSize(size));

    QTableWidgetItem * item2 = new QTableWidgetItem;
    item2->setText(time);

    QTableWidgetItem * item3 = new QTableWidgetItem;
    item3->setText(QString::number(shareLink));


    //添加控件到新增加的行中
    ui->table_share->setItem(rows,0,item0);
    ui->table_share->setItem(rows,1,item1);
    ui->table_share->setItem(rows,2,item2);
    ui->table_share->setItem(rows,3,item3);
}

FileInfo &MainDialog::slot_getDownloadFileinfobyFileid(int fileid)
{
    int row = ui->table_download->rowCount();

    for(int i = 0 ;i<row ;i++){
        mytablewightitem * item = (mytablewightitem *)ui->table_download->item(i,0);

        //获取对应的item
        if(item->m_info.fileid == fileid){
            return item->m_info;
        }
    }
}

FileInfo &MainDialog::slot_getUploadFileinfobyFileid(int fileid)
{
    int row = ui->table_upload->rowCount();

    for(int i = 0 ;i<row ;i++){
        mytablewightitem * item = (mytablewightitem *)ui->table_upload->item(i,0);

        //获取对应的item
        if(item->m_info.fileid == fileid){
            return item->m_info;
        }
    }
}




void MainDialog::on_pb_addFile_clicked()
{
    m_menuAddFile.exec(QCursor::pos());
}



void MainDialog::on_table_file_cellDoubleClicked(int row, int column)
{
    mytablewightitem * item = (mytablewightitem *)ui->table_file->item(row,0);
    //文件双击不做处理
    if(item->m_info.type == "file") return;

    QString path = ui->pb_path->text() + item->text() + "/";
    ui->pb_path->setText(path);

    Q_EMIT SIG_changeDir(path);

}


void MainDialog::on_pb_prevDir_clicked()
{
    QString path = ui->pb_path->text();
    //到头了，不能继续向前
    if(path == '/') return;
    path = path.left(path.lastIndexOf("/")); // lastIndexOf(a);返回a字符所在的下标
    path = path.left(path.lastIndexOf("/") + 1);//left(a) 从左截取 a的长度
    ui->pb_path->setText(path);
    qDebug()<<path;

    Q_EMIT SIG_changeDir(path);
}





void MainDialog::on_table_upload_cellClicked(int row, int column)
{
    QTableWidgetItem * item = ui->table_upload->item(row,0);

    if(item->checkState() == Qt::Unchecked){
        item->setCheckState(Qt::Checked);
    }else{
        item->setCheckState(Qt::Unchecked);
    }
}


void MainDialog::on_table_download_cellClicked(int row, int column)
{
    QTableWidgetItem * item = ui->table_download->item(row,0);

    if(item->checkState() == Qt::Unchecked){
        item->setCheckState(Qt::Checked);
    }else{
        item->setCheckState(Qt::Unchecked);
    }
}

