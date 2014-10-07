#include <iostream>
#include <string>
#include <deque>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <stdlib.h>

#include <memory>

#include "lua-wrapper.hpp"
#include "lua-objects.hpp"

namespace lo = lua::objects;

lo::base_sptr g_table = NULL;

lo::base_sptr create( lua_State *L, int idx );

lo::base_sptr create_table( lua_State *L, int index )
{
    lua_pushvalue(L, index);
    lua_pushnil(L);

    std::shared_ptr<lo::table> new_table( new lo::table );
    while (lua_next(L, -2)) {

        lua_pushvalue(L, -2);
        std::shared_ptr<lo::pair> new_pair
                ( new lo::pair( create( L, -1 ), create( L, -2 ) ) );
        new_table->push_back( new_pair );
        lua_pop(L, 2);
    }

    lua_pop(L, 1);

    return new_table;
}

lua::objects::base_sptr create_string( lua_State *L, int idx )
{
    size_t length = 0;
    const char *ptr = lua_tolstring( L, idx, &length );
    return lo::base_sptr( new lo::string( ptr, length ) );
}

lua::objects::base_sptr create( lua_State *L, int idx )
{
    int t = lua_type( L, idx );
    switch( t ) {
    case LUA_TBOOLEAN:
        return lo::base_sptr(
                    new lo::boolean( lua_toboolean( L, idx ) ));
    case LUA_TLIGHTUSERDATA:
        return lo::base_sptr(
                    new lo::light_userdata( lua_touserdata( L, idx ) ));
    case LUA_TNUMBER:
        return lo::base_sptr(
                    new lo::number( lua_tonumber( L, idx ) ));
    case LUA_TSTRING:
        return create_string( L, idx );
    case LUA_TFUNCTION:
        return lo::base_sptr(
                    new lo::function( lua_tocfunction( L, idx ) ));
    case LUA_TTABLE:
        return g_table = create_table( L, idx );
//    case LUA_TUSERDATA:
//        return "userdata";
//    case LUA_TTHREAD:
//        return "thread";
    }
    return lo::base_sptr(new lo::nil);
}

int l_print( lua_State *L )
{
    int n = lua_gettop( L );

    lua::state tvm( L );

    if( n < 0 ) {
        n = ( (n * -1) + 1 );
    }

    for ( int b = 1; b <= n; ++b ) {
        std::cout << create( tvm.get_state( ), b )->str( );
    }

    std::cout << "\n";
    tvm.pop( n );

    return 0;
}

int main( ) try
{
    lua::state v;
    v.register_call( "print", l_print );

//    lua_pushcfunction(v.state( ), l_sin, 1);
//    lua_setglobal(v.state( ), "mysin");

    v.set_in_table2( "globt.tress.tt", "counter", &v );
    v.set_in_table2( "globt.tress.tt", "counter2", &v );

    v.check_call_error(luaL_loadfile(v.get_state( ), "test.lua"));
    v.check_call_error(lua_pcall(v.get_state( ), 0, LUA_MULTRET, 0));

    return 0;
} catch( const std::exception &ex ) {
    std::cerr << "Error: " << ex.what( ) << "\n";
    return 1;
}

