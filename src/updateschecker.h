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
#ifndef AUTO_REMESHER_UPDATES_CHECKER_H
#define AUTO_REMESHER_UPDATES_CHECKER_H
#include <QObject>
#include <QtNetwork>

class UpdatesChecker : public QObject
{
    Q_OBJECT
signals:
    void finished();
public:
    struct UpdateItem
    {
        QString forTags;
        QString version;
        QString humanVersion;
        QString descriptionUrl;
    };
    
    UpdatesChecker();
    void start();
    bool isLatest() const;
    bool hasError() const;
    const QString &message() const;
    const UpdateItem &matchedUpdateItem() const;
private slots:
    void downloadFinished(QNetworkReply *reply);
private:
    QNetworkAccessManager m_networkAccessManager;
    bool m_isLatest = false;
    QString m_message;
    QString m_latestUrl;
    bool m_hasError = true;
    UpdateItem m_matchedUpdateItem;
    
    bool parseUpdateInfoXml(const QByteArray &updateInfoXml, std::vector<UpdateItem> *updateItems);
    static bool isVersionLessThan(const QString &version, const QString &compareWith);
};

#endif
