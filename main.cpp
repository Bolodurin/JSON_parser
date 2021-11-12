
#include <iostream>
#include <memory>
#include "src/parser.h"

using namespace parser;


int main () {
    Parser pars;
    /* auto node = pars.parseJson (R"( 
    {
      "hello": "world", 
      "wrong":["str 1"],
      "struct" : {
        "first" : "property",
        "second_arr" : [
          "str1",
          "STR2",
          {
            "name": "NAME"
          }
        ]
      },
      "empty": {},
      "good": "bye"
    })"); */
    pars.parseJsonFromFile ("example.json");
    pars.printJson ();

    return 0;
}