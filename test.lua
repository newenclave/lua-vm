
--print( mysin( 90 ), '\n' )

--print( mysin( 'sdfsdf' ), '\n')

function test( a )
    print(a, 'test!\n')
end

t = {
     name =  "$$$$$$$$"
    ,value = "1"
    ,forward = {
         zero = 0
        ,one  = 1
        ,two  = 2
     }
    ,backward = {
          'zero'
         ,'one'
         ,'two'
         ,'three'
         ,'four'
         ,'five'
    }
    ,tg = globt
}

function create_table( id )

    local class = {
         call = print
        ,id   = id
    }

    class.get_id = function( c )
        return c.id
    end
    class.__gc = function( c )
        print(c.id, ' deleted')
    end
    return class
end

a = create_table( 1 )
b = create_table( 2 )

function test( )
    local c = create_table( 4 )
end

--a:call( t )

m = { ['t.tt'] = { i={ } } }

print( math )
print( string )
print( gtest )


