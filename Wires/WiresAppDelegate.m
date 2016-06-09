//
//  WiresAppDelegate.m
//  Wires
//
//  Created by Nick Porcino on 2014 05/2.
//  Copyright (c) 2014 PlanetIx. All rights reserved.
//

#import "WiresAppDelegate.h"
#import "WiresMyScene.h"

@implementation WiresAppDelegate

@synthesize window = _window;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    /* Pick a size for the scene */
    SKScene *scene = [WiresMyScene sceneWithSize:CGSizeMake(1024, 768)];

    /* Set the scale mode to scale to fit the window */
    scene.scaleMode = SKSceneScaleModeAspectFit;

    [self.skView presentScene:scene];

    self.skView.showsFPS = YES;
    self.skView.showsNodeCount = YES;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

@end
