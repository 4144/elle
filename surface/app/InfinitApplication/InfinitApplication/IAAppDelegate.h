//
//  IAAppDelegate.h
//  InfinitApplication
//
//  Created by infinit on 10/17/12.
//  Copyright (c) 2012 infinit. All rights reserved.
//

#import "IPCInterface.h"

#import <Cocoa/Cocoa.h>

#import "NotificationPanel/IANotificationPanel.h"

// When defined, the window is shown on its own
// Otherwise, it is injected into the Finder
#define DEBUG_WITHOUT_FINDER

@interface IAAppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;

@property (assign) IBOutlet NSMenu*                 status_menu;
@property (assign) IBOutlet IANotificationPanel*    notification_panel;

@property (retain) NSString*        drive_path;
@property (retain) NSStatusItem*    status_item;
@property (retain) NSImage*         status_icon;
@property (retain) NSImage*         default_status_icon;
@property (retain) IPCInterface*    ipc_proxy;

@end
