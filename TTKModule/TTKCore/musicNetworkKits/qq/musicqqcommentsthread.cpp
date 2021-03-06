#include "musicqqcommentsthread.h"
#include "musicdownloadqueryqqthread.h"
#include "musicsemaphoreloop.h"

#///QJson import
#include "qjson/parser.h"

MusicQQSongCommentsThread::MusicQQSongCommentsThread(QObject *parent)
    : MusicDownLoadCommentsThread(parent)
{
    m_pageSize = 20;
}

QString MusicQQSongCommentsThread::getClassName()
{
    return staticMetaObject.className();
}

void MusicQQSongCommentsThread::startToSearch(const QString &name)
{
    M_LOGGER_INFO(QString("%1 startToSearch %2").arg(getClassName()).arg(name));
    MusicSemaphoreLoop loop;
    MusicDownLoadQueryQQThread *query = new MusicDownLoadQueryQQThread(this);
    query->setQueryAllRecords(false);
    query->setQuerySimplify(true);
    query->startToSearch(MusicDownLoadQueryThreadAbstract::MusicQuery, name);
    connect(query, SIGNAL(downLoadDataChanged(QString)), &loop, SLOT(quit()));
    loop.exec();

    m_rawData["songID"].clear();
    if(!query->getMusicSongInfos().isEmpty())
    {
        m_rawData["songID"] = query->getRawData()["songID"];
        startToPage(0);
    }
}

void MusicQQSongCommentsThread::startToPage(int offset)
{
    if(!m_manager)
    {
        return;
    }

    M_LOGGER_INFO(QString("%1 startToSearch %2").arg(getClassName()).arg(offset));
    deleteAll();
    m_pageTotal = 0;
    m_interrupt = true;
    QUrl musicUrl = QString(MusicUtils::Algorithm::mdII(QQ_COMMIT_URL, false));

    QNetworkRequest request;
    request.setUrl(musicUrl);
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("User-Agent", MusicUtils::Algorithm::mdII(QQ_UA_URL_1, ALG_UA_KEY, false).toUtf8());
    setSslConfiguration(&request);

    m_reply = m_manager->post( request, MusicUtils::Algorithm::mdII(QQ_SG_COMMIT_DATA_URL, false)
                               .arg(m_rawData["songID"].toString()).arg(offset).arg(m_pageSize).toUtf8());
    connect(m_reply, SIGNAL(finished()), SLOT(downLoadFinished()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
}

void MusicQQSongCommentsThread::downLoadFinished()
{
    if(!m_reply)
    {
        deleteAll();
        return;
    }

    M_LOGGER_INFO(QString("%1 downLoadFinished").arg(getClassName()));
    m_interrupt = false;

    if(m_reply->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = m_reply->readAll();

        QJson::Parser parser;
        bool ok;
        QVariant data = parser.parse(bytes, &ok);
        if(ok)
        {
            QVariantMap value = data.toMap();
            if(value["code"].toInt() == 0)
            {
                m_pageTotal = value["commenttotal"].toInt();

                QVariantList comments = value["comment"].toList();
                foreach(const QVariant &comm, comments)
                {
                    if(comm.isNull())
                    {
                        continue;
                    }

                    if(m_interrupt) return;

                    MusicResultsItem comment;
                    value = comm.toMap();
                    comment.m_playCount = QString::number(value["praisenum"].toInt());
                    comment.m_updateTime = QString::number(value["time"].toLongLong()*1000);
                    comment.m_description = value["rootcommentcontent"].toString();

                    comment.m_nickName = value["nick"].toString();
                    comment.m_coverUrl = value["avatarurl"].toString();

                    emit createSearchedItem(comment);
                }
            }
        }
    }

    emit downLoadDataChanged(QString());
    deleteAll();
    M_LOGGER_INFO(QString("%1 downLoadFinished deleteAll").arg(getClassName()));
}



MusicQQPlaylistCommentsThread::MusicQQPlaylistCommentsThread(QObject *parent)
    : MusicDownLoadCommentsThread(parent)
{
    m_pageSize = 20;
}

QString MusicQQPlaylistCommentsThread::getClassName()
{
    return staticMetaObject.className();
}

void MusicQQPlaylistCommentsThread::startToSearch(const QString &name)
{
    M_LOGGER_INFO(QString("%1 startToSearch %2").arg(getClassName()).arg(name));

    m_rawData["songID"] = name;
    startToPage(0);
}

void MusicQQPlaylistCommentsThread::startToPage(int offset)
{
    if(!m_manager)
    {
        return;
    }

    M_LOGGER_INFO(QString("%1 startToSearch %2").arg(getClassName()).arg(offset));
    deleteAll();
    m_pageTotal = 0;
    m_interrupt = true;
    QUrl musicUrl = QString(MusicUtils::Algorithm::mdII(QQ_COMMIT_URL, false));

    QNetworkRequest request;
    request.setUrl(musicUrl);
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader("User-Agent", MusicUtils::Algorithm::mdII(QQ_UA_URL_1, ALG_UA_KEY, false).toUtf8());
    setSslConfiguration(&request);

    m_reply = m_manager->post( request, MusicUtils::Algorithm::mdII(QQ_PL_COMMIT_DATA_URL, false)
                               .arg(m_rawData["songID"].toString()).arg(offset).arg(m_pageSize).toUtf8());
    connect(m_reply, SIGNAL(finished()), SLOT(downLoadFinished()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
}

void MusicQQPlaylistCommentsThread::downLoadFinished()
{
    if(!m_reply)
    {
        deleteAll();
        return;
    }

    M_LOGGER_INFO(QString("%1 downLoadFinished").arg(getClassName()));
    m_interrupt = false;

    if(m_reply->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = m_reply->readAll();

        QJson::Parser parser;
        bool ok;
        QVariant data = parser.parse(bytes, &ok);
        if(ok)
        {
            QVariantMap value = data.toMap();
            if(value["code"].toInt() == 0)
            {
                m_pageTotal = value["commenttotal"].toInt();

                QVariantList comments = value["comment"].toList();
                foreach(const QVariant &comm, comments)
                {
                    if(comm.isNull())
                    {
                        continue;
                    }

                    if(m_interrupt) return;

                    MusicResultsItem comment;
                    value = comm.toMap();
                    comment.m_playCount = QString::number(value["praisenum"].toInt());
                    comment.m_updateTime = QString::number(value["time"].toLongLong()*1000);
                    comment.m_description = value["rootcommentcontent"].toString();

                    comment.m_nickName = value["nick"].toString();
                    comment.m_coverUrl = value["avatarurl"].toString();

                    emit createSearchedItem(comment);
                }
            }
        }
    }

    emit downLoadDataChanged(QString());
    deleteAll();
    M_LOGGER_INFO(QString("%1 downLoadFinished deleteAll").arg(getClassName()));
}
