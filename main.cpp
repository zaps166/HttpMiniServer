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

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDebug>
#include <QDir>
#ifdef Q_OS_WIN
    #include <QAbstractNativeEventFilter>
    #include <windows.h>
#endif
#include <csignal>

#include "HttpServer.hpp"

static void installSignalHandler()
{
    static const auto signalHandler = [](int sig) {
        QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
        qDebug() << "Quit";
        if (sig > 0)
            signal(sig, SIG_DFL);
    };
#ifdef Q_OS_WIN
    class WmCloseWatcher : public QAbstractNativeEventFilter
    {
        bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) final
        {
            Q_UNUSED(eventType)
            Q_UNUSED(result)
            if (((MSG *)message)->message == WM_CLOSE)
            {
                signalHandler(0);
                return true;
            }
            return false;
        }
    } static wmCloseWatcher;
    QCoreApplication::instance()->installNativeEventFilter(&wmCloseWatcher);
#else
    signal(SIGTERM, signalHandler);
#endif
    signal(SIGINT,  signalHandler);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    installSignalHandler();

    app.setApplicationName("HttpMiniServer");
    app.setApplicationVersion("v0.2");
    app.setOrganizationName("z166");

    QCommandLineParser parser;
    const QList<QCommandLineOption> options {
        {{"root",  "r"}, "Set server root path", "path"},
        {{"port",  "p"}, "Set server TCP port",  "port"},
        {{"local", "l"}, "Allow access only from localhost"}
    };
    parser.addOptions(options);
    parser.addHelpOption();
    parser.process(app);

    const QString root   = parser.value(options[0]);
    const quint16 port   = parser.value(options[1]).toUShort();
    const bool localOnly = parser.isSet(options[2]);

    QFileInfo rootFileInfo((root.isEmpty() ? QDir::currentPath() : root) + "/");

    if (!rootFileInfo.isDir())
    {
        qDebug() << "Root directory doesn't exist!";
        return -1;
    }
    if (port < 1024)
    {
        qDebug() << "Incorrect port, please set port from range [1024-65535]!";
        return -2;
    }

    HttpServer server(rootFileInfo.absolutePath());
    if (!server.listen(localOnly ? QHostAddress::LocalHost : QHostAddress::Any, port))
    {
        qDebug() << "Can't start server for given port!";
        return -3;
    }

    qDebug().nospace() << "Server started at port: " << port << ", root: " << rootFileInfo.absolutePath();

    return app.exec();
}
