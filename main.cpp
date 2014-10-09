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

int main( ) try
{
    lua::state v;
    v.register_call( "print", l_print );

    map_type m;

    std::shared_ptr<lo::table> t( lo::new_table( ) );
    t->add(
        lo::new_string( "test" ),
        lo::new_table(  )->add(
            lo::new_string( "internal" ),
            lo::new_string( "buff" )
        )->add(
            lo::new_string( "second" ),
            lo::new_string( "2" )
        )->add(
            lo::new_nil( ),
            lo::new_function( &l_print )
        )
    )->add(
        lo::new_string( "add" ),
        lo::new_function( &test_call )
    )->add(
        lo::new_string( "get" ),
        lo::new_function( &get )
    );

    lo::base_uptr mmm( lo::new_light_userdata( &m ) );

    m.insert( std::make_pair( 0, (void *)(0xFFFFFFF0) ) );
    m.insert( std::make_pair( 1, (void *)(0xFFFFFFF1) ) );
    m.insert( std::make_pair( 2, (void *)(0xFFFFFFF2) ) );

    v.set_object_in_global( "global_table", "map", *mmm );
    v.set_object_in_global( "global_table", "registry", *t );

    v.set_in_global( "global_table", "data", 100.090 );
    double d = v.get_from_global<double>( "global_table", "data");

    std::cout << "double " << d << "\n";

    v.check_call_error(v.load_file( "test.lua" ));
    v.check_call_error(v.exec_function( "main", *t ));

    l_print( v.get_state( ) );

    return 0;

} catch( const std::exception &ex ) {
    std::cerr << "Error: " << ex.what( ) << "\n";
    return 1;
}

