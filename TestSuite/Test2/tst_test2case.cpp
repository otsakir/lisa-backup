#include <QtTest>

// add necessary includes here

#include <../../../LisaBackup/Lib/core.h>
#include <../../../LisaBackup/Lib/utils.h>

class Test2Case : public QObject
{
    Q_OBJECT

public:
    Test2Case();
    ~Test2Case();

private slots:
    void test_case1();
    void test_checkDirectories();

};

Test2Case::Test2Case()
{

}

Test2Case::~Test2Case()
{

}

void Test2Case::test_case1()
{
    SourceDetails details;
    details.backupDepth = SourceDetails::rootOnly;

    SourceDetails details2(SourceDetails::all);

    Lb::username();
    Lb::userGroup();


    //details.backupDepth

}

    void Test2Case::test_checkDirectories() {
    //QCOMPARE(Lb::dataDirectory(), "asdf");
    //QCOMPARE(Lb::scriptsDirectory(), "fdsa");
    QCOMPARE(Lb::configDirectory(), "fdsa");
}

QTEST_APPLESS_MAIN(Test2Case)

#include "tst_test2case.moc"
