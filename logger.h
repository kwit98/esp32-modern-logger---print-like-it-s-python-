#pragma once
#include <Arduino.h>
#include <esp_log.h>
#include <algorithm>
#include <type_traits>
#include <vector>

#include "printableBuffer.h"

/* TODO:
 add support for std::string

make it possible to set log level per printer???
make queue of printablebuffors and execute them in asyc (to prevent blocking in event drive code, like socket responsess )

add pre-compilation check using macro to prevent adding empty prefix as "" (unnecessary overhead)
*/

#if __cplusplus < 201703L
#warning "Library should be compiled with C++17 or newer."
In platformio.ini set : (works both with Arduino IDE and ESP - IDF) build_unflags = -std = gnu++ 11 build_flags = -std = gnu++ 2a
#endif



namespace logger
{
  namespace detail {
  /////////////////////////////////////////////////////////////////////////
  // PRIVATE API, can be accesses by end user, but not recommended
  /////////////////////////////////////////////////////////////////////////
  inline SemaphoreHandle_t printMutex = NULL;
  inline printableBuffer buff(LOGGER_REVERSE_BUFFER_SIZE); // move from stati
  inline std::vector<Print *> printers;

  /* __FILE__ returns full path to file, it's used to cut it into file.ext. Done on primitive string to make it to compilator, less use of run-time
   * resources */
  constexpr const char *stripPath(const char *path) // Strings are only run time
  {
    const char *p = path;

    for (const char *s = path; *s != '\0'; ++s)
      if (*s == '/' || *s == '\\')
        p = s + 1;

    return p;
  }

  // helper needed because folding doesn't allow it (?? check if true)
  template <typename T> void printArg(printableBuffer &buff, const T &arg)
  {
    buff.print(arg);
    buff.print(' ');
  }

  // Implementation of the printing function, calling it directly omits using prefix. Macro log expands to it, needed to unfold __FILE__,
  // __FUNCTION__, __LINE__ values
  template <typename... Args> void log_imp(const Args &...args)
  {
    if (!printMutex)
      printMutex = xSemaphoreCreateMutex();

    xSemaphoreTake(printMutex, portMAX_DELAY);

    buff.clear();
    (printArg(buff, args), ...);
    buff.print('\n'); // using println with Serial caused errors where some text was cut or missing part in the middle

    for (auto p : printers)
      p->print(buff);

    xSemaphoreGive(printMutex);
  }

  /*  re-use of the system log level type, just because using another would be DRY, possible values
  typedef enum {
    ESP_LOG_NONE,   //!< No log output
    ESP_LOG_ERROR,  //!< Critical errors, software module can not recover on its own
    ESP_LOG_WARN,   //!< Error conditions from which recovery measures have been taken
    ESP_LOG_INFO,   //!< Information messages which describe normal flow of events
    ESP_LOG_DEBUG,  //!< Extra information which is not necessary for normal use  (values, pointers, sizes, etc).
    ESP_LOG_VERBOSE //!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output.
  } */
  inline esp_log_level_t level = ESP_LOG_INFO;

  inline String makePrefix(const char *tag, const char *file, const char *func, int line)
  {
#if LOGGER_USE_PREFIX == 1
    return String(tag) + "[" + String(stripPath(file)) + "::" + String(func) + "::" + String(line) + "] ";
#elif LOGGER_USE_PREFIX == 2
  return String(tag) + "[" + String(func) + "::" + String(line) + "] ";
#elif LOGGER_USE_PREFIX == 3
  return String(tag) + "[" + String(stripPath(file)) + "::" + String(line) + "] ";
#elif LOGGER_USE_PREFIX == 4
  return String(tag) + "[" + String(stripPath(file)) + "::" + String(func) + "] ";
#elif LOGGER_USE_PREFIX == 0
  return ""
#else
#warning "LOGGER_USE_PREFIX MUST BE SET TO 0-4 RANGE"
  return "";
#endif
  }

  // needed for evaulation of the statements even if log level is lower and will not print, for cases like: logi("wifi begin =", WiFi.begin(ssid,
  // pass));
  template <esp_log_level_t L, typename... Args>
  inline void log_checked(const char *tag, const char *file, const char *func, int line, const Args &...args)
  {
    // forwards references is in this contex safe (outlive their use), efficient (no copies), and necessary for non‑copyable types like
    // JsonMemberProxy
    auto args_tuple = std::forward_as_tuple(args...);

    if (level >= L)
      std::apply(
        [&](auto &&...a) {
          logger::detail::log_imp(makePrefix(tag, file, func, line), a...);
        },
        args_tuple);
  }

  } // namespace detail

  /////////////////////////////////////////////////////////////////////////
  // PUBLIC API
  /////////////////////////////////////////////////////////////////////////

  inline const char *version = "1.0";

  /** @brief Remove arbitrary amount of Print objects (or pointers) from the internal list
   * @return false if asked for removal of non-existing one, true when all were removed
   */
  template <typename... Ts> inline bool removePrinter(Ts && ...args)
  {
    // Fold with &&: each lambda returns true only if that argument was successfully removed
    return (true && ... && ([&] {
      using T = std::decay_t<decltype(args)>;
      Print *p = std::is_pointer_v<T> ? args : std::addressof(args);

      if (!p)
        return false; // nullptr fails

      auto it = std::find(detail::printers.begin(), detail::printers.end(), p);

      if (it == detail::printers.end())
        return false; // not found

      detail::printers.erase(it);
      return true; // successfully removed
    }()));
  }
  /**
   * @brief Add arbitrary amount of Print objects (or pointers) to the list.
   * @return true if **all printers were added, false if any were nullptr or duplicate
   * @details syntax: addPrinter(&Serial0)
   */
  template <typename... Ts> inline bool addPrinter(Ts && ...args)
  {
    // Fold with &&: each lambda returns true only if the argument was newly added
    return (true && ... && ([&] {
      using T = std::decay_t<decltype(args)>;
      Print *p;

      if constexpr (std::is_pointer_v<T>)
        p = args; // if pointer, use it directly
      else
        p = std::addressof(args); // if reference, get address

      if (!p)
        return false;

      for (auto x : detail::printers)
        if (x == p)
          return false;

      detail::printers.push_back(p);
      return true;
    }()));
  }

  inline const esp_log_level_t getLogLevel()
  {
    return detail::level;
  }

  inline bool setLogLevel(const esp_log_level_t newlvl)
  {
    detail::level = newlvl;
    return true;
  }

  // overload that automatically cast type
  inline bool setLogLevel(unsigned int lvl)
  {
    const unsigned long check = lvl;
    lvl = constrain(lvl, (unsigned int)ESP_LOG_NONE, (unsigned int)ESP_LOG_VERBOSE); // must be checked as primitive due to enum wrapping

    setLogLevel((esp_log_level_t)lvl);
    return (lvl == check);
  }

  /** @brief makes board logs redirect thru logger. Makes sense only when using multiple Printers (like webSerial)*/
  inline void reroute_og_logs()
  {
    esp_log_set_vprintf([](const char *format, va_list args) {
      // calculate needed size
      va_list args_copy;
      va_copy(args_copy, args);
      int len = vsnprintf(nullptr, 0, format, args_copy);
      va_end(args_copy);

      if (len < 0)
        return 0;

      // Allocate exact buffer (+1 for null terminator)
      char *buffer = (char *)malloc(len + 1);
      if (!buffer)
        return 0;

      // Format into buffer
      vsnprintf(buffer, len + 1, format, args);

      detail::log_imp(buffer);

      free(buffer);

      return len; // it must return what og was returning. It's not important how
      // many bytes were printed but how much oryginally should be
    });
  }

} // namespace logger

#define loge(...) logger::detail::log_checked<ESP_LOG_ERROR>("[E]", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define logw(...) logger::detail::log_checked<ESP_LOG_WARN>("[W]", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define logi(...) logger::detail::log_checked<ESP_LOG_INFO>("[I]", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define logd(...) logger::detail::log_checked<ESP_LOG_DEBUG>("[D]", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define logv(...) logger::detail::log_checked<ESP_LOG_VERBOSE>("[V]", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define log(...) logi(__VA_ARGS__)