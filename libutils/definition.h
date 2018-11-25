#ifndef  DEFINITION_H
#define  DEFINITION_H

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRINGS(STRING) #STRING,


#endif
