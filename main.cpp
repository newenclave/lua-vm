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

const char * lua_type_to_string( int t )
{
    switch( t ) {
    case LUA_TNIL:
        return "nil";
    case LUA_TBOOLEAN:
        return "boolean";
    case LUA_TLIGHTUSERDATA:
        return "lightuserdata";
    case LUA_TNUMBER:
        return "number";
    case LUA_TSTRING:
        return "string";
    case LUA_TTABLE:
        return "table";
    case LUA_TFUNCTION:
        return "function";
    case LUA_TUSERDATA:
        return "userdata";
    case LUA_TTHREAD:
        return "thread";
    }
    return "none";
}

template <int LuaTypeID>
struct lua_type_id {

    enum { type_index = LuaTypeID };
    static bool check( lua_State *L, int idx )
    {
        int r = lua_type( L, idx );
        return r == type_index;
    }
};

template <typename T>
struct lua_type_id_integral: public lua_type_id<LUA_TNUMBER> {
    static T get( lua_State *L, int idx )
    {
        return static_cast<T>(lua_tointeger( L, idx ));
    }
};

template <typename T>
struct lua_type_id_numeric: public lua_type_id<LUA_TNUMBER> {
    static T get( lua_State *L, int idx )
    {
        return static_cast<T>(lua_tonumber( L, idx ));
    }
};

template <typename T>
struct lua_type_id_pointer: public lua_type_id<LUA_TLIGHTUSERDATA> {
    static T get( lua_State *L, int idx )
    {
        return static_cast<T>(lua_topointer( L, idx ));
    }
};

template <typename T>
struct lua_type_id_string {

    enum { type_index = LUA_TSTRING };
    static bool check( lua_State * /*L*/, int /*idx*/ )
    {
        return true;
    }

    static T get( lua_State *L, int idx )
    {
        const char *t = lua_tostring( L, idx );
        return T( t ? t : "<nil>" );
    }
};

template <typename CT>
struct lua_type_trait {
    enum { type_index = LUA_TNONE };
};

template <>
struct lua_type_trait<int> : public
       lua_type_id_integral<int> { };

template <>
struct lua_type_trait<unsigned> : public
       lua_type_id_integral<unsigned> { };

template <>
struct lua_type_trait<long> : public
       lua_type_id_integral<long> { };

template <>
struct lua_type_trait<unsigned long> : public
       lua_type_id_integral<unsigned long> { };

template <>
struct lua_type_trait<short> : public
       lua_type_id_integral<short> { };

template <>
struct lua_type_trait<unsigned short> : public
       lua_type_id_integral<unsigned short> { };

template <>
struct lua_type_trait<char> : public
       lua_type_id_integral<char> { };

template <>
struct lua_type_trait<unsigned char> : public
       lua_type_id_integral<unsigned char> { };

template <>
struct lua_type_trait<std::string> : public
       lua_type_id_string<std::string> { };

template <>
struct lua_type_trait<const char *> : public
       lua_type_id_string<const char *> { };

template <>
struct lua_type_trait<float> : public
       lua_type_id_numeric<float> { };

template <>
struct lua_type_trait<double> : public
       lua_type_id_numeric<double> { };

template <>
struct lua_type_trait<long double> : public
       lua_type_id_numeric<long double> { };

template <>
struct lua_type_trait<const void *> : public
       lua_type_id_pointer<const void *> { };

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

    void pop( )
    {
        lua_pop( vm_, 1 );
    }

    void pop( int n )
    {
        lua_pop( vm_, n );
    }

    std::string pop_error( )
    {
        std::string res( lua_tostring(vm_, -1) );
        pop( );
        return res;
    }

    void register_call( const char *name, lua_CFunction fn )
    {
        lua_register( vm_, name, fn );
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

    void push( bool value )
    {
        lua_pushboolean( vm_, value ? 1 : 0 );
    }

    void push( const char* value )
    {
        lua_pushstring( vm_, value );
    }

    void push( const std::string& value )
    {
        lua_pushstring( vm_, value.c_str( ) );
    }

    void push( lua_CFunction value )
    {
        lua_pushcfunction( vm_, value );
    }

    template<typename T>
    void push( T * value )
    {
        lua_pushlightuserdata( vm_, reinterpret_cast<void *>( value ) );
    }

    int get_type( int id = -1 )
    {
        return lua_type( vm_, id );
    }

    int get_top( )
    {
        return lua_gettop( vm_ );
    }

    void clean() {
        pop( get_top( ) );
    }

    template<typename T>
    T get( int id = -1 )
    {
        if( !lua_type_trait<T>::check( vm_, id ) ) {
            throw std::runtime_error( std::string("bad type '")
                    + lua_type_to_string( lua_type_trait<T>::type_index )
                    + std::string("'. lua type is '")
                    + lua_type_to_string( get_type( id ) )
                    + std::string("'") );
        }
        return lua_type_trait<T>::get( vm_, id );
    }
};

void print_table( lua_State *L, int idx )
{
//    lua_pushvalue( L, idx ); // stack: map

//    lua_pushnil( L );          // stack: map nil

    std::string key_ = lua_tostring( L, idx - 3 );
    std::string val_ = lua_tostring( L, idx - 2 );

    std::cout << key_ << " = " << val_ << "\n";

//    while( lua_next(L, idx - 1) )   // stack: map key value
//    {
//        std::string key_ = lua_tostring( L, idx - 1 );
//        std::string val_ = lua_tostring( L, idx  );


//        idx += 1;
//        //lua_pop(L, 1); // stack: map key
//    }
    // stack: map
//    lua_pop(L, 1); // stack:
}

static int l_print( lua_State *L )
{
    int n = lua_gettop( L );

    lua_vm tvm( L );

    if( n < 0 ) {
        n = ( (n * -1) + 1 );
    }

    std::cout << n << "\n";

    for ( int b = 1; b <= n; ++b ) {
        if( tvm.get_type( b ) == LUA_TTABLE ) {
            std::cout << "Table found: \n";
            print_table( L, b );
        } else {
            std::cout << tvm.get<const char *>( b );
        }
    }

    lua_pop( L, n );

    return 0;
}

int main( ) try
{
    lua_vm v;
    v.register_call( "print", l_print );
//    lua_pushcfunction(v.state( ), l_sin, 1);
//    lua_setglobal(v.state( ), "mysin");

    v.check_call_error(luaL_loadfile(v.state( ), "test.lua"));
    v.check_call_error(lua_pcall(v.state( ), 0, LUA_MULTRET, 0));

    return 0;
} catch( const std::exception &ex ) {
    std::cerr << "Error: " << ex.what( ) << "\n";
    return 1;
}

