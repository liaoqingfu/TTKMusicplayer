#include "musicvideoplaywidget.h"
#include "musicvideoview.h"
#include "musiclocalsongsearchedit.h"
#include "musicvideofloatwidget.h"
#include "musicsongsharingwidget.h"
#include "musicobject.h"
#include "musicfunctionuiobject.h"
#include "musictinyuiobject.h"
#include "musicsettingmanager.h"
#include "musicapplication.h"
#include "musicotherdefine.h"

#include <QLabel>
#include <QBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QStackedWidget>
#ifdef Q_OS_UNIX
#include <QApplication>
#include <QDesktopWidget>
#endif

#define WINDOW_HEIGHT   533
#define WINDOW_WIDTH    678

MusicVideoPlayWidget::MusicVideoPlayWidget(QWidget *parent)
    : MusicAbstractMoveWidget(false, parent), m_windowPopup(false)
{
    setWindowTitle("TTKMovie");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    m_topWidget = new QWidget(this);
    m_topWidget->setStyleSheet(MusicUIObject::MBackgroundStyle06 + MusicUIObject::MBorderStyle01);

    QHBoxLayout *topLayout = new QHBoxLayout(m_topWidget);
    topLayout->setContentsMargins(9, 4, 9, 4);

    m_textLabel = new QLabel(m_topWidget);
    m_textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_textLabel->setStyleSheet(MusicUIObject::MColorStyle01);

    QWidget *searchWidget = new QWidget(m_topWidget);
    searchWidget->setFixedHeight(25);
    searchWidget->setStyleSheet(MusicUIObject::MBackgroundStyle11);
    QHBoxLayout *searchLayout = new QHBoxLayout(searchWidget);
    searchLayout->setContentsMargins(0, 0, 0, 0);
    searchLayout->setSpacing(0);

    m_searchEdit = new MusicLocalSongSearchEdit(searchWidget);
    m_searchEdit->setStyleSheet(MusicUIObject::MColorStyle09);
    m_searchEdit->setFixedHeight(25);
    m_searchButton = new QPushButton(searchWidget);
    m_searchButton->setIcon(QIcon(":/tiny/btn_search_main_hover"));
    m_searchButton->setCursor(QCursor(Qt::PointingHandCursor));
    m_searchButton->setIconSize(QSize(25, 25));
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchButton);
    searchWidget->setLayout(searchLayout);

    topLayout->addWidget(m_textLabel);
    topLayout->addStretch();
    topLayout->addWidget(searchWidget);

    m_closeButton = new QPushButton(this);
    m_closeButton->setToolTip(tr("Close"));
    m_closeButton->setFixedSize(14, 14);
    m_closeButton->setStyleSheet(MusicUIObject::MKGBtnPClose);
    m_closeButton->setCursor(QCursor(Qt::PointingHandCursor));
    connect(m_closeButton, SIGNAL(clicked()), parent, SLOT(musicVideoClosed()));
    topLayout->addWidget(m_closeButton);

#ifdef Q_OS_UNIX
    m_searchButton->setFocusPolicy(Qt::NoFocus);
    m_closeButton->setFocusPolicy(Qt::NoFocus);
#endif

    m_searchEdit->hide();
    m_searchButton->hide();

    m_backButton = nullptr;
    m_winTopButton = nullptr;

    m_topWidget->setLayout(topLayout);
    m_topWidget->setFixedHeight(35);

    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->setStyleSheet(MusicUIObject::MBorderStyle01);
    layout->addWidget(m_topWidget);
    layout->addWidget(m_stackedWidget);
    setLayout(layout);

    m_videoFloatWidget = new MusicVideoFloatWidget(this);
    m_videoTable = new MusicVideoTableWidget(this);
    m_videoView = new MusicVideoView(this);
    m_stackedWidget->addWidget(m_videoView);
    m_stackedWidget->addWidget(m_videoTable);
    m_stackedWidget->setCurrentIndex(VIDEO_WINDOW_INDEX_0);
    m_videoFloatWidget->setText(MusicVideoFloatWidget::FreshType, tr("PopupMode"));

    connect(m_searchButton,SIGNAL(clicked(bool)), SLOT(searchButtonClicked()));
    connect(m_videoTable, SIGNAL(mvURLNameChanged(MusicVideoItem)), SLOT(mvURLNameChanged(MusicVideoItem)));
    connect(m_videoTable, SIGNAL(restartSearchQuery(QString)),
                          SLOT(videoResearchButtonSearched(QString)));
    connect(m_searchEdit, SIGNAL(enterFinished(QString)), SLOT(videoResearchButtonSearched(QString)));

    connect(m_videoFloatWidget, SIGNAL(searchButtonClicked()), SLOT(switchToSearchTable()));
    connect(m_videoFloatWidget, SIGNAL(freshButtonClicked()), SLOT(freshButtonClicked()));
    connect(m_videoFloatWidget, SIGNAL(fullscreenButtonClicked()), SLOT(fullscreenButtonClicked()));
    connect(m_videoFloatWidget, SIGNAL(downloadButtonClicked()), SLOT(downloadButtonClicked()));
    connect(m_videoFloatWidget, SIGNAL(shareButtonClicked()), SLOT(shareButtonClicked()));

}

MusicVideoPlayWidget::~MusicVideoPlayWidget()
{
    delete m_winTopButton;
    delete m_closeButton;
    delete m_textLabel;
    delete m_searchEdit;
    delete m_backButton;
    delete m_searchButton;
    delete m_topWidget;
    delete m_videoView;
    delete m_stackedWidget;
}

QString MusicVideoPlayWidget::getClassName()
{
    return staticMetaObject.className();
}

void MusicVideoPlayWidget::popup(bool popup)
{
    m_videoFloatWidget->setText(MusicVideoFloatWidget::FreshType,
                                popup ? tr("InlineMode") : tr("PopupMode"));
    QHBoxLayout *topLayout = MStatic_cast(QHBoxLayout*, m_topWidget->layout());
    m_windowPopup = popup;
    blockMoveOption(!popup);

    if(popup)
    {
        QSize size = M_SETTING_PTR->value(MusicSettingManager::ScreenSize).toSize();
        setGeometry((size.width() - WINDOW_WIDTH)/2, (size.height() - WINDOW_HEIGHT)/2,
                    WINDOW_WIDTH, WINDOW_HEIGHT);
        resizeWindow(0, 0);
        setParent(nullptr);
        show();

        m_winTopButton = new QPushButton(m_topWidget);
        m_winTopButton->setFixedSize(14, 14);
        m_winTopButton->setCursor(QCursor(Qt::PointingHandCursor));
        m_winTopButton->setStyleSheet(MusicUIObject::MKGTinyBtnWintopOff);
        m_winTopButton->setToolTip(tr("windowTopOn"));
#ifdef Q_OS_UNIX
        m_winTopButton->setFocusPolicy(Qt::NoFocus);
#endif
        connect(m_winTopButton, SIGNAL(clicked()), SLOT(windowTopStateChanged()));
        topLayout->insertWidget(topLayout->count() - 1, m_winTopButton);
        m_winTopButton->setEnabled(false);
    }
    else
    {
        delete m_winTopButton;
        m_winTopButton = nullptr;
        m_videoFloatWidget->setText(MusicVideoFloatWidget::FullscreenType, " " + tr("FullScreenMode"));
    }
}

bool MusicVideoPlayWidget::isPopup() const
{
    return m_windowPopup;
}

void MusicVideoPlayWidget::resizeWindow()
{
    int width = M_SETTING_PTR->value(MusicSettingManager::WidgetSize).toSize().width();
    int height = M_SETTING_PTR->value(MusicSettingManager::WidgetSize).toSize().height();
    if(!isFullScreen())
    {
        resizeWindow(width - WINDOW_WIDTH_MIN, height - WINDOW_HEIGHT_MIN);
    }
}

void MusicVideoPlayWidget::resizeWindow(bool resize)
{
    if(resize)
    {
        showFullScreen();
    }
    else
    {
        showNormal();
        setGeometry(250, 150, WINDOW_WIDTH, WINDOW_HEIGHT);
    }
    QSize s = size();
#ifdef Q_OS_UNIX
    QDesktopWidget* desktopWidget = QApplication::desktop();
    if(isFullScreen() && desktopWidget && desktopWidget->screen())
    {
        s = desktopWidget->screen()->size();
    }
    else
    {
        s = QSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        showNormal();
    }
#endif
    resizeWindow(s.width() - WINDOW_WIDTH, s.height() - WINDOW_HEIGHT);
}

void MusicVideoPlayWidget::resizeWindow(int width, int height)
{
    m_videoView->resizeWindow(width, height);
    m_videoTable->resizeWindow(width);
    m_videoFloatWidget->resizeWindow(width, height);
}

QString MusicVideoPlayWidget::getSearchText() const
{
    return m_searchEdit->text().trimmed();
}

void MusicVideoPlayWidget::switchToSearchTable()
{
    QHBoxLayout *topLayout = MStatic_cast(QHBoxLayout*, m_topWidget->layout());
    delete m_backButton;
    m_backButton = new QToolButton(m_topWidget);
    m_backButton->setFixedSize(20, 20);
    m_backButton->setToolTip(tr("Back"));
    m_backButton->setCursor(QCursor(Qt::PointingHandCursor));
    m_backButton->setStyleSheet(MusicUIObject::MKGBtnBackBack);
    connect(m_backButton, SIGNAL(clicked()), SLOT(switchToPlayView()));
    topLayout->insertWidget(0, m_backButton);

    m_textLabel->clear();
    m_searchEdit->show();
    m_searchButton->show();
    m_stackedWidget->setCurrentIndex(VIDEO_WINDOW_INDEX_1);
}

void MusicVideoPlayWidget::switchToPlayView()
{
    delete m_backButton;
    m_backButton = nullptr;

    setTitleText(m_videoItem.m_name);
    m_searchEdit->hide();
    m_searchButton->hide();
    m_stackedWidget->setCurrentIndex(VIDEO_WINDOW_INDEX_0);
}

void MusicVideoPlayWidget::searchButtonClicked()
{
    videoResearchButtonSearched( getSearchText() );
}

void MusicVideoPlayWidget::windowTopStateChanged()
{
    Qt::WindowFlags flags = windowFlags();
    bool top = m_winTopButton->styleSheet().contains("btn_top_off_normal");
    setWindowFlags( top ? (flags | Qt::WindowStaysOnTopHint) : (flags & ~Qt::WindowStaysOnTopHint) );

    show();

    m_winTopButton->setToolTip(top ? tr("windowTopOff") : tr("windowTopOn"));
    m_winTopButton->setStyleSheet(top ? MusicUIObject::MKGTinyBtnWintopOn : MusicUIObject::MKGTinyBtnWintopOff);
}

void MusicVideoPlayWidget::videoResearchButtonSearched(const QString &name)
{
    switchToSearchTable();
    m_searchEdit->setText(name);
    m_videoTable->startSearchQuery(name);
}

void MusicVideoPlayWidget::videoResearchButtonSearched(const QVariant &data)
{
    m_videoTable->startSearchSingleQuery(data);
    MusicObject::MusicSongInformation info(data.value<MusicObject::MusicSongInformation>());
    MusicObject::MusicSongAttributes attrs = info.m_songAttrs;
    if(!attrs.isEmpty())
    {
        MusicObject::MusicSongAttribute attr = attrs.first();
        QString url = attr.m_multiParts ? attr.m_url.split(STRING_SPLITER).first() : attr.m_url;
        MusicVideoItem data;
        data.m_name = info.m_singerName + " - " + info.m_songName;
        data.m_url = url;
        data.m_id = info.m_songId;
        data.m_server = MUSIC_MOVIE_RADIO;
        mvURLNameChanged(data);
    }
}

void MusicVideoPlayWidget::startSearchSingleQuery(const QString &name)
{
    switchToSearchTable();
    m_videoTable->startSearchSingleQuery(name);
}

void MusicVideoPlayWidget::mvURLChanged(const QString &url)
{
    MusicApplication *w = MusicApplication::instance();
    if(w->isPlaying())
    {
        w->musicStatePlay();
    }
    ///stop current media play while mv starts.
    m_videoView->setMedia(url);
    m_videoView->play();

    switchToPlayView();
}

void MusicVideoPlayWidget::mvURLNameChanged(const MusicVideoItem &item)
{
    m_videoItem = item;
    setTitleText(item.m_name);
    mvURLChanged(item.m_url);
}

void MusicVideoPlayWidget::freshButtonClicked()
{
    QString text = m_videoFloatWidget->getText(MusicVideoFloatWidget::FreshType);
    emit freshButtonClicked( text == tr("PopupMode"));
}

void MusicVideoPlayWidget::fullscreenButtonClicked()
{
    if(m_videoFloatWidget->getText(MusicVideoFloatWidget::FreshType) == tr("PopupMode"))
    {
        return;
    }

    QString text = m_videoFloatWidget->getText(MusicVideoFloatWidget::FullscreenType) ==
                                tr("NormalMode") ? tr("FullScreenMode") : tr("NormalMode");
    m_videoFloatWidget->setText(MusicVideoFloatWidget::FullscreenType, " " + text);
    emit fullscreenButtonClicked( text == tr("NormalMode"));
}

void MusicVideoPlayWidget::downloadButtonClicked()
{
    m_videoTable->downloadLocalFromControl();
}

void MusicVideoPlayWidget::shareButtonClicked()
{
    QString name = m_videoItem.m_name.trimmed();
    QString id = m_videoItem.m_id.trimmed();
    if(name.isEmpty() || id.isEmpty())
    {
        return;
    }

    QVariantMap data;
    data["id"] = id;
    data["songName"] = name;
    data["queryServer"] = m_videoItem.m_server;

    MusicSongSharingWidget shareWidget(this);
    shareWidget.setData(MusicSongSharingWidget::Movie, data);
    shareWidget.exec();
}

void MusicVideoPlayWidget::resizeEvent(QResizeEvent *event)
{
    MusicAbstractMoveWidget::resizeEvent(event);
    resizeWindow();
}

void MusicVideoPlayWidget::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event);
}

void MusicVideoPlayWidget::setTitleText(const QString &text)
{
    m_textLabel->setText(MusicUtils::Widget::elidedText(font(), text, Qt::ElideRight, 275));
}
