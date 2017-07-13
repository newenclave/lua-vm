#include <iostream>
#include <string>
#include <deque>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <list>
#include <stdlib.h>
#include <thread>

#include <memory>
#include <map>
#include <tuple>

#include "lua-wrapper/lua-wrapper.hpp"
#include "lua-wrapper/lua-objects.hpp"

namespace lo = lua::objects;

struct test_meta {

    std::string messge;

    test_meta( const test_meta & ) = default;
    test_meta& operator = ( const test_meta & ) = default;

    test_meta( std::string mess )
        :messge(mess)
    { }


    ~test_meta( )
    {
        std::cout << "dtor " << std::hex << this << std::endl;
    }

    static
    int lcall_test( lua_State *L )
    {
        //return lua::state::create_metatable_call<test_meta>( L );
        auto res0 = lua::state::create_metatable_object<test_meta>( L, "0" );
        auto res1 = lua::state::create_metatable_object<test_meta>( L, "1" );
        lo::table t;
        t.add("c", lo::new_integer(1000));
        t.add("a", res0);
        t.add("b", res1);
        t.push(L);
        return 1;
    }

    static
    int lcall_my_message( lua_State *L )
    {
        lua::state ls(L);
        auto m = ls.check_metatable<test_meta>( 1 );
        ls.push( m->messge );
        return 1;
    }

    static
    int lcall_table( lua_State *L )
    {
        auto call = &lua::state::create_metatable_object<test_meta, std::string>;
        lua::objects::table res;
        res.add( "one", call( L, "one" ) );
        res.add( "two", call( L, "two" ) );
        res.push(L);
        return 1;
    }

    static
    int lcall_one( lua_State *L )
    {
//        return lua::state::create_metatable_call<test_meta>( L, "!!!!!!!!!!!" );
        auto call = &lua::state::create_metatable_object<test_meta, std::string>;
        call(L, "WWWWWW")->push( L );
        return 1;
    }

    static
    int lcall_message( lua_State *L )
    {
        lua::state ls(L);
        auto mt = ls.test_metatable<test_meta>( L, 1 );
        if( mt ) {
            std::ostringstream oss;
            oss << "Hello from " << std::hex << mt;
            ls.push( oss.str( ) );
            return 1;
        }
        return 0;
    }


    static
    const char *name( )
    {
        return "test_meta";
    }

    static
    const struct luaL_Reg *table(  )
    {
        static const struct luaL_Reg lib[ ] =   {
            { "__tostring", &lcall_message      },
            { "call",       &lcall_test         },
            { "get",        &lcall_one          },
            { "mess",       &lcall_my_message   },
            { "callt",      &lcall_table        },
            { nullptr,      nullptr             },
        };
        return lib;
    }
};


template <typename T>
lo::base_sptr create( lua_State *L )
{
    lua::state ls(L);
    lo::base_sptr res(new lo::metatable<T>( "Hello!" ) );
    return res;
}

int main( int argc, const char **argv )
{ try {

    const char * path = argc > 1 ? argv[1] : "test.lua";

    lua::state ls;

    ls.openlibs( );

    ls.register_metatable<test_meta>( );
    ls.register_call( "get_test",  &test_meta::lcall_test );

    ls.check_call_error(ls.load_file( path ));

    auto b = create<test_meta>( ls.get_state( ) );
    ls.exec_function( "callback", *b );

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


//static
//bool contains_ext_links( enviroment::sptr e ) //{
//    std::map<enviroment *, std::size_t> tmp;
//    for( auto &c: e->children( ) ) {
//        auto cl = c;
//        if( cl.get( ) ) {
//            tmp[cl.get( )] =
//                    static_cast<std::size_t>(cl.use_count( )) - 1;
//            if( contains_ext_links( cl )) {
////                        cl->introspect( );
//                return true;
//            }
//        }
//    }

////            for( auto &t: tmp ) {
////                //std::cout << t.first << " " << t.second << "\n";
////            }

//    for( auto &d: e->data( ) ) {
//        auto ev = object_env( d.second );
//        if( ev ) {
//            auto tf = tmp.find( ev.get( ) );
//            if( tf != tmp.end( ) ) {
//                //std::cout << "has " << tf->first << " " << tf->second << "\n";
//                tf->second -= (ev.use_count( ) - 1);
//                if(tf->second == 0) {
//                    tmp.erase( tf );
//                }
//            }
//        }
//    }

////            std::cout << "resrt: \n";
////            for( auto &t: tmp ) {
////                std::cout << t.first << " " << t.second << "\n";
////            }

//    return !tmp.empty( );
//}

//static
//void clean_children( enviroment::sptr e )
//{
//    return ;
//    for( auto &c: e->children( ) ) {
//        auto cl = c;
//        if( cl && !contains_ext_links( cl )) {
//            //std::cout << "!Has external: \n";
//            cl->GC( );
//        }
//    }

//}
