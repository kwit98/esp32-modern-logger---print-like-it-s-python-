#include "Arduino.h"
#include "logger.h"



void setup() {
  Serial.begin(115200);
  logger::addPrinter(&Serial);

  // Mix anty types that can be put into Serial.print
  const char* text = "hello";
  log(text, "world", String("..."), 3.14); 
  //  hello world ... 3.14


  // bool is being interpreted correctly
  log(0, "=/=", false);
  // 0 =/= false 


  // print multi dymensional arrays
  String food[ 2 ][ 3 ] = { {"apple", "banana", "tomato"}, {"paprika", "onion", "garlic"} };
  log(food);
  // arr[2][3] = {{apple, banana, tomato}, {paprika, onion, garlic}}


  /* 
  On default it support json format - ust dump whole doc at once.
  Can be printed pretty (indented), to turn it on add one of the below:
  
       build_flags = -DPRINTABLEBUFFOR_USE_JSON_PRETTY = 1 in iini file
       #define PRINTABLEBUFFOR_USE_JSON_PRETTY 1 before #include "logger.h"  */
  JsonDocument json;
  json["hello"] = "world";
  json["number"] = 121;
  log(json);
  // {"hello":"world", "number":121}
  // or
  // {
  //    "hello" : "world",
  //    "number" : 121
  // }

  //  JsonMemberProxy is deliberately non‑copyable from v7 - proxy cannot be resolved without casting
  log(json["hello"].as<String>());
}

void loop() {
}       