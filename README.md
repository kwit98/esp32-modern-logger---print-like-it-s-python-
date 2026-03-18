# esp32-modern-logger---print-like-it-s-python-

Thread-safe, different tasks can print at the same time and msg will not mix.

Can dynamically pick destinations (Arduino printers types). Add/remove mutltiple objs at once (or their reference).
```cpp
logger::addPrinter(Serial, &Serial0, webserial); 
```


Accepts arbitrary amount of args of any Printables or multidimensional arrays.
```cpp
String food[ 2 ][ 3 ] = { {"apple", "banana", "tomato"}, {"paprika", "onion", "garlic"} };
log("food equals to", food);
// [main.cpp::loop::16] food equals to arr[2][3] = {{apple, banana, tomato}, {paprika, onion, garlic}}

const char *text = "hello";
  log(text, "world", String("..."), 3.14);
  //  [main.cpp::loop::18] hello world ... 3.14

```

 On default qccepts json and (on defualt) do not print them pretty (indentation, line breaks).
 Both option can be changed before compilation.
```cpp
  JsonDocument json;
  json["hello"] = "world";
  json["number"] = 121;
  log(json); 

  //[main.cpp::loop::16] {"hello":"world","number":121} 
```


  Print bool as bool, not int. U will thank me while using json v7!
  ```cpp
    log(0, "=/=", false);
  // [main.cpp::loop::21] 0 =/= false

```

Better readibility by being side‑effect friendly – always evaluates arguments. 
No need for creating variable just to show it. 
```cpp
logger:setLogLevel(ESP_LOG_NONE);
logi("success", Wifi.begin()); // WiFi is sill running
```

Adds prefix with file/function name/line, on default 1 option
```cpp
// 0 // no prefix
// 1 = [filename.ext::funcName::line]  <- default
// 2 = [funcName::line]
// 3 = [fileName.ext::line]
// 4 = [fileName.ext::funcName]
`
Call #define LOGGER_USE_PREFIX == x before including library
```
  
Part of esp_logs of things on lower level api can be rerouted to logger, what enables to save them on fs or send to web
```cpp
logger:reroute_og_logs();
```

 Supports log levels, reused system one esp_log_level_t, default is ESP_LOG_INFO.
```cpp
typedef enum {
    ESP_LOG_NONE,       /*! = 0 < No log output */
    ESP_LOG_ERROR,      /*! = 1< Critical errors, software module can not recover on its own */
    ESP_LOG_WARN,       /*! = 2< Error conditions from which recovery measures have been taken */
    ESP_LOG_INFO,       /*! = 3 (< Information messages which describe normal flow of events */ 
    ESP_LOG_DEBUG,      /*! = 4< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    ESP_LOG_VERBOSE     /*! = 5< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
} esp_log_level_t;

logger::setLogLevel(3) // overloads support for int, so there is no need for manually casting it.
```


Data is put into buffer without fixed size, I recommended to keep default inital reserve to prevent fragmentantion caused by constant reallocation. 
I was able to print over 50kb into Serial (at once, not streaming) without crash (xiao-c3). 
If msg really can't fit, it would be truncated - it will not grow into heap and panic handler
```cpp  #define LOGGER_REVERSE_BUFFER_SIZE 1024```


Doesn't support c-string char arrays (null terminated) for logging.
Otherwise it would evaluate that any text written into editor like log("hello") is an array, not text.


Library must be compiled with C++17 or newer.
```ini
;platformio.ino
build_unflags = -std = gnu++ 11 
build_flags = -std = gnu++ 2
```
