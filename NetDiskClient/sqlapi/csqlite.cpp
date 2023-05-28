#include "csqlite.h"

CSqlite::CSqlite()
{
    //获取本地编码进行黄色至
    QTextCodec::setCodecForLocale( QTextCodec::codecForLocale());

    //装驱动
    if( QSqlDatabase::contains("qt_sql_default_connection")) //默认的qt sql驱动
    {
        m_db = QSqlDatabase::database("qt_sql_default_connection");
    }else
    {
        m_db = QSqlDatabase::addDatabase("QSQLITE"); // 添加 sqlite 驱动
    }
}

CSqlite::~CSqlite()
{
    DisConnect();
}

void CSqlite::ConnectSql(QString db)
{
    m_db.setDatabaseName(db); //传路径
}

void CSqlite::DisConnect()
{
    m_db.close();
}

//查询
bool CSqlite::SelectSql(QString sqlStr , int nColumn , QStringList & list)
{
    bool success ;
    m_mutex.lock();
    m_db.open();//打开
    QSqlQuery query;
    query.prepare( sqlStr );//准备
    success = query.exec();//执行
    if(!success)//执行失败，关闭sql对象，开锁
    {
        qDebug()<< sqlStr << QString("SelectData Error");
        m_db.close();
        m_mutex.unlock();
        return false;
    }else
    {
        for( ; query.next() ;)//遍历结果集中的行
        {
            for(int i= 0; i<nColumn ; i++)//遍历行中的每列
            {
                list.append( query.value(i).toString() );
            }
        }
    }
    m_db.close();
    m_mutex.unlock();
    return true;
}

//更新: 删除, 插入 , 修改
bool CSqlite::UpdateSql(QString sqlStr)
{
    bool success ;
    m_mutex.lock();
    if(!m_db.open()){
        qDebug()<< sqlStr << QString("UpdateData Error");
        m_db.close();
        m_mutex.unlock();
        return false;
    }
    QSqlQuery query;
    query.prepare( sqlStr );
    success = query.exec();
    if(!success)
    {
        qDebug()<< sqlStr << QString("UpdateData Error");
        m_db.close();
        m_mutex.unlock();
        return false;
    }
    m_db.close();
    m_mutex.unlock();
    return true;
}

