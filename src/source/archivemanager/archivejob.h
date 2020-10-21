/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     gaoxiang <gaoxiang@uniontech.com>
*
* Maintainer: gaoxiang <gaoxiang@uniontech.com>
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
#ifndef ARCHIVEJOB_H
#define ARCHIVEJOB_H

#include <QObject>
#include "commonstruct.h"

// 操作基类
class ArchiveJob : public QObject
{
    Q_OBJECT
public:
    explicit ArchiveJob(QObject *parent = nullptr);
    ~ArchiveJob() override;

    // 操作类型
    enum JobType {
        JT_Create,          // 创建压缩包
        JT_Add,             // 添加压缩文件
        JT_Load,            // 加载压缩包
        JT_Extract,         // 解压
    };

    /**
     * @brief start     开始操作
     */
    virtual void start() = 0;

Q_SIGNALS:
    /**
     * @brief slotFinished  操作结束处理
     * @param eType 结束类型
     */
    void signalJobFinshed();

    /**
     * @brief signalprogress    进度信号
     * @param iPercent  进度值
     */
    void signalprogress(double dPercentage);

    /**
     * @brief signalCurFileName 发送当前正在操作的文件名
     * @param strName   当前文件名
     */
    void signalCurFileName(const QString &strName);

public:
    JobType m_eJobType;     // 操作类型


};

#endif // ARCHIVEJOB_H