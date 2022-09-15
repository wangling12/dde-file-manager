/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "dfmplugin_sidebar_global.h"
#include "sidebarhelper.h"
#include "sidebarinfocachemananger.h"
#include "sidebaritem.h"
#include "sidebarwidget.h"

#include "events/sidebareventcaller.h"
#include "events/sidebareventreceiver.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/configs/settingbackend.h"
#include "dfm-base/base/configs/configsynchronizer.h"
#include "dfm-base/base/configs/dconfig/dconfigmanager.h"
#include "dfm-base/utils/systempathutil.h"
#include "dfm-base/dialogs/settingsdialog/settingdialog.h"

#include <dfm-framework/dpf.h>

#include <QMenu>

#include <DSysInfo>

DPSIDEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

QMap<quint64, SideBarWidget *> SideBarHelper::kSideBarMap {};
QMap<QString, SortFunc> SideBarHelper::kSortFuncs {};
bool SideBarHelper::contextMenuEnabled { true };

QList<SideBarWidget *> SideBarHelper::allSideBar()
{
    QMutexLocker locker(&SideBarHelper::mutex());
    QList<SideBarWidget *> list;
    auto keys = kSideBarMap.keys();
    for (auto k : keys)
        list.push_back(kSideBarMap[k]);

    return list;
}

SideBarWidget *SideBarHelper::findSideBarByWindowId(quint64 windowId)
{
    QMutexLocker locker(&SideBarHelper::mutex());
    if (!kSideBarMap.contains(windowId))
        return nullptr;

    return kSideBarMap[windowId];
}

void SideBarHelper::addSideBar(quint64 windowId, SideBarWidget *titleBar)
{
    QMutexLocker locker(&SideBarHelper::mutex());
    if (!kSideBarMap.contains(windowId))
        kSideBarMap.insert(windowId, titleBar);
}

void SideBarHelper::removeSideBar(quint64 windowId)
{
    QMutexLocker locker(&SideBarHelper::mutex());
    if (kSideBarMap.contains(windowId))
        kSideBarMap.remove(windowId);
}

quint64 SideBarHelper::windowId(QWidget *sender)
{
    return FMWindowsIns.findWindowId(sender);
}

SideBarItem *SideBarHelper::createItemByInfo(const ItemInfo &info)
{
    SideBarItem *item = new SideBarItem(info.icon,
                                        info.displayName,
                                        info.group,
                                        info.url);
    item->setFlags(info.flags);

    // create `unmount action` for removable device
    if (info.isEjectable) {
        DViewItemActionList lst;
        DViewItemAction *action = new DViewItemAction(Qt::AlignCenter, QSize(16, 16), QSize(), true);
        action->setIcon(QIcon::fromTheme("media-eject-symbolic"));
        action->setVisible(true);
        QObject::connect(action, &QAction::triggered, [info]() {
            SideBarEventCaller::sendEject(info.url);
        });
        lst.push_back(action);
        item->setActionList(Qt::RightEdge, lst);
    }

    return item;
}

SideBarItemSeparator *SideBarHelper::createSeparatorItem(const QString &group)
{
    SideBarItemSeparator *item = new SideBarItemSeparator(group);

    //Currently, only bookmark and tag groups support internal drag.
    //In the next stage, quick access would be instead of bookmark.
    if (item->group() == DefaultGroup::kBookmark || item->group() == DefaultGroup::kTag || item->group() == DefaultGroup::kCommon) {
        auto flags { Qt::ItemIsEnabled | Qt::ItemIsDropEnabled };
        item->setFlags(flags);
    } else
        item->setFlags(Qt::NoItemFlags);

    return item;
}

QString SideBarHelper::makeItemIdentifier(const QString &group, const QUrl &url)
{
    return group + url.url();
}

void SideBarHelper::defaultCdAction(quint64 windowId, const QUrl &url)
{
    if (!url.isEmpty())
        SideBarEventCaller::sendItemActived(windowId, url);
}

void SideBarHelper::defaultContextMenu(quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    // ref (DFMSideBarDefaultItemHandler::contextMenu)
    QMenu *menu = new QMenu;
    menu->addAction(QObject::tr("Open in new window"), [url]() {
        SideBarEventCaller::sendOpenWindow(url);
    });

    auto newTabAct = menu->addAction(QObject::tr("Open in new tab"), [windowId, url]() {
        SideBarEventCaller::sendOpenTab(windowId, url);
    });

    newTabAct->setDisabled(!SideBarEventCaller::sendCheckTabAddable(windowId));

    menu->addSeparator();
    menu->addAction(QObject::tr("Properties"), [url]() {
        SideBarEventCaller::sendShowFilePropertyDialog(url);
    });
    menu->exec(globalPos);
    delete menu;
}

bool SideBarHelper::registerSortFunc(const QString &subGroup, SortFunc func)
{
    if (kSortFuncs.contains(subGroup)) {
        qDebug() << subGroup << "has already been registered";
        return false;
    }
    kSortFuncs.insert(subGroup, func);
    return true;
}

SortFunc SideBarHelper::sortFunc(const QString &subGroup)
{
    return kSortFuncs.value(subGroup, nullptr);
}

void SideBarHelper::updateSideBarSelection(quint64 winId)
{
    auto all = SideBarHelper::allSideBar();
    for (auto sb : all) {
        if (!sb || SideBarHelper::windowId(sb) == winId)
            continue;
        sb->updateSelection();
    }
}

void SideBarHelper::bindSettings()
{
    static const std::map<QString, QString> kvs {
        { "advance.items_in_sidebar.home", "home" },
        { "advance.items_in_sidebar.desktop", "desktop" },
        { "advance.items_in_sidebar.videos", "videos" },
        { "advance.items_in_sidebar.music", "music" },
        { "advance.items_in_sidebar.pictures", "pictures" },
        { "advance.items_in_sidebar.documents", "documents" },
        { "advance.items_in_sidebar.downloads", "downloads" },
        { "advance.items_in_sidebar.trash", "trash" },
        { "advance.items_in_sidebar.computer", "computer" },
        { "advance.items_in_sidebar.vault", "vault" },
        { "advance.items_in_sidebar.builtin", "builtin_disks" },
        { "advance.items_in_sidebar.loop", "loop_dev" },
        { "advance.items_in_sidebar.other_disks", "other_disks" },
        { "advance.items_in_sidebar.computers_in_lan", "computers_in_lan" },
        { "advance.items_in_sidebar.my_shares", "my_shares" },
        { "advance.items_in_sidebar.mounted_share_dirs", "mounted_share_dirs" },
        { "advance.items_in_sidebar.tags", "tags" }
    };

    auto getter = [](const QString &key) {
        return hiddenRules().value(key, true);
    };
    auto saver = [](const QString &key, const QVariant &val) {
        auto curr = hiddenRules();
        curr[key] = val;
        DConfigManager::instance()->setValue(ConfigInfos::kConfName, ConfigInfos::kVisiableKey, curr);
    };

    auto bindConf = [getter, saver](const QString &settingKey, const QString &dconfKey) {
        using namespace std;
        using namespace std::placeholders;
        SettingBackend::instance()->addSettingAccessor(settingKey, bind(getter, dconfKey), bind(saver, dconfKey, _1));
    };

    std::for_each(kvs.begin(), kvs.end(), [bindConf](std::pair<QString, QString> pair) { bindConf(pair.first, pair.second); });

    if (DSysInfo::isCommunityEdition()) {
        SettingDialog::setItemVisiable("advance.items_in_sidebar.vault", false);
        qDebug() << "hide vault config in community edition.";
    }
}

QVariantMap SideBarHelper::hiddenRules()
{
    return DConfigManager::instance()->value(ConfigInfos::kConfName, ConfigInfos::kVisiableKey).toMap();
}

QVariantMap SideBarHelper::groupExpandRules()
{
    return DConfigManager::instance()->value(ConfigInfos::kConfName, ConfigInfos::kGroupExpandedKey).toMap();
}

void SideBarHelper::bindRecentConf()
{
    SyncPair pair {
        { SettingType::kGenAttr, Application::kShowRecentFileEntry },
        { ConfigInfos::kConfName, ConfigInfos::kVisiableKey },
        saveRecentToConf,
        syncRecentToAppSet,
        isRecentConfEqual
    };
    ConfigSynchronizer::instance()->watchChange(pair);
}

void SideBarHelper::saveRecentToConf(const QVariant &var)
{
    auto &&rule = hiddenRules();
    rule["recent"] = var.toBool();
    DConfigManager::instance()->setValue(ConfigInfos::kConfName, ConfigInfos::kVisiableKey, rule);
}

void SideBarHelper::syncRecentToAppSet(const QString &, const QString &, const QVariant &)
{
    Application::instance()->setGenericAttribute(Application::kShowRecentFileEntry, hiddenRules().value("recent", true).toBool());
}

bool SideBarHelper::isRecentConfEqual(const QVariant &dcon, const QVariant &dset)
{
    return dcon.toMap().value("recent", true).toBool() && dset.toBool();
}

void SideBarHelper::saveGroupsStateToConfig(const QVariant &var)
{
    const QStringList keys = var.toMap().keys();

    auto &&rule = groupExpandRules();
    foreach (const QString &key, keys) {
        bool value = var.toMap().value(key).toBool();
        rule[key] = value;
    }
    DConfigManager::instance()->setValue(ConfigInfos::kConfName, ConfigInfos::kGroupExpandedKey, rule);
}

QMutex &SideBarHelper::mutex()
{
    static QMutex m;
    return m;
}
