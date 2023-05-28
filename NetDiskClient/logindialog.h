#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include<QCloseEvent>
namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();

signals:
    //注册提交信号
    void SIG_registerCommit( QString tel , QString password , QString name);
    //登录提交信号
    void SIG_loginCommit( QString tel , QString password);
    //关闭窗口信号
    void SIG_close();

private slots:
    void on_pb_clear_clicked();

    void on_pb_commit_clicked();

    void on_pb_clear_register_clicked();



    void on_pb_commit_register_clicked();

private:
    Ui::LoginDialog *ui;
    //关闭事件 同意 会发送关闭信号
    void closeEvent(QCloseEvent *e);
};

#endif // LOGINDIALOG_H
