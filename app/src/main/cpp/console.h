#pragma once

#include "pch.h"

class Console {
public:
    static int AddConsoleLine(const std::string& text);
    static int AddConsoleLine(const std::string& text, int color);
    static void SetConsoleLine(int id, const std::string& text);
};