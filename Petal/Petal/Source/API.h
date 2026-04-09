#pragma once

// API Macro

#ifdef PETAL_EXPORTS
#define PETAL_API __declspec(dllexport)
#else
#define PETAL_API __declspec(dllimport)
#endif