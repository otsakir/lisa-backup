#include <QtTest/QtTest>

#include "core.h"

class TestSourceDetails: public QObject {
    Q_OBJECT

private slots:

    void construct() {
        SourceDetails* details = new SourceDetails();
        delete details;
    }

};

QTEST_MAIN(TestSourceDetails)
//#include "test_core.moc"
