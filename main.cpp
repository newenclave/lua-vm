#include <iostream>
#include <string>
#include <deque>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <stdlib.h>

#include <memory>
#include <map>

#include "lua-wrapper/lua-wrapper.hpp"
#include "lua-wrapper/lua-objects.hpp"

namespace lo = lua::objects;

void print_sptr( lua_State *L, const lo::base *o_, int iii )
{
    const std::string space( iii * 2, ' ' );
    if( iii == 3 ) {
        std::cout <<  space << o_->str( ) << "\n";
        return;
    }
    const lo::base *o = o_;
    lo::base_sptr sptr;
    lua::state ls(L);

    if( o->count( ) == 0 ) {
        sptr = ls.ref_to_object( o );
        o = sptr.get( );
    }

    //std::cout << "G " << o->count( ) << "\n";

    if(o->count( )) {
        for( size_t i=0; i<o->count( ); ++i ) {
            auto f = o->at( i );
            std::cout << space << f->at(0)->str( ) << " -> \n";
            auto t = f->at(1)->type_id( );
            if( ( t & LUA_TTABLE ) == LUA_TTABLE ) {
                print_sptr( L, f->at(1), iii + 1 );
            } else {
                std::cout << space << f->at(1)->str( ) << "\n";
            }
        }
    } else {
        std::cout << space << o_->str( ) << "\n";
    }
}

int lcall_print( lua_State *L )
{
    lua::state ls(L);
    int n = ls.get_top( );
    for( int i=1; i<=n; ++i ) {
        lo::base_sptr bp( ls.get_object( i, 1 ) );

//        for( size_t i=0; i<bp->count( ); ++i ) {
            print_sptr( L, bp.get( ), 0 );
//        }

        //std::cout << bp->str( );
    }
    ls.clean_stack( );
    return 0;
}

int lcall_new( lua_State *L )
{
    lua::state ls(L);
    int n = ls.get_top( );
    for( int i=1; i<=n; ++i ) {
        lo::base_sptr bp( ls.get_object( i, 1 ) );

//        for( size_t i=0; i<bp->count( ); ++i ) {
            print_sptr( L, bp.get( ), 0 );
//        }

        //std::cout << bp->str( );
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

