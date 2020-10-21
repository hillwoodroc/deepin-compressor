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

#include "progresspage.h"

#include <DFontSizeManager>

#include <QHBoxLayout>
#include <QFileIconProvider>
#include <QTimer>
#include <QDebug>
#include <QCoreApplication>


ProgressPage::ProgressPage(QWidget *parent)
    : DWidget(parent)
{
    initUI();
    initConnections();
}

ProgressPage::~ProgressPage()
{

}

void ProgressPage::setProgressType(Progress_Type eType)
{
    m_eType = eType;

    if (m_eType == PT_Compress || m_eType == PT_CompressAdd) { // 压缩
        m_pSpeedLbl->setText(tr("Speed", "compress") + ": " + tr("Calculating..."));
    }  else if (m_eType == PT_Delete) { // 删除
        m_pSpeedLbl->setText(tr("Speed", "delete") + ": " + tr("Calculating..."));
    }  else if (m_eType == PT_Convert) { // 格式转换
        m_pSpeedLbl->setText(tr("Speed", "convert") + ": " + tr("Calculating..."));
    } else { // 解压
        m_pSpeedLbl->setText(tr("Speed", "uncompress") + ": " + tr("Calculating..."));
    }

    m_pRemainingTimeLbl->setText(tr("Time left") + ": " + tr("Calculating...")); // 剩余时间计算中
}

void ProgressPage::setArchiveName(const QString &strArchiveName, qint64 qTotalSize)
{
    m_pArchiveNameLbl->setText(strArchiveName);     // 设置压缩包名称
    m_qTotalSize = qTotalSize;

    // 设置类型图片
    QFileInfo fileinfo(strArchiveName);
    QFileIconProvider provider;
    QIcon icon = provider.icon(QFileInfo("temp." + fileinfo.completeSuffix()));
    m_pPixmapLbl->setPixmap(icon.pixmap(128, 128));
}

void ProgressPage::setProgress(double dPercent)
{
    if ((m_dPerent - dPercent) == 0.0 || (m_dPerent > dPercent)) {
        return ;
    }
    m_dPerent = dPercent;
    m_pProgressBar->setValue(m_dPerent);     // 进度条刷新值
    m_pProgressBar->update();

    // 刷新界面显示
    double dSpeed;
    qint64 qRemainingTime;
    calSpeedAndRemainingTime(dSpeed, qRemainingTime);
    m_timer.restart();      // 重启定时器

    // 显示速度个剩余时间
    displaySpeedAndTime(dSpeed, qRemainingTime);

}

void ProgressPage::setCurrentFileName(const QString &strFileName)
{
    QFontMetrics elideFont(m_pFileNameLbl->font());

    if (m_eType == PT_Compress || m_eType == PT_CompressAdd) {   // 压缩和追加压缩
        m_pFileNameLbl->setText(elideFont.elidedText(tr("Compressing") + ": " + strFileName, Qt::ElideMiddle, 520));
    } else if (m_eType == PT_Delete) {   // 删除
        m_pFileNameLbl->setText(elideFont.elidedText(tr("Deleting") + ": " + strFileName, Qt::ElideMiddle, 520));
    } else if (m_eType == PT_Convert) {     // 转换
        m_pFileNameLbl->setText(elideFont.elidedText(tr("Converting") + ": " + strFileName, Qt::ElideMiddle, 520));
    } else {    // 解压
        m_pFileNameLbl->setText(elideFont.elidedText(tr("Extracting") + ": " + strFileName, Qt::ElideMiddle, 520));
    }
}

void ProgressPage::resetProgress()
{
    // 重置相关参数
    m_pProgressBar->setValue(0);
    m_pFileNameLbl->setText(tr("Calculating..."));
    m_timer.elapsed();
    m_dPerent = 0;
    m_qConsumeTime = 0;
}

void ProgressPage::initUI()
{
    // 初始化控件
    m_pPixmapLbl = new DLabel(this);
    m_pArchiveNameLbl = new DLabel(this);
    m_pProgressBar = new DProgressBar(this);
    m_pFileNameLbl = new DLabel(this);
    m_pSpeedLbl = new DLabel(this);
    m_pRemainingTimeLbl = new DLabel(this);
    m_pCancelBtn = new DPushButton(tr("Cancel"), this);
    m_pPauseContinueButton = new DSuggestButton(tr("Pause"), this);

    // 初始化压缩包名称样式
    DFontSizeManager::instance()->bind(m_pArchiveNameLbl, DFontSizeManager::T5, QFont::DemiBold);
    m_pArchiveNameLbl->setForegroundRole(DPalette::ToolTipText);

    // 配置进度条
    m_pProgressBar->setRange(0, 100);
    m_pProgressBar->setFixedSize(240, 8);
    m_pProgressBar->setValue(1);
    m_pProgressBar->setOrientation(Qt::Horizontal);  //水平方向
    m_pProgressBar->setAlignment(Qt::AlignVCenter);
    m_pProgressBar->setTextVisible(false);

    // 设置文件名样式
    m_pFileNameLbl->setMaximumWidth(520);
    m_pFileNameLbl->setForegroundRole(DPalette::TextTips);
    DFontSizeManager::instance()->bind(m_pFileNameLbl, DFontSizeManager::T8);

    // 设置速度和剩余时间样式
    DFontSizeManager::instance()->bind(m_pSpeedLbl, DFontSizeManager::T8);
    DFontSizeManager::instance()->bind(m_pRemainingTimeLbl, DFontSizeManager::T8);

    // 设置取消按钮样式
    m_pCancelBtn->setMinimumSize(200, 36);

    // 设置暂停继续按钮样式
    m_pPauseContinueButton->setMinimumSize(200, 36);
    m_pPauseContinueButton->setCheckable(true);

    // 速度和剩余时间布局
    QHBoxLayout *pSpeedLayout = new QHBoxLayout;
    pSpeedLayout->addStretch();
    pSpeedLayout->addWidget(m_pSpeedLbl);
    pSpeedLayout->addSpacing(15);
    pSpeedLayout->addWidget(m_pRemainingTimeLbl);
    pSpeedLayout->addStretch();

    // 按钮布局
    QHBoxLayout *pBtnLayout = new QHBoxLayout;
    pBtnLayout->addStretch(1);
    pBtnLayout->addWidget(m_pCancelBtn, 2);
    pBtnLayout->addSpacing(10);
    pBtnLayout->addWidget(m_pPauseContinueButton, 2);
    pBtnLayout->addStretch(1);

    // 主布局
    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    pMainLayout->addStretch();
    pMainLayout->addWidget(m_pPixmapLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pMainLayout->addSpacing(5);
    pMainLayout->addWidget(m_pArchiveNameLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pMainLayout->addSpacing(25);
    pMainLayout->addWidget(m_pProgressBar, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pMainLayout->addSpacing(5);
    pMainLayout->addWidget(m_pFileNameLbl, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    pMainLayout->addSpacing(5);
    pMainLayout->addLayout(pSpeedLayout);
    pMainLayout->addStretch();
    pMainLayout->addLayout(pBtnLayout);
    pMainLayout->setContentsMargins(12, 6, 20, 20);

    setBackgroundRole(DPalette::Base);
    setAutoFillBackground(true);

    // 临时测试界面
    setArchiveName("新建归档文件.zip", 102400);
    setProgress(50);
    setCurrentFileName("55555.txt");
}

void ProgressPage::initConnections()
{

}

void ProgressPage::calSpeedAndRemainingTime(double &dSpeed, qint64 &qRemainingTime)
{
    if (m_qConsumeTime < 0) {
        m_timer.start();
    }
    m_qConsumeTime += m_timer.elapsed();

    // 计算速度
    if (m_qConsumeTime == 0) {
        dSpeed = 0.0; //处理速度
    } else {
        if (m_eType == PT_Convert) {
            dSpeed = 2 * (m_qTotalSize / 1024.0) * (m_dPerent / 100) / m_qConsumeTime * 1000;
        } else {
            dSpeed = (m_qTotalSize / 1024.0) * (m_dPerent / 100) / m_qConsumeTime * 1000;
        }
    }

    // 计算剩余时间
    double sizeLeft = 0;
    if (m_eType == PT_Convert) {
        sizeLeft = (m_qTotalSize * 2 / 1024.0) * (100 - m_dPerent) / 100; //剩余大小
    } else {
        sizeLeft = (m_qTotalSize / 1024.0) * (100 - m_dPerent) / 100; //剩余大小
    }

    if (dSpeed != 0.0) {
        qRemainingTime = qint64(sizeLeft / dSpeed); //剩余时间
    }

    if (qRemainingTime != 100 && qRemainingTime == 0) {
        qRemainingTime = 1;
    }
}

void ProgressPage::displaySpeedAndTime(double dSpeed, qint64 qRemainingTime)
{
    // 计算剩余需要的小时。
    qint64 hour = qRemainingTime / 3600;
    // 计算剩余的分钟
    qint64 minute = (qRemainingTime - hour * 3600) / 60;
    // 计算剩余的秒数
    qint64 seconds = qRemainingTime - hour * 3600 - minute * 60;
    // 格式化数据
    QString hh = QString("%1").arg(hour, 2, 10, QLatin1Char('0'));
    QString mm = QString("%1").arg(minute, 2, 10, QLatin1Char('0'));
    QString ss = QString("%1").arg(seconds, 2, 10, QLatin1Char('0'));

    //add update speed and time label
    if (m_eType == PT_Compress || m_eType == PT_CompressAdd) {
        if (dSpeed < 1024) {
            // 速度小于1024k， 显示速度单位为KB/S
            m_pSpeedLbl->setText(tr("Speed", "compress") + ": " + QString::number(dSpeed, 'f', 2) + "KB/S");
        } else if (dSpeed > 1024 && dSpeed < 1024 * 300) {
            // 速度大于1M/S，且小于300MB/S， 显示速度单位为MB/S
            m_pSpeedLbl->setText(tr("Speed", "compress") + ": " + QString::number((dSpeed / 1024), 'f', 2) + "MB/S");
        } else {
            // 速度大于300MB/S，显示速度为>300MB/S
            m_pSpeedLbl->setText(tr("Speed", "compress") + ": " + ">300MB/S");
        }
    } else if (m_eType == PT_Delete) {
        if (dSpeed < 1024) {
            m_pSpeedLbl->setText(tr("Speed", "delete") + ": " + QString::number(dSpeed, 'f', 2) + "KB/S");
        } else {
            m_pSpeedLbl->setText(tr("Speed", "delete") + ": " + QString::number((dSpeed / 1024), 'f', 2) + "MB/S");
        }

    } else if (m_eType == PT_UnCompress) {
        if (dSpeed < 1024) {
            m_pSpeedLbl->setText(tr("Speed", "uncompress") + ": " + QString::number(dSpeed, 'f', 2) + "KB/S");
        } else if (dSpeed > 1024 && dSpeed < 1024 * 300) {
            m_pSpeedLbl->setText(tr("Speed", "uncompress") + ": " + QString::number((dSpeed / 1024), 'f', 2) + "MB/S");
        } else {
            m_pSpeedLbl->setText(tr("Speed", "uncompress") + ": " + ">300MB/S");
        }
    } else if (m_eType == PT_Convert) {
        if (dSpeed < 1024) {
            m_pSpeedLbl->setText(tr("Speed", "convert") + ": " + QString::number(dSpeed, 'f', 2) + "KB/S");
        } else if (dSpeed > 1024 && dSpeed < 1024 * 300) {
            m_pSpeedLbl->setText(tr("Speed", "convert") + ": " + QString::number((dSpeed / 1024), 'f', 2) + "MB/S");
        } else {
            m_pSpeedLbl->setText(tr("Speed", "convert") + ": " + ">300MB/S");
        }
    }

    // 设置剩余时间
    m_pRemainingTimeLbl->setText(tr("Time left") + ": " + hh + ":" + mm + ":" + ss);
}