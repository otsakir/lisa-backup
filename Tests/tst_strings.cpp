#include <QtTest/QtTest>

#include "../App/core.h"
#include "../App/utils.h"

class TestQString: public QObject
{
    Q_OBJECT

private slots:
    void testFromUtf8();
    void testToUtf8();
};


void TestQString::testFromUtf8()
{
    std::string str("Hello");
    auto qstr = QString::fromUtf8(str.data());
    QCOMPARE(qstr, QString("Hello"));
    Lb::appVersion();
}

void TestQString::testToUtf8()
{
    QString qstr("Hello");
    std::string str(qstr.toUtf8().data());
    QCOMPARE(str, std::string("Hello"));
}


//QTEST_MAIN(TestQString)

int main(int argc, char *argv[])
{
    TestQString testqstring;
    return QTest::qExec(&testqstring, argc, argv);
}

#include "tst_strings.moc"
