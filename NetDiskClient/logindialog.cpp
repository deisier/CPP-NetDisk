#include "logindialog.h"
#include "ui_logindialog.h"
#include"QMessageBox"
#include<QRegExp>   //正则头文件

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    this->setWindowTitle("登录&注册");
    this->setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint
                         |Qt::WindowCloseButtonHint);\
    //设置当前分页为登录分页（1：注册，2：登录）
    ui->tw_login_register->setCurrentIndex(2);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}


//登录提交
void LoginDialog::on_pb_commit_clicked()
{
    //从控件中获取文本信息
    QString tel = ui->le__tel->text();
    QString password = ui->le_password->text();

    //验证
    //验证是否有空
    if(tel.isEmpty() || password.isEmpty()){
        QMessageBox::about( this , "注册提示" , "输入不可为空！" );
        return;
    }
    //验证手机号是否合法 -- 正则表达式验证
    QRegExp exp(QString("^(1[356789])[0-9]\{9\}$"));//正则表达式
    bool ret = exp.exactMatch(tel);//精准匹配
    if(!ret){
        QMessageBox::about( this , "注册提示" , "手机号输入格式有误！" );
        return;
    }
    //验证密码
    if(password.length() > 20){
        QMessageBox::about( this , "注册提示" , "输入密码过长！" );
        return;
    }


    //发送文本信息给核心类
    Q_EMIT SIG_loginCommit( tel , password);

}

//注册提交
void LoginDialog::on_pb_commit_register_clicked()
{
    //从控件中获取文本信息
    QString tel = ui->le__tel_register->text();
    QString password = ui->le_password_register->text();
    QString confirm = ui->le_confirm_register->text();
    QString name = ui->le__name_register->text();
    //验证
    //验证是否有空
    if(tel.isEmpty() || password.isEmpty() ||
            confirm.isEmpty() || name.isEmpty()){
        QMessageBox::about( this , "注册提示" , "输入不可为空！" );
        return;
    }
    //验证手机号是否合法 -- 正则表达式验证
    QRegExp exp(QString("^(1[356789])[0-9]\{9\}$"));//正则表达式
    bool ret = exp.exactMatch(tel);//精准匹配
    if(!ret){
        QMessageBox::about( this , "注册提示" , "手机号输入格式有误！" );
        return;
    }
    //验证密码
    if(password.length() > 20){
        QMessageBox::about( this , "注册提示" , "输入密码过长！" );
        return;
    }
    //验证确认密码
    if(password != confirm){
        QMessageBox::about( this , "注册提示" , "密码和确认不匹配！" );
        return;
    }
    //验证昵称 -- 长度、敏感词
    if(name.length() > 10){
        QMessageBox::about( this , "注册提示" , "昵称过长！" );
        return;
    }

    //发送文本信息给核心类
    Q_EMIT SIG_registerCommit( tel , password , name);
}


//登录清空槽函数
void LoginDialog::on_pb_clear_clicked()
{
    ui->le__tel->setText("");
    ui->le_password->setText("");
}
//注册清空槽函数
void LoginDialog::on_pb_clear_register_clicked()
{
    ui->le__tel_register->setText("");
    ui->le_password_register->setText("");
    ui->le_confirm_register->setText("");
    ui->le__name_register->setText("");
}

void LoginDialog::closeEvent(QCloseEvent *e)
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



