
#pragma once
#include <scriptcontext.hpp>


void scripting_init();
void scripting_load(const char*);
int8_t scripting_handle();
ScriptContext* scripting_context();
