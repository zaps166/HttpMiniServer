/*
    MIT License

    Copyright (c) 2017 Błażej Szczygieł

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "HttpClient.hpp"

#include <QCoreApplication>
#include <QHostAddress>
#include <QTcpSocket>
#include <QFileInfo>
#include <QDebug>
#include <QDir>

HttpClient::HttpClient(const QString &root, QTcpSocket *socket) :
    m_root(root),
    m_socket(socket)
{
    qDebug() << "Client connected:" << m_socket->peerAddress();

    QObject::connect(m_socket, &QTcpSocket::readyRead, [this] {
        readClient();
    });
    QObject::connect(m_socket, &QTcpSocket::disconnected, [this] {
        qDebug() << "Client disconnected:" << m_socket->peerAddress();
        m_socket->deleteLater();
        delete this;
    });
}
HttpClient::~HttpClient()
{}

void HttpClient::readClient()
{
    m_buffer += m_socket->readAll();

    const int idx = m_buffer.indexOf("\r\n\r\n");
    if (idx > -1)
    {
        const int getIdx = m_buffer.indexOf("GET ");
        if (getIdx > -1)
            m_requestedPath = QByteArray::fromPercentEncoding(m_buffer.mid(getIdx + 4, m_buffer.indexOf(' ', getIdx + 4) - 4));
        else
            m_requestedPath.clear();
        m_buffer.remove(0, idx + 4);
        serveFile();
    }
}

void HttpClient::serveFile()
{
    if (m_requestedPath.isEmpty())
        return;

    QString path = m_root + m_requestedPath;
    while (path.endsWith("//"))
        path.chop(1);
    if (QFileInfo(path).isDir())
    {
        if (!path.endsWith('/'))
            path += '/';
        if (QFileInfo(path + "index.html").isFile())
            path += "index.html";
    }

    const QFileInfo pathFileInfo(path);
    path = pathFileInfo.absoluteFilePath();

    if (!path.startsWith(m_root))
    {
        const QByteArray errTxt("<b>Access forbidden!</b>");

        qDebug() << "Access forbidden to:" << path << "for" << m_socket->peerAddress();

        m_socket->write(getResponse("403 Forbidden", errTxt.length()));
        m_socket->write(errTxt);
    }
    else if (pathFileInfo.isDir())
    {
        const QStringList entries = QDir(path).entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::Name | QDir::DirsFirst);
        const QByteArray relativePath = path.right(path.length() - m_root.length()).toUtf8();
        QByteArray response = "<b>" + path.toUtf8() + "</b>";

        if (!entries.isEmpty())
            response += "<ul>";
        for (const QString &entry : entries)
            response += "<li><a href='" + relativePath + entry + "'>" + entry + "</a></li>";
        if (!entries.isEmpty())
            response += "</ul>";

        qDebug() << "Serve directory:" << path << "for" << m_socket->peerAddress();

        m_socket->write(getResponse("200 OK", response.length(), "text/html"));
        m_socket->write(response);
    }
    else
    {
        QFile f(path);
        if (f.open(QFile::ReadOnly))
        {
            QByteArray mimeType;
            const QMimeType fileMimeType = m_mimeDB.mimeTypeForFile(pathFileInfo);
            if (fileMimeType.isValid())
                mimeType = fileMimeType.name().toLatin1();

            qDebug() << "Serve file:" << path << "for" << m_socket->peerAddress();

            m_socket->write(getResponse("200 OK", f.size(), mimeType));
            m_socket->write(f.readAll());
        }
        else
        {
            const QByteArray errTxt("<b>Path doesn't exist!</b>");

            qDebug() << "Path doesn't exist:" << path << "for" << m_socket->peerAddress();

            m_socket->write(getResponse("404 Not Found", errTxt.length()));
            m_socket->write(errTxt);
        }
    }
}

QByteArray HttpClient::getResponse(const QByteArray &responseCode, const quint32 length, const QByteArray &mimeType) const
{
    QByteArray response = "HTTP/1.1 "
            + responseCode + "\r\nServer: "
            + QCoreApplication::applicationName().toUtf8()
            + "\r\nConnection: Keep-Alive\r\nContent-Length: "
            + QByteArray::number(length);
    if (!mimeType.isEmpty())
        response += "\r\nContent-Type: " + mimeType + "; charset=utf-8";
    return response + "\r\n\r\n";
}
