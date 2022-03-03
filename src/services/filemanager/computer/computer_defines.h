/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef COMPUTER_DEFINES_H
#define COMPUTER_DEFINES_H

#include "dfm_filemanager_service_global.h"

DSB_FM_BEGIN_NAMESPACE

// custom event type
namespace EventType {
extern const int kContextActionTriggered;   // (quint64 winId, QUrl, QString actionText)
extern const int kOnOpenItem;   // (quint64, QUrl)
};   // namespace EventType

DSB_FM_END_NAMESPACE

#endif   // COMPUTER_DEFINES_H
