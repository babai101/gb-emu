#pragma once
#include "common.hpp"
namespace bootloader 
{
   bool load(char*);
   char* load_cart(char*); 
   void parse(char*);
}