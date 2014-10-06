#include <iostream>

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

class lua_vm {
    lua_State *vm_;
    bool       own_;
public:
    lua_vm( )
        :vm_(lua_open( ))
        ,own_(true)
    { }
    ~lua_vm( )
    {
        if( own_ && vm_ ) {
            lua_close( vm_ );
        }
    }
};

int main( )
{
    lua_vm v;
    return 0;
}

