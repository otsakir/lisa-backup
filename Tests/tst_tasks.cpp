#include <QTest>
#include "../App/task.h"
//#include "../App/triggering.h"


class TestTasks: public QObject
{
    Q_OBJECT

private slots:

    void testTaskList();

};



void TestTasks::testTaskList()
{
    TaskLoader taskLoader("task-repos/repo1");
    QList<QString> taskInfoItems = taskLoader.getTaskInfo();

    QVERIFY(taskInfoItems.size() == 2);

    QVERIFY(taskInfoItems.at(0) == "task1");


}



//QTEST_MAIN(TestTriggering)

int main(int argc, char *argv[])
{
    TestTasks testTasks;
    return QTest::qExec(&testTasks, argc, argv);
}

#include "tst_tasks.moc"
