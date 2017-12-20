//
//  main.cpp
//  minisqlmypart
//
//  Created by 张倬豪 on 2017/5/28.
//  Copyright © 2017年 Icarus. All rights reserved.
//
#include "stdafx.h"
#include <iostream>
#include "Interpreter.h"

int main(int argc, const char * argv[]) {
    
    Interpreter it;
    
    it.mainLoop();

    it.destruct();
    
    return 0;
}
