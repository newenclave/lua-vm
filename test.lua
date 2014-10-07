
--print( mysin( 90 ), '\n' )

--print( mysin( 'sdfsdf' ), '\n')

function test( a )
    print(a, 'test!\n')
end

local t = {
     name =  "test"
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
}

print( 123423423 * 234234234234234, 2, "123\n", t, "\n===============\n" )
