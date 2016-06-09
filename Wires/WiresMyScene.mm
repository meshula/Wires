//
//  WiresMyScene.m
//  Wires
//
//  Created by Nick Porcino on 2014 05/2.
//  Copyright (c) 2014 PlanetIx. All rights reserved.
//

#import "WiresMyScene.h"
#include "Wires.h"
//#include "Dg.h"
#include "SpoDg.h"
#include <iostream>
#include <string>
using namespace std;
using namespace Wires;

@implementation WiresMyScene

#define TEST1 "\
trivial ---> trivial2 \n\
foo ------->bar"

#define TEST1a "\
trivial ---> trivial2 \n\
              val:2   \n\
foo ------->bar"



#define TEST2 "\
trivial ---+                  \n\
           |                  \n\
           )                  \n\
           +---->   trivial2"

#define TEST3 "          \n\
foo -------+             \n\
trivial1 --)-->trivial2  \n\
           +->     bar   "

#define TEST4 "\
sample0 ---------+                 \n\
sample1 -----+   |                 \n\
sample2 -----)---)----->output2    \n\
             +---)----->output1    \n\
                 |                 \n\
                 +--->output0     "

#define TEST5 "\
sample --+                                                                                           \n\
         +-> bassFilter ---> bassGain --------+                                                      \n\
         +-> midFilter ----> midGain ---------+                                                      \n\
         +-> trebleFilter ----> trebleGain ---+---> gain -+---> recorder ------+                     \n\
                                                          +---> monitor  ------+---> oscilloscope    \n\
                                                          +---> analyser                             \n\
                                                          +---> audiocontext                         "


#define TEST6 "\
[buffer:sample] ----------+                                                                                             \n\
  file:'human-voice.mp4'  |                                                                                             \n\
                          +-> [filter:bassFilter] ---> [gain:bassGain] --------+                                        \n\
                          |        type:'lowpass'          gain:2.0            |                                        \n\
                          |   frequency:160                                    |                                        \n\
                          |           q:1.2                                    |                                        \n\
                          +-> [filter:midFilter] ----> [gain:midGain] ---------+                                        \n\
                          |        type:'bandpass'          gain:4.0           |                                        \n\
                          |   frequency:500                                    |                                        \n\
                          |           q:1.2                                    |                                        \n\
                          +-> [filter:trebleFilter] ----> [gain:trebleGain] ---+---> gain -----+---> recorder           \n\
                                type:'highpass'            gain:3.0                 gain:1.0   +---> monitor            \n\
                           frequency:2000                                                      +---> oscilloscope       \n\
                                   q:1.2                                                       +---> analyser           \n\
                                                                                               +---> audiocontext         "


#define TEST7 "\
   gain          master_gain    \n\
value:1.0 --------> value:0.5   "

#define TEST8 "\
   gainA                                            \n\
value:1.0 ----+                                     \n\
              |  .--------------.                   \n\
   gainB      +->|A   (+ A B)   |         output    \n\
value:0.5 ------>|B             |----> value:0.0    \n\
                 '--------------'                   "

#define TEST9 "\
.-----------------------.                                                                                               \n\
| buffer                |-+                                                                                             \n\
| file:'human-voice.mp4'| |   .------------------.                                                                      \n\
'-----------------------' +-> |filter:bassFilter |---> [gain:bassGain] --------+                                        \n\
                          |   |   type:'lowpass' |         gain:2.0            |                                        \n\
                          |   | frequency:160    |                             |                                        \n\
                          |   |       q:1.2      |                             |                                        \n\
                          |   '------------------'                             |                                        \n\
                          |   .------------------.                             |                                        \n\
                          +-> |filter:midFilter  |---> [gain:midGain] ---------+                                        \n\
                          |   |  type:'bandpass' |          gain:4.0           |                                        \n\
                          |   | frequency:500    |                             |                                        \n\
                          |   |    q:1.2         |                             |                                        \n\
                          |   '------------------'                             |                                        \n\
                          |   .-------------------.                            |                                        \n\
                          +-> |filter:trebleFilter| ----> [gain:trebleGain] ---+---> gain -----+---> recorder           \n\
                              | type:'highpass'   |        gain:3.0                 gain:1.0   +---> monitor            \n\
                              | frequency:2000    |                                            +---> oscilloscope       \n\
                              |    q:1.2          |                                            +---> analyser           \n\
                              '-------------------'                                            +---> audiocontext         "

-(id)initWithSize:(CGSize)size {
    if (self = [super initWithSize:size]) {

        parse(TEST7, true);
            
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
        fsm.setValue("main", "state", string("ping"));
        
        return self;
        parse(TEST1, false);
        parse(TEST1a, false);
        parse(TEST2, false);
        parse(TEST3, false);
        parse(TEST4, false);
        parse(TEST5, false);
        parse(TEST6, false);
    }
    return self;
}

-(void)mouseDown:(NSEvent *)theEvent {
     /* Called when a mouse click occurs */

    CGPoint location = [theEvent locationInNode:self];

    SKSpriteNode *sprite = [SKSpriteNode spriteNodeWithImageNamed:@"Spaceship"];
    
    sprite.position = location;
    sprite.scale = 0.5;
    
    SKAction *action = [SKAction rotateByAngle:M_PI duration:1];
    
    [sprite runAction:[SKAction repeatActionForever:action]];
    
    [self addChild:sprite];
}

-(void)update:(CFTimeInterval)currentTime {
    /* Called before each frame is rendered */
}

@end
