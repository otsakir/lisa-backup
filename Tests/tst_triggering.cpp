//#include <QtTest/QtTest>
#include <QTest>
#include <QSettings>
#include "../App/triggering.h"

#include "../App/core.h"

class TestTriggering: public QObject
{
    Q_OBJECT

public:
    TestTriggering() {
        registerQtMetatypes();
        QSettings settings;
        qCritical() << "QSettings stored in " << settings.fileName();

    }
private slots:

    void testTriggerSetAndGet();
    void testGetTaskForUuid();
    void testGetUuidForTask();

    /*
    void test1() {
        QSettings settings;
        TriggerEntry entry1;
        entry1.lastLabel = "aaa";
        entry1.lastMountPoint = "bbb";

        QVariant var1;
        var1.setValue(entry1);
        settings.setValue("key1", var1);
        TriggerEntry entry2 = settings.value("key1").value<TriggerEntry>();
        QCOMPARE(entry2.lastLabel, "aaa");

    }*/

};


void TestTriggering::testTriggerSetAndGet()
{
    QSettings settings;
    settings.clear();
    MountedDevice triggerEntry;
    QVERIFY(!Triggering::triggerExists("task1"));
    Triggering::enableMountTrigger("task1", triggerEntry);
    QVERIFY(Triggering::triggerExists("task1"));
    Triggering::disableMountTrigger("task1");
    QVERIFY(!Triggering::triggerExists("task1"));

}

void TestTriggering::testGetTaskForUuid()
{
    QSettings settings;
    settings.clear();
    QStringList tasks;
    Triggering::tasksForUuid("missing", tasks);
    QVERIFY(tasks.empty());

    tasks.clear();
    MountedDevice triggerEntry("uuid1");
    Triggering::enableMountTrigger("task1", triggerEntry);
    Triggering::tasksForUuid("uuid1", tasks);
    QCOMPARE(tasks.at(0), "task1");
}

void TestTriggering::testGetUuidForTask()
{
    QSettings settings;
    settings.clear();
    MountedDevice triggerEntry("uuid1");
    QVERIFY(!Triggering::triggerEntryForTask("missing", triggerEntry));

    Triggering::enableMountTrigger("task1", triggerEntry);
    qInfo() << "triggerEntry1: " << triggerEntry;

    MountedDevice triggerEntry2;
    qInfo() << "triggerEntry2 before" << triggerEntry2;
    QVERIFY(Triggering::triggerEntryForTask("task1", triggerEntry2));
    qInfo() << "triggerEntry2 after" << triggerEntry2;

    QCOMPARE(triggerEntry.uuid, triggerEntry2.uuid);
}


//QTEST_MAIN(TestTriggering)

int main(int argc, char *argv[])
{
    TestTriggering testtriggering;
    return QTest::qExec(&testtriggering, argc, argv);
}

#include "tst_triggering.moc"
