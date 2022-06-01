
#if defined(__APPLE__)

#import "ta_cpp_helper.h"
#import <Cocoa/Cocoa.h>
#import <IOKit/IOKitLib.h>

void thinkingdata::ta_mac_tool::updateAccount(const char *token, const char *accountId)
{
    [[NSUserDefaults standardUserDefaults] setObject:[NSString stringWithUTF8String:accountId]
                                              forKey:[NSString stringWithFormat:@"td-accountid-%@", [NSString stringWithUTF8String:token]]];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

void thinkingdata::ta_mac_tool::updateDistinctId(const char *token, const char *distinctId) {
    [[NSUserDefaults standardUserDefaults] setObject:[NSString stringWithUTF8String:distinctId]
                                              forKey:[NSString stringWithFormat:@"td-distinctId-%@", [NSString stringWithUTF8String:token]]];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

const char * thinkingdata::ta_mac_tool::loadAccount(const char *token) {
    return [[[NSUserDefaults standardUserDefaults] objectForKey:[NSString stringWithFormat:@"td-accountid-%@", [NSString stringWithUTF8String:token]]] UTF8String];
}
const char * thinkingdata::ta_mac_tool::loadDistinctId(const char *token) {
    return [[[NSUserDefaults standardUserDefaults] objectForKey:[NSString stringWithFormat:@"td-distinctId-%@", [NSString stringWithUTF8String:token]]] UTF8String];
}


static char ta_uuid_buf[512] = "";

const char * thinkingdata::ta_mac_tool::getDeviceID()  {
    
    if (strcmp(ta_uuid_buf, "") == 0) {
            int bufSize = sizeof(ta_uuid_buf);
            io_registry_entry_t ioRegistryRoot = IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/");
            CFStringRef uuidCf = (CFStringRef) IORegistryEntryCreateCFProperty(ioRegistryRoot, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);
            IOObjectRelease(ioRegistryRoot);
            CFStringGetCString(uuidCf, ta_uuid_buf, bufSize, kCFStringEncodingMacRoman);
            CFRelease(uuidCf);
        }
    return ta_uuid_buf;
}

#endif

