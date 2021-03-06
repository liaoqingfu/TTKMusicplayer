#include "musicdownloadquerywsmoviethread.h"
#include "musicdownloadqueryyytthread.h"
#include "musicsemaphoreloop.h"
#include "musicnumberdefine.h"
#///QJson import
#include "qjson/parser.h"

MusicDownLoadQueryWSMovieThread::MusicDownLoadQueryWSMovieThread(QObject *parent)
    : MusicDownLoadQueryMovieThread(parent)
{
    m_queryServer = "WuSing";
}

QString MusicDownLoadQueryWSMovieThread::getClassName()
{
    return staticMetaObject.className();
}

void MusicDownLoadQueryWSMovieThread::startToSearch(QueryType type, const QString &text)
{
    if(!m_manager)
    {
        return;
    }

    M_LOGGER_INFO(QString("%1 startToSearch %2").arg(getClassName()).arg(text));
    m_searchText = text.trimmed();
    m_currentType = type;
    deleteAll();
    m_interrupt = true;

    QTimer::singleShot(MT_MS, this, SLOT(downLoadFinished()));
}

void MusicDownLoadQueryWSMovieThread::downLoadFinished()
{
    if(!m_manager)
    {
        deleteAll();
        return;
    }

    M_LOGGER_INFO(QString("%1 downLoadFinished").arg(getClassName()));
    emit clearAllItems();
    m_musicSongInfos.clear();
    m_interrupt = false;

    ///extra yyt movie
    if(!m_interrupt && m_queryExtraMovie && m_currentType == MovieQuery)
    {
        MusicSemaphoreLoop loop;
        MusicDownLoadQueryYYTThread *query = new MusicDownLoadQueryYYTThread(this);
        connect(query, SIGNAL(createSearchedItem(MusicSearchedItem)), SIGNAL(createSearchedItem(MusicSearchedItem)));
        connect(query, SIGNAL(downLoadDataChanged(QString)), &loop, SLOT(quit()));
        query->startToSearch(MusicDownLoadQueryYYTThread::MovieQuery, m_searchText);
        loop.exec();
        m_musicSongInfos << query->getMusicSongInfos();
    }

    emit downLoadDataChanged(QString());
    deleteAll();
    M_LOGGER_INFO(QString("%1 downLoadFinished deleteAll").arg(getClassName()));
}
