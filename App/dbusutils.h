#ifndef DBUSUTILS_H
#define DBUSUTILS_H

#include <QMap>
#include <QVariant>
#include <QDBusMessage>
#include "core.h"

class DbusUtils : public QObject
{
    Q_OBJECT

private:
    typedef QMap<QString, QMap<QString, QVariant>> InterfaceList;

public:
    void registerOnMount();
    void unregisterOnMount();
    static bool getMountedDevices(QList<MountedDevice>& mountedDevices);

    signals:
    void labeledDeviceMounted(const QString& label, const QString& uuid); // hook this to get notified when a mount point is up


private slots:
    void onRemoteSignal(const QDBusMessage& message);
    bool parseInterfacesAddedSignal(const QString& objectPath, const InterfaceList interfaceList, QString& label, QString& uuid);

};



#endif // DBUSUTILS_H
