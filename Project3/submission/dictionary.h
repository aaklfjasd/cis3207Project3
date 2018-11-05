//
//  dictionary.h
//  
//
//  Created by Leo Vergnetti on 11/4/18.
//

#ifndef dictionary_h
#define dictionary_h

#include <stdio.h>
void initDictionary(int argc, char** argv);
void destroyDictionary();
int lookupWord(char* word);
#endif /* dictionary_h */
