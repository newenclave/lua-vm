#include <iostream>
#include <string>
#include <deque>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <stdlib.h>

#include <memory>
#include <map>

#include "../ferro-remote/include/fr-lua/lua-wrapper.hpp"
#include "../ferro-remote/include/fr-lua/lua-objects.hpp"


namespace lo = lua::objects;


int lcall_print( lua_State *L )
{
    lua::state ls(L);
    int n = ls.get_top( );
    for( int i=1; i<=n; ++i ) {
        lo::base_sptr bp( ls.get_object( i, 1 ) );
        std::cout << bp->str( );
    }
    ls.clean_stack( );
    return 0;
}

int set_callback( lua_State *L )
{
    lua::state ls(L);

    const char *name = ls.get<const char *>( 1 );

    int tt = ls.get_type( 2 );
    if( tt == LUA_TFUNCTION ) {
        ls.set_value( name, 2 );
    }

    ls.clean_stack( );

    return 0;
}

int main( int argc, const char **argv )
{ try {

    const char * path = argc > 1 ? argv[1] : "test.lua";

    lua::state ls;

    ls.register_call( "print", &lcall_print );
    ls.register_call( "set_callback", &set_callback );

    ls.check_call_error(ls.load_file( path ));

    return 0;

} catch( const std::exception &ex ) {
    std::cerr << "Error: " << ex.what( ) << "\n";
    return 1;
}}

