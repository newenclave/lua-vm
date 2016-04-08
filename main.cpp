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
    if( iii == 10 ) {
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

class lua_meta_sample {

    typedef lua_meta_sample this_type;

    static int lcall_gc( lua_State *L )
    {
        void *ud = luaL_testudata( L, 1, meta_name( ) );
        if( ud ) {
            lua_meta_sample *inst = static_cast<lua_meta_sample *>(ud);
            inst->~lua_meta_sample( );
        }
        return 0;
    }

    static lua_meta_sample *get_inst( lua_State *L )
    {
        void *ud = luaL_checkudata( L, 1, meta_name( ) );
        return static_cast<lua_meta_sample *>(ud);
    }

    static int lcall_tostring( lua_State *L )
    {
        lua_meta_sample *inst = get_inst( L );
        lua::state ls(L);
        if( inst ) {
            ls.push( "test_tableinst" );
            return 1;
        }
        return 0;
    }

public:

    static int lcall_create( lua_State *L )
    {
        void *ud = lua_newuserdata( L, sizeof(this_type));
        if( ud ) {
            new (ud) lua_meta_sample;
            luaL_getmetatable( L, meta_name( ) );
            lua_setmetatable(L, -2);
            return 1;
        }
        return 0;
    }

    static const char *meta_name( )
    {
        return "metaname_test";
    }

    static void register_table( lua_State *L )
    {
        const struct luaL_Reg lib[ ] = {
             { "new",        &this_type::lcall_create  }
            ,{ "__gc",       &this_type::lcall_gc  }
            ,{ "__tostring", &this_type::lcall_tostring  }
        };
        lua::state ls(L);
        lo::metatable mt( meta_name( ), lib );
        mt.push( L );
    }
};

int main( int argc, const char **argv )
{ try {

    const char * path = argc > 1 ? argv[1] : "test.lua";

    lua::state ls;

    ls.openlibs( );

    //ls.register_call( "print", &lcall_print );
    ls.register_call( "set_callback", &set_callback );

    lua_meta_sample::register_table( ls.get_state( ) );
    ls.register_call( "new_table", &lua_meta_sample::lcall_create );

    ls.check_call_error(ls.load_file( path ));

    return 0;

} catch( const std::exception &ex ) {
    std::cerr << "Error: " << ex.what( ) << "\n";
    return 1;
}}

