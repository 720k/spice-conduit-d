#include "ServiceApplication.h"
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    ServiceApplication      app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("spice-conduit-d"));
    QCoreApplication::setApplicationVersion(QStringLiteral("2020.0218.1"));
    return app.exec();
}
