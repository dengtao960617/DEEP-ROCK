#include "pch.h"
#include "Myfun.h"
#include <iostream>
#include "detours.h"
using namespace std;


typedef int (WINAPI* Randcall_t)();

extern "C" Randcall_t old_Randcall = NULL;
extern "C" Randcall_t old_Randcall2 = NULL;
extern "C" int WINAPI MyAsmRandcall();
extern "C" int WINAPI MyAsmRandcall2();


VOID SetGold(DWORD num) {
    HANDLE hgameassembly = GetModuleHandleA("gameassembly.dll");
    try
    {
        for (size_t i = 0; i < 2; i++)
        {
            DWORD64 TEMP = *(DWORD64*)((DWORD64)hgameassembly + 0x355AB00);
            TEMP = *(DWORD64*)(TEMP + 0xB8);
            TEMP = *(DWORD64*)(TEMP + 0x8);
            TEMP = *(DWORD64*)(TEMP + 0x40);
            TEMP = *(DWORD64*)(TEMP + 0x78);
            if (i == 1) {
                TEMP = *(DWORD64*)(TEMP + 8 * 8 + 0x20);
            }
            else {
                TEMP = *(DWORD64*)(TEMP + 1 * 8 + 0x20);
            }
            TEMP = *(DWORD64*)(TEMP + 0x40);

            DWORD64 GoldAddr = *(DWORD64*)(TEMP + 0x10);
            GoldAddr = GoldAddr + 0x14;
            //printf("TEMP:%p,GoldAddr:%p\n", TEMP, GoldAddr);
            //MessageBoxA(0, "11", 0, MB_OK);
            DWORD64 redGoldAddr = *(DWORD64*)(TEMP + 0x20);
            redGoldAddr = redGoldAddr + 0x14;

            DWORD64 unknowGoldAddr = *(DWORD64*)(TEMP + 0x18);
            unknowGoldAddr = unknowGoldAddr + 0x14;

            DWORD GoldNum = *(DWORD64*)(GoldAddr);
            if (GoldNum <= 100) {
                *(DWORD64*)(GoldAddr) = num;
                *(DWORD64*)(redGoldAddr) = num;
                *(DWORD64*)(unknowGoldAddr) = num;
                break;
            }
        }



    }
    catch (const std::exception&)
    {
        printf("SetGold Error!");
    }
}
VOID 设置重选倍率(float a) {
    //FloatToDword b;
    //b.floatValue = a;
    //printf("%08X,%f\n", b.dwordValue,a);
    //return;
    HANDLE hgameassembly = GetModuleHandleA("gameassembly.dll");
    try
    {
        DWORD64 TEMP = (DWORD64)hgameassembly + 0x29DADF4;
        float 倍率 = *(float*)TEMP;
        DWORD lpflOldProtect = 0;
        VirtualProtect((LPVOID)TEMP, 1, PAGE_EXECUTE_READWRITE, &lpflOldProtect);
        //printf("倍率:%f", 倍率);
        *(float*)TEMP = a;
        printf("原始倍率：%f,设置倍率：%f\n", 倍率, a);

    }
    catch (const std::exception&)
    {
        printf("设置重选倍率 error!");
    }
}

VOID SetGoldThread() {
    printf("SetGoldThread");
    while (true)
    {
        SetGold(9999999);
        Sleep(5000);
    }
}

VOID HOOK_RandCall() {
    // 开始 hook 操作
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    // 获取原始的  函数指针
    HANDLE hgameassembly = GetModuleHandleA("gameassembly.dll");
    old_Randcall = (Randcall_t)((DWORD64)hgameassembly + 0x48E640);
    old_Randcall2 = (Randcall_t)((DWORD64)hgameassembly + 0x48E640 - 0x40);
    //DWORD lpflOldProtect = 0;
    //VirtualProtect(old_Randcall, 1, PAGE_EXECUTE_READWRITE, &lpflOldProtect);
    //HOOK
    LONG hook_ret = 0;
    hook_ret = DetourAttach(&(PVOID&)old_Randcall, MyAsmRandcall);
    printf("old_Randcall:%p,MyRandcall:%p\n", old_Randcall, MyAsmRandcall);
    printf("HOOK MyRandcall:%d\n", hook_ret);
    hook_ret = DetourAttach(&(PVOID&)old_Randcall2, MyAsmRandcall2);
    printf("old_Randcall:%p,MyRandcall:%p\n", old_Randcall2, MyAsmRandcall2);
    printf("HOOK MyRandcall:%d\n", hook_ret);
    // 完成 hook 操作
    DetourTransactionCommit();
}
VOID UNHOOK_RandCall() {
    // 取消 hook 操作
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    // 恢复原始的 函数
    DetourDetach(&(PVOID&)old_Randcall, MyAsmRandcall);
    // 完成取消 hook 操作
    DetourTransactionCommit();
}