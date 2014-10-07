#include <iostream>
#include <string>
#include <deque>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <stdlib.h>

#include <memory>

#include "lua-wrapper.hpp"

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
struct lua_type_id_integer: public lua_type_id<LUA_TNUMBER> {
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
struct lua_type_trait<signed int> : public
       lua_type_id_integer<signed int> { };

template <>
struct lua_type_trait<unsigned int> : public
       lua_type_id_integer<unsigned int> { };

template <>
struct lua_type_trait<signed long> : public
       lua_type_id_integer<signed long> { };

template <>
struct lua_type_trait<unsigned long> : public
       lua_type_id_integer<unsigned long> { };

template <>
struct lua_type_trait<signed short> : public
       lua_type_id_integer<signed short> { };

template <>
struct lua_type_trait<unsigned short> : public
       lua_type_id_integer<unsigned short> { };

template <>
struct lua_type_trait<signed char> : public
       lua_type_id_integer<signed char> { };

template <>
struct lua_type_trait<unsigned char> : public
       lua_type_id_integer<unsigned char> { };

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
        ,TYPE_INTEGER       = TYPE_LOCAL_INDEX + 2
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

    virtual void push( lua_State *L ) const
    {
        throw std::runtime_error( "push for 'none' is not available" );
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

typedef std::shared_ptr<lua_object> lua_object_sptr;
typedef std::unique_ptr<lua_object> lua_object_uptr;

class lua_object_bool: public lua_object {

    bool value_;

public:

    lua_object_bool( bool val )
        :value_(val)
    { }

    virtual int type_id( ) const
    {
        return lua_object::TYPE_BOOL;
    }

    virtual lua_object *clone( ) const
    {
        return new lua_object_bool( value_ );
    }

    void push( lua_State *L ) const
    {
        lua_pushboolean( L, value_ ? 1 : 0 );
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

class lua_object_nil: public lua_object {

public:
    virtual int type_id( ) const
    {
        return lua_object::TYPE_NIL;
    }

    virtual lua_object *clone( ) const
    {
        return new lua_object_nil;
    }

    void push( lua_State *L ) const
    {
        lua_pushnil( L );
    }

    std::string str( ) const
    {
        return "nil";
    }
};

class lua_object_light_userdata: public lua_object {

    void *ptr_;

public:

    lua_object_light_userdata( void *ptr )
        :ptr_(ptr)
    { }

    virtual int type_id( ) const
    {
        return lua_object::TYPE_LUSERDATA;
    }

    virtual lua_object *clone( ) const
    {
        return new lua_object_light_userdata( ptr_ );
    }

    void push( lua_State *L ) const
    {
        lua_pushlightuserdata( L, ptr_ );
    }

    std::string str( ) const
    {
        std::ostringstream oss;
        oss << std::hex << ptr_;
        return oss.str( );
    }
};

class lua_object_number: public lua_object {

    lua_Number num_;

public:

    explicit lua_object_number( lua_Number num )
        :num_(num)
    { }

    virtual int type_id( ) const
    {
        return lua_object::TYPE_NUMBER;
    }

    virtual lua_object *clone( ) const
    {
        return new lua_object_number( num_ );
    }

    void push( lua_State *L ) const
    {
        lua_pushnumber( L, num_ );
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

class lua_object_integer: public lua_object {

    lua_Integer num_;

public:

    explicit lua_object_integer( lua_Integer num )
        :num_(num)
    { }

    virtual int type_id( ) const
    {
        return lua_object::TYPE_INTEGER;
    }

    virtual lua_object *clone( ) const
    {
        return new lua_object_integer( num_ );
    }

    void push( lua_State *L ) const
    {
        lua_pushinteger( L, num_ );
    }

    std::string str( ) const
    {
        std::ostringstream oss;
        oss << num_;
        return oss.str( );
    }

    lua_Number num( ) const
    {
        return static_cast<lua_Number>( num_ );
    }

};

class lua_object_string: public lua_object {

    std::string cont_;

public:

    lua_object_string( const std::string &cont )
        :cont_(cont)
    { }

    lua_object_string( const char * cont )
        :cont_(cont)
    { }

    lua_object_string( const char * cont, size_t len )
        :cont_(cont, len)
    { }

    virtual int type_id( ) const
    {
        return lua_object::TYPE_STRING;
    }

    virtual lua_object *clone( ) const
    {
        return new lua_object_string( cont_ );
    }

    void push( lua_State *L ) const
    {
        lua_pushstring( L, cont_.c_str( ) );
    }

    virtual size_t count( ) const
    {
        return cont_.size( );
    }

    std::string str( ) const
    {
        return std::string( "'" ).append( cont_ ).append( "'" );
    }

    lua_Number num( ) const
    {
        return atof(cont_.c_str( ));
    }

};

class lua_object_pair: public lua_object {

    std::pair<lua_object_sptr, lua_object_sptr> pair_;

public:

    virtual int type_id( ) const
    {
        return lua_object::TYPE_PAIR;
    }

    lua_object_pair( const lua_object_sptr f, const lua_object_sptr s )
        :pair_(std::make_pair(f, s))
    { }

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

    void push( lua_State *L ) const
    {
        pair_.first->push( L );
        pair_.second->push( L );
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

typedef std::shared_ptr<lua_object_pair> lua_object_pair_sptr;

class lua_object_table: public lua_object {


    typedef std::vector<lua_object_pair_sptr> pair_vector;
    pair_vector list_;

public:

    lua_object_table( const lua_object_table &o )
        :list_(o.list_)
    { }

    lua_object_table( )
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
        return list_.at(index).get( );
    }

    void push_back( lua_object_pair_sptr &val )
    {
        list_.push_back( val );
    }

    virtual lua_object *clone( ) const
    {
        return new lua_object_table( *this );
    }

    void push( lua_State *L ) const
    {
        typedef pair_vector::const_iterator citr;
        lua_newtable( L );
        for( citr b(list_.begin( )), e(list_.end( )); b!=e; ++b ) {
            (*b)->push( L );
            lua_settable(L, -3);
        }
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

        typedef pair_vector::const_iterator citer;

        oss << "{ ";
        bool fst = true;
        for( citer b(list_.begin( )), e(list_.end( )); b!=e; ++b  ) {
            std::string res(str( 0, *b->get( ) ));
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

class lua_object_function: public lua_object {

    lua_CFunction func_;

public:

    lua_object_function( lua_CFunction func )
        :func_(func)
    { }

    virtual int type_id( ) const
    {
        return lua_object::TYPE_FUNCTION;
    }

    virtual lua_object *clone( ) const
    {
        return new lua_object_function( func_ );
    }

    void push( lua_State *L ) const
    {
        lua_pushcfunction( L, func_ );
    }

    std::string str( ) const
    {
        std::ostringstream oss;
        oss << "call@" << func_;
        return oss.str( );
    }
};

lua_object_sptr g_table = NULL;

lua_object_sptr create( lua_State *L, int idx );

lua_object_sptr create_table( lua_State *L, int index )
{
    lua_pushvalue(L, index);
    lua_pushnil(L);

    std::shared_ptr<lua_object_table> new_table( new lua_object_table );
    while (lua_next(L, -2)) {

        lua_pushvalue(L, -2);
        std::shared_ptr<lua_object_pair> new_pair
                ( new lua_object_pair( create( L, -1 ), create( L, -2 ) ) );
        new_table->push_back( new_pair );
        lua_pop(L, 2);
    }

    lua_pop(L, 1);

    return new_table;
}

lua_object_sptr create_string( lua_State *L, int idx )
{
    size_t length = 0;
    const char *ptr = lua_tolstring( L, idx, &length );
    return lua_object_sptr( new lua_object_string( ptr, length ) );
}

lua_object_sptr create( lua_State *L, int idx )
{
    int t = lua_type( L, idx );
    switch( t ) {
    case LUA_TBOOLEAN:
        return lua_object_sptr(
                    new lua_object_bool( lua_toboolean( L, idx ) ));
    case LUA_TLIGHTUSERDATA:
        return lua_object_sptr(
                    new lua_object_light_userdata( lua_touserdata( L, idx ) ));
    case LUA_TNUMBER:
        return lua_object_sptr(
                    new lua_object_number( lua_tonumber( L, idx ) ));
    case LUA_TSTRING:
        return create_string( L, idx );
    case LUA_TFUNCTION:
        return lua_object_sptr(
                    new lua_object_function( lua_tocfunction( L, idx ) ));
    case LUA_TTABLE:
        return g_table = create_table( L, idx );
//    case LUA_TUSERDATA:
//        return "userdata";
//    case LUA_TTHREAD:
//        return "thread";
    }
    return lua_object_sptr(new lua_object_nil);
}

static int l_print( lua_State *L )
{
    int n = lua_gettop( L );

    lua::state tvm( L );

    if( n < 0 ) {
        n = ( (n * -1) + 1 );
    }

    for ( int b = 1; b <= n; ++b ) {
        std::cout << create( tvm.get_state( ), b )->str( );
    }

    std::cout << "\n";
    tvm.pop( n );

    return 0;
}

int main( ) try
{
    lua::state v;
    v.register_call( "print", l_print );

//    lua_pushcfunction(v.state( ), l_sin, 1);
//    lua_setglobal(v.state( ), "mysin");

    v.set_in_global_table( "globt", "counter", &v );

    v.check_call_error(luaL_loadfile(v.get_state( ), "test.lua"));
    v.check_call_error(lua_pcall(v.get_state( ), 0, LUA_MULTRET, 0));

    return 0;
} catch( const std::exception &ex ) {
    std::cerr << "Error: " << ex.what( ) << "\n";
    return 1;
}

