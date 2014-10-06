#include <iostream>
#include <string>
#include <deque>
#include <stdexcept>

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <math.h>

class lua_vm {

    lua_State *vm_;
    bool       own_;

public:

    enum state_owning {
         NOT_OWN_STATE = 0
        ,OWN_STATE     = 1
    };

    lua_vm( lua_State *vm, state_owning os = NOT_OWN_STATE )
        :vm_(vm)
        ,own_(os == OWN_STATE)
    { }

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

    lua_State *state( )
    {
        return vm_;
    }

    const lua_State *state( ) const
    {
        return vm_;
    }

    std::string pop_error( )
    {
        std::string res( lua_tostring(vm_, -1) );
        lua_pop( vm_, 1 );
        return res;
    }

    std::string error( )
    {
        std::string res( lua_tostring(vm_, -1) );
        return res;
    }

    void check_call_error( int res )
    {
        if( 0 != res ) {
            throw std::runtime_error( pop_error( ) );
        }
    }
};

static int l_sin ( lua_State *L )
{
    double d = lua_tonumber(L, 1);  /* get argument */
    lua_pushnumber(L, sin(d));      /* push result */
    return 1;                       /* number of results */
}

static int l_print( lua_State *L )
{
    int n = lua_gettop( L );

    if( n < 0 ) {
        n = ( (n * -1) + 1 );
    }

    for ( int b = 1; b <= n; ++b ) {

        size_t len = 0;

        const char *s = lua_tolstring(L, b, &len);  /* get result */

        if (s != NULL) {
            std::cout << std::string(s, len);
        } else {
            std::cout << std::string("<none>");
        }
    }

    lua_pop( L, n );

    return 0;
}

int main( ) try
{
    lua_vm v;

    lua_pushcfunction(v.state( ), l_print);
    lua_setglobal(v.state( ), "print");

    lua_pushcfunction(v.state( ), l_sin);
    lua_setglobal(v.state( ), "mysin");

    v.check_call_error(luaL_loadfile(v.state( ), "test.lua"));
    v.check_call_error(lua_pcall(v.state( ), 0, LUA_MULTRET, 0));

    return 0;
} catch( const std::exception &ex ) {
    std::cerr << "Error: " << ex.what( ) << "\n";
    return 1;
}

