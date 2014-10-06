#include <iostream>
#include <string>
#include <deque>
#include <stdexcept>

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}


struct lua_object {

    virtual ~lua_object( ) { }

    virtual int type_id( ) const
    {
        return LUA_TNONE;
    }

    virtual bool is_container( ) const
    {
        return false;
    }


};

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

//    template<typename T>
//    T get( int id = -1 )
//    {
//        if( !lua_type_trait<T>::check( vm_, id ) ) {
//            throw std::runtime_error( std::string("bad type '")
//                    + lua_type_to_string( lua_type_trait<T>::type_index )
//                    + std::string("'. lua type is '")
//                    + lua_type_to_string( get_type( id ) )
//                    + std::string("'") );
//        }
//        return lua_type_trait<T>::get( vm_, id );
//    }
};


int main( ) try
{
    lua_vm v;

    return 0;
} catch( const std::exception &ex ) {
    std::cerr << "Error: " << ex.what( ) << "\n";
    return 1;
}
