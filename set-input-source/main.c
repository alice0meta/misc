// forks https://github.com/harciga/text-input-source-selector
// compile: gcc main.c -o main -lobjc -framework foundation -framework carbon
// usage: ./main [-s|e|d|t name]
//   -s : select source name (enable it as needed)
//   -e : enable source name
//   -d : disable source name
//   -t : toggle enabled/disabled of source name
//   no arguments : print current source name

#include <Carbon/Carbon.h>
#include <libgen.h>    // basename

TISInputSourceRef createInputSourceByName(char *);
int main(int argc, const char * argv[]){
	int ret = -1;
	int c;
	TISInputSourceRef tis;
	CFStringRef name;
	OSStatus err = 0;
	
	while ((c = getopt(argc, argv, "s:e:d:t:h")) != -1){
		switch (c){
			case 's':
				tis = createInputSourceByName(optarg);
				if (tis){
					CFBooleanRef enabled = TISGetInputSourceProperty(tis, kTISPropertyInputSourceIsEnabled);
					if (enabled == kCFBooleanFalse)
						TISEnableInputSource(tis);
					err = TISSelectInputSource(tis);
					CFRelease(tis);
				}
				ret = tis ? (int) err : 1;
				break;
			case 'e':
				tis = createInputSourceByName(optarg);
				if (tis){
					err = TISEnableInputSource(tis);
					CFRelease(tis);
				}
				ret = tis ? (int) err : 1;
				break;
			case 'd':
				tis = createInputSourceByName(optarg);
				if (tis){
					err = TISDisableInputSource(tis);
					CFRelease(tis);
				}
				ret = tis ? (int) err : 1;
				break;
			case 't':
				tis = createInputSourceByName(optarg);
				if (tis){
					CFBooleanRef enabled = TISGetInputSourceProperty(tis, kTISPropertyInputSourceIsEnabled);
					if (enabled == kCFBooleanTrue)
						err = TISDisableInputSource(tis);
					else
						err = TISEnableInputSource(tis);
					CFRelease(tis);
				}
				ret = tis ? (int) err : 1;
				break;
		}
	}
	if (ret == -1){ // no args: print current keyboard input source
		tis = TISCopyCurrentKeyboardInputSource();
		name = TISGetInputSourceProperty(tis, kTISPropertyLocalizedName);
		CFRelease(tis);
		int len = CFStringGetLength(name) * 4 + 1;
		char cname[len];
		bool b = CFStringGetCString(name, cname, len, kCFStringEncodingUTF8);
		printf("%s\n", b ? cname : "");
		ret = b ? 0 : 1;
	}
	if (err != noErr)
		fprintf(stderr, "Text Input Source Services error: %d\n", (int) err);
	return ret;
}

TISInputSourceRef
createInputSourceByName(char *cname){
	//     char *cname : input source name in UTF-8 terminated by null character
	//     return TISInputSourceRef or NULL : text input source reference (retained)
	
	CFStringRef name = CFStringCreateWithCString(kCFAllocatorDefault, cname, kCFStringEncodingUTF8);
	CFStringRef keys[] = { kTISPropertyLocalizedName };
	CFStringRef values[] = { name };
	CFDictionaryRef dict = CFDictionaryCreate(kCFAllocatorDefault, (const void **)keys, (const void **)values, 1, NULL, NULL);
	CFArrayRef array = TISCreateInputSourceList(dict, true);
	CFRelease(dict);
	CFRelease(name);
	if (!array){
		fprintf(stderr, "No such text input source: %s\n", cname);
		return NULL;
	}
	TISInputSourceRef tis = (TISInputSourceRef) CFArrayGetValueAtIndex(array, 0);
	CFRetain(tis);
	CFRelease(array);
	return tis;
}
