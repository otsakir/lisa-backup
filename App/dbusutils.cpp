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

