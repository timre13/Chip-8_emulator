#include "Logger.h"

namespace Logger
{

Logger log{Logger::Type::Log};
Logger warn{Logger::Type::Warning};
Logger err{Logger::Type::Error};

} // End of namespace Logger
