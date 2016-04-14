#include <iostream>
#include <string>
#include <deque>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <list>
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
    lo::base_sptr bp( ls.get_object( 1, 1 ) );

    auto r = lua::object_by_path( L, bp.get( ), "m.m.m.m.'cool'" );

    std::cout << r->str( ) << "\n";

    return 0;

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

class fs_metatable {

    static int tostring( lua_State *L )
    {
        lua::state ls(L);
        fs_metatable *inst = ls.test_metatable<fs_metatable>( );
        std::ostringstream oss;

        oss << std::hex << "fs:" << inst;

        ls.push( oss.str( ).c_str( ) );

        return 1;
    }

    void *test_;

public:

    fs_metatable( )
    {
        test_ = malloc( 10112233 );
    }

    ~fs_metatable( )
    {
        free( test_ );
        std::cout << "~fs_metatable\n";
    }

    static const char *name( )
    {
        return "fstable";
    }

    static const struct luaL_Reg *table(  )
    {
        static const struct luaL_Reg lib[ ] = {
            { "__tostring", &tostring }
           ,{ NULL, NULL }
        };
        return lib;
    }
};

class lua_meta_sample {

    typedef lua_meta_sample this_type;

    static lua_meta_sample *get_inst( lua_State *L )
    {
        void *ud = luaL_checkudata( L, 1, name( ) );
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

    static int lcall_print( lua_State *L )
    {
        lua_meta_sample *inst = get_inst( L );
        inst->print( );
        return 0;
    }

    void print( )
    {
        std::cout << "Hello from test!\n";
    }

public:

    ~lua_meta_sample( )
    {
        std::cout << "~lua_meta_sample\n";
    }

    static const char *name( )
    {
        return "metaname.test.test2";
    }

    static const struct luaL_Reg *table(  )
    {
        static
        const struct luaL_Reg lib[ ] = {
            //,{ "__gc",       &this_type::lcall_gc       }
             { "__tostring", &this_type::lcall_tostring }
            ,{ "print",      &this_type::lcall_print    }

            ,{ nullptr,      nullptr                    }
        };
        return lib;
    }
};

//typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);

void hook( lua_State *L, lua_Debug *ar )
{
    std::cout << "Hook! " << ar->event << "\n";
    lua::state ls(L);
    luaL_error( L, "Error toolong" );
}

int main( int argc, const char **argv )
{ try {

    const char * path = argc > 1 ? argv[1] : "test.lua";

    lua::state ls;

    ls.openlibs( );

    ls.register_call( "print", &lcall_print );
    ls.register_call( "set_callback", &set_callback );

    //lua_meta_sample::register_table( ls.get_state( ) );

    ls.register_metatable<lua_meta_sample>( );
    ls.register_metatable<fs_metatable>( );

    ls.register_call( "new_table", &lua::state::create_metatable_call<lua_meta_sample> );
    ls.register_call( "new_fs",    &lua::state::create_metatable_call<fs_metatable>);

//    lua_sethook( ls.get_state( ), hook, -1, 1000000 );

    ls.check_call_error(ls.load_file( path ));

    return 0;

} catch( const std::exception &ex ) {
    std::cerr << "Error: " << ex.what( ) << "\n";
    return 1;
}}



//#include <iostream>

//struct Hello
//{
//    static int helloworld( int ) { return 0; }
//};

//struct Hello2
//{
//    //void helloworld( int ) {  }
//    static float helloworld( ) { return 0.1; }
//};

//struct Generic {};

//typedef int (*test)( int );

//struct fun_trait {
//    template <typename T, typename Q>
//    static char test( T(*)( Q ) ) { return '1'; }

//    template <typename T>
//    static short test( T t ) { return 1; }
//};

//// SFINAE test
//template <typename T>
//class has_helloworld
//{
//    typedef char one;
//    typedef long two;

//    template <typename C> static one test( decltype(&C::helloworld) ) ;
//    template <typename C> static two test(...);

//public:
//    enum { value = sizeof(test<T>(0)) == sizeof(char) };
//};

//int main(int argc, char *argv[])
//{

//    std::cout << sizeof(fun_trait::test(1)) << "\n";
//    std::cout << sizeof(fun_trait::test(&Hello::helloworld)) << "\n";

//    std::cout << has_helloworld<Hello>::value << std::endl;
//    std::cout << has_helloworld<Hello2>::value << std::endl;
//    std::cout << has_helloworld<Generic>::value << std::endl;
//    return 0;
//}
