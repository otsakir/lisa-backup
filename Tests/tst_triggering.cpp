//#include <QtTest/QtTest>
#include <QTest>
#include "../App/triggering.h"


class TestTriggering: public QObject
{
    Q_OBJECT

private slots:

    void testTriggerSetAndGet();
    void testGetTaskForUuid();
    void testGetUuidForTask();

};


void TestTriggering::testTriggerSetAndGet()
{
    QSettings settings;
    settings.clear();
    QVERIFY(!Triggering::triggerExists("task1"));
    Triggering::enableMountTrigger("task1", "uuid1");
    QVERIFY(Triggering::triggerExists("task1"));
    Triggering::disableMountTrigger("task1");
    QVERIFY(!Triggering::triggerExists("task1"));

}

void TestTriggering::testGetTaskForUuid()
{
    QSettings settings;
    settings.clear();
    QString taskid = Triggering::triggerTaskForUuid("missing");
    QVERIFY(taskid.isEmpty());

    Triggering::enableMountTrigger("task1", "uuid1");
    taskid = Triggering::triggerTaskForUuid("uuid1");
    QCOMPARE(taskid, "task1");
}

void TestTriggering::testGetUuidForTask()
{
    QSettings settings;
    settings.clear();
    QString taskid = Triggering::triggerUuidForTask("missing");
    QVERIFY(taskid.isEmpty());

    Triggering::enableMountTrigger("task1", "uuid1");
    taskid = Triggering::triggerUuidForTask("task1");
    QCOMPARE(taskid, "uuid1");
}


//QTEST_MAIN(TestTriggering)

int main(int argc, char *argv[])
{
    TestTriggering testtriggering;
    return QTest::qExec(&testtriggering, argc, argv);
}

#include "tst_triggering.moc"
