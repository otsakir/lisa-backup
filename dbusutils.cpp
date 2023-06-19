#include "dbusutils.h"
#include "qdebug.h"

#include <QDBusArgument>
#include <QDBusConnection>


void DbusUtils::onRemoteSignal(const QDBusMessage& message)
{
    const QDBusArgument argument = message.arguments().at(1).value<QDBusArgument>();
    InterfaceList interfaceList;
    argument >> interfaceList;
    QString label, uuid;
    if (parseInterfacesAddedSignal(message.arguments().at(0).value<QDBusObjectPath>().path(), interfaceList, label, uuid))
    {
        emit labeledDeviceMounted(label, uuid);
    }
}


void DbusUtils::registerOnMount()
{
    bool registered = QDBusConnection::systemBus().connect("org.freedesktop.UDisks2",
                  "/org/freedesktop/UDisks2",
                  "org.freedesktop.DBus.ObjectManager",
                  "InterfacesAdded", "oa{sa{sv}}", this, SLOT(onRemoteSignal(const QDBusMessage&)));
    if (!registered)
        qWarning() << "DbusUtils: canot watch 'InterfacesAdded' signal";
}

void DbusUtils::unregisterOnMount()
{
    QDBusConnection::systemBus().disconnect("org.freedesktop.UDisks2",
                                            "/org/freedesktop/UDisks2",
                                            "org.freedesktop.DBus.ObjectManager",
                                            "InterfacesAdded", "oa{sa{sv}}", this, SLOT(onRemoteSignal(const QDBusMessage&)));
}


/**
 * @brief Parse DBus InterfacesAdded signal
 *
 * Parse signal data for added `block_devices`. Return `label` and `uuid` if present or return "".
 *
 * Sample DBus response
 *
 *  * object path "/org/freedesktop/UDisks2/block_devices/sda1"
   array [
      dict entry(
         string "org.freedesktop.UDisks2.Block"

         dict entry(
               string "IdLabel"
               variant                   string "CAR"
            )
            dict entry(
               string "IdUUID"
               variant                   string "6F58-F5AA"
            )
 *
 * @param objectPath
 * @param interfaceList
 * @param label
 * @param uuid
 * @return true if this is about a new block device
 */
bool DbusUtils::parseInterfacesAddedSignal(const QString& objectPath, const InterfaceList interfaceList, QString& label, QString& uuid)
{
    if (objectPath.startsWith("/org/freedesktop/UDisks2/block_devices/"))
    {
        QVariant qvariant = interfaceList["org.freedesktop.UDisks2.Block"]["IdLabel"];
        label = qvariant.isValid() ? qvariant.toString() : "";
        qvariant = interfaceList["org.freedesktop.UDisks2.Block"]["IdUUID"];
        uuid = qvariant.isValid() ? qvariant.toString() : "";
        return true;
    }
    return false;
}


typedef QMap<QString, QMap<QString, QVariant>> BlockDevice; // a{oa{sa{sv}}}
typedef QMap<QDBusObjectPath, BlockDevice> ManagedObjects;
typedef QList<QByteArray> MountPoints;

bool DbusUtils::getMountedDevices(QList<MountedDevice>& mountedDevices)
{
    // org.freedesktop.DBus.ObjectManager.GetManagedObjects (out ARRAY of DICT_ENTRY<OBJPATH,ARRAY of DICT_ENTRY<STRING,ARRAY of DICT_ENTRY<STRING,VARIANT>>> objpath_interfaces_and_properties);
    // sudo dbus-send --system --print-reply --reply-timeout=2000 --type=method_call --dest=org.freedesktop.UDisks2 /org/freedesktop/UDisks2 org.freedesktop.DBus.ObjectManager.GetManagedObjects

    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.isConnected())
        return false; // error

    QDBusMessage message;
    message = QDBusMessage::createMethodCall("org.freedesktop.UDisks2","/org/freedesktop/UDisks2","org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
    QDBusMessage reply = QDBusConnection::systemBus().call(message);

    if (reply.type() != QDBusMessage::ReplyMessage)
        return false; // error

    mountedDevices.clear();
    ManagedObjects managedObjects;
    const QDBusArgument argument = reply.arguments().at(0).value<QDBusArgument>();
    argument >> managedObjects;

    ManagedObjects::const_iterator managedObjects_i = managedObjects.constBegin();

    while (managedObjects_i != managedObjects.constEnd())
    {
        const QString& objectPath = managedObjects_i.key().path();

        if (objectPath.startsWith("/org/freedesktop/UDisks2/block_devices/"))
        {
            MountedDevice mountedDevice;

            const BlockDevice& blockDevice = managedObjects_i.value();
            QDBusArgument mountPointsArgument = blockDevice["org.freedesktop.UDisks2.Filesystem"]["MountPoints"].value<QDBusArgument>();

            MountPoints mountPoints;
            if (mountPointsArgument.currentType() != QDBusArgument::UnknownType)
            {
                mountPointsArgument >>  mountPoints;
                MountPoints::const_iterator mountPoints_i = mountPoints.constBegin();
                while (mountPoints_i != mountPoints.constEnd())
                {
                    QString mountPointString(*mountPoints_i);
                    mountedDevice.mountPoints.append(mountPointString);

                    mountPoints_i++;
                }
                mountedDevice.label = blockDevice["org.freedesktop.UDisks2.Block"]["IdLabel"].toString();
                mountedDevice.uuid = blockDevice["org.freedesktop.UDisks2.Block"]["IdUUID"].toString();

                qDebug() << mountedDevice;
                mountedDevices.append(mountedDevice);
                //qDebug() << objectPath << ": " << stringList << "|" << "IdLabel:" <<  blockDevice["org.freedesktop.UDisks2.Block"]["IdLabel"] << "|" << blockDevice["org.freedesktop.UDisks2.Block"]["IdUUID"];
            }
        }
        managedObjects_i++;
    }

    return true;
}

