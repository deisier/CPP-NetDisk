#ifndef CLOGIC_H
#define CLOGIC_H

#include"TCPKernel.h"
#include<utility>
class CLogic
{
public:
    CLogic( TcpKernel* pkernel )
    {
        m_pKernel = pkernel;
        m_sql = pkernel->m_sql;
        m_tcp = pkernel->m_tcp;
    }
public:
    //设置协议映射
    void setNetPackMap();
    /************** 发送数据*********************/
    void SendData( sock_fd clientfd, char*szbuf, int nlen )
    {
        m_pKernel->SendData( clientfd ,szbuf , nlen );
    }
    /************** 网络处理 *********************/
    //注册
    void RegisterRq(sock_fd clientfd, char*szbuf, int nlen);
    //登录
    void LoginRq(sock_fd clientfd, char*szbuf, int nlen);
    //用户文件列表请求
    void UserFileListRq(sock_fd clientfd, char*szbuf, int nlen);
    //文件下载请求
    void DownloadFileRq(sock_fd clientfd, char*szbuf, int nlen);
    //文件请求头回复
    void FileHeadRs(sock_fd clientfd, char*szbuf, int nlen);
    //文件内容回复
    void FileContentRs(sock_fd clientfd, char*szbuf, int nlen);
    //文件上传请求
    void UploadFileRq(sock_fd clientfd, char*szbuf, int nlen);
    //文件下载请求
    void FileContentRq(sock_fd clientfd, char*szbuf, int nlen);
    //新建文件夹请求
    void AddFolderRq(sock_fd clientfd, char*szbuf, int nlen);
    //删除文件请求
    void DeleteFileRq(sock_fd clientfd, char*szbuf, int nlen);
    //更新分享列表
    void ShareFileListRq(sock_fd clientfd, char*szbuf, int nlen);
    //分享文件请求
    void ShareFileRq(sock_fd clientfd, char*szbuf, int nlen);
    //获取文件请求
    void GetShareFileRq(sock_fd clientfd, char*szbuf, int nlen);
    //断点下载请求
    void ContinueDownloadRq(sock_fd clientfd, char*szbuf, int nlen);
    //断点上传请求
    void ContinueUploadRq(sock_fd clientfd, char*szbuf, int nlen);

    /*******************************************/


    //删除一项
    void DeleteItem(int uid,int fid,string dir);
    //删除文件
    void DeleteFile(int uid,int fid,string dir,string name,int count,string path);
    //删除文件夹
    void DeleteFolder(int uid,int fid,string dir,string name,int count,string path);

    //分享一项
    void ShareItem(int userid,int fid,string time,int link,string dir);
    //分享文件
    void ShareFile(int userid,int fid,string dir,string time,int link);
    //分享文件夹
    void ShareFolder(int userid,int fid,string dir,string time,int link,string name);

    //获取分享文件
    void GetShareFile(int userid,int fid,string dir);
    //获取分享文件夹
    void GetShareFolder(int userid,int fid,string dir,string name,int fromUid,string fromDir);

    //下载文件
    void DownloadFile(list<string> & lstRes, int userid, int clientfd);
    //下载文件夹
    void DownloadFolder(int clientfd, int userid, int fileid, string dir, string filename);




private:
    TcpKernel* m_pKernel;
    CMysql * m_sql;
    Block_Epoll_Net * m_tcp;

    //用户信息映射表
    MyMap<int , UserInfo *> m_mapIdToUserInfo;
    //u_id f_id to fileInfo : 一个用户可以有多个文件，一个文件可以被多个文件拥有，这样设置防止混淆
    MyMap<pair<int,int> , FileInfo *> m_mapFileIdToFileInfo;
};

#endif // CLOGIC_H
