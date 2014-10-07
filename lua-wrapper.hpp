#ifndef LUA_WRAPPER_HPP
#define LUA_WRAPPER_HPP

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#ifdef LUA_WRAPPER_TOP_NAMESPACE

namespace LUA_WRAPPER_TOP_NAMESPACE {

#endif

namespace lua {

    class state {
        lua_State *vm_;
        bool       own_;

    public:

        enum state_owning {
             NOT_OWN_STATE = 0
            ,OWN_STATE     = 1
        };

        state( lua_State *vm, state_owning os = NOT_OWN_STATE )
            :vm_(vm)
            ,own_(os == OWN_STATE)
        {

        }

        state( )
            :vm_(lua_open( ))
            ,own_(true)
        { }

        ~state( )
        {
            if( own_ && vm_ ) {
                lua_close( vm_ );
            }
        }

        lua_State *get_state( )
        {
            return vm_;
        }

        const lua_State *get_state( ) const
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
            lua_pushlstring( vm_, value.c_str( ), value.size( ) );
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

        template <typename T>
        void set_in_global_table( const char *table_name,
                                  const char *key, T value )
        {
            push( table_name );
            push( table_name );

            lua_gettable( vm_, LUA_GLOBALSINDEX );

            // ==> table name | nil or table
            if (!lua_istable(vm_, -1))
            {
                if (lua_isnoneornil(vm_, -1)) {
                    pop( 1 );
                    lua_newtable( vm_ );
                }
                else {
                    lua_pop(vm_, 2);
                    throw std::logic_error( "Not table" );
                }
            }

            push( key );               // ==> table name | table | item
            push( value );             // ==> table name | table | item | value
            lua_settable( vm_, -3 );   // ==> table name | table
            lua_settable( vm_, LUA_GLOBALSINDEX );   // ==>
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

}

#ifdef LUA_WRAPPER_TOP_NAMESPACE

} // LUA_WRAPPER_TOP_NAMESPACE

#endif


#endif // LUAWRAPPER_HPP
