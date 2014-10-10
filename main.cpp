#include <iostream>
#include <string>
#include <deque>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <stdlib.h>

#include <memory>
#include <map>

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

typedef std::map<int, void *> map_type;

int test_call( lua_State *L )
{
    lua::state v( L );
    int data = v.get<int>( );

    unsigned long mm = v.get_from_global<unsigned long>( "global_table", "map" );
    if( mm ) {
        map_type *m((map_type *)mm);
        std::cout << "Parameters: " << (*m)[data] << "\n";
    }

    return 0;
}

int get( lua_State *L )
{
    lua::state v( L );
    v.push( 2 );
    return 1;
}

size_t get_top_path( const char *path )
{
    size_t res = 0;
    while( (*path != '.') && (*path != '\0') ) {
        ++path;
        ++res;
    }
    return res;
}

void create_set( lua_State *L, const char *rest, int value )
{
    if( !*rest ) {
        lua_pushinteger( L, value );
    } else {

        size_t path_len = get_top_path( rest );
        std::string p( rest, path_len );
        const char *nr = rest + path_len;

        lua_newtable( L );
        lua_pushstring( L, p.c_str( ) );
        create_set( L, !*nr ? "" : nr + 1, value );
        lua_settable( L, -3 );
    }
}

/// table found
void set_if_found( lua_State *L, const char *rest, int value )
{
    size_t path_len = get_top_path( rest );
    std::string p( rest, path_len );
    const char *nr = rest + path_len;

    if( !*nr ) {
        lua_pushstring( L, p.c_str( ) );
        lua_pushinteger( L, value );
        lua_settable( L, -3 );
    } else {
        lua_getfield( L, -1, p.c_str( ) );
        if( !lua_istable( L, -1 ) ) {
            lua_pop( L, 1 );
            lua_pushstring( L, p.c_str( ) );
            create_set( L, nr + 1, value );
            lua_settable( L, -3 );
        } else {
            set_if_found( L, nr + 1, value );
            lua_pop( L, 1 );
        }
    }
}

void set_in_global( lua_State *L, const char *path, int value )
{
    size_t path_len = get_top_path( path );
    std::string p( path, path_len );

    lua_getglobal( L, p.c_str( ) ); // stack ->> table || nil

    const char *rest = path + path_len;

    if( !lua_istable( L, -1 ) ) {
        lua_pop( L, 1 );
        if( !*rest ) {
            lua_pushinteger( L, value );
        } else {
            create_set( L, rest + 1, value );
        }
    } else {
        if( !*rest ) {
            lua_pop( L, 1 );
            lua_pushinteger( L, value );
        } else {
            set_if_found( L, rest + 1, value );
        }
    }

    lua_setglobal( L, p.c_str( ) );
}

int main( ) try
{
    lua::state v;
    v.register_call( "print", l_print );

    set_in_global( v.get_state( ), "gtest.maxpart.x", 1 );
    set_in_global( v.get_state( ), "gtest.maxpart.x.xx", 800 );
    set_in_global( v.get_state( ), "gtest.minpart.y", 0 );
    set_in_global( v.get_state( ), "gtest.midpart.z.r", 5 );
    set_in_global( v.get_state( ), "gtest.maxpart.x.xx.i", -222 );

    v.check_call_error(v.load_file( "test.lua" ));

    l_print( v.get_state( ) );

    return 0;

} catch( const std::exception &ex ) {
    std::cerr << "Error: " << ex.what( ) << "\n";
    return 1;
}

