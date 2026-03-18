#pragma once
// Size of reserve of fthe internal buffer used for assembling log messages.
#ifndef LOGGER_REVERSE_BUFFER_SIZE
#define LOGGER_REVERSE_BUFFER_SIZE 1024
#endif

#ifndef LOGGER_USE_PREFIX
// 1 = [filename.ext::funcName::line]
// 2 = [funcName::line]
// 3 = [fileName.ext::line]
// 4 = [fileName.ext::funcName]
//  no format added
#define LOGGER_USE_PREFIX 1
#endif



#ifndef PRINTABLEBUFFOR_SUPPORT_JSON
#define PRINTABLEBUFFOR_SUPPORT_JSON 1
#endif

#if PRINTABLEBUFFOR_SUPPORT_JSON == 1

#ifndef PRINTABLEBUFFOR_USE_JSON_PRETTY
#define PRINTABLEBUFFOR_USE_JSON_PRETTY 0
#endif

#if PRINTABLEBUFFOR_USE_JSON_PRETTY == 1
#define serializationMethod(source, destination) serializeJsonPretty(source, destination)
#else
#define serializationMethod(source, destination) serializeJson(source, destination)
#endif

#endif
