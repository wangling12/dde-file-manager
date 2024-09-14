// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DAEMONPLUGIN_FILEMANAGER1_GLOBAL_H
#define DAEMONPLUGIN_FILEMANAGER1_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DAEMONPFILEMANAGER1_NAMESPACE daemonplugin_filemanager1

#define DAEMONPFILEMANAGER1_BEGIN_NAMESPACE namespace DAEMONPFILEMANAGER1_NAMESPACE {
#define DAEMONPFILEMANAGER1_END_NAMESPACE }
#define DAEMONPFILEMANAGER1_USE_NAMESPACE using namespace DAEMONPFILEMANAGER1_NAMESPACE;

DAEMONPFILEMANAGER1_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DAEMONPFILEMANAGER1_NAMESPACE)
DAEMONPFILEMANAGER1_END_NAMESPACE

#endif   // DAEMONPLUGIN_FILEMANAGER1_GLOBAL_H
