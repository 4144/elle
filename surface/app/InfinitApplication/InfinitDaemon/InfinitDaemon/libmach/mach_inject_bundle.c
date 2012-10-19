/*******************************************************************************
	mach_inject_bundle.c
		Copyright (c) 2003-2009 Jonathan 'Wolf' Rentzsch: <http://rentzsch.com>
		Some rights reserved: <http://opensource.org/licenses/mit-license.php>

	***************************************************************************/

#include "mach_inject_bundle.h"
#include "mach_inject.h"
#include "mach_inject_bundle_stub.h"
#include <CoreServices/CoreServices.h>

	mach_error_t
mach_inject_bundle_pid(
		const char *bundlePackageFileSystemRepresentation,
        const char *machInjectBundleStubPath,
		pid_t pid )
{
	assert( bundlePackageFileSystemRepresentation );
	assert( pid > 0 );
	
	mach_error_t	err = err_none;
	
	//	Get the framework's bundle.
    CFURLRef myPluginDir = 
        CFURLCreateWithFileSystemPath(kCFAllocatorDefault, 
                                      CFStringCreateWithCString(NULL,
                                                                machInjectBundleStubPath,
                                                                kCFStringEncodingUTF8),
                                      kCFURLPOSIXPathStyle,
                                      TRUE);
	CFURLRef injectionURL = myPluginDir;
	//	Create injection bundle instance.
	CFBundleRef injectionBundle = NULL;
	if( !err ) {
		injectionBundle = CFBundleCreate( kCFAllocatorDefault, injectionURL );
		if( !injectionBundle )
			err = err_mach_inject_bundle_couldnt_load_injection_bundle;
	}
	
	//	Load the thread code injection.
	void *injectionCode = NULL;
	if( !err ) {
		injectionCode = CFBundleGetFunctionPointerForName( injectionBundle,
			CFSTR( INJECT_ENTRY_SYMBOL ));
		if( injectionCode == NULL )
			err = err_mach_inject_bundle_couldnt_find_inject_entry_symbol;
	}
	
	//	Allocate and populate the parameter block.
	mach_inject_bundle_stub_param *param = NULL;
	size_t paramSize;
	if( !err ) {
		size_t bundlePathSize = strlen( bundlePackageFileSystemRepresentation )
			+ 1;
		paramSize = sizeof( ptrdiff_t ) + bundlePathSize;
		param = malloc( paramSize );
		bcopy( bundlePackageFileSystemRepresentation,
			   param->bundlePackageFileSystemRepresentation,
			   bundlePathSize );
	}
	
	//	Inject the code.
	if( !err ) {
		err = mach_inject( injectionCode, param, paramSize, pid, 0 );
	}
	
	//	Clean up.
	if( param )
		free( param );
	/*if( injectionBundle )
		CFRelease( injectionBundle );*/
	if( injectionURL )
		CFRelease( injectionURL );
	
	return err;
}