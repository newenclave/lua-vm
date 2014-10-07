#include <iostream>
#include <string>
#include <deque>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <stdlib.h>

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

struct lua_object {

    enum {
         TYPE_NONE          = LUA_TNONE
        ,TYPE_NIL           = LUA_TNIL
        ,TYPE_BOOL          = LUA_TBOOLEAN
        ,TYPE_LUSERDATA     = LUA_TLIGHTUSERDATA
        ,TYPE_NUMBER        = LUA_TNUMBER
        ,TYPE_STRING        = LUA_TSTRING
        ,TYPE_TABLE         = LUA_TTABLE
        ,TYPE_FUNCTION      = LUA_TFUNCTION
        ,TYPE_USERDATA      = LUA_TUSERDATA
        ,TYPE_THREAD        = LUA_TTHREAD
        ,TYPE_LOCAL_INDEX   = 1000
        ,TYPE_PAIR          = TYPE_LOCAL_INDEX + 1
    };

    virtual ~lua_object( ) { }

    virtual int type_id( ) const
    {
        return TYPE_NONE;
    }

    virtual bool is_container( ) const
    {
        return false;
    }

    virtual lua_object * clone( ) const
    {
        return new lua_object;
    }

    virtual size_t count( ) const
    {
        return 0;
    }

    const lua_object * at( size_t /*index*/ ) const
    {
        throw std::out_of_range( "bad index" );
    }

    virtual std::string str( ) const
    {
        return std::string( );
    }

    virtual lua_Number num( ) const
    {
        return 0;
    }

};

class lua_object_keeper {

    lua_object *obj_;

public:

    lua_object_keeper (lua_object_keeper &other)
        :obj_(other.get( ) ? other->clone( ) : NULL)
    { }

    lua_object_keeper &operator = (lua_object_keeper &other)
    {
        reset( other.get( ) ? other->clone( ) : NULL );
    }

    explicit lua_object_keeper( lua_object *obj )
        :obj_(obj)
    { }

    lua_object_keeper( )
        :obj_(NULL)
    { }

    ~lua_object_keeper( )
    {
        if( obj_ ) delete obj_;
    }

    void reset( lua_object *new_obj = NULL )
    {
        if( obj_ ) delete obj_;
        obj_ = new_obj;
    }

    lua_object * get( )
    {
        return obj_;
    }

    const lua_object * get( ) const
    {
        return obj_;
    }

    lua_object * operator -> ( )
    {
        return get( );
    }

    const lua_object * operator -> ( ) const
    {
        return get( );
    }
};

class lua_bool: public lua_object {

    bool value_;

public:

    lua_bool( bool val )
        :value_(val)
    { }

    virtual int type_id( ) const
    {
        return lua_object::TYPE_BOOL;
    }

    virtual lua_object *clone( ) const
    {
        return new lua_bool( value_ );
    }

    std::string str( ) const
    {
        static const char * vals[2] = { "0", "1" };
        return vals[value_ ? 1 : 0];
    }

    lua_Number num( ) const
    {
        return static_cast<lua_Number>(value_ ? 1 : 0);
    }

};

class lua_nil: public lua_object {

public:
    virtual int type_id( ) const
    {
        return lua_object::TYPE_NIL;
    }

    virtual lua_object *clone( ) const
    {
        return new lua_nil;
    }

    std::string str( ) const
    {
        return "nil";
    }
};

class lua_light_userdata: public lua_object {

    void *ptr_;

public:

    lua_light_userdata( void *ptr )
        :ptr_(ptr)
    { }

    virtual int type_id( ) const
    {
        return lua_object::TYPE_LUSERDATA;
    }

    virtual lua_object *clone( ) const
    {
        return new lua_light_userdata( ptr_ );
    }

    std::string str( ) const
    {
        std::ostringstream oss;
        oss << "0x" << std::hex << ptr_;
        return oss.str( );
    }
};

class lua_number: public lua_object {

    lua_Number num_;

public:

    lua_number( lua_Number num )
        :num_(num)
    { }

    virtual int type_id( ) const
    {
        return lua_object::TYPE_NUMBER;
    }

    virtual lua_object *clone( ) const
    {
        return new lua_number( num_ );
    }

    std::string str( ) const
    {
        std::ostringstream oss;
        oss << num_;
        return oss.str( );
    }

    lua_Number num( ) const
    {
        return num_;
    }

};

class lua_string: public lua_object {

    std::string cont_;

public:

    lua_string( std::string cont )
        :cont_(cont)
    { }

    virtual int type_id( ) const
    {
        return lua_object::TYPE_STRING;
    }

    virtual lua_object *clone( ) const
    {
        return new lua_string( cont_ );
    }

    virtual size_t count( ) const
    {
        return cont_.size( );
    }

    std::string str( ) const
    {
        return cont_;
    }

    lua_Number num( ) const
    {
        return atof(cont_.c_str( ));
    }

};

class lua_object_pair: public lua_object {

    std::pair<lua_object_keeper, lua_object_keeper> pair_;

public:

    virtual int type_id( ) const
    {
        return lua_object::TYPE_PAIR;
    }

    lua_object_pair( const lua_object *f, const lua_object *s )
    {
        pair_.first.reset( f->clone( ) );
        pair_.second.reset( s->clone( ) );
    }

    lua_object_pair( const lua_object_pair &other )
    {
        pair_.first.reset( other.pair_.first->clone( ) );
        pair_.second.reset( other.pair_.second->clone( ) );
    }

    ~lua_object_pair( )
    { }

    virtual bool is_container( ) const
    {
        return true;
    }

    virtual lua_object *clone( ) const
    {
        return new lua_object_pair( *this );
    }

    const lua_object * at( size_t index ) const
    {
        if( index < 2 ) {
            return index ? pair_.second.get( ) : pair_.first.get( );
        }
        throw std::out_of_range( "bad index" );
    }

    std::string str( ) const
    {
        std::ostringstream oss;
        oss << pair_.first->str( )
            << "="
            << pair_.second->str( );

        return oss.str( );
    }
};

class lua_table: public lua_object {

    std::vector<lua_object_pair> list_;

public:

    lua_table( const lua_table &o )
        :list_(o.list_)
    { }

    lua_table( )
    { }

    int type_id( ) const
    {
        return lua_object::TYPE_TABLE;
    }

    bool is_container( ) const
    {
        return true;
    }

    const lua_object * at( size_t index ) const
    {
        return &list_.at(index);
    }

    void push( lua_object_pair *val )
    {
        list_.push_back( *val );
    }

    virtual lua_object *clone( ) const
    {
        return new lua_table( *this );
    }

    std::string str( size_t shift, const lua_object_pair &pair ) const
    {
        std::string spaces( shift << 2, ' ' );
        std::ostringstream oss;
        oss << spaces << pair.str( );
        return oss.str( );
    }

    std::string str( ) const
    {
        std::ostringstream oss;

        typedef std::vector<lua_object_pair>::const_iterator citer;

        oss << "{ ";
        bool fst = true;
        for( citer b(list_.begin( )), e(list_.end( )); b!=e; ++b  ) {
            std::string res(str( 0, *b ));
            if( !fst ) {
                oss << ", ";
            } else {
                fst = false;
            }
            oss << res;
        }
        oss << " }";

        return oss.str( );
    }
};

lua_object * create( lua_State *L, int idx );

lua_object * create_table( lua_State *L, int index )
{
    lua_pushvalue(L, index);
    lua_pushnil(L);

    lua_table *new_table = new lua_table;
    while (lua_next(L, -2)) {

        lua_pushvalue(L, -2);
        lua_object_pair *new_pair
                = new lua_object_pair( create( L, -1 ), create( L, -2 ) );
        new_table->push( new_pair );
        lua_pop(L, 2);
    }

    lua_pop(L, 1);

    return new_table;
}

lua_object * create( lua_State *L, int idx )
{
    int t = lua_type( L, idx );
    switch( t ) {
    case LUA_TBOOLEAN:
        return new lua_bool( lua_toboolean( L, idx ) );
    case LUA_TLIGHTUSERDATA:
        return new lua_light_userdata( lua_touserdata( L, idx ) );
    case LUA_TNUMBER:
        return new lua_number( lua_tonumber( L, idx ) );
    case LUA_TSTRING:
        return new lua_string( lua_tostring( L, idx ) );
    case LUA_TTABLE:
        return create_table( L, idx );
//    case LUA_TFUNCTION:
//        return "function";
//    case LUA_TUSERDATA:
//        return "userdata";
//    case LUA_TTHREAD:
//        return "thread";
    }
    return new lua_nil;
}

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

static int l_print( lua_State *L )
{
    int n = lua_gettop( L );

    lua_vm tvm( L );

    if( n < 0 ) {
        n = ( (n * -1) + 1 );
    }

    std::cout << n << "\n";

    for ( int b = 1; b <= n; ++b ) {
        std::cout << create( L, b )->str( );
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

