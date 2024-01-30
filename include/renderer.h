#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Render {

void setup(HWND hwnd, int width, int height);
void StartFrame();
void EndFrame();
void teardown();

void UploadData();
void RecordDraws();

}
