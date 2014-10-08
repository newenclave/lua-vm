#ifndef LUA_WRAPPER_HPP
#define LUA_WRAPPER_HPP

#include <stdexcept>

extern "C" {
#include "lualib.h"
#include "lauxlib.h"
}

#include "lua-type-wrapper.hpp"
#include "lua-objects.hpp"

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
            const char *str = lua_tostring(vm_, -1);
            std::string res( str ? str : "Unknown error" );
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

        void push( const char* value, size_t len )
        {
            lua_pushlstring( vm_, value, len );
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

        template<typename T>
        void push( T value )
        {
            lua_pushnumber( vm_, static_cast<T>( value ) );
        }

        int get_type( int id = -1 )
        {
            return lua_type( vm_, id );
        }

        int get_type( const char* name, int id = LUA_GLOBALSINDEX ) const
        {
            lua_pushstring( vm_, name );
            lua_rawget( vm_, id );
            int type = lua_type(vm_, -1);
            lua_pop(vm_, 1);
            return type;
        }

        bool exist( const char* name ) const
        {
            return get_type( name ) != LUA_TNIL;
        }

        int get_top( )
        {
            return lua_gettop( vm_ );
        }

        template<typename T>
        T get( int id = -1 )
        {
            typedef types::id_traits<T> traits;
            if( !traits::check( vm_, id ) ) {
                throw std::runtime_error( std::string("bad type '")
                        + lua_type_to_string( traits::type_index )
                        + std::string("'. lua type is '")
                        + types::id_to_string( get_type( id ) )
                        + std::string("'") );
            }
            return traits::get( vm_, id );
        }

        template <typename T>
        void set_in_global( const char *table_name,
                            const char *key, T value )
        {
            push( table_name );
            push( table_name );

            lua_gettable( vm_, LUA_GLOBALSINDEX );

            // ==> table name | nil or table
            if ( !lua_istable( vm_, -1 ) ) {
                if ( lua_isnoneornil( vm_, -1 ) ) {
                    pop( 1 );
                    lua_newtable( vm_ );
                } else {
                    lua_pop(vm_, 2);
                    throw std::logic_error( "Not a table" );
                }
            }

            push( key );             // ==> table name | table | item
            push( value );           // ==> table name | table | item | value
            lua_settable( vm_, -3 ); // ==> table name | table
            lua_settable( vm_, LUA_GLOBALSINDEX ); // ==>
        }

        void set_object_in_global( const char *table_name,
                                   const char *key, const objects::base &bo )
        {
            push( table_name );
            push( table_name );

            lua_gettable( vm_, LUA_GLOBALSINDEX );

            // ==> table name | nil or table
            if ( !lua_istable( vm_, -1 ) ) {
                if ( lua_isnoneornil( vm_, -1 ) ) {
                    pop( 1 );
                    lua_newtable( vm_ );
                } else {
                    lua_pop(vm_, 2);
                    throw std::logic_error( "Not a table" );
                }
            }

            push( key );
            bo.push( vm_ );
            lua_settable( vm_, -3 ); // ==> table name | table
            lua_settable( vm_, LUA_GLOBALSINDEX ); // ==>
        }

        template<typename T>
        T get_from_global( const char* table_name,
                           const char* key )
        {
            push( table_name );

            lua_gettable( vm_, LUA_GLOBALSINDEX ); // ==> nil or table

            if ( !lua_istable( vm_, -1 ) ) {
                lua_pop( vm_, 1 );
                throw std::logic_error( "Not a table" );
            }

            push( key );               // ==> table | key
            lua_gettable( vm_, -2 );   // ==> table | value

            T result;
            try {
                result = get<T>( );
            } catch( ... ) {
                lua_pop( vm_, 2 );
                throw;
            }

            lua_pop( vm_, 2 );
            return result;
        }

        int exec_function( const char* func )
        {
            push( func );
            lua_gettable(vm_, LUA_GLOBALSINDEX);
            int rc = lua_pcall( vm_, 0, LUA_MULTRET, 0 );
            pop( 1 );
            return rc;
        }

        int exec_function( const char* func, const objects::base &bo )
        {
            push( func );
            lua_gettable(vm_, LUA_GLOBALSINDEX);
            bo.push( vm_ );
            int rc = lua_pcall( vm_, 1, LUA_MULTRET, 0 );

            return rc;
        }

        int load_file( const char *path )
        {
            int res = luaL_loadfile( vm_, path );
            if( 0 == res ) {
                res = lua_pcall( vm_, 0, LUA_MULTRET, 0);
            }
            return res;
        }

    };
}

#ifdef LUA_WRAPPER_TOP_NAMESPACE

} // LUA_WRAPPER_TOP_NAMESPACE

#endif


#endif // LUAWRAPPER_HPP
