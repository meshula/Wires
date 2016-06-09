//
//  FsmDg.cpp
//  Wires
//
//  Created by Nick Porcino on 8/1/14.
//  Copyright (c) 2014 PlanetIx. All rights reserved.
//

#include "Dg.h"
#include <string>
#include <iostream>
using namespace std;

void fsmDg() {
    
    Dg fsm;
    fsm.addNode("ping");
    fsm.addNode("pong");
    fsm.addAttribute("ping", "(after 0.5 '(goto pong)");
    fsm.addAttribute("pong", "(after 0.5 '(goto ping)");
    fsm.addNode("main");
    fsm.addAttribute("main", "state");
    fsm.setValue("main", "state", string("ping"));
    fsm.addObserver("main", "state", bind([](){ cout << "state changed" << endl; }));
    fsm.setValue("main", "state", string("pong"));
}




