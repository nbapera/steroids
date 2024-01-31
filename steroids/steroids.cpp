#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <windows.h>
#include <thread>

#include "features/clicker.hpp"
#include "minecraft/minecraft.hpp"
#include "menu/menu.hpp"



std::string ascii = R"(

         __                                __
        /\ \__                      __    /\ \
    ____\ \ ,_\    __   _ __   ___ /\_\   \_\ \    ____
   /',__\\ \ \/  /'__`\/\`'__\/ __`\/\ \  /'_` \  /',__\
  /\__, `\\ \ \_/\  __/\ \ \//\ \L\ \ \ \/\ \L\ \/\__, `\
  \/\____/ \ \__\ \____\\ \_\\ \____/\ \_\ \___,_\/\____/
   \/___/   \/__/\/____/ \/_/ \/___/  \/_/\/__,_ /\/___/
)";

int main()
{


    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    system("cls");

    SetConsoleTextAttribute(console, 13);
    std::cout << ascii << std::endl;
    Sleep(2000);
    minecraft::mc = FindWindowA("LWJGL", nullptr);

    while (!minecraft::mc) {
        minecraft::mc = FindWindowA("LWJGL", nullptr);
        Sleep(1000);
        SetConsoleTextAttribute(console, 12);
        std::cout << "[-] Minecraft not found... Retrying" << std::endl;
    }


    std::thread clicker(aclicker::run);
    std::thread keybinds(menu::listen_keybinds);
    std::thread rainbow(menu::rainbow_menu);
    ShowWindow(::GetConsoleWindow(), SW_HIDE);
    menu::run();


    return 0;
}