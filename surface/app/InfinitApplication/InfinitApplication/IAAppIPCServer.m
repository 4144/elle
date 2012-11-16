//
//  IAAppIPCServer.m
//  InfinitApplication
//
//  Created by infinit on 11/15/12.
//  Copyright (c) 2012 infinit. All rights reserved.
//

#import "IAAppDelegate.h"
#import "IAAppIPCServer.h"

@implementation IAAppIPCServer
{
@private
    IAAppDelegate* _app;
}

- (id) init
{
    assert(false);
    return nil;
}

- (id)initWithAppDelegate:(IAAppDelegate*)app
{
    self = [super init];
    if (self)
    {
        _app = app;
    }
    return self;
}

- (void)sendUserToken:(NSString*)token
{
    assert(_app != NULL);
    NSLog(@"Receive user token: %@", token);
}

@end
