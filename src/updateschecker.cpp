/*
 *  Copyright (c) 2020 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved. 
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:

 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.

 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
#include <QXmlStreamReader>
#include "updateschecker.h"
#include "version.h"

UpdatesChecker::UpdatesChecker()
{
    connect(&m_networkAccessManager, &QNetworkAccessManager::finished, this, &UpdatesChecker::downloadFinished);
}

void UpdatesChecker::start()
{
    QUrl url(APP_UPDATES_CHECKER_URL);
    QNetworkRequest request(url);
    m_networkAccessManager.get(request);
}

const QString &UpdatesChecker::message() const
{
    return m_message;
}

bool UpdatesChecker::isLatest() const
{
    return m_isLatest;
}

bool UpdatesChecker::hasError() const
{
    return m_hasError;
}

bool UpdatesChecker::parseUpdateInfoXml(const QByteArray &updateInfoXml, std::vector<UpdateItem> *updateItems)
{
    std::vector<QString> elementNameStack;
    QXmlStreamReader reader(updateInfoXml);
    while (!reader.atEnd()) {
        reader.readNext();
        if (!reader.isStartElement() && !reader.isEndElement()) {
            if (!reader.name().toString().isEmpty())
                qDebug() << "Skip xml element:" << reader.name().toString() << " tokenType:" << reader.tokenType();
            continue;
        }
        QString baseName = reader.name().toString();
        if (reader.isStartElement()) {
            elementNameStack.push_back(baseName);
            if (elementNameStack.size() > 10) {
                qDebug() << "Invalid xml, element name stack exceed limits";
                return false;
            }
        }
        QStringList nameItems;
        for (const auto &nameItem: elementNameStack) {
            nameItems.append(nameItem);
        }
        QString fullName = nameItems.join(".");
        if (reader.isEndElement())
            elementNameStack.pop_back();
        if (reader.isStartElement()) {
            if (fullName == "updates.update") {
                UpdateItem updateItem;
                updateItem.forTags = reader.attributes().value("for").toString().toLower();
                updateItem.version = reader.attributes().value("version").toString();
                updateItem.humanVersion = reader.attributes().value("humanVersion").toString();
                updateItem.descriptionUrl = reader.attributes().value("descriptionUrl").toString();
                if (!updateItem.forTags.isEmpty() &&
                        !updateItem.version.isEmpty() &&
                        !updateItem.humanVersion.isEmpty() &&
                        !updateItem.descriptionUrl.isEmpty()) {
                    if (updateItems->size() >= 100)
                        return false;
                    updateItem.forTags = "," + updateItem.forTags + ",";
                    updateItems->push_back(updateItem);
                }
            }
        }
    }
    return true;
}

bool UpdatesChecker::isVersionLessThan(const QString &version, const QString &compareWith)
{
    auto versionTokens = version.split(".");
    auto compareWithTokens = compareWith.split(".");
    if (versionTokens.size() > 4) {
        return false;
    }
    while (compareWithTokens.size() < 4)
        compareWithTokens.push_back("0");
    if (compareWithTokens.size() > 4) {
        return false;
    }
    while (compareWithTokens.size() < 4)
        compareWithTokens.push_back("0");
    for (size_t i = 0; i < 4; ++i) {
        int left = versionTokens[i].toInt();
        int right = compareWithTokens[i].toInt();
        if (left > right)
            return false;
        else if (left < right)
            return true;
    }
    return false;
}

const UpdatesChecker::UpdateItem &UpdatesChecker::matchedUpdateItem() const
{
    return m_matchedUpdateItem;
}

void UpdatesChecker::downloadFinished(QNetworkReply *reply)
{
    if (reply->error()) {
        qDebug() << "Download update info failed:" << qPrintable(reply->errorString());
        m_message = tr("Fetch update info failed, please retry later");
    } else {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (200 != statusCode) {
            qDebug() << "Download update info failed, statusCode:" << statusCode;
            m_message = tr("Fetch update info failed, please retry later");
        } else {
            std::vector<UpdateItem> updateItems;
            if (!parseUpdateInfoXml(reply->readAll(), &updateItems)) {
                m_message = tr("Fetch update info failed, please retry later");
            } else {
                m_isLatest = true;
                m_hasError = false;
                m_message = tr("%1 %2 is currently the newest version available").arg(APP_NAME).arg(APP_HUMAN_VER);
                QString platform = QString(APP_PLATFORM).toLower();
                if (!platform.isEmpty()) {
                    platform = "," + platform + ",";
                    for (const auto &it: updateItems) {
                        if (-1 == it.forTags.indexOf(platform))
                            continue;
                        if (isVersionLessThan(APP_VER, it.version)) {
                            m_isLatest = false;
                            m_message = tr("An update is available: %1 %2").arg(APP_NAME).arg(it.humanVersion);
                            m_matchedUpdateItem = it;
                            break;
                        }
                    }
                }
            }
        }
    }
    reply->deleteLater();
    emit finished();
}
