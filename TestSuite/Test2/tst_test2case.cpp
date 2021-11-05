#include <QtTest>

// add necessary includes here

#include <../../../LisaBackup/Lib/core.h>

class Test2Case : public QObject
{
    Q_OBJECT

public:
    Test2Case();
    ~Test2Case();

private slots:
    void test_case1();

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

    //details.backupDepth

}

QTEST_APPLESS_MAIN(Test2Case)

#include "tst_test2case.moc"
