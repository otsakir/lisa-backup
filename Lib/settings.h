#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>


#define GET_INT_SETTING(enumtype) static_cast<enumtype>(settings.value(enumtype##Key).toInt());
#define ADD_COMBO_ITEM(comboptr,enumitem) comboptr->addItem(Settings::toCaption(enumitem), static_cast<int>(enumitem));


namespace Settings
{
    enum class Loglevel {
        All = 1,
        Errors = 2
    };

    static QString LoglevelKey = "logging/Loglevel";

    static QString toCaption(Loglevel loglevel)
    {
        switch (loglevel)
        {
            case Loglevel::All:
                return "Everything";
            case Loglevel::Errors:
                return "Errors only";
        }
    }

    enum class Taskrunner
    {
        Gui = 1,
        Script = 2
    };

    static QString TaskrunnerKey = "taskrunner/mode";

    static QString toCaption(Taskrunner taskrunnerMode)
    {
        switch (taskrunnerMode)
        {
            case Taskrunner::Gui:
                return "GUI";
            case Taskrunner::Script:
                return "Bash scripts (insecure)";
        }
    }


}


#endif // SETTINGS_H
