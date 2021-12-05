#ifndef SCRIPTING_H
#define SCRIPTING_H

#include <core.h>

namespace Lb {

bool generateBackupScript(QString scriptTemplate, QString outfilename, const PersistenceModel& appstate);

}
#endif // SCRIPTING_H
