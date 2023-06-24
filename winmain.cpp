
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


#define OS_LAYER_SELF_TEST 1
#include "os_layer.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    
    // see end of "os_layer.h"
    os_window_self_test();

}
