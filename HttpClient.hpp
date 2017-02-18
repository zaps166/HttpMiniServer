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

#ifndef HTTPCLIENT_HPP
#define HTTPCLIENT_HPP

#include <QMimeDatabase>
#include <QByteArray>
#include <QString>

class QTcpSocket;

class HttpClient
{
public:
    HttpClient(const QString &root, QTcpSocket *socket);
    ~HttpClient();

private:
    void readClient();

    void serveFile();

    QByteArray getResponse(const QByteArray &responseCode, const quint32 length, const QByteArray &mimeType = QByteArray()) const;

    const QString m_root;

    QTcpSocket *m_socket;

    QString m_requestedPath;

    QByteArray m_buffer;

    QMimeDatabase m_mimeDB;
};

#endif // HTTPCLIENT_HPP
