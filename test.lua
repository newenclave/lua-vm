
--print( mysin( 90 ), '\n' )

--print( mysin( 'sdfsdf' ), '\n')

function test( a )
    print(a, 'test!\n')
end

local t = {
     name =  "$$$$$$$$"
    ,value = "1"
    ,forward = {
         zero = 0
        ,one  = 1
        ,two  = 2
     }
    ,backward = {
          [0] = 'zero'
         ,[1] = 'one'
         ,[2] = 'two'
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


function main( argv )
    print( argv )
end

